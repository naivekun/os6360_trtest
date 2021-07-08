/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvGw16If.c
*
* DESCRIPTION:
*       GW16nm SERDES configuration
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/gw16/mvGw16If.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#ifndef MV_HWS_REDUCED_BUILD
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#endif

#ifndef CO_CPU_RUN
static char* mvHwsGw16SerdesTypeGetFunc(void)
{
  return "GW16";
}
#endif

/************************* Globals *******************************************************/

static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};

static GT_BOOL d2dPrintEnableFullConfiguration = GT_FALSE;

/************************* pre-declaration ***********************************************/

/*****************************************************************************************/

        /************************/
        /*  - Debug function -  */
        /************************/
GT_STATUS gw16LinkStateGet()
{
    GT_U8 devNum;
    GT_U32 d2dIndex;
    GT_U32 regData;
    GT_U32 regAddr;

    for(devNum = 0; devNum < 6; devNum++)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RXC_AUTOSTART);
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFF));
            hwsOsPrintf("device=%d, d2d_index=%d, link_status=0x%x\n", devNum, d2dIndex, regData);
        }
    }

    return GT_OK;
}

        /************************/
        /*  - Debug function -  */
        /************************/
GT_STATUS gw16LoopbackSet()
{
    GT_U8 devNum;
    GT_U32 d2dIndex;
    GT_U32 regAddr;

    for(devNum = 0; devNum < 6; devNum++)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RX_CFG);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 3, 3));

            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_TX_CFG);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 3, 3));
        }

        regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_TXC_OVRCTRL);
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 1, 1));

        regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RXC_CFG);
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 3, 3));
    }


    for(devNum = 0; devNum < 6; devNum++)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RXC_AUTOSTART);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x0, 0x0));
        }
    }

    for(devNum = 0; devNum < 6; devNum++)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RXC_AUTOSTART);
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0xF, 0xF));
        }
    }


    return GT_OK;
}

GT_U32  good_values_eq[6][8] = { {/*gp0m0*/4,  4,  5,  4, /*gp0m1*/1, 12,  4, 12},
                                 {/*gp1m0*/1,  1,  4,  4, /*gp1m1*/5, 12, 12, 13},
                                 {/*gp2m0*/4, 12, 12, 13, /*gp2m1*/12, 5,  5,  5},
                                 {/*gp3m0*/12, 4,  5,  5, /*gp3m1*/12, 5,  4,  5},
                                 {/*gp4m0*/12, 12, 5, 12, /*gp4m1*/4, 13,  5,  1},
                                 {/*gp5m0*/0,  0,  0,  0, /*gp5m1*/15,13, 13, 12}};

GT_U32  good_values_ib[6][8] = { {/*gp0m0*/9, 10, 10,  9, /*gp0m1*/9,  9,  10,  9},
                                 {/*gp1m0*/11,11, 11, 12, /*gp1m1*/11, 10,  9, 10},
                                 {/*gp2m0*/11, 9,  9,  8, /*gp2m1*/8,  10,  9, 10},
                                 {/*gp3m0*/10, 9,  9,  9, /*gp3m1*/11, 10,  9,  9},
                                 {/*gp4m0*/9,  9,  8, 10, /*gp4m1*/10, 9,   9,  9},
                                 {/*gp5m0*/0,  0,  0,  0, /*gp5m1*/8,  9,   9,  9}};

/**
* @internal mvHwsGW16SerdesAutoStartInit function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsGW16SerdesAutoStartInit
(
    CPSS_HW_DRIVER_STC *driverPtr,
    GT_U8 devNum,
    GT_U32 d2dNum
)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regMask;
    GT_U32 lane, eq, ib;
    GT_U32 d2dIndex;
    GT_U32 baseAddr, unitNum, unitIndex;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            /* skip the links that go out of package */
            if((devNum == 5) && (d2dIndex == 0))
            {
                continue;
            }

            /* The PMA layer is released fom TX/RX reset with the following sequence */
            regAddr = PRV_HWS_PMA_REG_ADDR_CALC_MAC(d2dIndex, D2D_PMA_CFG_RESET);
            regData = 0x0;
            regMask = 0xFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
            regData = 0xFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Configure 1 APB wait statePB wait state for D2D */
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_TXC_APBWAIT);
            regData = 0x2;
            regMask = 0x7;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_TXC_PFBD);
            regData = 80;
            regMask = 0xFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Set TX PLL to high bandwidth */
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_TXC_PLL_LOOP_CTRL);
            regData = 0x0D;
            regMask = 0x3F;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_TXC_PLL_CP_CTRL);
            regData = 0x17;
            regMask = 0x1F;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* set 0.875V */
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RXC_PREG_TRIM);
            regData = 0x1;
            regMask = 0x7;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Trim rxc_i50u_r */
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RXC_BIAS_CONFIG);
            regData = 11;
            regMask = 0x1F;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            /* Set the default DAC step to 1.5mV */
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RXC_DAC_CONFIG);
            regData = 0x5A;
            regMask = 0xFF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            for(lane = 0; lane < 4; lane++)
            {
                /* Pre-tapeout known issue: wrong default value */
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RX_SCHEXP + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 21, 0x1F));

                /* Silicon bug: CDA PI invert */
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RX_CDACFG + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x1, 0x1));

                /* Set pMode enable */
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RX_PLL_LOCK_CTRL + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x4, 0x4));

    #if 0
                eq = good_values_eq[devNum][d2dIndex*4 + lane];
                if(eq == 0)
                {
                    eq = 13;
                }
                ib = good_values_ib[devNum][d2dIndex*4 + lane];
                if(ib == 0)
                {
                    ib = 10;
                }
    #endif
                eq = 12;
                ib = 11;

                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RX_CTLE_EQ + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, eq, 0xF));

                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RX_CTLE_CTRL + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, (ib<<1), 0x1F));

                /* TX drive strengths (change to equal eye mode) */
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_TX_DRVSLCEN_7_0 + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0xDF, 0xFF));
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_TX_DRVSLCEN_11_8 + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x7, 0xF));
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_TX_EQCMSLCEN + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x60, 0x3F));

                /* Temporary disable bgcal until fine offset works */
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RX_SCHCFG + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x3, 0x7));

                /* Temporary disable fine offset and QMC */
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RX_AUTOCAL + lane * 0x800);
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x76, 0x7F));
            }

    #if 0
            /* Enable/Disable RXC autocal */
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex, D2D_PHY_RXC_AUTOCAL);
            regMask = 0x3;
            regData = 0x0;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    #endif
            /* Set autostart field */
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RXC_AUTOSTART);
            regData = 0x0;
            regMask = 0xF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
            regData = 0xF;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
        }

        hwsOsTimerWkFuncPtr(10);
    } /*raven*/
    else if (hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
        if (!PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
        {
            /*Raven, done through SMI:
            Wrote register via map d2d_reg_block[1].RXC_RegFile.uvm_reg_map: d2d_reg_block[1].RXC_RegFile.autostart=0xf
            Wrote register via map d2d_reg_block[1].D2D_RegFile.uvm_reg_map: d2d_reg_block[1].D2D_RegFile.PCS_CFG_ENA=0xd003d*/
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_RAVEN_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));
            regData = 0xF;
            regMask = 0xF;
            regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PHY_RXC_AUTOSTART;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);


            /* Write to: PCS Global Control */
            /*regData = 0x000d003d;
            regMask = 0xFFFFFFFF;
            regAddr = baseAddr + D2D_PCS_PCS_CFG_ENA;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);*/
        }
        else
        { /* eagle, done with PEX */
            /*  Wrote register via map d2d_reg_block[0].RXC_RegFile.uvm_reg_map: d2d_reg_block[0].RXC_RegFile.autostart=0xf
              Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_CFG_ENA=0xd003d
              Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_8=0x100808
              Wrote register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_IFC_REMAP_9=0x100808*/
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));
            regData = 0xF;
            regMask = 0xF;
            regAddr = baseAddr /*+ unitIndex * unitNum */+ D2D_PHY_RXC_AUTOSTART;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);


            /* Write to: PCS Global Control */
            /*regData = 0x000d003d;
            regMask = 0xFFFFFFFF;
            regAddr = baseAddr + HWS_D2D_PCS_PCS_CFG_ENA;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);*/

            /*Write PCS_RX_IFC_REMAP_8,9=0x100808*/
            /*regData = 0x100808;
            regMask = 0xFFFFFFFF;
            regAddr = baseAddr + D2D_PCS_RX_IFC_REMAP_8;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);
            regAddr = baseAddr + D2D_PCS_RX_IFC_REMAP_9;
            HWS_REGISTER_WRITE_REG(driverPtr, devNum, regAddr, regData, regMask);*/

        }


    }

    return GT_OK;
}

/**
* @internal mvHwsGW16SerdesAutoStartInitStatusGet function
* @endinternal
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsGW16SerdesAutoStartInitStatusGet
(
    GT_U8 devNum,
    GT_U32 d2dNum
)
{
    GT_U32 regAddr;
    GT_U32 regData;
    GT_U32 regData1;
    GT_U32 regMask;
    GT_U32 cnt;
    GT_U32 d2dIndex;
    GT_U32 lane, baseAddr, unitIndex, unitNum;

    if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        for(d2dIndex = 0; d2dIndex < 2; d2dIndex++)
        {
            /* skip the links that go out of package */
            if((devNum == 5) && (d2dIndex == 0))
            {
                continue;
            }

            cnt = 0;
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RXC_AUTOSTART);

            /* Poll the 4 autostart_done bits */
            regMask = 0xF0;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
            while(regData != regMask)
            {
                hwsOsTimerWkFuncPtr(10);
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
                cnt++;
                if(cnt == 20)
                {
                    hwsOsPrintf("D2D[%d][%d] PHY init was failed, link_status=0x%x\n", devNum, d2dIndex, regData);
                    break;
                }
            }

            /**********************************
            * Check the start was successful
            ***********************************/

            /* Check the TX PLLs are locked */
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_TXC_PLL_CTRL);
            regMask = 0xFF;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

            regData |= 0x80;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
            regData &= ~0x80;
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_TXC_PLL_LOCK_STS);
            regMask = 0x1F;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
            if((regData & 0x3) == 0x1)
            {
                if(d2dPrintEnableFullConfiguration)
                    hwsOsPrintf("PASS: D2D[%d][%d] Check TX PLL locked (and sticky clear)\n", devNum, d2dIndex);
            }
            else
            {
                if(d2dPrintEnableFullConfiguration)
                    hwsOsPrintf("FAIL: D2D[%d][%d] Check TX PLL not locked (and sticky clear)\n", devNum, d2dIndex);
            }

            for(lane = 0; lane < 4; lane++)
            {
                /* Check the RX PLLs are locked */
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RX_PLL_LOCK_CTRL + lane * 0x800);
                regMask = 0x7F;
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

                regData |= 0x2;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
                regData &= ~0x2;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RX_PLL_LOCK_STS + lane * 0x800);
                regMask = 0xFF;
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

                if((regData & 0x9) == 0x1)
                {
                    if(d2dPrintEnableFullConfiguration)
                        hwsOsPrintf("PASS: D2D[%d][%d] Check RX PLL locked (and sticky clear)\n", devNum, d2dIndex);
                }
                else
                {
                    if(d2dPrintEnableFullConfiguration)
                        hwsOsPrintf("FAIL: D2D[%d][%d] Check RX PLL not locked (and sticky clear)\n", devNum, d2dIndex);
                }

                /* Check auto start status is correct */
                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RX_AUTOSTAT + lane * 0x800);
                regMask = 0x7F;
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

                regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RX_AUTOCAL + lane * 0x800);
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData1, regMask));

                if(regData == regData1)
                {
                    if(d2dPrintEnableFullConfiguration)
                        hwsOsPrintf("PASS: D2D[%d][%d] Check RX Auto status: CAL=0x%x stat=0x%x\n", devNum, d2dIndex, regData1, regData);
                }
                else
                {
                    if(d2dPrintEnableFullConfiguration)
                        hwsOsPrintf("FAIL: D2D[%d][%d] Check RX Auto status: CAL=0x%x stat=0x%x\n", devNum, d2dIndex, regData1, regData);
                }
            }

            /* Check the auto start completed without errors */
            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RXC_AUTOSTAT);
            regMask = 0x3;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
            if(regData == 0x3)
            {
                if(d2dPrintEnableFullConfiguration)
                    hwsOsPrintf("PASS: D2D[%d][%d] RXC Auto status good\n", devNum, d2dIndex);
            }
            else
            {
                if(d2dPrintEnableFullConfiguration)
                    hwsOsPrintf("FAIL: D2D[%d][%d] RXC Auto status bad, stat=0x%x\n", devNum, d2dIndex, regData);
            }


            regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(d2dIndex,D2D_PHY_RXC_AUTOSTART_ERR);
            regMask = 0xFF;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
            if(regData == 0xF)
            {
                if(d2dPrintEnableFullConfiguration)
                    hwsOsPrintf("PASS: D2D[%d][%d] Auto start good\n", devNum, d2dIndex);
            }
            else
            {
                if(d2dPrintEnableFullConfiguration)
                    hwsOsPrintf("FAIL: D2D[%d][%d] Auto start bad, stat=0x%x\n", devNum, d2dIndex, regData);
            }

        }
    }
    else
    { /*device is falcon */
        /*Eagle: (pull indication is for Raven & Eagle)
        pull for bit [18:17] = 2'b11
        Read  register via map d2d_reg_block[0].D2D_RegFile.uvm_reg_map: d2d_reg_block[0].D2D_RegFile.PCS_RX_STATUS*/
        d2dIndex = PRV_HWS_D2D_GET_D2D_IDX_MAC(d2dNum);
        if (PRV_HWS_D2D_IS_EAGLE_D2D_MAC(d2dNum))
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, D2D_EAGLE_UNIT, d2dIndex, &baseAddr, &unitIndex, &unitNum /* localUnitNumPtr */));
            cnt = 0;
            regMask = 0x60000;
            regAddr = baseAddr + D2D_PCS_PCS_RX_STATUS;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
#ifdef ASIC_SIMULATION
            regData = 0x60000;
#endif /*ASIC_SIMULATION*/
            while (regData != 0x60000) {
                if(cnt == 20)
                {
                    hwsOsPrintf("D2D[%d][%d] PHY init was failed, link_status=0x%x\n", devNum, d2dNum, regData);
                    return GT_FAIL;
                }
#ifndef MV_HWS_REDUCED_BUILD
                if(cpssDeviceRunCheck_onEmulator())
                {
                    hwsOsTimerWkFuncPtr(100);
                }
                else
#endif /*!MV_HWS_REDUCED_BUILD*/
                {
                    hwsOsTimerWkFuncPtr(10);
                }
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
                cnt++;
            }
        }
    }

    return GT_OK;
}







/**
* @internal mvHwsD2dPmaLaneTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
*/
GT_STATUS mvHwsD2dPmaLaneTestGenStatus
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
)
{
    GT_U32          regAddr;
    GT_U32          regValue;
    GT_U32          sublane;
    GT_U32          byteCount;

    counterAccumulateMode = counterAccumulateMode;
    txPattern = txPattern;

#if 0
    /* Write to: FIFO_FRZ_CTRL (freeze error counters) */
    regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_FIFO_FRZ_CTRL + 0x1000*serdesNum));
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 7, 0x7F));
#endif

    status->lockStatus = 1;
    status->txFramesCntr.l[0] = 0;
    status->errorsCntr = 0;

    /* Read  from: PAT_STS (Sticky pattern pass flag )  -  0x2 PASS else FAIL*/
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_STS + 0x1000*serdesNum + 0x80*sublane));
        CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 2, &regValue));

        if((regValue & 0x2) != 0x2)
        {
            status->lockStatus = 0;
        }

        if((regValue & 0x1) == 0x1)
        {
            /* Take error counter snapshot */
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 2, 1, 1));

            for(byteCount = 0; byteCount < 3; byteCount++)
            {
                regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_PERR_CNT_LANE + 0x1000*serdesNum + 0x80*sublane + byteCount*4));
                CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, 0, regAddr, 0, 8, &regValue));

                status->errorsCntr += regValue;
            }

            /* Take error counter snapshot - reset to default value */
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 2, 1, 0));

            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dPmaLaneTestGenInjectErrorEnable function
* @endinternal
*
* @brief   Activates the D2D PMA error injection.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesDirection          - SERDES direction
*/
GT_STATUS mvHwsD2dPmaLaneTestGenInjectErrorEnable
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,  /* D2D index */
    GT_UOPT                     serdesNum,  /* Lane index 0..3*/
    MV_HWS_SERDES_DIRECTION     serdesDirection
)
{
    GT_U32      regAddr;
    GT_U32      sublane;

    if((portGroup > 1) || (serdesNum > 3))
    {
        hwsOsPrintf("mvHwsD2dPmaLaneTestGenInjectErrorEnable: bad param for devNum=%d, portGroup=%d, serdesNum=%d\n", devNum, portGroup, serdesNum);
        return GT_BAD_PARAM;
    }

    for(sublane = 0; sublane < 5; sublane++)
    {
        if(serdesDirection == RX_DIRECTION)
        {
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
        }
        else
        {
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
        }
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, 1));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, 0));
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dPmaLaneTestGeneneratorConfig function
* @endinternal
*
* @brief   Activates the D2D PMA Lane test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - PMA lane index (APPLICABLE RANGES: 0..3)
*/
GT_STATUS mvHwsD2dPmaLaneTestGeneneratorConfig
(
    GT_U8                           devNum,
    GT_U32                          portGroup,  /* D2D index */
    GT_U32                          serdesNum,  /* Lane index 0..3*/
    MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 regData;         /* register data */
    GT_U32 regMask;         /* register mask */
    GT_U32 prbsRegData;     /* PRBS HW value */
    GT_U32 sublane;         /* sublane loop iterator */

    if((portGroup > 1) || (serdesNum > 3))
    {
        hwsOsPrintf("mvHwsD2dPmaLaneTestGeneneratorConfig: bad param for devNum=%d, portGroup=%d, serdesNum=%d\n", devNum, portGroup, serdesNum);
        return GT_BAD_PARAM;
    }

    if(configPtr->mode == SERDES_NORMAL)
    {
        regAddr = PRV_HWS_PMA_REG_ADDR_CALC_MAC(portGroup, D2D_PMA_CFG_EN);
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 2, 2, 0));

        /* Write to: PAT_CTRL_LANE (Clear errors =1) */
        for(sublane = 0; sublane < 5; sublane++)
        {
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));

            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));

            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 0));
        }

        return GT_OK;
    }


    switch(configPtr->serdesPattern)
    {
        case  PRBS9:
            prbsRegData = 0;
            break;
        case  PRBS15:
            prbsRegData = 1;
            break;
        case  PRBS23:
            prbsRegData = 2;
            break;
        case  PRBS31:
            prbsRegData = 3;
            break;
        case Other:
            prbsRegData = 4;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    /* The lane tester requires - write to PMA Operation Control � PMA_TX_SEL set to PMA lane generator */
    regAddr = PRV_HWS_PMA_REG_ADDR_CALC_MAC(portGroup, D2D_PMA_CFG_EN);
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 2, 2, 2));


    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_FIFO_LANE_CTRL + 0x1000*serdesNum));
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 3));

    hwsOsTimerWkFuncPtr(1);

    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_FIFO_LANE_CTRL + 0x1000*serdesNum));
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 0));

    /* RX+TX RS-FEC enable/disable configuration */
    regData = (configPtr->fecMode == RS_FEC) ? 1 : 0;
    regAddr = PRV_HWS_PMA_REG_ADDR_CALC_MAC(portGroup, D2D_PMA_CFG_EN);
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 5, 1, regData));

    /* Write to: PAT_CTRL_LANE (Clear errors =1) */
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));

        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));
    }

    /******************************************************/
    /* User pattern configuration                         */
    /******************************************************/
    if(configPtr->userDefPatternEnabled == GT_TRUE)
    {
        for(sublane = 0; sublane < 5; sublane++)
        {
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_PUSER_PAT_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[0]));
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_PUSER_PAT_LANE + 4 + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[1]));
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_PUSER_PAT_LANE + 8 + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[2]));
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_PUSER_PAT_LANE + 0xC + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[3]));
        }
    }

    /* Write to: PAT_PSEL_LANE [2:0] (select pattern) */
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_PSEL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 3, prbsRegData));
    }

    /* Write to: PAT_PSEL_LANE [4] (FEC) */
    regData = (configPtr->fecMode == RS_FEC) ? 1 : 0;
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_PSEL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, regData));
    }

    switch(configPtr->direction)
    {
        case HWS_D2D_DIRECTION_MODE_BOTH_E:
            regData = 0x3;
            regMask = 0x3;
            break;
        case HWS_D2D_DIRECTION_MODE_RX_E:
            regData = 0x2;
            regMask = 0x2;
            break;
        case HWS_D2D_DIRECTION_MODE_TX_E:
            regData = 0x1;
            regMask = 0x1;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* Write to: PAT_CTRL_LANE(Enable Pattern Gen/Checker) */
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_TX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_FIFO_LANE_CTRL + 0x1000*serdesNum));
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 0));

    return GT_OK;
}

/**
* @internal mvHwsD2dPmaLaneTestCheckerConfig function
* @endinternal
*
* @brief   Activates the D2D PMA Lane test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - PMA lane index (APPLICABLE RANGES: 0..3)
*/
GT_STATUS mvHwsD2dPmaLaneTestCheckerConfig
(
    GT_U8                           devNum,
    GT_U32                          portGroup,  /* D2D index */
    GT_U32                          serdesNum,  /* Lane index 0..3*/
    MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 regData;         /* register data */
    GT_U32 regMask;         /* register mask */
    GT_U32 prbsRegData;     /* PRBS HW value */
    GT_U32 sublane;         /* sublane loop iterator */

    if((portGroup > 1) || (serdesNum > 3))
    {
        hwsOsPrintf("mvHwsD2dPmaLaneTestCheckerConfig: bad param for devNum=%d, portGroup=%d, serdesNum=%d\n", devNum, portGroup, serdesNum);
        return GT_BAD_PARAM;
    }

    if(configPtr->mode == SERDES_NORMAL)
    {
        regAddr = PRV_HWS_PMA_REG_ADDR_CALC_MAC(portGroup, D2D_PMA_CFG_EN);
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 0));

        /* Write to: PAT_CTRL_LANE (Clear errors =1) */
        for(sublane = 0; sublane < 5; sublane++)
        {
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));

            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));

            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 1, 1, 0));
        }

        return GT_OK;
    }


    switch(configPtr->serdesPattern)
    {
        case  PRBS9:
            prbsRegData = 0;
            break;
        case  PRBS15:
            prbsRegData = 1;
            break;
        case  PRBS23:
            prbsRegData = 2;
            break;
        case  PRBS31:
            prbsRegData = 3;
            break;
        case Other:
            prbsRegData = 4;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    /* The lane tester requires - write to PMA Operation Control � PMA_TX_SEL set to PMA lane generator */
    regAddr = PRV_HWS_PMA_REG_ADDR_CALC_MAC(portGroup, D2D_PMA_CFG_EN);
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 1));


    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_FIFO_LANE_CTRL + 0x1000*serdesNum));
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 3));

    hwsOsTimerWkFuncPtr(1);

    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_FIFO_LANE_CTRL + 0x1000*serdesNum));
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 0));

    /* RX+TX RS-FEC enable/disable configuration */
    regData = (configPtr->fecMode == RS_FEC) ? 1 : 0;
    regAddr = PRV_HWS_PMA_REG_ADDR_CALC_MAC(portGroup, D2D_PMA_CFG_EN);
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, regData));

    /* Write to: PAT_CTRL_LANE (Clear errors =1) */
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 1));

        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 3, 1, 0));
    }

    /******************************************************/
    /* User pattern configuration                         */
    /******************************************************/
    if(configPtr->userDefPatternEnabled == GT_TRUE)
    {
        for(sublane = 0; sublane < 5; sublane++)
        {
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_PUSER_PAT_LANE + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[0]));
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_PUSER_PAT_LANE + 4 + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[1]));
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_PUSER_PAT_LANE + 8 + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[2]));
            regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_PUSER_PAT_LANE + 0xC + 0x80*sublane + 0x1000*serdesNum));
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 8, configPtr->userDefPattern[3]));
        }
    }

    /* Write to: PAT_PSEL_LANE [2:0] (select pattern) */
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_PSEL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 3, prbsRegData));
    }

    /* Write to: PAT_PSEL_LANE [4] (FEC) */
    regData = (configPtr->fecMode == RS_FEC) ? 1 : 0;
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_PSEL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 4, 1, regData));
    }

    switch(configPtr->direction)
    {
        case HWS_D2D_DIRECTION_MODE_BOTH_E:
            regData = 0x3;
            regMask = 0x3;
            break;
        case HWS_D2D_DIRECTION_MODE_RX_E:
            regData = 0x2;
            regMask = 0x2;
            break;
        case HWS_D2D_DIRECTION_MODE_TX_E:
            regData = 0x1;
            regMask = 0x1;
            break;
        default:
            return GT_BAD_PARAM;
    }
    /* Write to: PAT_CTRL_LANE(Enable Pattern Gen/Checker) */
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_PAT_RX_CTRL_LANE + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    /* Write to KDU - reset tx sub-lane pattern checkers simultaneously */
    regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_FIFO_LANE_CTRL + 0x1000*serdesNum));
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 2, 0));


    if((configPtr->serdesPattern == Other/* user defined pattern */) ||
       (configPtr->fecMode == RS_FEC))
    {
        regData = 3;
    }
    else
    {
        regData = 0;
    }
    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, ((HWS_D2D_KDU_PAT_RX_PUSER_MASK_LANE + 0xC/* byte num 3 */) + 0x80*sublane + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 6, 2, regData));
    }

    return GT_OK;
}

/**
* @internal mvHwsD2dPmaLaneTestLoopbackSet function
* @endinternal
*
* @brief   Activates loopback between lane checker and lane generator.
*          To enable loopback XBAR configuration should be done + FIFO reset/unreset
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - PMA lane index (APPLICABLE RANGES: 0..3)
* @param[in] lbType                   - loopback type
*/
GT_STATUS mvHwsD2dPmaLaneTestLoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,  /* D2D index */
    GT_U32                  serdesNum,  /* Lane index 0..3*/
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    GT_U32 regAddr;     /* register address */
    GT_U32 regValue;    /* register value */
    GT_U32 regMask;    /* register value */
    GT_U32 sublane;     /* sublane loop iterator */

    regMask = 0xC;
    switch(lbType)
    {
        case RX_2_TX_LB:
            regValue = 4;
            break;
        case DISABLE_LB:
            regValue = 0;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    /* PMA configuration to enable loopback */
    regAddr = PRV_HWS_PMA_REG_ADDR_CALC_MAC(portGroup, D2D_PMA_CFG_EN);
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regValue, regMask));

    if(lbType == RX_2_TX_LB)
    {
        /* FIFO reset/unreset configuration */
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_FIFO_CTRL + 0x1000*serdesNum));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 1));
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 1, 0));
    }

    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_KDU_REG_ADDR_CALC_MAC(portGroup, (HWS_D2D_KDU_FIFO_XBARCFG_LANE + 0x80*sublane + 0x1000*serdesNum));
        switch(lbType)
        {
            case RX_2_TX_LB:
                regValue = sublane+1;
                break;
            case DISABLE_LB:
                regValue = 0;
                break;
            default:
                return GT_NOT_SUPPORTED;
        }
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 0, 3, regValue));
    }

    return GT_OK;
}





static MV_HWS_D2D_DIRECTION_MODE_ENT hwsGw16TileTestGenMode[HWS_MAX_DEVICE_NUM] = {HWS_D2D_DIRECTION_MODE_BOTH_E};

GT_STATUS mvHwsGw16TileTestGenModeSet
(
    GT_U8                               devNum,
    MV_HWS_D2D_DIRECTION_MODE_ENT  mode
)
{
    hwsGw16TileTestGenMode[devNum] = mode;

    return GT_OK;
}

/**
* @internal mvHwsGw16TileTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[in] txPattern                - pattern to transmit
* @param[in] mode                     - test mode or normal
*/
GT_STATUS mvHwsGw16TileTestGen
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,  /* D2D index */
    GT_UOPT                     serdesNum,  /* Channel index */
    MV_HWS_SERDES_TX_PATTERN    txPattern,
    MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_U32      regMask;

    if((portGroup > 1) || (serdesNum > 3))
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad param for devNum=%d, portGroup=%d, serdesNum=%d\n", devNum, portGroup, serdesNum);
        return GT_BAD_PARAM;
    }

    if(hwsRegisterSetFuncPtr == NULL)
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad pointer hwsRegisterSetFuncPtr for devNum=%d\n", devNum);
        return GT_BAD_PTR;
    }

    switch(txPattern)
    {
        case  PRBS31:
            regData = (mode == SERDES_TEST) ? 0x3F : 0;
            regMask = 0x7F;
            break;

        case  PRBS15:
            regData = (mode == SERDES_TEST) ? 0x1F : 0;
            regMask = 0x7F;
            break;

        default:
            return GT_NOT_SUPPORTED;
    }

    /* RX debug enigines should be enabled */
    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_ANALYSIS + serdesNum * 0x400));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 1, 1));

    if((hwsGw16TileTestGenMode[devNum] == HWS_D2D_DIRECTION_MODE_BOTH_E) ||
       (hwsGw16TileTestGenMode[devNum] == HWS_D2D_DIRECTION_MODE_TX_E))
    {
        /* 1. Write to Tx_pattern_address  data 0x1F */
        regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_TX_PATSEL1 + serdesNum * 0x400));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    if((hwsGw16TileTestGenMode[devNum] == HWS_D2D_DIRECTION_MODE_BOTH_E) ||
       (hwsGw16TileTestGenMode[devNum] == HWS_D2D_DIRECTION_MODE_RX_E))
    {
        /* 2. Write to Rx_pattern_address  data 0x1F */
        regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATPRBS + serdesNum * 0x800));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));
    }

    /* 3. Enable/Disable PRBS */
    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATRST + serdesNum * 0x800));
    regData = (mode == SERDES_TEST) ? 0x1F : 0;
    regMask = 0x1F;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));

    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATRST + serdesNum * 0x800));
    regData = 0;
    regMask = 0x1F;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, regMask));


    return GT_OK;
}

/**
* @internal mvHwsGw16TileTestGenGet function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[out] txPattern               - (pointer to) pattern to transmit
* @param[out] mode                    - (pointer to) test mode or normal
*/
GT_STATUS mvHwsGw16TileTestGenGet
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    MV_HWS_SERDES_TX_PATTERN    *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_U32      regMask;

    if((portGroup > 1) || (serdesNum > 3))
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad param for devNum=%d, portGroup=%d, serdesNum=%d\n", devNum, portGroup, serdesNum);
        return GT_BAD_PARAM;
    }

    if(hwsRegisterGetFuncPtr == NULL)
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad pointer hwsRegisterGetFuncPtr for devNum=%d\n", devNum);
        return GT_BAD_PTR;
    }

    *txPatternPtr = PRBS31;

    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATRST + serdesNum * 0x800));
    regMask = 0x1F;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));

    *modePtr = (regData == 0x1F) ? SERDES_TEST : SERDES_NORMAL;

    return GT_OK;
}

/**
* @internal mvHwsGw16TileTestGenStatus function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*
* @param[in] devNum                   - The Pp's device number
* @param[in] portGroup                - D2D index (APPLICABLE RANGES: 0..1)
* @param[in] serdesNum                - GW16 channel index (APPLICABLE RANGES: 0..3)
* @param[in] txPattern                - pattern to transmit
*/
GT_STATUS mvHwsGw16TileTestGenStatus
(
    GT_U8                     devNum,
    GT_U32                    portGroup,
    GT_U32                    serdesNum,
    MV_HWS_SERDES_TX_PATTERN  txPattern,
    GT_BOOL                   counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS *status
)
{
    GT_U32      regAddr;
    GT_U32      regData;
    GT_U32      regData1;
    GT_U32      tmpData;
    GT_U32      regMask;
    GT_U32      sublane;

    counterAccumulateMode = counterAccumulateMode;
    txPattern = txPattern;

    if((portGroup > 1) || (serdesNum > 3))
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad param for devNum=%d, portGroup=%d, serdesNum=%d\n", devNum, portGroup, serdesNum);
        return GT_BAD_PARAM;
    }

    if(hwsRegisterGetFuncPtr == NULL)
    {
        hwsOsPrintf("mvHwsGw16TestGen: bad pointer hwsRegisterGetFuncPtr for devNum=%d\n", devNum);
        return GT_BAD_PTR;
    }

    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATSTAT + serdesNum * 0x800));
    regMask = 0x1F;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData1, regMask));

    status->errorsCntr = 0;
    status->txFramesCntr.l[0] = 0;
    status->lockStatus = 1;

    /* take snapshot */
    regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PATSNAP + serdesNum * 0x800));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 1, 1));
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0, 1));

    for(sublane = 0; sublane < 5; sublane++)
    {
        regAddr = PRV_HWS_PHY_REG_ADDR_CALC_MAC(portGroup,(D2D_PHY_RX_PAT_ERR_COUNT_BASE + sublane * 0x10 + serdesNum * 0x800));
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, regMask));
        tmpData = regData;
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr + 4, &regData, regMask));
        tmpData = tmpData | (regData << 8);
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr + 8, &regData, regMask));
        tmpData = tmpData | (regData << 16);
        status->errorsCntr = tmpData;
    }

    if(regData1 != 0)
    {
        status->lockStatus = 0;

    }

    return GT_OK;
}


/**
* @internal mvHwsGw16IfClose function
* @endinternal
*
* @brief   Release all system resources allocated by Serdes IF functions.
*
* @param[in] devNum                   - The Pp's device number
*/
void mvHwsGw16IfClose
(
    GT_U8      devNum
)
{
    hwsInitDone[devNum] = GT_FALSE;
}

/**
* @internal mvHwsD2dPhyIfInit function
* @endinternal
*
* @brief   Init GW16 Serdes IF functions.
*
* @param[in] devNum                   - The Pp's device number
*/
GT_STATUS mvHwsD2dPhyIfInit
(
    GT_U8  devNum,
    MV_HWS_SERDES_FUNC_PTRS *funcPtrArray
)
{
    hwsInitDone[devNum] = GT_TRUE;

    /* DB init */
    funcPtrArray[D2D_PHY_GW16].serdesArrayPowerCntrlFunc    = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesPowerCntrlFunc         = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesManualTxCfgFunc        = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesManualRxCfgFunc        = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesAutoTuneCfgFunc        = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesRxAutoTuneStartFunc    = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesTxAutoTuneStartFunc    = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesAutoTuneStartFunc      = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesAutoTuneResultFunc     = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesAutoTuneStatusFunc     = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesAutoTuneStatusShortFunc =NULL;
    funcPtrArray[D2D_PHY_GW16].serdesResetFunc              = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesDigitalReset           = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesCoreReset              = NULL;
#ifndef CO_CPU_RUN
    funcPtrArray[D2D_PHY_GW16].serdesSeqGetFunc             = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesSeqSetFunc             = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesDfeCfgFunc             = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesLoopbackCfgFunc        = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesLoopbackGetFunc        = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesPpmCfgFunc             = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesPpmGetFunc             = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesTestGenFunc            = mvHwsGw16TileTestGen;
    funcPtrArray[D2D_PHY_GW16].serdesTestGenGetFunc         = mvHwsGw16TileTestGenGet;
    funcPtrArray[D2D_PHY_GW16].serdesTestGenStatusFunc      = mvHwsGw16TileTestGenStatus;
    funcPtrArray[D2D_PHY_GW16].serdesEomGetFunc             = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesDfeStatusFunc          = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesDfeStatusExtFunc       = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesPolarityCfgFunc        = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesPolarityCfgGetFunc     = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesDfeOptiFunc            = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesFfeCfgFunc             = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesCalibrationStatusFunc  = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesTxEnableFunc           = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesTxEnableGetFunc        = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesTxIfSelectFunc         = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesTxIfSelectGetFunc      = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesSqlchCfgFunc           = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesAcTermCfgFunc          = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesAutoTuneStopFunc       = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesDfeCheckFunc           = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesSpeedGetFunc           = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesManualRxCfgGetFunc     = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesManualTxCfgGetFunc     = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesSignalDetectGetFunc    = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesCdrLockStatusGetFunc   = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesScanSamplerFunc        = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesFixAlign90Start        = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesFixAlign90Stop         = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesFixAlign90Status       = NULL;
    funcPtrArray[D2D_PHY_GW16].serdesTypeGetFunc            = mvHwsGw16SerdesTypeGetFunc;
#endif /* CO_CPU_RUN */

    return GT_OK;
}

