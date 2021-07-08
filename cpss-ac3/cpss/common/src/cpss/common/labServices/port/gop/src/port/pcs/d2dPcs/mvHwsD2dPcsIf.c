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
*******************************************************************************
* @file mvHwsD2dPcsIf.c
*
* @brief D2D PCS interface API
*
* @version 1
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/d2dPcs/mvHwsD2dPcsIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/gw16/mvGw16If.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>

static char* mvHwsD2dPcsTypeGetFunc(void)
{
  return "D2D_PCS";
}
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};


GT_STATUS mvHwsD2dPcsReset
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_BOOL enable
)
{
    GT_U32 regAddr, regData, regMask;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;

    d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
    if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
    {
        /*Eagle: Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_OOR_CFG=0x1*/
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    else
    {   /*RAVEN D2D */
        /*Raven: Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_OOR_CFG=0x1*/
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    }
    regData = enable;
    regMask = 0x1;
    regAddr = baseAddr + D2D_PCS_PCS_OOR_CFG;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    return GT_OK;
}
/**
* @internal mvHwsD2dPcsTestGen function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                -
* @param[in] pcsNum                   - port number
*/
GT_STATUS mvHwsD2dPcsTestGen
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          pcsNum,
    MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
)
{
    GT_U32 regAddr1,regAddr2, regData, regMask;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;
    portGroup = portGroup;

    if(configPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    d2dIndex = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(pcsNum);
    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        regAddr1 = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_CFG_ENA);
        regAddr2 = PRV_HWS_PMA_REG_ADDR_CALC_MAC(d2dIndex, D2D_PMA_CFG_EN);
    }
    else /*falcon*/
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        regAddr1 = baseAddr + D2D_PCS_PCS_CFG_ENA;
        regAddr2 = baseAddr + D2D_PMA_CFG_EN;
    }

    if(configPtr->portPattern == TEST_GEN_Normal)
    {

        regMask = 0x30003;
        regData = 0x10001;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr1, regData, regMask));

        /* Configured PMA gearbox bypass to allow PRBS check in PCS */
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr2, 1, 1, 0));
    }
    else if(configPtr->portPattern == TEST_GEN_PRBS31)
    {
        /* Write PCS Global Control register */
        switch(configPtr->direction)
        {
            case HWS_D2D_DIRECTION_MODE_BOTH_E:
                regMask = 0x30003;
                regData = 0x20002;
                break;
            case HWS_D2D_DIRECTION_MODE_RX_E:
                regMask = 0x30000;
                regData = 0x20000;
                break;
            case HWS_D2D_DIRECTION_MODE_TX_E:
                regMask = 0x3;
                regData = 0x2;
                break;
            default:
                return GT_BAD_PARAM;
        }
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr1, regData, regMask));

        if((configPtr->direction == HWS_D2D_DIRECTION_MODE_BOTH_E) || (configPtr->direction == HWS_D2D_DIRECTION_MODE_RX_E))
        {
            /* Configured PMA gearbox bypass to allow PRBS check in PCS */
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr2, 1, 1, 1));
        }
    }
    else
    {
        return GT_BAD_PARAM;
    }

    return GT_OK;
}



/**
* @internal mvHwsD2dPcsLoopbackSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] lbType                   - loopback type
*/

GT_STATUS mvHwsD2dPcsLoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    GT_U32 regAddr, regData;
    GT_U32 unitIndex, unitNum;
    GT_U32 regMask, baseAddr;
    GT_U32 d2dIndex;
    portGroup = portGroup;
    portMode = portMode;

    switch(lbType)
    {
        case DISABLE_LB:
            regData = 0x0;
            break;

        case TX_2_RX_LB:
            regData = 0x1;
            break;

        case RX_2_TX_LB:
        default:
            return GT_NOT_SUPPORTED;
    }
    regMask = (GT_U32)1<<31;
    d2dIndex = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(phyPortNum);
    CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, (regData<<31), regMask));

    return GT_OK;
}

/**
* @internal mvHwsD2dPcsLoopbackGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] lbType                   - loopback type
*/
GT_STATUS mvHwsD2dPcsLoopbackGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_U32 regAddr, regData;
    GT_U32 unitIndex, unitNum;
    GT_U32 regMask, baseAddr;
    GT_U32 d2dIndex;
    portGroup = portGroup;
    portMode = portMode;

    regMask = (GT_U32)1<<31;
    d2dIndex = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(phyPortNum);
    CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
    regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    switch (regData>>31) {
        case 0:
            *lbType = DISABLE_LB;
            break;
        case 1:
            *lbType = TX_2_RX_LB;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }
    return GT_OK;
}

/**
* @internal mvHwsD2dPcsErrorInjectionSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                -
* @param[in] portNum                   - port number
* @param[in] laneIdx                   - lane 0..3
* @param[in] errorNum                 - number of errors to inject
*/
GT_STATUS mvHwsD2dPcsErrorInjectionSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,  /* 1024.. */
    GT_U32                  laneIdx,  /* lane 0..3 */
    GT_U32                  errorNum    /* 1..0x7FF, 0x7FF enables continuously injection */
)
{
    GT_U32 regAddr1,regAddr2, regData1, regData2;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;
    portGroup = portGroup;

    switch(errorNum)
    {
        case 0:
            regData1 = 0;
            regData2 = 0;
            break;

        case 1:
            regData1 = (0xF << 8);
            regData2 = 1;
            break;

        case 2:
            return GT_NOT_SUPPORTED;

        case 0x7FF:
            regData1 = (0xF << 8) | (0x7FF << 20);
            regData2 = 1;

        default:
            regData1 = (0xF << 8) | ((errorNum - 2) << 20);
            regData2 = 1;
            break;
    }

    if((portGroup > 1) || (laneIdx > 3))
    {
        return GT_BAD_PARAM;
    }
    d2dIndex = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(portNum);
    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        regAddr1 = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_DEBUG_ERROR_INJECT);
        regAddr2 = PRV_HWS_PMA_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_DBG_LANE_CTRL + laneIdx * 0x4);
    }
    else /*falcon*/
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        regAddr1 = baseAddr + D2D_PCS_DEBUG_ERROR_INJECT;
        regAddr2 = baseAddr + D2D_PCS_DBG_LANE_CTRL + laneIdx * 0x4;
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr1, regData1, 0x7FFFFF00));

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr2, 6, 2, regData2));

    return GT_OK;
}

/**
* @internal mvHwsD2dPcsErrorCounterGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group
* @param[in] port                     - port number 1024..
* @param[in] laneIdx                  - lane index
* @param[out] errorCntPtr             - (pointer to) number of errors counter
*/
GT_STATUS mvHwsD2dPcsErrorCounterGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  port,       /* port number */
    GT_U32                  laneIdx,  /* lane 0..3 */
    GT_U32                  *errorCntPtr
)
{
    GT_U32 regAddr, regData;
    GT_U32 baseAddr, unitNum, unitIndex;
    GT_U32 d2dIndex;
    portGroup = portGroup;

    if(errorCntPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    if((portGroup > 1) || (laneIdx > 3))
    {
        return GT_BAD_PARAM;
    }
    d2dIndex = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(port);
    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_AM_BER_CNT_LANE + laneIdx * 0x4);
    }
    else /*falcon*/
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum ));
        regAddr = baseAddr + D2D_PCS_AM_BER_CNT_LANE  + laneIdx * 0x4;
    }

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0));

    *errorCntPtr = regData;

    return GT_OK;
}


GT_STATUS mvHwsD2dPcsInit
(
    CPSS_HW_DRIVER_STC  *driverPtr,
    GT_U8               devNum,
    GT_U32              d2dNum

)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 d2dIndex;
    GT_U32 rowIdx;
    GT_U32 baseAddr, unitNum, unitIndex;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            for(rowIdx = 0; rowIdx < 32; rowIdx++)
            {
                /* Write to: PCS Transmit Calendar Slot Configuration %n */
                regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_TX_CAL_BASE + 4*rowIdx));
                regData = 0x0008103E;
                regMask = 0x00FFFFFF;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

                /* Write to: PCS Receive Calendar Slot Configuration %n */
                regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, (D2D_PCS_PCS_RX_CAL_BASE + 4*rowIdx));
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
            }

            /* Write to: PCS Transmit Calendar Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_CAL_CTRL);
            regData = 0x0040007F;
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Receive Calendar Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_RX_CAL_CTRL);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Transmit Rate Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_TX_RATE_CTRL);
            regData = (6 << 26) | (0xA << 22) | (0x62F << 11) | (0x5DC);
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Write to: PCS Global Control */
            regAddr = PRV_HWS_D2D_PCS_REG_ADDR_CALC_MAC(d2dIndex, D2D_PCS_PCS_CFG_ENA);
            regData = 0x00050005;
            regMask = 0xFFFFFFFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
        }
    }
    else
    {   /* falcon */
        d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
        if (!PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
        {
            /*Raven, done through SMI:
            Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_CFG_ENA=0xd003d*/
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));


            /* Write to: PCS Global Control */
            regData = 0x000d003d;
            regMask = 0xFFFFFFFF;
            regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
        }
        else
        { /* eagle, done with PEX */
            /*  Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
              Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
             mmap.D2D_RegFile.PCS_RX_IFC_REMAP_8.PCS_RX_IFC_REMAP_OFFSET_8.set( 8 );
             CP Interface
            mmap.D2D_RegFile.PCS_RX_IFC_REMAP_9.PCS_RX_IFC_REMAP_OFFSET_9.set( 8 );
            TX remap
             CPU port
            mmap.D2D_RegFile.PCS_RX_IFC_REMAP_16.PCS_TX_IFC_REMAP_OFFSET_16.set( 24 );
             CP Interface
            mmap.D2D_RegFile.PCS_RX_IFC_REMAP_17.PCS_TX_IFC_REMAP_OFFSET_17.set( 24 );*/

            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));

            /* Write to: PCS Global Control */
            regData = 0x000d003d;
            regMask = 0xFFFFFFFF;
            regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_PCS_CFG_ENA;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

            /*Write PCS_RX_IFC_REMAP_8,9=0x8*/
            regData = 0x08;
            regMask = 0x1F;
            regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_8;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
            regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_9;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
            /*Write PCS_TX_IFC_REMAP_16,17=0x1800*/
            regData = 0x1800;
            regMask = 0x1F00;
            regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_16;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
            regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PCS_RX_IFC_REMAP_17;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);

        }

    }
    return GT_OK;
}

/**
* @internal mvHwsD2dPcsIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
GT_STATUS mvHwsD2dPcsIfInit(GT_U8 devNum, MV_HWS_PCS_FUNC_PTRS *funcPtrArray)
{
    if ((hwsInitDone[devNum] == GT_FALSE) && (hwsDeviceSpecInfo[devNum].devType == Raven))
    {
        /* init sequences DB */
        CHECK_STATUS(mvHwsD2dPcsInit(NULL,devNum, HWS_D2D_ALL));
    }

    funcPtrArray[D2D_PCS].pcsResetFunc     = NULL;
    funcPtrArray[D2D_PCS].pcsModeCfgFunc   = NULL;
    funcPtrArray[D2D_PCS].pcsLbCfgFunc     = mvHwsD2dPcsLoopbackSet;
    funcPtrArray[D2D_PCS].pcsLbCfgGetFunc = mvHwsD2dPcsLoopbackGet;
    funcPtrArray[D2D_PCS].pcsTypeGetFunc   = mvHwsD2dPcsTypeGetFunc;
    funcPtrArray[D2D_PCS].pcsFecCfgFunc    = NULL;
    funcPtrArray[D2D_PCS].pcsFecCfgGetFunc = NULL;
    funcPtrArray[D2D_PCS].pcsCheckGearBoxFunc = NULL;
    funcPtrArray[D2D_PCS].pcsAlignLockGetFunc = NULL;
    funcPtrArray[D2D_PCS].pcsActiveStatusGetFunc = NULL;
    funcPtrArray[D2D_PCS].pcsSendFaultSetFunc = NULL;

    hwsInitDone[devNum] = GT_TRUE;

    return GT_OK;
}

/**
* @internal hwsD2dPcsIfClose function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
void hwsD2dPcsIfClose
(
    GT_U8      devNum
)
{
    hwsInitDone[devNum] = GT_FALSE;
}


