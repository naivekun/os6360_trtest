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
* mvHwsGeneralCpllInit.c
*
* DESCRIPTION:
*       BobK CPLL init
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
/************************* definition *****************************************************/
#ifdef CHX_FAMILY
extern  GT_U32 hwsFalconNumOfRavens;
#endif
/************************* Globals *******************************************************/
GT_U32 cpllDataArray25to156[MAX_CPLL_DATA_INDEX]=
{
    0x7010005,  0x0,        0x7080000,  0x0,        0x7080000,  0x0,        0x711080A,
    0x0,        0x711080A,  0x0,        0x70E1B18,  0x0,        0x726E641,  0x0,
    0x7200450,  0x0,        0x71f3800,  0x0,        0x719448D,  0x0,        0x7275600,
    0x0,        0x7092150,  0x0,        0x70D4400,  0x0,        0x70A8556,  0x0,
    0x70B2FC9,  0x0,        0x7092170,  0x0,        0x7092150,  0x0,        0x7080010,
    0x0
};
/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */

GT_U32 cpllDataArray25to78[MAX_CPLL_DATA_INDEX] =
{
    0x7010005,  0x0,        0x7080000,  0x0,        0x7080000,  0x0,        0x711080A,
    0x0,        0x711080A,  0x0,        0x70E1B18,  0x0,        0x726E641,  0x0,
    0x7200450,  0x0,        0x71f3800,  0x0,        0x719448D,  0x0,        0x7275600,
    0x0,        0x7092150,  0x0,        0x70D4800,  0x0,        0x70A8556,  0x0,
    0x70B2FC9,  0x0,        0x7092170,  0x0,        0x7092150,  0x0,        0x7080010,
    0x0
};
/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */

GT_U32 cpllDataArray25to200[MAX_CPLL_DATA_INDEX] =
{
    0x7010005,  0x0,        0x7080000,  0x0,        0x7080000,  0x0,        0x711080A,
    0x0,        0x711080A,  0x0,        0x70E1B18,  0x0,        0x726E641,  0x0,
    0x7200450,  0x0,        0x71f3800,  0x0,        0x719448D,  0x0,        0x7275600,
    0x0,        0x7092210,  0x0,        0x70DA400,  0x0,        0x70A5800,  0x0,
    0x70B0000,  0x0,        0x7092230,  0x0,        0x7092210,  0x0,        0x7080010,
    0x0
};
/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */

GT_U32 cpllDataArray156to156[MAX_CPLL_DATA_INDEX] =
{
    0x7010005,  0x0,        0x7080000,  0x0,        0x7080000,  0x0,        0x711080A,
    0x0,        0x711080A,  0x0,        0x70E1B18,  0x0,        0x726E641,  0x0,
    0x7200420,  0x0,        0x71f3800,  0x0,        0x719448D,  0x0,        0x7275600,
    0x0,        0x709C010,  0x0,        0x70D4400,  0x0,        0x70A8000,  0x0,
    0x70BD1B7,  0x0,        0x709C030,  0x0,        0x709C010,  0x0,        0x7080010,
    0x0
};
/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */

GT_U32 cpllDataArray156to78[MAX_CPLL_DATA_INDEX] =
{
    0x7010005,  0x0,        0x7080000,  0x0,        0x7080000,  0x0,        0x711080A,
    0x0,        0x711080A,  0x0,        0x70E1B18,  0x0,        0x726E641,  0x0,
    0x7200420,  0x0,        0x71f3800,  0x0,        0x719448D,  0x0,        0x7275600,
    0x0,        0x709C010,  0x0,        0x70D4800,  0x0,        0x70A8000,  0x0,
    0x70BD1B7,  0x0,        0x709C030,  0x0,        0x709C010,  0x0,        0x7080010,
    0x0
};
/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */

GT_U32 cpllDataArray156to200[MAX_CPLL_DATA_INDEX] ={0};

GT_U32 *cpllDataArray [MV_HWS_MAX_INPUT_FREQUENCY][MV_HWS_MAX_OUTPUT_FREQUENCY]=
{
    {cpllDataArray25to156,  cpllDataArray25to78,  cpllDataArray25to200, NULL, NULL, NULL},
    {cpllDataArray156to156, cpllDataArray156to78, cpllDataArray156to200, NULL, NULL, NULL}
};

/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */
GT_U32 bc3CpllDataArray25to156[MAX_CPLL_DATA_INDEX]=
{
    0xF010005,  0x0C000000,
    0xF080000,  0x0C000000,
    0xF080000,  0x0C000000,
    0xF11080A,  0x0C000000,
    0xF11080A,  0x0C000000,
    0xF0E1B18,  0x0C000000,
    0xF26E641,  0x0C000000,
    0xF200450,  0x0C000000,
    0xF1f3800,  0x0C000000,
    0xF19448D,  0x0C000000,
    0xF275600,  0x0C000000,
    0xF0D4400,  0x0C000000,
    0xF092150,  0x0C000000,
    0xF0A8555,  0x0C000000,
    0xF0B5555,  0x0C000000,
    0xF092170,  0x0C000000,
    0xF092150,  0x0C000000,
    0xF080010,  0x0C000000
};

/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */
GT_U32 bc3CpllDataArray25to312[MAX_CPLL_DATA_INDEX]=
{
    0xF010005,  0x0C000000,
    0xF080000,  0x0C000000,
    0xF080000,  0x0C000000,
    0xF11080A,  0x0C000000,
    0xF11080A,  0x0C000000,
    0xF0E1B18,  0x0C000000,
    0xF26E641,  0x0C000000,
    0xF200450,  0x0C000000,
    0xF1f3800,  0x0C000000,
    0xF19448D,  0x0C000000,
    0xF275600,  0x0C000000,
    0xF0D4000,  0x0C000000,
    0xF092150,  0x0C000000,
    0xF0A8556,  0x0C000000, /* +25 PPM */
    0xF0B2FC9,  0x0C000000, /* +25 PPM */
    0xF092170,  0x0C000000,
    0xF092150,  0x0C000000,
    0xF080010,  0x0C000000
};

/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */
GT_U32 bc3CpllDataArray25to164[MAX_CPLL_DATA_INDEX]=
{
    0xF010005, 0x0C000000,
    0xF080000, 0x0C000000,
    0xF080000, 0x0C000000,
    0xF11080A, 0x0C000000,
    0xF11080A, 0x0C000000,
    0xF0E1B18, 0x0C000000,
    0xF26E641, 0x0C000000,
    0xF200450, 0x0C000000,
    0xF1f3800, 0x0C000000,
    0xF19448D, 0x0C000000,
    0xF275600, 0x0C000000,
    0xF0D4400, 0x0C000000,
    0xF092150, 0x0C000000,
    0xF0A8C28, 0x0C000000,  /* +25 PPM */
    0xF0B325D, 0x0C000000,  /* +25 PPM */
    0xF092170, 0x0C000000,
    0xF092150, 0x0C000000,
    0xF080010, 0x0C000000
};

GT_U32 *bc3CpllDataArray [MV_HWS_MAX_INPUT_FREQUENCY][MV_HWS_MAX_OUTPUT_FREQUENCY]=
{
    {bc3CpllDataArray25to156,   NULL,   NULL,   bc3CpllDataArray25to312,    NULL,   bc3CpllDataArray25to164},
    {NULL,                      NULL,   NULL,   NULL,                       NULL,   NULL                   }
};

/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */
GT_U32 pipeCpllDataArray25to156[MAX_CPLL_DATA_INDEX] =
{
    0x40430005,  0x00030000,
    0x42030000,  0x00030000,
    0x42030000,  0x00030000,
    0x4443080A,  0x00030000,
    0x4443080A,  0x00030000,
    0x43831B18,  0x00030000,
    0x4983E641,  0x00030000,
    0x48030450,  0x00030000,
    0x47C33800,  0x00030000,
    0x4643448D,  0x00030000,
    0x49C35600,  0x00030000,
    0x43434400,  0x00030000,
    0x42432150,  0x00030000,
    0x42838555,  0x00030000,
    0x42C35555,  0x00030000,
    0x42432170,  0x00030000,
    0x42432150,  0x00030000,
    0x42030010,  0x00030000
};
/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */
GT_U32 pipeCpllDataArray25to312[MAX_CPLL_DATA_INDEX] =
{
    0x40430005,  0x00030000,
    0x42030000,  0x00030000,
    0x42030000,  0x00030000,
    0x4443080A,  0x00030000,
    0x4443080A,  0x00030000,
    0x43831B18,  0x00030000,
    0x4983E641,  0x00030000,
    0x48030450,  0x00030000,
    0x47C33800,  0x00030000,
    0x4643448D,  0x00030000,
    0x49C35600,  0x00030000,
    0x43434000,  0x00030000,
    0x42432150,  0x00030000,
    0x42838556,  0x00030000, /* +25 PPM */
    0x42C32FC9,  0x00030000, /* +25 PPM */
    0x42432170,  0x00030000,
    0x42432150,  0x00030000,
    0x42030010,  0x00030000
};

/* if power up sequence is updated - please update mvHwsAvagoSerdeCpllOutputRefClkGet() */
GT_U32 pipeCpllDataArray25to164[MAX_CPLL_DATA_INDEX] =
{
    0x40430005,	0x00030000,
    0x42030000,	0x00030000,
    0x42030000,	0x00030000,
    0x4443080A,	0x00030000,
    0x4443080A,	0x00030000,
    0x43831B18,	0x00030000,
    0x4983E641,	0x00030000,
    0x48030450,	0x00030000,
    0x47C33800,	0x00030000,
    0x4643448D,	0x00030000,
    0x49C35600,	0x00030000,
    0x43434400,	0x00030000,
    0x42432150,	0x00030000,
    0x42838C28,	0x00030000, /* +25 PPM */
    0x42C3325D,	0x00030000, /* +25 PPM */
    0x42432170,	0x00030000,
    0x42432150,	0x00030000,
    0x42030010,	0x00030000
};

GT_U32 *pipeCpllDataArray [MV_HWS_MAX_INPUT_FREQUENCY][MV_HWS_MAX_OUTPUT_FREQUENCY]=
{
    {pipeCpllDataArray25to156,  NULL,   NULL,   pipeCpllDataArray25to312,   NULL,   pipeCpllDataArray25to164},
    {NULL,                      NULL,   NULL,   NULL,                       NULL,   NULL                    }
};

GT_U32 ravenCpllDataArray156to156[MAX_RAVEN_CPLL_DATA_INDEX] =
{
  0x05010000,
  0x04000000,
  0x05010004,
  0x04000000,
    /* CPLL register misconfiguration fix */
  0x05124802,
  0x04000000,
    /* end */
  0x05020092,
  0x04000000,
  0x05021892,
  0x04000000,
  0x0509C01F,
  0x04000000,
  0x050C0000,
  0x04000000,
  0x050B0000,
  0x04000000,
  0x050A0C1F,
  0x04000000,
  0x05080010,
  0x04000000
};

GT_U32 *ravenCpllDataArray [MV_HWS_MAX_INPUT_FREQUENCY][MV_HWS_MAX_OUTPUT_FREQUENCY]=
{
    {NULL,   NULL,   NULL,   NULL,    NULL,   NULL},
    {ravenCpllDataArray156to156,   NULL,   NULL,   NULL,    NULL,   NULL}
};


/************************* pre-declaration ***********************************************/


/*****************************************************************************************/
/**
* @internal mvHwsGeneralCpllControl function
* @endinternal
*
* @brief   Configure the requested CPLL (0, 1) or bypass on specific CPLL
*
* @param[in] devNum                   - system device number
* @param[in] cpllNum                  - cpll number
*                                      bypassEn - bypass enable/disable
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralCpllControl
(
    CPSS_HW_DRIVER_STC              *driverPtr,
    GT_U8                           devNum,
    MV_HWS_CPLL_NUMBER              cpllNum,
    GT_BOOL                         bypass_en,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
)
{
    GT_U32  i, bypassMask, dataMask;
    GT_U32  *data_ptr;
    GT_U32  data;
    GT_BOOL *initDonePtr;
    MV_HWS_CPLL_OUTPUT_FREQUENCY    *currOutFreq;
    GT_U32  regAddr, dataAddr;
    GT_U32  cpssArraySize = MAX_CPLL_DATA_INDEX;

    if (hwsDeviceSpecInfo[devNum].devType == Bobcat3 || hwsDeviceSpecInfo[devNum].devType == Aldrin2)
    {
        switch (cpllNum)
        {
            case CPLL0: /* CPLL0_0 + CPLL0_1*/
                regAddr = DEVICE_GENERAL_CONTROL_17;
                dataAddr = DEVICE_GENERAL_CONTROL_17;
                break;
            case CPLL1: /* CPLL1_0 + CPLL1_1 */
                regAddr = DEVICE_GENERAL_CONTROL_18;
                dataAddr = DEVICE_GENERAL_CONTROL_18;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        bypassMask = 0x4000000;
        dataMask = 0xFFFFFFFF;
        data_ptr = bc3CpllDataArray[inFreq][outFreq];
    }
    else if (hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        switch (cpllNum)
        {
            case CPLL0:
                regAddr = DEVICE_GENERAL_CONTROL_1;
                dataAddr = DEVICE_GENERAL_CONTROL_1;
                break;
            case CPLL1:
                regAddr = DEVICE_GENERAL_CONTROL_2;
                dataAddr = DEVICE_GENERAL_CONTROL_2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        cpssArraySize = MAX_RAVEN_CPLL_DATA_INDEX;
        bypassMask = 0x2000000;
        dataMask = 0xFFFFFFFF;
        data_ptr = ravenCpllDataArray[inFreq][outFreq];
    }
    else if (hwsDeviceSpecInfo[devNum].devType == BobK)
    {
        switch (cpllNum)
        {
            case CPLL0: /* clum_gop_cpll1_bypass */
                dataMask  = 0xFCFFFFFF;
                bypassMask = 0x400;
                break;
            case CPLL1: /* clum_gop_cpll0_bypass */
                dataMask  = 0xFAFFFFFF;
                bypassMask = 0x200;
                break;
            case CPLL2: /* ctus_gop_cpll_bypass */
                dataMask  = 0xF9FFFFFF;
                bypassMask = 0x100;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = DEVICE_GENERAL_CONTROL_20;
        dataAddr = DEVICE_GENERAL_CONTROL_21;
        data_ptr = cpllDataArray[inFreq][outFreq];
    }
    else if (hwsDeviceSpecInfo[devNum].devType == Aldrin)
    {
        if (cpllNum > 0) {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = DEVICE_GENERAL_CONTROL_12;
        dataAddr = DEVICE_GENERAL_CONTROL_13;
        bypassMask  = 0x80000000;
        data_ptr = cpllDataArray[inFreq][outFreq];
        dataMask = 0x01FFFFFF;
    }
    else /*Pipe */
    {
        if (cpllNum > 0) {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = DEVICE_GENERAL_CONTROL_16;
        dataAddr = DEVICE_GENERAL_CONTROL_16;
        bypassMask  = 0x10000;
        data_ptr = pipeCpllDataArray[inFreq][outFreq];
        dataMask = 0xFFC3FFFF;
    }

    initDonePtr = &(hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[cpllNum]);
    currOutFreq = &(hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[cpllNum]);

    if (bypass_en)
    {
        HWS_SERVER_READ_REG(driverPtr, devNum, regAddr, &data)

        if (hwsDeviceSpecInfo[devNum].devType == Raven)
        {
            HWS_SERVER_WRITE_REG(driverPtr,devNum,regAddr,data| bypassMask);
        }
        else
        {
            HWS_SERVER_WRITE_REG(driverPtr,devNum,regAddr,data & ~bypassMask);
        }
    }
    else
    {
        /* check if CPLL init already done */
        if(*initDonePtr == GT_TRUE)
        {
            if(*currOutFreq == outFreq)
            {
                return GT_OK;
            }
        }

        HWS_SERVER_READ_REG(driverPtr, devNum, regAddr, &data)

        if (hwsDeviceSpecInfo[devNum].devType == Raven)
        {
            HWS_SERVER_WRITE_REG(driverPtr,devNum,regAddr,data & ~bypassMask);
        }
        else
        {
            HWS_SERVER_WRITE_REG(driverPtr,devNum,regAddr,data | bypassMask);
        }

        if(data_ptr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
        }

        for (i=0; i < cpssArraySize; i++)
        {
            HWS_SERVER_WRITE_REG(driverPtr,devNum,dataAddr,(*(data_ptr+i) & dataMask));

            /* set 20ms delay */
            hwsOsExactDelayPtr(devNum, devNum, 20);
        }
        *initDonePtr = GT_TRUE;
        *currOutFreq = outFreq;
    }

    return GT_OK;
}

/**
* @internal mvHwsFalconCpllControl function
* @endinternal
*
* @brief   Configure the requested CPLL (0, 1) or bypass on specific CPLL
*
* @param[in] devNum                   - system device number
* @param[in] cpllNum                  - cpll number
*                                      bypassEn - bypass enable/disable
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#ifdef CHX_FAMILY
GT_STATUS mvHwsFalconCpllControl
(
    CPSS_HW_DRIVER_STC              *driverPtr,
    GT_U8                           devNum,
    MV_HWS_CPLL_NUMBER              cpllNum,
    GT_BOOL                         bypass_en,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
)
{
    GT_U32  i, bypassMask, dataMask;
    GT_U32  *data_ptr;
    GT_U32  data;
    GT_BOOL *initDonePtr;
    MV_HWS_CPLL_OUTPUT_FREQUENCY    *currOutFreq;
    GT_U32  regAddr, dataAddr;
    GT_U32  cpssArraySize = MAX_CPLL_DATA_INDEX;
    GT_U32  baseAddr = 0, unitIndex, unitNum;
    GT_U32  numOfLoops, loop;

    switch (cpllNum)
    {
        case CPLL0:     /*serdes */
            regAddr = DEVICE_GENERAL_CONTROL_1;
            dataAddr = DEVICE_GENERAL_CONTROL_1;
            break;
        case CPLL1:     /*d2d*/
            regAddr = DEVICE_GENERAL_CONTROL_2;
            dataAddr = DEVICE_GENERAL_CONTROL_2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr += RAVEN_DFX_BASE_ADDR;
    dataAddr += RAVEN_DFX_BASE_ADDR;
    cpssArraySize = MAX_RAVEN_CPLL_DATA_INDEX;
    bypassMask = 0x2000000;
    dataMask = 0xFFFFFFFF;
    data_ptr = ravenCpllDataArray[inFreq][outFreq];

    initDonePtr = &hwsDeviceSpecInfo[devNum].serdesInfo.cpllInitDoneStatusArr[cpllNum];
    currOutFreq = &hwsDeviceSpecInfo[devNum].serdesInfo.cpllCurrentOutFreqArr[cpllNum];
    if (driverPtr != NULL)
    { /* called from smi for each raven (all with the same devNum*/
        *initDonePtr = GT_FALSE;
        numOfLoops = 1;

    }
    else
    {
        numOfLoops = hwsFalconNumOfRavens;
    }

    for (loop = 0; loop < numOfLoops; loop++)
    {
        if (driverPtr == NULL)
        {
            CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_UNIT_BASE, loop, &baseAddr, &unitIndex, &unitNum));
        }
        if (bypass_en)
        {
            HWS_SERVER_READ_REG(driverPtr, devNum, (regAddr + baseAddr), &data)
            HWS_SERVER_WRITE_REG(driverPtr, devNum,(regAddr + baseAddr) ,data| bypassMask);
        }
        else
        {
            /* check if CPLL init already done */
            if(*initDonePtr == GT_TRUE)
            {
                if(*currOutFreq == outFreq)
                {
                    return GT_OK;
                }
            }
            HWS_SERVER_READ_REG(driverPtr, devNum, (regAddr + baseAddr), &data)
            HWS_SERVER_WRITE_REG(driverPtr,devNum, (regAddr + baseAddr) ,data & ~bypassMask);

            if(data_ptr == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
            }

            for (i=0; i < cpssArraySize; i++)
            {
                HWS_SERVER_WRITE_REG(driverPtr, devNum, (dataAddr + baseAddr), (*(data_ptr+i) & dataMask));
                /* set 20ms delay */
                hwsOsExactDelayPtr(devNum, devNum, 20);
            }
        }
    }
    *initDonePtr = GT_TRUE;
    *currOutFreq = outFreq;

    return GT_OK;
}
#endif /*CHX_FAMILY*/



/**
* @internal mvHwsCpllControl function
* @endinternal
*
* @brief   Configure the requested CPLL (0, 1, 2) or bypass on specific CPLL
*
* @param[in] devNum                   - system device number
* @param[in] cpllNum                  - cpll number
*                                      bypassEn - bypass enable/disable
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsCpllControl
(
    CPSS_HW_DRIVER_STC              *driverPtr,
    GT_U8                           devNum,
    MV_HWS_CPLL_NUMBER              cpllNum,
    GT_BOOL                         bypass_en,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
)
{
    GT_STATUS rc = GT_OK;

    if(!cpssDeviceRunCheck_onEmulator())
    {
        if((hwsDeviceSpecInfo[devNum].devType == Bobcat3) || (hwsDeviceSpecInfo[devNum].devType == Pipe) ||
           (hwsDeviceSpecInfo[devNum].devType == Aldrin) || (hwsDeviceSpecInfo[devNum].devType == BobK) ||
           (hwsDeviceSpecInfo[devNum].devType == Raven) || (hwsDeviceSpecInfo[devNum].devType == Aldrin2))
        {
            rc = mvHwsGeneralCpllControl(driverPtr, devNum, cpllNum, bypass_en, inFreq, outFreq);
        }
#ifdef CHX_FAMILY
        else if (hwsDeviceSpecInfo[devNum].devType == Falcon)
        {
            rc = mvHwsFalconCpllControl(driverPtr, devNum, cpllNum, bypass_en, inFreq, outFreq);
        }
#endif
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
        }
    }
    return rc;
}
/**
* @internal mvHwsBobKSrcClockModeConfigSet function
* @endinternal
*
* @brief   Configure the setting of registers and requested CPLL(0, 1, 2) on SERDES
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - SERDES number
* @param[in] extConfigRegValue        - setting value for SERDES_EXTERNAL_CONFIGURATION_0
* @param[in] setGeneralCtlReg17       - set/clear bit #19 in DEVICE_GENERAL_CONTROL_17
*                                      GT_FALSE: clear bit #19
*                                      GT_TRUE: set bit #19
* @param[in] setGeneralCtlReg20       - set/clear bit #11 in DEVICE_GENERAL_CONTROL_20
*                                      GT_FALSE: clear bit #11
*                                      GT_TRUE: set bit #11
* @param[in] cpllNum                  - cpll number
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsBobKSrcClockModeConfigSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    GT_U32                          extConfigRegValue,
    GT_U32                          setGeneralCtlReg17,
    GT_U32                          setGeneralCtlReg20,
    MV_HWS_CPLL_NUMBER              cpllNum,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
)
{
    GT_U32  data;

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, extConfigRegValue, (1 << 8)));

    if ((setGeneralCtlReg17 == 1) || (setGeneralCtlReg17 == 0))
    {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, DEVICE_GENERAL_CONTROL_17, &data));
        data = (setGeneralCtlReg17) ? (data | (1 << 19)) : (data & ~(1 << 19));
        CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, DEVICE_GENERAL_CONTROL_17, data));
    }

    if ((setGeneralCtlReg20 == 1) || (setGeneralCtlReg20 == 0))
    {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, DEVICE_GENERAL_CONTROL_20, &data));
        data = (setGeneralCtlReg20) ? (data | (1 << 11)) : (data & ~(1 << 11));
        CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, DEVICE_GENERAL_CONTROL_20, data));
    }

    if ((cpllNum == CPLL0) || (cpllNum == CPLL1) || (cpllNum == CPLL2))
        CHECK_STATUS(mvHwsCpllControl(NULL, devNum, cpllNum, GT_FALSE, inFreq, outFreq));

    return GT_OK;
}

/**
* @internal mvHwsGeneralSrcClockModeConfigSet function
* @endinternal
*
* @brief   Configure the setting of registers and requested CPLL(0) on SERDES
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - SERDES number
* @param[in] extConfigRegValue        - setting value for SERDES_EXTERNAL_CONFIGURATION_0
* @param[in] setGeneralCtlReg         - set/clear  in DEVICE_GENERAL_CONTROL_XX
*                                      GT_FALSE: clear bit
*                                      GT_TRUE: set bit
* @param[in] cpllNum                  - cpll number
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsGeneralSrcClockModeConfigSet
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    GT_U32                          extConfigRegValue,
    GT_U32                          setGeneralCtlReg,
    MV_HWS_CPLL_NUMBER              cpllNum,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
)
{
    GT_U32  data, addr, bit = 0;

    if((hwsDeviceSpecInfo[devNum].devType == Raven) && (serdesNum == 17))
    {
        /* do nothing */
        /* there is no SERDES_EXTERNAL_CONFIGURATION_0 register in Raven for SERDES_17 => GW16 identifier */
    }
    else
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, extConfigRegValue, (1 << 8)));
    }

    if ((setGeneralCtlReg == 1) || (setGeneralCtlReg == 0))
    {
        addr = 0xFFFFFFFF;
        if(hwsDeviceSpecInfo[devNum].devType == Aldrin)
        {
            addr = DEVICE_GENERAL_CONTROL_11;
            bit = 31;
        }
        else if(hwsDeviceSpecInfo[devNum].devType == Bobcat3 || hwsDeviceSpecInfo[devNum].devType == Aldrin2)
        {
#ifdef CHX_FAMILY
            if(hwsFalconAsBobcat3Check())
            {
                return GT_OK;
            }
#endif  /* CHX_FAMILY*/
            addr = DEVICE_GENERAL_CONTROL_16;
            bit = 6;
        }
 /*TODO do we need for falcon*/
        else if(hwsDeviceSpecInfo[devNum].devType == Raven)
        {
            if(cpllNum == CPLL0)
            {
                addr = DEVICE_GENERAL_CONTROL_12;
                bit = 29;
            }
            else
            {   /* cpllNum == CPLL1 */
                addr = DEVICE_GENERAL_CONTROL_18;
                bit = 25;
            }
        }
        else if(hwsDeviceSpecInfo[devNum].devType == Pipe)
        {
            addr = DEVICE_GENERAL_CONTROL_20;
            bit = 13;
        }
        if (addr != 0xFFFFFFFF) {
            CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, addr, &data));
            data = (setGeneralCtlReg) ? (data | (1 << bit)) : (data & ~(1 << bit));
            CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, addr, data));
        }
    }

    return mvHwsCpllControl(NULL, devNum, cpllNum, GT_FALSE, inFreq, outFreq);
}

/**
* @internal mvHwsSerdesClockGlobalControl function
* @endinternal
*
* @brief   Global configuration per Serdes for referance clock control source.
*         Need to be done before any Serdes in the system is powered up.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - SERDES number
* @param[in] srcClock                 - Input clock source
* @param[in] inFreq                   - input frequency
* @param[in] outFreq                  - output frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesClockGlobalControl
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    MV_HWS_INPUT_SRC_CLOCK          srcClock,
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq
)
{
#ifdef CHX_FAMILY
    if(hwsDeviceSpecInfo[devNum].devType == Aldrin)
    {
        switch(srcClock)
        {
            case MV_HWS_CPLL:
                /* CPLL: set bit #8=1 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #31 in DEVICE_GENERAL_CONTROL_11 */
                CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,
                                                             (1 << 8), 0,
                                                              CPLL0,inFreq, outFreq));
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if(hwsDeviceSpecInfo[devNum].devType == Bobcat3)
    {
        switch(srcClock)
        {
            case MV_HWS_CPLL:

                /********************************************************************************
                    In Bobcat3 - '0' stands for CPLL & '1' stands for CMN(external ref) source.
                *********************************************************************************/
                if ((serdesNum < 36) || (serdesNum == 72))
                {
                    /* CPLL: set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #6 in DEVICE_GENERAL_CONTROL_16 */
                    CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,(0 << 8), 0, CPLL0, inFreq, outFreq));
                }
                else if ((serdesNum < 72) || (serdesNum == 73))
                {
                    /* CPLL: set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #6 in DEVICE_GENERAL_CONTROL_16 */
                    CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,(0 << 8), 0, CPLL1, inFreq, outFreq));
                }
                else
                {
                    if(hwsFalconAsBobcat3Check())
                    {
                        /* do not generate ERROR , no need to 'write' the register ... anyway the WM do nothing with it */
                        return GT_OK;
                    }
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if (hwsDeviceSpecInfo[devNum].devType == Aldrin2)
    {
        switch(srcClock)
        {
            case MV_HWS_CPLL:

                /********************************************************************************
                    In Aldrin2 - '0' stands for CPLL & '1' stands for CMN(external ref) source.
                *********************************************************************************/
                if ( serdesNum <= 23 )
                {
                    /* CPLL: set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #6 in DEVICE_GENERAL_CONTROL_16 */
                    CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,(0 << 8), 0, CPLL0, inFreq, outFreq));
                }
                else /* all others including CPU */
                {
                    /* CPLL: set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #6 in DEVICE_GENERAL_CONTROL_16 */
                    CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,(0 << 8), 0, CPLL1, inFreq, outFreq));
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if(hwsDeviceSpecInfo[devNum].devType == BobK)
    {
        if (serdesNum < 12)
        {
            switch(srcClock)
            {
                case MV_HWS_ANA_GRP1:
                    /* For CPLL1: set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0,
                       set bit #11 in DEVICE_GENERAL_CONTROL_20 */
                    CHECK_STATUS(mvHwsBobKSrcClockModeConfigSet(devNum, portGroup, serdesNum,
                                                            0, 0xFFFF, 1, CPLL1,
                                                            inFreq, outFreq));
                    break;
                case MV_HWS_ANA_GRP0:
                    /* 0xFFFF = No CPLL configuration */
                    /* set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0,
                       clear bit #11 in DEVICE_GENERAL_CONTROL_20 */
                    CHECK_STATUS(mvHwsBobKSrcClockModeConfigSet(devNum, portGroup, serdesNum,
                                                            0, 0xFFFF, 0, 0xFFFF,
                                                            inFreq, outFreq));
                    break;
                case MV_HWS_CPLL:
                    /* For CPLL0: set bit #8=1 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #19 in DEVICE_GENERAL_CONTROL_17 */
                    CHECK_STATUS(mvHwsBobKSrcClockModeConfigSet(devNum, portGroup, serdesNum,
                                                            (1 << 8), 0, 0xFFFF, CPLL0,
                                                            inFreq, outFreq));
                    break;
                case MV_HWS_RCVR_CLK_IN:
                    /* For CPLL0: set bit #8=1 in SERDES_EXTERNAL_CONFIGURATION_0, set bit #19 in DEVICE_GENERAL_CONTROL_17 */
                    CHECK_STATUS(mvHwsBobKSrcClockModeConfigSet(devNum, portGroup, serdesNum,
                                                            (1 << 8), 1, 0xFFFF, CPLL0,
                                                            inFreq, outFreq));
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            switch (srcClock)
            {
               case MV_HWS_ANA_GRP1:
                   CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, 0, (1 << 8)));
                   break;
               case MV_HWS_CPLL:
                   /* 0xFFFF = No configuration for DEVICE_GENERAL_CONTROL_20 */
                   /* For CPLL2: set bit #8=1 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #19 in DEVICE_GENERAL_CONTROL_17 */
                   CHECK_STATUS(mvHwsBobKSrcClockModeConfigSet(devNum, portGroup, serdesNum,
                                                           (1 << 8), 0, 0xFFFF, CPLL2,
                                                           inFreq, outFreq));
                   break;
               case MV_HWS_RCVR_CLK_IN:
                   /* 0xFFFF = No configuration for DEVICE_GENERAL_CONTROL_20 */
                   /* For CPLL2: set bit #8=1 in SERDES_EXTERNAL_CONFIGURATION_0, set bit #19 in DEVICE_GENERAL_CONTROL_17 */
                   CHECK_STATUS(mvHwsBobKSrcClockModeConfigSet(devNum, portGroup, serdesNum,
                                                           (1 << 8), 1, 0xFFFF, CPLL2,
                                                           inFreq, outFreq));
                   break;
               default:
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
           }
        }
    }
    else if(hwsDeviceSpecInfo[devNum].devType == Raven)
    {
        switch(srcClock)
        {
            case MV_HWS_CPLL:

                /********************************************************************************
                    In Raven - '0' stands for CPLL & '1' stands for CMN(external ref) source.
                *********************************************************************************/
                if (serdesNum < 17)
                {
                    /* CPLL_0: set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #6 in DEVICE_GENERAL_CONTROL_12 */
                    CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,(0 << 8), 1, CPLL0, inFreq, outFreq));
                }
                else if (serdesNum == 17)
                {
                    /* CPLL_1: set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #6 in DEVICE_GENERAL_CONTROL_18 */
                    CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,(0 << 8), 0, CPLL1, inFreq, outFreq));
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if(hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        switch(srcClock)
        {
            case MV_HWS_CPLL:

                /********************************************************************************
                    In Raven - '0' stands for CPLL & '1' stands for CMN(external ref) source.
                *********************************************************************************/
                /* CPLL_0 is connected to serdes */
                /* CPLL_1 is connected to the D2D */

                /* CPLL_0: set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #6 in DEVICE_GENERAL_CONTROL_12 */

                {
                    CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,(0 << 8), 1, CPLL0, inFreq, outFreq));
                }
                /*else called from hws d2d init
                {
                    CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,(0 << 8), 1, CPLL1, inFreq, outFreq));
                }*/
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    else
#endif /*#ifdef CHX_FAMILY*/
#ifdef PX_FAMILY
    if(hwsDeviceSpecInfo[devNum].devType == Pipe)
    {
        switch(srcClock)
        {
            case MV_HWS_CPLL:

                /********************************************************************************
                    In Pipe - '0' stands for CPLL & '1' stands for CMN(external ref) source.
                *********************************************************************************/
                /* CPLL: set bit #8=0 in SERDES_EXTERNAL_CONFIGURATION_0, clear bit #13 in DEVICE_GENERAL_CONTROL_20 */
                CHECK_STATUS(mvHwsGeneralSrcClockModeConfigSet(devNum, portGroup, serdesNum,
                                                             (0 << 8), 0,
                                                              CPLL0, inFreq, outFreq));
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
#endif
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}



