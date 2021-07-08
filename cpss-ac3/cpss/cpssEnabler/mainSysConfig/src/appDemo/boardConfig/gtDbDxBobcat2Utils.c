/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoTmConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2GenUtils.h>

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
/*Phy*/
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
#include <cpss/dxCh/dxChxGen/phy/prvCpssDxChSmiUnitDrv.h>
#include <cpss/dxCh/dxChxGen/phy/private/prvCpssDxChPhySmi.h>
/* port, Pizza Arbiter and slices management for specific units */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortInterlaken.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <gtOs/gtOsTimer.h>
#include <cpss/generic/private/utils/prvCpssTimeRtUtils.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/common/smi/cpssGenSmi.h>
#ifdef ASIC_SIMULATION
    #include <asicSimulation/SCIB/scib.h>
#endif



#define PRV_CPSS_DXCH_BC2_UNUSED_PARAM_MAC(x) x = x

#define BAD_VALUE (GT_U32)(~0)

GT_STATUS prvBobcat2PortMapShadowPrint
(
    IN  GT_U8   devNum,
    IN  GT_U32  firstPhysicalPortNumber,
    IN  GT_U32  portMapShadowArraySize
)
{
    GT_U32 i; /* loop iterator */

    if((firstPhysicalPortNumber > 255) || ((firstPhysicalPortNumber + portMapShadowArraySize) > 255))
    {
        return GT_BAD_PARAM;
    }

    for(i = firstPhysicalPortNumber; i <= (firstPhysicalPortNumber + portMapShadowArraySize); i++)
    {
        cpssOsPrintf("PhysPort: %d   Valid = %d\r\n", i, PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[i].valid);
        cpssOsPrintf("MAC:%d  TXQ:%d  TX_DMA:%d  RX_DMA:%d\r\n", PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[i].portMap.macNum,
                                                         PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[i].portMap.txqNum,
                                                         PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[i].portMap.txDmaNum,
                                                         PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[i].portMap.rxDmaNum);
        cpssOsPrintf("portGroupId:%d  MAP_TYPE:%d\r\n\r\n", PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[i].portMap.portGroup,
                                                         PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[i].portMap.mappingType);
    }


    return GT_OK;
}

/*-------------------------------------------------------------------------------
**     Pizza Arbiter State Print Function :
**         prints configuration and state of all slice for each unit
**         <Rx/Tx DMA DMA_CTU  BM TxQ
**-------------------------------------------------------------------------------*/


#ifdef WIN32
    #define cpssOsPrintf osPrintf
#endif


static void gtBobcat2PortPizzaArbiterIfDumpSliceStatePrint(GT_U32 configuredSlices, GT_U32 sliceId, GT_BOOL isEnable, GT_U32 occupiedBy)
{
    if (GT_FALSE  != isEnable )
    {
        cpssOsPrintf(" %2d",occupiedBy);
    }
    else
    {
        if (sliceId < configuredSlices)
        {
            cpssOsPrintf(" %2s",".");
        }
        else
        {
            cpssOsPrintf(" %2s","x");
        }
    }
}

static void gtBobcat2PortPizzaArbiterIfDumpUnitStatePrint
(
    CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC * pUnitPtr,
    char * msg
)
{
    GT_U32      sliceId;
    GT_U32      slicesNumberPerGop = 20;
    GT_U32      nonEmptySlices;

    /* count number of non-X assigned slices*/
    nonEmptySlices = 0;
    for (sliceId = 0 ; sliceId < pUnitPtr->totalSlicesOnUnit ; sliceId++)
    {
        if (pUnitPtr->slice_enable[sliceId] != GT_FALSE)
        {
            nonEmptySlices ++;
        }
    }

    cpssOsPrintf("\n    %s",msg);
    cpssOsPrintf("\n       Total/Configured Slices/of them assigned  = %d/%d/%d" , pUnitPtr->totalSlicesOnUnit,
                                                                        pUnitPtr->totalConfiguredSlices,
                                                                        nonEmptySlices);
    cpssOsPrintf("\n       Work Conserving Bit = %d",pUnitPtr->workConservingBit);
    if (pUnitPtr->totalSlicesOnUnit > 0)
    {
        cpssOsPrintf("\n       Slice to port map   : slice :");
        for (sliceId = 0 ; sliceId < slicesNumberPerGop ; sliceId++)
        {
            cpssOsPrintf(" %2d",sliceId);
        }
        cpssOsPrintf("\n                                   :");
        for (sliceId = 0 ; sliceId < slicesNumberPerGop ; sliceId++)
        {
            cpssOsPrintf("---");
        }


        for (sliceId = 0 ; sliceId < pUnitPtr->totalSlicesOnUnit ; sliceId++)
        {
            if (0 == sliceId % slicesNumberPerGop)
            {
                cpssOsPrintf("\n       Slice %4d : %4d   : port  :",sliceId, sliceId + slicesNumberPerGop -1 );
            }
            gtBobcat2PortPizzaArbiterIfDumpSliceStatePrint(pUnitPtr->totalConfiguredSlices,sliceId,
                                                                      pUnitPtr->slice_enable[sliceId],
                                                                      pUnitPtr->slice_occupied_by[sliceId]);
        }
    }
}



typedef struct PA_UNIT_2_STR_STCT
{
    CPSS_DXCH_PA_UNIT_ENT  paUnit;
    GT_CHAR               *str;
}PA_UNIT_2_STR_STC;

static PA_UNIT_2_STR_STC pa_unit2str_List[] =
{
     { CPSS_DXCH_PA_UNIT_RXDMA_0_E,            "PA UNIT RXDMA_0"       }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_1_E,            "PA UNIT RXDMA_1"       }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_2_E,            "PA UNIT RXDMA_2"       }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_3_E,            "PA UNIT RXDMA_3"       }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_4_E,            "PA UNIT RXDMA_4"       }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_5_E,            "PA UNIT RXDMA_5"       }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_0_E,              "PA UNIT TXQ_0"         }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_1_E,              "PA UNIT TXQ_1"         }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_2_E,              "PA UNIT TXQ_2"         }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_3_E,              "PA UNIT TXQ_3"         }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_4_E,              "PA UNIT TXQ_4"         }
    ,{ CPSS_DXCH_PA_UNIT_TXQ_5_E,              "PA UNIT TXQ_5"         }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_0_E,            "PA UNIT TXDMA_0"       }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_1_E,            "PA UNIT TXDMA_1"       }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_2_E,            "PA UNIT TXDMA_2"       }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_3_E,            "PA UNIT TXDMA_3"       }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_4_E,            "PA UNIT TXDMA_4"       }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_5_E,            "PA UNIT TXDMA_5"       }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_0_E,          "PA UNIT TX_FIFO_0"     }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_1_E,          "PA UNIT TX_FIFO_1"     }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_2_E,          "PA UNIT TX_FIFO_2"     }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_3_E,          "PA UNIT TX_FIFO_3"     }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_4_E,          "PA UNIT TX_FIFO_4"     }
    ,{ CPSS_DXCH_PA_UNIT_TX_FIFO_5_E,          "PA UNIT TX_FIFO_5"     }
    ,{ CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_0_E,      "PA UNIT ETH_TX_FIFO_0" }
    ,{ CPSS_DXCH_PA_UNIT_ETH_TX_FIFO_1_E,      "PA UNIT ETH_TX_FIFO_1" }
    ,{ CPSS_DXCH_PA_UNIT_ILKN_TX_FIFO_E,       "PA UNIT ILKN_TX_FIFO"  }
    ,{ CPSS_DXCH_PA_UNIT_RXDMA_GLUE_E,         "PA UNIT RXDMA_GLUE"    }
    ,{ CPSS_DXCH_PA_UNIT_TXDMA_GLUE_E,         "PA UNIT TXDMA_GLUE"    }
    ,{ CPSS_DXCH_PA_UNIT_MPPM_E,               "PA UNIT MPPM"          }
    ,{ CPSS_DXCH_PA_UNIT_UNDEFINED_E,          (GT_CHAR *)NULL         }
};


static GT_CHAR * paUnit22StrFind
(
    CPSS_DXCH_PA_UNIT_ENT  paUnit
)
{
    GT_U32 i;
    static GT_CHAR badUnitNameMsg[30];
    for (i = 0 ; pa_unit2str_List[i].paUnit != CPSS_DXCH_PA_UNIT_UNDEFINED_E; i++)
    {
        if (pa_unit2str_List[i].paUnit  == paUnit)
        {
            return pa_unit2str_List[i].str;
        }
    }
    osSprintf(badUnitNameMsg,"Unit %d : Unknown Name",paUnit);
    return &badUnitNameMsg[0];
}



GT_STATUS appDemoRegAddrGet
(
    IN GT_U8     devNum,
    IN GT_U32    offsInStr,
    OUT GT_U32  *regAddrPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;
    GT_U8                                *regsAddrVer1_U8;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_LION2_E | CPSS_XCAT2_E);

    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    regsAddrVer1 = &PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1;
    regsAddrVer1_U8 = (GT_U8 *)regsAddrVer1;
    *regAddrPtr = (GT_U32)*((GT_U32*)&regsAddrVer1_U8[offsInStr]);
    return GT_OK;
}

#if 0
GT_CHAR *  appDemoPizzaArbiterFldPrint(PRV_CPSS_DXCH_PA_REG_FIELDC_STC *ptr)
{
    static GT_CHAR str[20];
    cpssOsSprintf(str,"%2d-%2d",ptr->offset, ptr->offset+ptr->len-1);
    return &str[0];
}
#endif

GT_CHAR *  appDemoPizzaArbiterFldPrint
(
    GT_U32 offset,
    GT_U32 len
)
{
    static GT_CHAR str[20];
    cpssOsSprintf(&str[0],"%2d-%2d,%2d",offset, offset+ len-1,len);
    return &str[0];
}



GT_STATUS appDemoPizzaArbiterUnitsDefPrint
(
    GT_U8 dev
)
{
    CPSS_DXCH_PA_UNIT_ENT i;
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC * unitDefPtr;
    GT_CHAR *unitNamePtr;
    GT_U32 ctrlRegAddr;
    GT_U32 mapRegAddr;
    PRV_CPSS_DXCH_PP_CONFIG_STC *devPtr;

    cpssOsPrintf("\n+----------------+---------------------------------------------+---------------------------------------------------------+");
    cpssOsPrintf("\n|                |                Control                      |           Map                                           |");
    cpssOsPrintf("\n|                +------------+--------------------------------+--------------------+--------------+---------------------+");
    cpssOsPrintf("\n|   Unit         |            |          Fields                |        Register    | Flds Statist |  Single  Fld        |");
    cpssOsPrintf("\n|                | Register   +----------+----------+----------+------------+-------+------+-------+----------+----------+");
    cpssOsPrintf("\n|                | adress     |  Slice   |  Map     |    WC    | Adress     | Total |  per | Total |  Slice   |  Slice   |");
    cpssOsPrintf("\n|                |            | To-Run   | Load-En  |    mode  |            |   N   |  reg |       |   Map    |   En     |");
    cpssOsPrintf("\n+----------------+------------+----------+----------+----------+------------+-------+------+-------+----------+----------+");


    devPtr = PRV_CPSS_DXCH_PP_MAC(dev);
    for (i = (CPSS_DXCH_PA_UNIT_ENT)0 ; i < CPSS_DXCH_PA_UNIT_MAX_E; i++)
    {
        unitDefPtr = (PRV_CPSS_DXCH_BC2_PA_UNIT_STC*)&(devPtr->paData.paUnitDrvList[i]);
        if (unitDefPtr->unit != CPSS_DXCH_PA_UNIT_UNDEFINED_E)
        {
            unitNamePtr = paUnit22StrFind(unitDefPtr->unit);
            appDemoRegAddrGet(dev,unitDefPtr->ctrlReg.baseAddressPtrOffs,&ctrlRegAddr);
            appDemoRegAddrGet(dev,unitDefPtr->mapReg.baseAddressPtrOffs,&mapRegAddr);
            cpssOsPrintf("\n| %-14s |",&unitNamePtr[7]);
            cpssOsPrintf(" 0x%08x |",ctrlRegAddr);
            cpssOsPrintf(" %s |",appDemoPizzaArbiterFldPrint(unitDefPtr->ctrlReg.fldSliceNum2run.offset,      unitDefPtr->ctrlReg.fldSliceNum2run.len));
            cpssOsPrintf(" %s |",appDemoPizzaArbiterFldPrint(unitDefPtr->ctrlReg.fldMapLoadEn.offset,         unitDefPtr->ctrlReg.fldMapLoadEn.len));
            cpssOsPrintf(" %s |",appDemoPizzaArbiterFldPrint(unitDefPtr->ctrlReg.fldWorkConservModeEn.offset, unitDefPtr->ctrlReg.fldWorkConservModeEn.len));

            cpssOsPrintf(" 0x%08x |",mapRegAddr);
            cpssOsPrintf(" %5d |",unitDefPtr->mapReg.totalReg);
            cpssOsPrintf(" %4d |",unitDefPtr->mapReg.numberFldsPerReg);
            cpssOsPrintf(" %5d |",unitDefPtr->mapReg.totalFlds);
            cpssOsPrintf(" %s |",appDemoPizzaArbiterFldPrint(unitDefPtr->mapReg.fldSlice2PortMap.offset, unitDefPtr->mapReg.fldSlice2PortMap.len));
            cpssOsPrintf(" %s |",appDemoPizzaArbiterFldPrint(unitDefPtr->mapReg.fldSliceEn.offset,       unitDefPtr->mapReg.fldSliceEn.len));

        }
    }
    cpssOsPrintf("\n+----------------+------------+----------+----------+----------+------------+-------+------+-------+----------+----------+");
    return GT_OK;
}


/**
* @internal gtBobcat2PortPizzaArbiterIfStateDump function
* @endinternal
*
* @brief   Dump Bobcat2 Pizza Arbiter Registers
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - number of port group
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtBobcat2PortPizzaArbiterIfStateDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId
)
{
    GT_STATUS   rc;         /* return code */
    static CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC  pizzaDeviceState;
    GT_U32 unitIdx;
    GT_CHAR * unitNamePtr;
    CPSS_DXCH_PA_UNIT_ENT unitType;


    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);

    rc = cpssDxChPortPizzaArbiterDevStateGet(devNum,portGroupId, /*OUT*/&pizzaDeviceState);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\nPizza Arbiter State device = %2d port group = %2d", devNum,portGroupId);
    for (unitIdx = 0 ; pizzaDeviceState.devState.bobK.unitList[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = pizzaDeviceState.devState.bobK.unitList[unitIdx];
        unitNamePtr = paUnit22StrFind(unitType);
        gtBobcat2PortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.bobK.unitState[unitIdx], unitNamePtr);
    }
    cpssOsPrintf("\n");

    /*
    cpssOsPrintf("\nPizza Arbiter State device = %2d port group = %2d", devNum,portGroupId);
    gtBobcat2PortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.bc2.rxDMA,     "RxDMA  Pizza Arbiter:");
    gtBobcat2PortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.bc2.txDMA,     "TxDMA  Pizza Arbiter:");
    gtBobcat2PortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.bc2.TxQ,       "TxQ    Pizza Arbiter:");
    gtBobcat2PortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.bc2.txFIFO,    "TxFIFO Pizza Arbiter:");
    gtBobcat2PortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.bc2.ethFxFIFO, "ethTxFIFO Pizza Arbiter:");
    gtBobcat2PortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.bc2.ilknTxFIFO,"IlknTxFIFO Pizza Arbiter");
    cpssOsPrintf("\n");
    */
    return GT_OK;
}

/**
* @internal gtBobcat2PortPizzaArbiterIfUnitStateDump function
* @endinternal
*
* @brief   Dump Bobcat2 Pizza Arbiter Specific Unit Registers
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupId              - number of port group
* @param[in] unit                     -  (RxDMA/TxDMA/TxQ/Tx-FIFO/Eth-Tx-FIFO/Ilkn-Tx-FIFO)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtBobcat2PortPizzaArbiterIfUnitStateDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  CPSS_DXCH_PA_UNIT_ENT unit
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      unitIdx;
    GT_BOOL     unitFound;
    static CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC  pizzaDeviceState;
    GT_CHAR *   unitNamePtr;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);

    rc = cpssDxChPortPizzaArbiterDevStateGet(devNum,portGroupId, /*OUT*/&pizzaDeviceState);
    if (rc != GT_OK)
    {
        return rc;
    }

    unitFound = GT_FALSE;
    cpssOsPrintf("\nPizza Arbiter State device = %2d port group = %2d", devNum,portGroupId);
    for (unitIdx = 0 ; pizzaDeviceState.devState.bobK.unitList[unitIdx] != CPSS_DXCH_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        if (pizzaDeviceState.devState.bobK.unitList[unitIdx] == unit)
        {
            unitFound = GT_TRUE;
            break;
        }
    }
    if (unitFound == GT_TRUE)
    {
        unitNamePtr = paUnit22StrFind(unit);
        gtBobcat2PortPizzaArbiterIfDumpUnitStatePrint(&pizzaDeviceState.devState.bobK.unitState[unitIdx], unitNamePtr);
    }
    else
    {
        cpssOsPrintf("\nunit %d : Undefined Unit",unit);
        return GT_BAD_PARAM;
    }
    cpssOsPrintf("\n");
    return GT_OK;
}

GT_STATUS gtBobcat2findLocalPhysicalPortByTxQ
(
    IN  GT_U8 devNum,
    IN  GT_U32 txqNum,
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
)
{
    GT_STATUS rc;
    GT_U32 maxPortNum;
    GT_PHYSICAL_PORT_NUM portIdx;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);

    if (portNumPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    rc = appDemoDxPossiblePhysPortNumGet(devNum,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    *portNumPtr = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;
    for (portIdx = 0 ; portIdx < maxPortNum; portIdx++)
    {
        static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum,portIdx,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->valid == GT_TRUE)
        {
            if (portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                if (portMapPtr->portMap.txqNum == txqNum)
                {
                    *portNumPtr = portIdx;
                    return GT_OK;
                }
            }
        }
    }
    return GT_OK;
}



/**
* @internal gtBobcat2TxQHighSpeedPortPrint function
* @endinternal
*
* @brief   Dump Bobcat2 TxQ High speed port Dump
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] dev                      - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtBobcat2TxQHighSpeedPortPrint
(
    IN  GT_U8  dev
)
{
    typedef struct HighSpeedPortList_STC
    {
        GT_U32 size;
        GT_U32 txQPortNumLlist[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS];
        GT_U32 highSpeedPortIdxLlist[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS];
    }HighSpeedPortList_STC;

    GT_STATUS finalRc;
    GT_STATUS rc;
    GT_U32 i;
    HighSpeedPortList_STC getList;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_U32               maxPortNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC  portMapShadow;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_BOOL              tmExists;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(dev, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);


    /*------------------------------------------------------------------------------------------------*
     * now check whether TM is enabled, i.e. there exists at least one ETHERNET port with enabled TM  *
     *------------------------------------------------------------------------------------------------*/
    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    tmExists = GT_FALSE;
    for (portNum = 0; portNum < maxPortNum; portNum++)
    {
        portMapShadowPtr = &portMapShadow;
        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapShadowPtr->valid == GT_TRUE )
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
            {
                if (portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE)
                {
                    tmExists = GT_TRUE;
                    break;
                }
            }
        }
    }

    rc = cpssDxChPortDynamicPATxQHighSpeedPortGet(dev,/*OUT*/&getList.size,&getList.txQPortNumLlist[0],&getList.highSpeedPortIdxLlist[0]);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("\n+---------+----------+---------------------+");
    cpssOsPrintf("\n| portNum | TxQ Port | High Speed Port Idx |");
    cpssOsPrintf("\n+---------+----------+---------------------+");
    finalRc = GT_OK;
    if (getList.size > 0)
    {
        for (i = 0 ; i < getList.size; i++)
        {
            if (tmExists == GT_FALSE || (tmExists == GT_TRUE && getList.txQPortNumLlist[i] != PRV_CPSS_DXCH_PORT_NUM_TM_TxQ_CNS))
            {
                rc = gtBobcat2findLocalPhysicalPortByTxQ(dev,getList.txQPortNumLlist[i],&portNum);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (portNum != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
                {
                    cpssOsPrintf("\n|   %3d   |   %3d    |      %3d            |",portNum,getList.txQPortNumLlist[i],getList.highSpeedPortIdxLlist[i]);
                }
                else
                {
                    cpssOsPrintf("\n|   ---   |   %3d    |      %3d            | *** something wrong",getList.txQPortNumLlist[i],getList.highSpeedPortIdxLlist[i]);
                    finalRc = GT_FAIL;
                }
            }
            else /* TM TxQ port */
            {
                cpssOsPrintf("\n|   TM    |   %3d    |      %3d            |",getList.txQPortNumLlist[i],getList.highSpeedPortIdxLlist[i]);
            }
        }
        cpssOsPrintf("\n+---------+----------+---------------------+");
    }
    else
    {
        cpssOsPrintf("\n  No high speed ports");
    }

    cpssOsPrintf("\n");
    return finalRc;
}

GT_STATUS gtBobcat2TxQHighSpeed_DQ_LL_Dump
(
    IN  GT_U8  dev
)
{
    GT_STATUS rc;
    GT_U32 dqList[8];
    GT_U32 llList[8];
    GT_U32 i;

    rc = cpssDxChPortDynamicPATxQHighSpeedPortDumpGet(dev,&dqList[0], &llList[0]);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("\nIndex :");
    for (i = 0 ; i < 8; i++)
    {
        cpssOsPrintf("%5d",i);
    }
    cpssOsPrintf("\nLL    :");
    for (i = 0 ; i < 8; i++)
    {
        cpssOsPrintf("%5d",llList[i]);
    }
    cpssOsPrintf("\nDQ    :");
    for (i = 0 ; i < 8; i++)
    {
        cpssOsPrintf("%5d",dqList[i]);
    }
    cpssOsPrintf("\n\n");
    return GT_OK;
}

/**
* @internal gtBobcat2PortMappingDump function
* @endinternal
*
* @brief   Dump Bobcat2 Port Mapping (valid ports only)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* @param[in] dev                      - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS gtBobcat2PortMappingDump
(
    IN  GT_U8  dev
)
{
    GT_STATUS   rc;         /* return code */
    GT_PHYSICAL_PORT_NUM portIdx;

    cpssOsPrintf("\n+------+-----------------+------------------------------+");
    cpssOsPrintf("\n| Port |   mapping Type  | rxdma txdma mac txq ilkn  tm |");
    cpssOsPrintf("\n+------+-----------------+------------------------------+");

    for (portIdx = 0 ; portIdx < 256; portIdx++)
    {
        static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;

        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portIdx,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->valid == GT_TRUE)
        {
            cpssOsPrintf("\n| %4d |",portIdx);
            cpssOsPrintf(" %-15s | %5d %5d %3d %3d %4d %3d |"
                                                ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                ,portMapPtr->portMap.rxDmaNum
                                                ,portMapPtr->portMap.txDmaNum
                                                ,portMapPtr->portMap.macNum
                                                ,portMapPtr->portMap.txqNum
                                                ,portMapPtr->portMap.ilknChannel
                                                ,portMapPtr->portMap.tmPortIdx);
        }
    }
    cpssOsPrintf("\n+------+-----------------+------------------------------+");
    cpssOsPrintf("\n");
    return GT_OK;
}

/*--------------------------------------------------------------------------------------*
 *  Port SMI maping                                                                     *
 *--------------------------------------------------------------------------------------*/
GT_STATUS gtBobcat2SMIMappingDump
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroupId
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      smiInstance;
    GT_U32      smiLocalPort;
    static      PRV_CPSS_DXCH_SMI_STATE_STC state;

    rc = prvCpssDxChSMIStateGet(devNum,portGroupId,/*OUT*/&state);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n+-----+--------------------+");
    cpssOsPrintf("\n| SMI |  Autopolling Number|");
    cpssOsPrintf("\n+-----+--------------------+");
    for (smiInstance = CPSS_PHY_SMI_INTERFACE_0_E; smiInstance < CPSS_PHY_SMI_INTERFACE_MAX_E; smiInstance++)
    {
        cpssOsPrintf("\n| %3d | %4d",smiInstance,state.autoPollNumOfPortsArr[smiInstance]);
    }
    cpssOsPrintf("\n+------+-------+-------+-------+-------+");
    cpssOsPrintf("\n| Port | SMI-0 | SMI-1 | SMI-2 | SMI-3 |");
    cpssOsPrintf("\n+------+-------+-------+-------+-------+");
    for (smiLocalPort = 0; smiLocalPort < NUMBER_OF_PORTS_PER_SMI_UNIT_CNS; smiLocalPort++)
    {
        cpssOsPrintf("\n| %4d |",smiLocalPort);
        for (smiInstance = CPSS_PHY_SMI_INTERFACE_0_E; smiInstance < CPSS_PHY_SMI_INTERFACE_MAX_E; smiInstance++)
        {
            cpssOsPrintf("  0x%02X |",state.phyAddrRegArr[smiInstance][smiLocalPort]);
        }
    }
    cpssOsPrintf("\n+------+-------+-------+-------+-------+");
    cpssOsPrintf("\n");
    return GT_OK;
}



GT_STATUS gtBobcat2SMIPortMappingList
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroupId
)
{
    GT_STATUS   rc;         /* return code */
    CPSS_PHY_SMI_INTERFACE_ENT              smiInstance;
    GT_U32                                  smiLocalPort;
    GT_U32                                  autoPollingPlace;
    GT_PHYSICAL_PORT_NUM                    portNum;
    GT_U32                                  maxPortNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;


    rc = appDemoDxPossiblePhysPortNumGet(devNum,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = gtBobcat2SMIMappingDump(devNum,portGroupId);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n+---------------------------------------------------------------+");
    cpssOsPrintf("\n|  Auto Polling : Port->SMI->Phy                                |");
    cpssOsPrintf("\n+------+-----------------+-----+---------+---------------+------+");
    cpssOsPrintf("\n| Port |   mapping Type  | mac |   OOB   |   SMI         | phy  |");
    cpssOsPrintf("\n|      |                 |     | polling | IF port place | addr |");
    cpssOsPrintf("\n+------+-----------------+-----+---------+---------------+------+");


    for (portNum = 0 ; portNum < maxPortNum; portNum++)
    {
        GT_U8   phyAddFromHw;
        GT_U8   phyAddFromDb;
        CPSS_PHY_SMI_INTERFACE_ENT smiIf;
        GT_BOOL  inBandAutoNegEnable;

        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum,portNum,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->valid == GT_FALSE)
        {
            continue;
        }

        if (portMapPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            rc = cpssDxChPhyPortAddrGet(devNum,portNum,&phyAddFromHw, &phyAddFromDb);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (phyAddFromHw != 0xFF)
            {
                rc = cpssDxChPhyPortSmiInterfaceGet(devNum, portNum, /*OUT*/&smiIf);
                if(rc != GT_OK)
                {
                    return rc;
                }

                rc = cpssDxChPortInbandAutoNegEnableGet(devNum,portNum,&inBandAutoNegEnable);
                if(rc != GT_OK)
                {
                    return rc;
                }

                rc = prvCpssDxChPortSMIAutoPollingMappingGet(devNum,portNum,/*OUT*/&smiInstance,&smiLocalPort,&autoPollingPlace);
                if (rc != GT_OK)
                {
                    return rc;
                }
                /* smiPortNumMap[smiInstance][smiLocalPort] = portNum;  */
                cpssOsPrintf("\n| %4d |",portNum);
                cpssOsPrintf(" %-15s | %3d |",CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                             ,portMapPtr->portMap.macNum);

                if (inBandAutoNegEnable == GT_FALSE) /* OOB Autopolling is enabled */
                {
                    cpssOsPrintf("    +    |");
                    cpssOsPrintf(" %2d  %3d   %3d |",smiIf,smiLocalPort,autoPollingPlace);
                }
                else
                {
                    cpssOsPrintf("    -    |");
                    cpssOsPrintf(" %2d  %3s   %3s |",smiIf,"--","--");
                }
                cpssOsPrintf(" 0x%02X |",phyAddFromHw);
            }
        }
    }
    cpssOsPrintf("\n+------+-----------------+-----+---------+---------------+------+");
    cpssOsPrintf("\n");
    return GT_OK;
}



/*--------------------------------------------------------------------------------------*
 *  Port Resource Print                                                                 *
 *--------------------------------------------------------------------------------------*/
typedef struct
{
    CPSS_DXCH_BC2_PORT_RESOURCES_FLD_ENT resEnm;
    GT_U32                               fldOffs;
}PRV_CPSS_DXCH_BC2_RESEM_OFFS_STC;

static PRV_CPSS_DXCH_BC2_RESEM_OFFS_STC  prv_bc2res2OffsInitList[] =
{
     { BC2_PORT_FLD_RXDMA_IfWidth,                             offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , rxdmaScdmaIncomingBusWidth)              }
    ,{ BC2_PORT_FLD_TXDMA_SCDMA_TxQDescriptorCredit,           offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , txdmaTxQCreditValue)                     }
    ,{ BC2_PORT_FLD_TXDMA_SCDMA_burstAlmostFullThreshold,      offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , txdmaBurstAlmostFullThreshold)           }
    ,{ BC2_PORT_FLD_TXDMA_SCDMA_burstFullThreshold,            offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , txdmaBBurstFullThreshold)                }
    ,{ BC2_PORT_FLD_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold,   offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , txdmaTxfifoHeaderCounterThresholdScdma)  }
    ,{ BC2_PORT_FLD_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold,  offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , txdmaTxfifoPayloadCounterThresholdScdma) }
    ,{ BC2_PORT_FLD_TXFIFO_IfWidth,                            offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , txfifoScdmaOutgoingBusWidth)             }
    ,{ BC2_PORT_FLD_TXFIFO_shifterThreshold,                   offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , txfifoScdmaShiftersThreshold)            }
    ,{ BC2_PORT_FLD_TXFIFO_payloadThreshold,                   offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , txfifoScdmaPayloadThreshold)             }
    ,{ BC2_PORT_FLD_Eth_TXFIFO_IfWidth,                        offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , ethTxfifoOutgoingBusWidth)               }
    ,{ BC2_PORT_FLD_Eth_TXFIFO_shifterThreshold,               offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , ethTxFifoShifterThreshold)               }
    ,{ BC2_PORT_FLD_Eth_TXFIFO_payloadThreshold,               offsetof(PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC , ethTxfifoScdmaPayloadThreshold)          }
    ,{ BC2_PORT_FLD_INVALID_E,                                 (GT_U32) (~0)                                                                           }
};

static GT_U32 * prv_bc2res2OffsArr[BC2_PORT_FLD_MAX];

GT_STATUS appDemoBobcat2PortListResourcesPrint
(
    IN GT_U8 dev
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 portIdx;
    GT_PHYSICAL_PORT_NUM portNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;

    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    PRV_CPSS_DXCH_BC2_PORT_RESOURCE_STC     resource;
    CPSS_DXCH_BCAT2_PORT_RESOURCE_LIST_STC  resList;
    GT_U32 maxPortNum;


    for (i = 0 ; i < sizeof(prv_bc2res2OffsArr)/sizeof(prv_bc2res2OffsArr[0]); i++)
    {
        prv_bc2res2OffsArr[i] = (GT_U32*)NULL;
    }
    for (i = 0 ; prv_bc2res2OffsInitList[i].resEnm != BC2_PORT_FLD_INVALID_E; i++)
    {
        prv_bc2res2OffsArr[prv_bc2res2OffsInitList[i].resEnm] = (GT_U32*)((GT_U8*)&resource + prv_bc2res2OffsInitList[i].fldOffs);
    }

    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    cpssOsPrintf("\n+------------------------------------------------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port resources                                                                                               |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+--------------------+-----+-----------------------------+-----------------+-----------------+");
    cpssOsPrintf("\n|    |      |          |       |              |                    |RXDMA|      TXDMA SCDMA            |     TX-FIFO     |   Eth-TX-FIFO   |");
    cpssOsPrintf("\n|    |      |          |       |              |                    |-----|-----+-----+-----+-----------|-----+-----+-----|-----+-----+-----|");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | rxdma txq txdma tm |  IF | TxQ |Burst|Burst| TX-FIFO   | Out |Shift| Pay | Out |Shift| Pay |");
    cpssOsPrintf("\n|    |      |          |       |              |                    |Width|Descr| All | Full|-----+-----|Going|  er | Load|Going|  er | Load|");
    cpssOsPrintf("\n|    |      |          |       |              |                    |     |     | Most|Thrsh| Hdr | Pay | Bus |Thrsh|Thrsh| Bus |Thrsh|Thrsh|");
    cpssOsPrintf("\n|    |      |          |       |              |                    |     |     | Full|     |Thrsh|Load |Width|     |     |Width|     |     |");
    cpssOsPrintf("\n|    |      |          |       |              |                    |     |     |Thrsh|     |     |Thrsh|     |     |     |     |     |     |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+");

    portIdx = 0;

    for (portNum = 0 ; portNum < maxPortNum; portNum++)
    {
        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->valid == GT_FALSE)
        {
            continue;
        }
        if (portMapPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            rc = cpssDxChPortSpeedGet(dev,portNum,/*OUT*/&speed);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChPortInterfaceModeGet(dev,portNum,/*OUT*/&ifMode);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else /* CPU */
        {
            speed  = CPSS_PORT_SPEED_1000_E;
            ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        }

        if (speed != CPSS_PORT_SPEED_NA_E)
        {
            rc = prvCpssDxChBcat2PortResoursesStateGet(dev,portNum,/*OUT*/&resList);
            if (rc  != GT_OK)
            {
                return rc;
            }

            for (i = 0 ; i < sizeof(resList.arr)/sizeof(resList.arr[0]); i++)
            {
                if (resList.arr[i].fldId != BC2_PORT_FLD_INVALID_E)
                {
                    if (prv_bc2res2OffsArr[resList.arr[i].fldId] != NULL)
                    {
                        *prv_bc2res2OffsArr[resList.arr[i].fldId] = resList.arr[i].fldVal;
                    }
                }
            }

            cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                    ,portNum
                                                    ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                    ,CPSS_SPEED_2_STR(speed)
                                                    ,CPSS_IF_2_STR(ifMode));
            cpssOsPrintf(" %5d %3d %5d %2d |"  ,portMapPtr->portMap.rxDmaNum
                                               ,portMapPtr->portMap.txqNum
                                               ,portMapPtr->portMap.txDmaNum
                                               ,portMapPtr->portMap.tmPortIdx);

            cpssOsPrintf("%4s |",RXDMA_IfWidth_2_STR(resource.rxdmaScdmaIncomingBusWidth));
            cpssOsPrintf("%4d |",resource.txdmaTxQCreditValue);
            cpssOsPrintf("%4d |",resource.txdmaBurstAlmostFullThreshold);
            cpssOsPrintf("%4d |",resource.txdmaBBurstFullThreshold);
            cpssOsPrintf("%4d |",resource.txdmaTxfifoHeaderCounterThresholdScdma);
            cpssOsPrintf("%4d |",resource.txdmaTxfifoPayloadCounterThresholdScdma);
            cpssOsPrintf("%4s |",TX_FIFO_IfWidth_2_STR(resource.txfifoScdmaOutgoingBusWidth));
            cpssOsPrintf("%4d |",resource.txfifoScdmaShiftersThreshold);
            cpssOsPrintf("%4d |",resource.txfifoScdmaPayloadThreshold);
            if (portMapPtr->portMap.trafficManagerEn == GT_FALSE)
            {
                cpssOsPrintf("%4s |%4s |%4s |","--","--","--");
            }
            else
            {
                cpssOsPrintf("%4s |",TX_FIFO_IfWidth_2_STR(resource.ethTxfifoOutgoingBusWidth));
                cpssOsPrintf("%4d |",resource.ethTxFifoShifterThreshold);
                cpssOsPrintf("%4d |",resource.ethTxfifoScdmaPayloadThreshold);
            }
            portIdx++;
        }
    }
    cpssOsPrintf("\n+----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+");
    cpssOsPrintf("\n");
    return GT_OK;
}

GT_STATUS appDemoBobcat2PortListSerdesPolarityPrint
(
    IN GT_U8 dev
)
{
    GT_STATUS rc;
    GT_U32 portIdx;
    GT_PHYSICAL_PORT_NUM portNum;
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;

    CPSS_PORT_SPEED_ENT speed;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    GT_U32 maxPortNum;
    GT_U32 startSerdes, numOfSerdesLanes;

    rc = appDemoDxPossiblePhysPortNumGet(dev,/*OUT*/&maxPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    cpssOsPrintf("\n+---------------------------------------------------------------------------------------------------+");
    cpssOsPrintf("\n|                             Port serdes polarity                                                  |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-----+---------------+-------+-------+-------+-------+");
    cpssOsPrintf("\n| #  | Port | map type | Speed |    IF        | mac | start    Num  |Line 0 |Line 1 |Line 2 |Line 3 |");
    cpssOsPrintf("\n|    |      |          |       |              |     | serdes   Of   +-------+-------+-------+-------|");
    cpssOsPrintf("\n|    |      |          |       |              |     |         lines |  Inv  |  Inv  |  Inv  |  Inv  |");
    cpssOsPrintf("\n|    |      |          |       |              |     |               | RX TX | RX TX | RX TX | RX TX |");
    cpssOsPrintf("\n|    |      |          |       |              |     |               |       |       |       |       |");
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-----+---------------+-------+-------+-------+-------+");

    portIdx = 0;

    for (portNum = 0 ; portNum < maxPortNum; portNum++)
    {
        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (portMapPtr->valid == GT_FALSE)
        {
            continue;
        }
        if (portMapPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            continue;
        }
        rc = cpssDxChPortSpeedGet(dev,portNum,/*OUT*/&speed);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPortInterfaceModeGet(dev,portNum,/*OUT*/&ifMode);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (speed != CPSS_PORT_SPEED_NA_E)
        {
            GT_U32 lineIdx;
            GT_BOOL invertTx;
            GT_BOOL invertRx;
            GT_CHAR invertTxStr[5] = "??";
            GT_CHAR invertRxStr[5] = "??";

            rc = prvCpssDxChPortIfModeSerdesNumGet(dev, portNum, ifMode,
                                        &startSerdes, &numOfSerdesLanes);
            if (rc != GT_OK)
            {
                return rc;
            }

            cpssOsPrintf("\n| %2d | %4d | %-8s | %s | %s |",portIdx
                                                    ,portNum
                                                    ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                                    ,CPSS_SPEED_2_STR(speed)
                                                    ,CPSS_IF_2_STR(ifMode));
            cpssOsPrintf(" %3d |"  ,portMapPtr->portMap.macNum);
            cpssOsPrintf(" %6d %6d |",startSerdes,numOfSerdesLanes);
            for (lineIdx = 0; lineIdx < numOfSerdesLanes; lineIdx++)
            {
                invertTx = 100;
                invertRx = 100;
                rc = cpssDxChPortSerdesPolarityGet(dev,portNum,lineIdx,&invertTx,&invertRx);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (invertTx == GT_TRUE || invertTx == GT_FALSE)
                {
                    cpssOsSprintf(invertTxStr,"%2d",invertTx);
                    cpssOsSprintf(invertRxStr,"%2d",invertRx);
                }
                else
                {
                    cpssOsSprintf(invertTxStr,"%2s","??");
                    cpssOsSprintf(invertRxStr,"%2s","??");
                }
                cpssOsPrintf(" %2s %2s |",invertTxStr,invertRxStr);
            }
            for (lineIdx = numOfSerdesLanes; lineIdx < 4; lineIdx++)
            {
                cpssOsPrintf(" %2s %2s |","--","--");
            }

        }
    }
    cpssOsPrintf("\n+----+------+----------+-------+--------------+-----+---------------+-------+-------+-------+-------+");
    return GT_OK;
}



GT_STATUS appDemoBobcat2portsMac2PhysicalConvPrint(GT_U8 devNum)
{
    GT_U32 macNum;
    GT_U32 arrSize;
    GT_U32 line;
    GT_U32 itemsAtLine = 10;
    GT_PHYSICAL_PORT_NUM portNum;
    GT_PHYSICAL_PORT_NUM *mac2PhysPortArrPtr;

    arrSize = sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMac2PhyscalConvArr)/sizeof(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[0]);

    cpssOsPrintf("\n Mac --> Physical port conversion");
    cpssOsPrintf("\n %3s :","----");
    for (macNum = 0; macNum < itemsAtLine; macNum++)
    {
        cpssOsPrintf("%4s","----");
    }

    cpssOsPrintf("\n %3s :","mac");
    for (macNum = 0; macNum < itemsAtLine; macNum++)
    {
        cpssOsPrintf(" %3d",macNum);
    }

    cpssOsPrintf("\n %3s :","----");
    for (macNum = 0; macNum < itemsAtLine; macNum++)
    {
        cpssOsPrintf("%4s","----");
    }

    mac2PhysPortArrPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMac2PhyscalConvArr[0];
    line = 0;
    for (macNum = 0; macNum < arrSize; macNum++)
    {
        if (macNum %itemsAtLine == 0)
        {
            cpssOsPrintf("\n %3d :",line);
            line++;
        }
        portNum = mac2PhysPortArrPtr[macNum];
        cpssOsPrintf(" %3d",portNum);
    }
    cpssOsPrintf("\n\n");
    return GT_OK;
}

GT_STATUS appDemoPortsDisconnectConnectSimulation
(
    IN  GT_U8    devNum,
    IN  GT_U16   NumOfLoops,
    IN  GT_U16   timeout,
    IN  GT_U8    NumOfPorts,
    ...
)
{

    va_list  ap;                 /* arguments list pointer */
    GT_U32   i,j;
    GT_U32 portNum,LocalReturnCode,phyType;
    GT_U16 data;

    va_start(ap, NumOfPorts);

    data = 0;

    cpssOsPrintf("\nNumOfLoops= %d\n",NumOfLoops);
    cpssOsPrintf("\nNumOfPorts= %d\n",NumOfPorts);

    for(i = 1; i <= NumOfPorts;i++)
    {
        portNum = va_arg(ap, GT_U32);
        phyType = (portNum & 0xFF00)>> 8;
        portNum = portNum & 0xFF;

        cpssOsPrintf("\nphyType= %d\n",phyType);
        cpssOsPrintf("\nportNum= %d\n",portNum);

        if (portNum != CPSS_CPU_PORT_NUM_CNS)
        {
            if((portNum >= PRV_CPSS_PP_MAC(devNum)->numOfPorts) ||
                !PRV_CPSS_PHY_PORT_IS_EXIST_MAC(devNum,portNum))
            {
                va_end(ap);
                return GT_BAD_PARAM;

            }
        }

        for (j=1; j <=NumOfLoops; j++)
        {
            if (phyType== 0) /*No PHY*/
            {
                LocalReturnCode=cpssDxChPortSerdesTxEnableSet(devNum,portNum,0);
                if (LocalReturnCode!=GT_OK)
                {
                    cpssOsPrintf("\n Port %d Error Disable %d\n",portNum,LocalReturnCode);
                    va_end(ap);
                    return LocalReturnCode;
                }
            }
            else
            {
                LocalReturnCode=cpssDxChPhyPortSmiRegisterRead(devNum,portNum,16,&data); /*Read reg 16 - Copper Specific Control reg 1*/
                if (LocalReturnCode!=GT_OK)
                {
                    cpssOsPrintf("\n SmiRegisterRead on port %d Error %d\n",portNum,LocalReturnCode);
                    va_end(ap);
                    return LocalReturnCode;
                }

                data=data | 0x8 ; /*Set bit 3 Copper transmitter disable */
                LocalReturnCode=cpssDxChPhyPortSmiRegisterWrite(devNum,portNum,16,data);
                if (LocalReturnCode!=GT_OK)
                {
                    cpssOsPrintf("\n SmiRegisterWrite on port %d Error %d\n",portNum,LocalReturnCode);
                    va_end(ap);
                    return LocalReturnCode;
                }
            }


            if (timeout)
            {
                osTimerWkAfter(timeout);
            }

            if (phyType== 0) /*No PHY*/
            {
                LocalReturnCode=cpssDxChPortSerdesTxEnableSet(devNum,portNum,1);
                if (LocalReturnCode!=GT_OK)
                {
                    cpssOsPrintf("\n Port %d Error Enable %d\n",portNum,LocalReturnCode);
                    va_end(ap);
                    return LocalReturnCode;
                }
            }
            else
            {
                data=data & 0xFFFFFFF7 ; /*Clear bit 3 -Copper transmitter enable*/
                LocalReturnCode=cpssDxChPhyPortSmiRegisterWrite(devNum,portNum,16,data);
                if (LocalReturnCode!=GT_OK)
                {
                    cpssOsPrintf("\n SmiRegisterWrite on port %d Error %d\n",portNum,LocalReturnCode);
                    va_end(ap);
                    return LocalReturnCode;
                }
            }
            if (timeout)
            {
                osTimerWkAfter(timeout);
            }
        }
    }
    va_end(ap);
    return GT_OK;
}


/*******************************************************************************
* intMaskSet
*
* DESCRIPTION:
*       Mask/unmask specific interrupts on port that can trigger WA's restart
*       or other not needed for now actions.
*
* INPUTS:
*       devNum      - device number
*       portNum     - physical port number
*       ifMode      - current interface on port
*       operation   - mask/unmask
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       Affected interrupts: CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
*                           CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.
*
*******************************************************************************/
/* #ifndef ASIC_SIMULATION */
GT_STATUS intMaskSet
(
    IN  GT_U8                   dev,
    IN  GT_PHYSICAL_PORT_NUM    port,
    IN  CPSS_EVENT_MASK_SET_ENT operation
)
{
    GT_STATUS   rc;
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
    CPSS_PORT_SPEED_ENT            speed;

    rc = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
    if(rc != GT_OK)
    {
        cpssOsPrintSync("cpssDxChPortInterfaceModeGet(portNum=%d,ifMode=%d):rc=%d\n",
                        port, ifMode, rc);
        return rc;
    }
    rc = cpssDxChPortSpeedGet(dev, port, &speed);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(        (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode && CPSS_PORT_SPEED_5000_E != speed)
            || (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_CR_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_CR2_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_KR_S_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_CR_S_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode))
    {
        rc = cpssEventDeviceMaskWithEvExtDataSet(dev,
                                     CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                     port, operation);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssEventDeviceMaskSet(MMPCS_SIGNAL,port=%d,op=%d):%d",
                         port, operation, rc);
        }
    }
    return rc;

}
/* #endif */

extern GT_BOOL  changeToDownHappend[CPSS_MAX_PORTS_NUM_CNS];

extern GT_STATUS prvAppDemoPortsConfig
(
    IN  GT_U8                           devNumber,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp,
    IN  GT_U32                          squelch,
    CPSS_PORTS_BMP_STC                  initPortsBmp
);

/**
* @internal bc2AppDemoPortsConfig function
* @endinternal
*
* @brief   Init required ports of Bobcat2, Bobk and Aldrin to specified ifMode and speed
*
* @param[in] devNumber                - device number in appDemo
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] powerUp                  -   GT_TRUE - port power up
*                                      GT_FALSE - serdes power down
* @param[in] numOfPorts               - quantity of ports to configure
*                                      use 0 if all device ports wanted
*                                      ... - numbers of ports to configure
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Imlemented for Bobcat2 B0 only, since
*       function cpssDxChPortSerdesSignalDetectGet is not imlemented
*       for Bobcat2 A0
*
*/
GT_STATUS bc2AppDemoPortsConfig
(
    IN  GT_U8                           devNumber,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp,
    IN  GT_U32                          numOfPorts,
    ...
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      i;                  /* interator */
    GT_PHYSICAL_PORT_NUM portNum=0; /* port number */
    va_list     ap;                 /* arguments list pointer */

    CPSS_PORTS_BMP_STC initPortsBmp,/* bitmap of ports to init */
                        *initPortsBmpPtr;/* pointer to bitmap */

    initPortsBmpPtr = &initPortsBmp;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);
    va_start(ap, numOfPorts);
    for(i = 1; i <= numOfPorts; i++)
    {
        portNum = va_arg(ap, GT_U32);
        CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr,portNum);
    }
    va_end(ap);
    rc = prvAppDemoPortsConfig(devNumber, ifMode, speed, powerUp, 1000, initPortsBmp); /*squelch = 1000 is out of range 1 - 15 and it ignored.*/
    return rc;
}


/**
* @internal bc2AppDemoPortsInit function
* @endinternal
*
* @brief   Init required ports of Bobcat2 to specified ifMode and speed for init system
*
* @param[in] devNumber                - device number in appDemo
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] powerUp                  -   GT_TRUE - port power up
*                                      GT_FALSE - serdes power down
* @param[in] numOfPorts               - quantity of ports to configure
*                                      use 0 if all device ports wanted
*                                      ... - numbers of ports to configure
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Imlemented for Bobcat2 B0 only, since
*       function cpssDxChPortSerdesSignalDetectGet is not imlemented
*       for Bobcat2 A0
*
*/
GT_STATUS bc2AppDemoPortsInit
(
    IN  GT_U8                           devNumber,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp,
    IN  GT_U32                          numOfPorts,
    ...
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      i;                  /* interator */
    GT_PHYSICAL_PORT_NUM portNum;   /* port number */
    va_list     ap;                 /* arguments list pointer */
    CPSS_PORTS_BMP_STC initPortsBmp,/* bitmap of ports to init */
                        *initPortsBmpPtr;/* pointer to bitmap */
    CPSS_PORTS_BMP_STC sdTrainingBmp; /* bitmap of ports to run serdes
                                                    auto-tuning/taining on */
    GT_U32  serdesTraining; /* serdes training required or not */
    GT_U32  trxTraining;    /* what kind of auto-tuning run on port */
    GT_BOOL isLinkUp;       /* link status on port */
    GT_BOOL signalState;    /* is there signal on serdes */
    CPSS_PORT_INTERFACE_MODE_ENT    currentIfMode;  /* interface mode configured
                                                                already on port */
    CPSS_PORT_SPEED_ENT             currentSpeed;   /* speed configured
                                                                already on port */
    GT_U8                           dev;   /* drvice number in cpss */

    dev = appDemoPpConfigList[devNumber].devNum;

    /* Not implemented for Bobcat2 A0 */
    if(PRV_CPSS_PP_MAC(dev)->revision == 0)
    {
        return GT_FAIL;
    }

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&sdTrainingBmp);

    initPortsBmpPtr = &initPortsBmp;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);

    va_start(ap, numOfPorts);
    for(i = 1; i <= numOfPorts; i++)
    {
        portNum = va_arg(ap, GT_U32);

        rc = cpssDxChPortInterfaceModeGet(dev, portNum, &currentIfMode);
        if (rc != GT_OK)
        {
            va_end(ap);
            return rc;
        }

        rc = cpssDxChPortSpeedGet(dev, portNum, &currentSpeed);
        if (rc != GT_OK)
        {
            va_end(ap);
            return rc;
        }

        if((GT_TRUE == powerUp) && (currentIfMode == ifMode) && (currentSpeed == speed))
        {
            rc = cpssDxChPortLinkStatusGet(dev, portNum, &isLinkUp);
            if (rc != GT_OK)
            {
                va_end(ap);
                return rc;
            }

            if(isLinkUp)
            { /* nothing to do this is just reconfiguration */
                continue;
            }
        }

        rc = cpssDxChPortEnableSet(dev, portNum, GT_FALSE);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortEnableSet", rc);
            va_end(ap);
            return rc;
        }

#ifndef GM_USED
        CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr,portNum);
        rc = waTriggerInterruptsMask(dev, portNum, ifMode, CPSS_EVENT_MASK_E);
        if(rc != GT_OK)
        {
            va_end(ap);
            return rc;
        }
#endif

    }
    va_end(ap);

    rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, powerUp, ifMode, speed);
    if(rc != GT_OK)
    {
        /* TRACE commented out due to JIRA CPSS-5442 */
        /*CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortModeSpeedSet", rc);*/
        return rc;
    }

    if(0 == powerUp)
    {/* don't unmask interrupts if ports powered down to not get them when port
        will be restored */
        return GT_OK;
    }

    if ((CPSS_PORT_INTERFACE_MODE_KR_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_XHGS_E  == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E  == ifMode)
       )
    {
        if(appDemoDbEntryGet("serdesTraining", &serdesTraining) != GT_OK)
        {
            serdesTraining = 1;
        }
    }
    else
    {
        serdesTraining = 0;
    }

    if(serdesTraining)
    {
        CPSS_PORTS_BMP_BITWISE_OR_MAC(&sdTrainingBmp, &sdTrainingBmp, initPortsBmpPtr);
    }

    if(appDemoDbEntryGet("trxTraining", &trxTraining) != GT_OK)
    {
        trxTraining = 0;
    }

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(initPortsBmpPtr, portNum))
        {
            continue;
        }

        TRAINING_DBG_PRINT_MAC(("portNum=%d:ifMode=%d,speed=%d\n", portNum, ifMode, speed));
        rc = cpssDxChPortSerdesSignalDetectGet(dev, portNum, &signalState);
        if(rc != GT_OK)
        {
            cpssOsPrintSync("cpssDxChPortSerdesSignalDetectGet:rc=%d\n", rc);
            return rc;
        }
        TRAINING_DBG_PRINT_MAC(("port=%d:signalState=%d\n", portNum, signalState));
        if(!signalState)
        {
            changeToDownHappend[portNum] = GT_TRUE;
            rc = intMaskSet(dev, portNum, CPSS_EVENT_UNMASK_E);
            if(rc != GT_OK)
            {
                return rc;
            }
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&sdTrainingBmp, portNum);
            continue;
        }
        else
        {
            changeToDownHappend[portNum] = GT_FALSE;
        }
        if(serdesTraining)
        {
            if ( (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode) ||
                 (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode) ||
                 (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode) ||
                 (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode) ||
                 (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode) ||
                 (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode) ||
                 (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode)
               )
            {
                if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&sdTrainingBmp, portNum))
                {
                    continue;
                }
                TRAINING_DBG_PRINT_MAC(("port=%d:start training\n", portNum));
                if(trxTraining)
                {/* if other side started TRX training no gearBox lock could be */
                    TRAINING_DBG_PRINT_MAC(("SERDES TRx training - port %d\n",portNum));

                    rc = cpssDxChPortSerdesAutoTune(dev, portNum,
                                                    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("cpssDxChPortSerdesAutoTune(TRX_CFG,portNum=%d):rc=%d\n", portNum, rc);
                        return rc;
                    }

                    rc = cpssDxChPortSerdesAutoTune(dev, portNum,
                                                    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("cpssDxChPortSerdesAutoTune(TRX_START,portNum=%d):rc=%d\n", portNum, rc);
                        return rc;
                    }
                }
                else
                {
                    TRAINING_DBG_PRINT_MAC(("SERDES RX training - port %d\n",portNum));
                    rc = cpssDxChPortSerdesAutoTune(dev, portNum,
                            CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);

                    if(rc != GT_OK)
                    {
                        TRAINING_DBG_PRINT_MAC(("bc2AppDemoPortsInit:RX training failed:portNum=%d:rc=%d\n", portNum, rc));
                        changeToDownHappend[portNum] = GT_TRUE;
                        CPSS_PORTS_BMP_PORT_CLEAR_MAC(&sdTrainingBmp, portNum);
                        rc = intMaskSet(dev, portNum, CPSS_EVENT_UNMASK_E);
                        if(rc != GT_OK)
                        {
                            cpssOsPrintSync("intMaskSet(port=%d,CPSS_EVENT_UNMASK_E):rc=%d\n", portNum, rc);
                        }
                        continue;
                    }
                }
            }
        }
        else
        {
            rc = cpssDxChPortEnableSet(dev, portNum, GT_TRUE);
            if(rc != GT_OK)
            {
                cpssOsPrintSync("cpssDxChPortEnableSet(portNum=%d,true):rc=%d\n", portNum, rc);
            }

            (GT_VOID)waTriggerInterruptsMask(dev, portNum, ifMode, CPSS_EVENT_UNMASK_E);

            continue;
        }
    }

    if (    (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode)
         || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode)
       )
    {
        if(trxTraining)
        {
            cpssOsTimerWkAfter(500);
        }

        for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&sdTrainingBmp, portNum))
            {
                continue;
            }

            if(trxTraining)
            {
#ifndef ASIC_SIMULATION
                rc = cpssDxChPortSerdesAutoTune(dev, portNum,
                                                CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STATUS_E);
                if(rc != GT_OK)
                {
                    TRAINING_DBG_PRINT_MAC(("bc2AppDemoPortsInit:TRx training failed:portNum=%d:rc=%d\n", portNum, rc));
                    changeToDownHappend[portNum] = GT_TRUE;
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&sdTrainingBmp, portNum);
                    rc = intMaskSet(dev, portNum, CPSS_EVENT_UNMASK_E);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("intMaskSet(port=%d,CPSS_EVENT_UNMASK_E):rc=%d\n", portNum, rc);
                    }
                    continue;
                }

                TRAINING_DBG_PRINT_MAC(("port %d Trx training PASS\n",portNum));
#endif
            }
        }

        if(trxTraining)
        {
            cpssOsTimerWkAfter(10);
        }

        for(i = 0; i < 10; i++)
        {
            for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
            {
                if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&sdTrainingBmp, portNum))
                {
                    continue;
                }

                if((i > 0) && (!changeToDownHappend[portNum]))
                {/* WA succeeded in one of previous iterations */
                    continue;
                }

                TRAINING_DBG_PRINT_MAC(("i=%d)(portNum=%d):",
                                        i, portNum));
                if(rc != GT_OK)
                {
                    if(rc != GT_NOT_READY)
                    {
                        cpssOsPrintSync("ConfigPortsDefaultByProfile:appDemoDxChLion2PortLinkWa(portNum=%d):rc=%d\n",
                                        portNum, rc);
                    }
                    changeToDownHappend[portNum] = GT_TRUE;
                }
                else
                {
                    rc = cpssDxChPortLinkStatusGet(dev, portNum, &isLinkUp);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("bc2AppDemoPortsInit:cpssDxChPortLinkStatusGet(portNum=%d):rc=%d\n",
                                        portNum, rc);
                        return rc;
                    }
                    if(isLinkUp)
                    {
                        changeToDownHappend[portNum] = GT_FALSE;
                    }
                }
                TRAINING_DBG_PRINT_MAC(("rc=%d,changeToDownHappend=%d\n",
                                        rc, changeToDownHappend[portNum]));
            }
            cpssOsTimerWkAfter(10);
        }

        for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&sdTrainingBmp, portNum))
            {
                continue;
            }

            if(!changeToDownHappend[portNum])
            {
                rc = cpssDxChPortEnableSet(dev, portNum, GT_TRUE);
                if(rc != GT_OK)
                {
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortEnableSet", rc);
                    return rc;
                }
            }
            else
            {
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&sdTrainingBmp, portNum);
                rc = intMaskSet(dev, portNum, CPSS_EVENT_UNMASK_E);
                if(rc != GT_OK)
                {
                    cpssOsPrintSync("intMaskSet(port=%d,CPSS_EVENT_UNMASK_E):rc=%d\n", portNum, rc);
                }
                TRAINING_DBG_PRINT_MAC(("ConfigPortsDefaultByProfile:portNum=%d down\n",
                                portNum));
            }
        }
    }

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(initPortsBmpPtr, portNum))
        {
            continue;
        }

        rc = cpssDxChPortEnableSet(dev, portNum, GT_TRUE);
        if(rc != GT_OK)
        {
            cpssOsPrintSync("cpssDxChPortEnableSet(portNum=%d,true):rc=%d\n", portNum, rc);
        }

        (GT_VOID)waTriggerInterruptsMask(dev, portNum, ifMode, CPSS_EVENT_UNMASK_E);
    }

    return GT_OK;
}




extern GT_STATUS prvCpssDxChPortPATimeTakeEnable
(
    GT_VOID
);

extern GT_STATUS prvCpssDxChPortPATimeTakeDisable
(
    GT_VOID
);

extern GT_STATUS prvCpssDxChPortPATimeDiffGet
(
    OUT GT_U32 *prv_paTime_msPtr,
    OUT GT_U32 *prv_paTime_usPtr
);



GT_BOOL  prv_portInitimeTake = GT_FALSE;

GT_STATUS prvAppDeoPortInitTimeTakeEnable
(
    GT_VOID
)
{
    prv_portInitimeTake = GT_TRUE;
    return prvCpssDxChPortPATimeTakeEnable();
}

GT_STATUS prvAppDeoPortInitTimeTimeDisable
(
    GT_VOID
)
{
    prv_portInitimeTake = GT_FALSE;
    return prvCpssDxChPortPATimeTakeDisable();
}

/* flag to allow optimization of the amount of times that 'PIZZA' configured
   for 'X' ports.
   assumption is that caller knows that no traffic should exists in the device
   while 'optimizing'
*/
extern GT_STATUS prvCpssDxChPortDynamicPizzaArbiterIfConfigure_optimizeSet(IN GT_BOOL optimize);

GT_STATUS appDemoBc2PortListInit
(
    IN GT_U8 dev,
    IN PortInitList_STC * portInitList,
    IN GT_BOOL            skipCheckEnable
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 currBoardType;
    GT_U32 portIdx;
    GT_U32 maxPortIdx;
    GT_U32 regAddr;
    CPSS_PORTS_BMP_STC initPortsBmp,/* bitmap of ports to init */
                      *initPortsBmpPtr;/* pointer to bitmap */
    GT_PHYSICAL_PORT_NUM portNum;
    /* PortInitList_STC * portInitList; */
    PortInitList_STC * portInitPtr;
    GT_U32         coreClockDB;
    GT_U32         coreClockHW;
    static PortInitInternal_STC    portList[CPSS_MAX_PORTS_NUM_CNS];
    static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;

    GT_U32 prv_paTime_ms;
    GT_U32 prv_paTime_us;
    GT_U32 prv_portTime_ms;
    GT_U32 prv_portTime_us;
    GT_TIME timeStart = {0,0};
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    GT_U32 pipeId,tileId,dpIndex;
    CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC  mcFifoWeigths;
    GT_U32  mcFifo_0_1 = 0, mcFifo_2_3 = 0, mcFifo_0_1_2_3 = 0; /* assign mc fifos to ports */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    for (portIdx = 0 ; portIdx < sizeof(portList)/sizeof(portList[0]); portIdx++)
    {
        portList[portIdx].portNum       = APP_INV_PORT_CNS;
        portList[portIdx].speed         = CPSS_PORT_SPEED_NA_E;
        portList[portIdx].interfaceMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    }

    maxPortIdx = 0;
    portInitPtr = portInitList;
    for (i = 0 ; portInitPtr->entryType != PORT_LIST_TYPE_EMPTY; i++,portInitPtr++)
    {
        switch (portInitPtr->entryType)
        {
            case PORT_LIST_TYPE_INTERVAL:
                for (portNum = portInitPtr->portList[0] ; portNum <= portInitPtr->portList[1]; portNum += portInitPtr->portList[2])
                {
                    if (skipCheckEnable)
                    {
                        CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
                    }
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->interfaceMode;
                    maxPortIdx++;
                }
            break;
            case PORT_LIST_TYPE_LIST:
                for (portIdx = 0 ; portInitPtr->portList[portIdx] != APP_INV_PORT_CNS; portIdx++)
                {
                    portNum = portInitPtr->portList[portIdx];
                    if (skipCheckEnable)
                    {
                        CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
                    }
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->interfaceMode;
                    maxPortIdx++;
                }
            break;
            default:
                {
                    return GT_NOT_SUPPORTED;
                }
        }
    }

    rc = cpssDxChHwCoreClockGet(dev,/*OUT*/&coreClockDB,&coreClockHW);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\nClock %d MHz",coreClockHW);

#define SIP6_ROW_DELEMITER \
                     "\n|----+------+-------+--------------+------------+----+-----+-----+-----+----|"

    if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        cpssOsPrintf(SIP6_ROW_DELEMITER);
        cpssOsPrintf("\n| #  | Port | Speed | MAC IF MODE  | map  Type  | mac| txq | pipe| tile| dp |");
        cpssOsPrintf(SIP6_ROW_DELEMITER);
    }
    else
    {
        cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+------+");
        cpssOsPrintf("\n| #  | Port | Speed |    IF        |   mapping Type  | rxdma txdma mac txq ilkn  tm |  res |");
        cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+------+");
    }

    initPortsBmpPtr = &initPortsBmp;

    for (portIdx = 0 ; portIdx < maxPortIdx; portIdx++)
    {
        timeStart.sec = 0;
        timeStart.nanoSec = 0;

        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portList[portIdx].portNum,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("\n| %2d | %4d | %s | %s |",portIdx,
                                              portList[portIdx].portNum,
                                              CPSS_SPEED_2_STR(portList[portIdx].speed),
                                              CPSS_IF_2_STR(portList[portIdx].interfaceMode));
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* global pipeId (not relative to the tileId) */
            rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(dev,
                portMapPtr->portMap.macNum/*global MAC port*/,
                &pipeId,
                NULL);/*local MAC*/

            rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,
                portMapPtr->portMap.rxDmaNum/*global DMA port*/,
                &dpIndex,
                NULL);/*local DMA*/

            tileId = pipeId / PRV_CPSS_PP_MAC(dev)->multiPipe.numOfPipesPerTile;

            cpssOsPrintf(" %-10s |%3d |%4d | %3d | %3d | %2d |"
                                            ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                            ,portMapPtr->portMap.macNum
                                            ,portMapPtr->portMap.txqNum
                                            ,pipeId
                                            ,tileId
                                            ,dpIndex);
        }
        else
        {
            cpssOsPrintf(" %-15s | %5d %5d %3d %3d %4d %3d |"
                                            ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                            ,portMapPtr->portMap.rxDmaNum
                                            ,portMapPtr->portMap.txDmaNum
                                            ,portMapPtr->portMap.macNum
                                            ,portMapPtr->portMap.txqNum
                                            ,portMapPtr->portMap.ilknChannel
                                            ,portMapPtr->portMap.tmPortIdx);
        }

        if (prv_portInitimeTake == GT_TRUE)
        {
            rc = prvCpssOsTimeRTns(&timeStart);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);
        CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr,portList[portIdx].portNum);

        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
            if(cpssDeviceRunCheck_onEmulator())
            {
                if(portIdx == 0)
                {
                    /* reduce the number of HW calls */
                    (void)prvCpssDxChPortDynamicPizzaArbiterIfConfigure_optimizeSet(GT_TRUE/*optimize*/);
                }
                else
                if(portIdx == (maxPortIdx-1))
                {
                    /* restore to default .. allow 'last port' to set the PIZZA for ALL ports !!!*/
                    (void)prvCpssDxChPortDynamicPizzaArbiterIfConfigure_optimizeSet(GT_FALSE/* STOP the optimization */);
                }
            }

            if((CPSS_PP_FAMILY_DXCH_BOBCAT2_E == appDemoPpConfigList[dev].devFamily) &&
                (CPSS_PP_SUB_FAMILY_NONE_E == PRV_CPSS_PP_MAC(appDemoPpConfigList[dev].devNum)->devSubFamily) &&
                (PRV_CPSS_PP_MAC(appDemoPpConfigList[dev].devNum)->revision > 0))

            {
                rc = bc2AppDemoPortsInit(dev, portList[portIdx].interfaceMode, portList[portIdx].speed, GT_TRUE, 1, portList[portIdx].portNum);
            }
            else
            {
                rc = appDemoBoardTypeGet(&currBoardType);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBoardTypeGet", rc);
                if(rc != GT_OK)
                {
                    return rc;
                }

                if((currBoardType == APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS) ||
                   (currBoardType == APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS))
                {
                    rc = cpssDxChPortRefClockSourceOverrideEnableSet(dev, portList[portIdx].portNum, GT_TRUE, CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortRefClockSourceOverrideEnableSet", rc);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }
                rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_TRUE,
                                            portList[portIdx].interfaceMode,
                                            portList[portIdx].speed);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if((currBoardType == APP_DEMO_CAELUM_BOARD_CYGNUS_RD_CNS)||
                  (currBoardType == APP_DEMO_CAELUM_BOARD_DB_CNS))
                {
                    if((portList[portIdx].interfaceMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E) &&
                       (portList[portIdx].portNum % 4 == 0))
                    {
                        CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC rxConfig;
                        CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC txConfig;

                        rxConfig.align90 = 0;
                        rxConfig.bandWidth = 13;
                        rxConfig.dcGain = 80;
                        rxConfig.ffeCap = 15;
                        rxConfig.ffeRes = 6;
                        rxConfig.loopBandwidth = 0;
                        rxConfig.sqlch = 100;

                        rc = cpssDxChPortSerdesManualRxConfigSet(dev, portList[portIdx].portNum, 0, &rxConfig);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        txConfig.emph0 = 0;
                        txConfig.emph1 = 0;
                        txConfig.txAmp = 24;
                        txConfig.txAmpAdjEn = GT_FALSE;
                        txConfig.txAmpShft = GT_FALSE;

                        rc = cpssDxChPortSerdesManualTxConfigSet(dev, portList[portIdx].portNum, 0, &txConfig);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        /* Serdes Digital Reset/UnReset */
                        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->serdesConfig[(portList[portIdx].portNum / 4)].serdesExternalReg2;
                        rc = prvCpssHwPpSetRegField(dev, regAddr, 3, 1, 0);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        cpssOsTimerWkAfter(10);

                        rc = prvCpssHwPpSetRegField(dev, regAddr, 3, 1, 1);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                }
                else
                if((currBoardType == APP_DEMO_XCAT3X_A0_BOARD_DB_CNS))
                {
                    if((portList[portIdx].interfaceMode == CPSS_PORT_INTERFACE_MODE_RXAUI_E))
                    {
                        GT_PHYSICAL_PORT_NUM                portNum;
                        CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC rxConfig;

                        portNum = portList[portIdx].portNum;


                        /* Get parameters for lane #0 */
                        rc = cpssDxChPortSerdesManualRxConfigGet(dev, portNum, 0, &rxConfig);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        /* Adjust parameters according to platform analysis */
                        rxConfig.bandWidth = 13;
                        rxConfig.dcGain = 76;
                        rxConfig.ffeCap = 15;
                        rxConfig.ffeRes = 8;

                        rc = cpssDxChPortSerdesManualRxConfigSet(dev, portNum, 0, &rxConfig);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        /* Get parameters for lane #1 */
                        rc = cpssDxChPortSerdesManualRxConfigGet(dev, portNum, 1, &rxConfig);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        /* Adjust parameters according to platform analysis */
                        rxConfig.bandWidth = 13;
                        rxConfig.dcGain = 76;
                        rxConfig.ffeCap = 15;
                        rxConfig.ffeRes = 8;

                        rc = cpssDxChPortSerdesManualRxConfigSet(dev, portNum, 1, &rxConfig);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        /* Serdes Digital Reset/UnReset */
                        rc = cpssDxChPortSerdesResetStateSet (dev, portNum, GT_TRUE);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        cpssOsTimerWkAfter(10);

                        rc = cpssDxChPortSerdesResetStateSet (dev, portNum, GT_FALSE);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                }
                else if (currBoardType == APP_DEMO_BOBCAT3_BOARD_DB_CNS ||
                         currBoardType == APP_DEMO_ALDRIN_BOARD_DB_CNS ||
                         currBoardType == APP_DEMO_ALDRIN2_BOARD_DB_CNS )
                {
                    /* default HWS squelch for some of the port modes on those DB boards can be problematic,
                       so there is a need to set a custom value */
                    if (portList[portIdx].speed == CPSS_PORT_SPEED_10000_E)
                    {
                        /* value 2 is the normalized value of signal_ok level (squelch) of value 100mV, to Avago API units */
                        rc = cpssDxChPortSerdesSquelchSet(dev, portList[portIdx].portNum, 2);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                }
            }

            if(rc != GT_OK)
            {
                cpssOsPrintf("\n--> ERROR : cpssDxChPortModeSpeedSet(portNum=%d, ifMode=%d, speed=%d) :rc=%d\n",
                portList[portIdx].portNum, portList[portIdx].interfaceMode, portList[portIdx].speed, rc);
                return rc;
            }
        }

        if (prv_portInitimeTake == GT_TRUE)
        {
            prvCpssOsTimeRTDiff(timeStart, /*OUT*/&prv_portTime_ms, &prv_portTime_us);
        }


        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* we not need it in the printed info */
        }
        else
        {
            cpssOsPrintf("  OK  |");
        }

        if (prv_portInitimeTake == GT_TRUE)
        {
            cpssOsPrintf(" total : %4d.%03d ms",prv_portTime_ms,prv_portTime_us);
            if (GT_OK == prvCpssDxChPortPATimeDiffGet(&prv_paTime_ms,&prv_paTime_us))
            {
                cpssOsPrintf(" PA-time : %4d.%03d ms",prv_paTime_ms,prv_paTime_us);

            }
        }

        /* MC FIFO mapping for SIP devices */
        /* BC2 and Caelum device have FIFO 0-1 for port speeds <= 1G and rest port speeds 
         * have FIFO 2-3.
         */
        if (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(dev) || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(dev))
        {
            if (portList[portIdx].speed == CPSS_PORT_SPEED_1000_E) 
            {
                rc = cpssDxChPortTxMcFifoSet(dev, portList[portIdx].portNum, mcFifo_0_1%2);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortTxMcFifoSet", rc);
                if (GT_OK != rc)
                {
                    return rc;
                }
                mcFifo_0_1++;
            }
            else
            {
                rc = cpssDxChPortTxMcFifoSet(dev, portList[portIdx].portNum, ((mcFifo_2_3%2) + 2));
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortTxMcFifoSet", rc);
                if (GT_OK != rc)
                {
                    return rc;
                }
                mcFifo_2_3++;
            }
        }
        /* Caetus device has only 12 ports i.e. 56-59 and 64-71, so MC FIFO
         * mapping of 12 ports across 4 FIFOs 0-3.
         * Aldrin device has 33 ports i.e. 0-32, so MC FIFO mapping of
         * ports across 4 FIFOs 0-3.
         */
        if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(dev) || PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(dev))
        {
            rc = cpssDxChPortTxMcFifoSet(dev, portList[portIdx].portNum, mcFifo_0_1_2_3%4);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortTxMcFifoSet", rc);
            if (GT_OK != rc)
            {
                return rc;
            }
            mcFifo_0_1_2_3++;
        }
        /* BC3 and Aldrin2 device have FIFO 0-1 for port speeds <= 10G and rest port speeds 
         * have FIFO 2-3.
         */
        if (PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(dev) || PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(dev))
        {
            if (portList[portIdx].speed <= CPSS_PORT_SPEED_10000_E ||
                portList[portIdx].speed == CPSS_PORT_SPEED_2500_E ||
                portList[portIdx].speed == CPSS_PORT_SPEED_5000_E) 
            {
                rc = cpssDxChPortTxMcFifoSet(dev, portList[portIdx].portNum, mcFifo_0_1%2);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortTxMcFifoSet", rc);
                if (GT_OK != rc)
                {
                    return rc;
                }
                mcFifo_0_1++;
            }
            else
            {
                rc = cpssDxChPortTxMcFifoSet(dev, portList[portIdx].portNum, ((mcFifo_2_3%2) + 2));
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortTxMcFifoSet", rc);
                if (GT_OK != rc)
                {
                    return rc;
                }
                mcFifo_2_3++;
            }
        }
    }
    /* Device has 4 FIFOs, 100G share FIFO 2/3 and rest share FIFO 0/1. Total bandwidth 
     * considering 100G and 10G ports is 48*10G + 6*100G, so arbitration bw
     * FIFOs give 480 and 600 i.e. 24,24,30,30. Considering bursty traffic its
     * kept as 12,12,15,15.
     */
    if (PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(dev) || PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(dev))
    {
        mcFifoWeigths.mcFifo0 = 0x0B;
        mcFifoWeigths.mcFifo1 = 0x0B;
        mcFifoWeigths.mcFifo2 = 0x0F;
        mcFifoWeigths.mcFifo3 = 0x0F;
        rc = cpssDxChPortTxMcFifoArbiterWeigthsSet(dev, &mcFifoWeigths);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortTxMcFifoArbiterWeigthsSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }  
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        GT_U32    numOfNetIfs,mgUnitId;
        GT_U8     sdmaQueueIndex = 0;
        GT_U32    portNum;

        cpssOsPrintf(SIP6_ROW_DELEMITER);

        rc = prvCpssDxChNetIfMultiNetIfNumberGet(dev,&numOfNetIfs);
        if (rc != GT_OK)
        {
            return rc;
        }

        /***************************/
        /* list the SDMA CPU ports */
        /***************************/
        for(/*portIdx*/;portIdx < (maxPortIdx+numOfNetIfs); portIdx++ , sdmaQueueIndex+=8)
        {
            rc = cpssDxChNetIfSdmaQueueToPhysicalPortGet(dev,sdmaQueueIndex,&portNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portNum,/*OUT*/portMapPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(dev,
                portMapPtr->portMap.rxDmaNum/*global DMA port*/,
                &dpIndex,
                NULL);/*local DMA*/

            PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(dev,(sdmaQueueIndex>>3),mgUnitId);

            pipeId = dpIndex / 4;
            tileId = pipeId  / PRV_CPSS_PP_MAC(dev)->multiPipe.numOfPipesPerTile;

            cpssOsPrintf("\n| %2d | %4d | CPU-SDMA queue [%3.1d..%3.1d]  MG[%2.1d]",portIdx,
                                                  portNum,
                                                  sdmaQueueIndex,
                                                  sdmaQueueIndex+7,
                                                  mgUnitId);

            cpssOsPrintf(" |%3d |%4d | %3d | %3d | %2d |"
                                            ,portMapPtr->portMap.rxDmaNum
                                            ,portMapPtr->portMap.txqNum
                                            ,pipeId
                                            ,tileId
                                            ,dpIndex);
        }

        cpssOsPrintf(SIP6_ROW_DELEMITER);
    }
    else
    {
        cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+------+");
    }
    cpssOsPrintf("\nPORT INTERFACE Init Done.\n");
    return GT_OK;
}


GT_STATUS appDemoBc2PortListDelete
(
    IN GT_U8 dev,
    IN PortInitList_STC * portInitList
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_U32 portIdx;
    GT_U32 maxPortIdx;
    CPSS_PORTS_BMP_STC initPortsBmp,/* bitmap of ports to init */
                      *initPortsBmpPtr;/* pointer to bitmap */
    GT_PHYSICAL_PORT_NUM portNum;
    /* PortInitList_STC * portInitList; */
    PortInitList_STC * portInitPtr;
    GT_U32         coreClockDB;
    GT_U32         coreClockHW;
    static PortInitInternal_STC    portList[256];


    for (portIdx = 0 ; portIdx < sizeof(portList)/sizeof(portList[0]); portIdx++)
    {
        portList[portIdx].portNum       = APP_INV_PORT_CNS;
        portList[portIdx].speed         = CPSS_PORT_SPEED_NA_E;
        portList[portIdx].interfaceMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    }

    maxPortIdx = 0;
    portInitPtr = portInitList;
    for (i = 0 ; portInitPtr->entryType != PORT_LIST_TYPE_EMPTY; i++,portInitPtr++)
    {
        switch (portInitPtr->entryType)
        {
            case PORT_LIST_TYPE_INTERVAL:
                for (portNum = portInitPtr->portList[0] ; portNum <= portInitPtr->portList[1]; portNum += portInitPtr->portList[2])
                {
                    CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->interfaceMode;
                    maxPortIdx++;
                }
            break;
            case PORT_LIST_TYPE_LIST:
                for (portIdx = 0 ; portInitPtr->portList[portIdx] != APP_INV_PORT_CNS; portIdx++)
                {
                    portNum = portInitPtr->portList[portIdx];
                    CPSS_ENABLER_PORT_SKIP_CHECK(dev, portNum);
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->interfaceMode;
                    maxPortIdx++;
                }
            break;
            default:
                {
                    return GT_NOT_SUPPORTED;
                }
        }
    }

    rc = cpssDxChHwCoreClockGet(dev,/*OUT*/&coreClockDB,&coreClockHW);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\nClock %d MHz",coreClockHW);
    cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+------+");
    cpssOsPrintf("\n| #  | Port | Speed |    IF        |   mapping Type  | rxdma txdma mac txq ilkn  tm |  res |");
    cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+------+");

    initPortsBmpPtr = &initPortsBmp;

    for (portIdx = 0 ; portIdx < maxPortIdx; portIdx++)
    {
        static CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapPtr = &portMap;
        rc = cpssDxChPortPhysicalPortDetailedMapGet(dev,portList[portIdx].portNum,/*OUT*/portMapPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("\n| %2d | %4d | %s | %s |",portIdx,
                                              portList[portIdx].portNum,
                                              CPSS_SPEED_2_STR(portList[portIdx].speed),
                                              CPSS_IF_2_STR(portList[portIdx].interfaceMode));
        cpssOsPrintf(" %-15s | %5d %5d %3d %3d %4d %3d |"
                                            ,CPSS_MAPPING_2_STR(portMapPtr->portMap.mappingType)
                                            ,portMapPtr->portMap.rxDmaNum
                                            ,portMapPtr->portMap.txDmaNum
                                            ,portMapPtr->portMap.macNum
                                            ,portMapPtr->portMap.txqNum
                                            ,portMapPtr->portMap.ilknChannel
                                            ,portMapPtr->portMap.tmPortIdx);


        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(initPortsBmpPtr);
        CPSS_PORTS_BMP_PORT_SET_MAC(initPortsBmpPtr,portList[portIdx].portNum);
        rc = cpssDxChPortModeSpeedSet(dev, &initPortsBmp, GT_FALSE,
                                        portList[portIdx].interfaceMode,
                                        portList[portIdx].speed);
        if(rc != GT_OK)
        {
            cpssOsPrintf("\n--> ERROR : cpssDxChPortModeSpeedSet(portNum=%d, ifMode=%d, speed=%d) :rc=%d\n",
            portList[portIdx].portNum, portList[portIdx].interfaceMode, portList[portIdx].speed, rc);
            return rc;
        }
        cpssOsPrintf("  OK  |");
    }
    cpssOsPrintf("\n+----+------+-------+--------------+-----------------+------------------------------+------+");
    cpssOsPrintf("\nPORT List Delete Done.");
    return GT_OK;
}


GT_STATUS bobcat2PizzaArbiterUnitSet
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_PA_UNIT_ENT unitId,
    IN GT_U32   sliceNum,
    ...
)
{
    GT_STATUS rc;
    GT_U32 clientId;
    GT_U32 slice2PortMap[20];
    GT_U32 i;
    va_list ap;


    va_start(ap, sliceNum);
    for(i = 0; i < sliceNum;i++)
    {
        clientId = va_arg(ap, GT_U32);
        if (clientId == 111)
        {
            clientId = PA_INVALID_PORT_CNS;
        }
        slice2PortMap[i] = clientId;
    }
    va_end(ap);

    rc = bobcat2PizzaArbiterUnitDrvSet(devNum,unitId,sliceNum,&slice2PortMap[0]);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}




