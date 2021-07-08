/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/userExit/userEventHandler.h>
#include <gtExtDrv/drivers/gtUartDrv.h>
#include <gtOs/gtOsExc.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <gtExtDrv/drivers/gtGenDrv.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>

#define SMI_DEVICE_IDENTIFIER1_REG_ADDR_CNS 2
#define DFX_SERVER_STATUS_REG_ADDR_CNS 0xF8010

#if 0
static CPSS_PP_DEVICE_TYPE  devType;
#endif

extern GT_BOOL systemInitialized;


/*******************************************************************************
* boardTypePrint
*
* DESCRIPTION:
*       This function prints type of PX board.
*
* INPUTS:
*       boardName - board name
*       devName   - device name
*
* OUTPUTS:
*       none
*
* RETURNS:
*       none
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_VOID boardTypePrint
(
    IN GT_CHAR  *boardName,
    IN GT_CHAR *devName
)
{
    GT_CHAR *environment;

#ifdef GM_USED
    environment = "GM (Golden Model) - simulation";
#elif defined ASIC_SIMULATION
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "WM (White Model) - simulation (special EMULATOR Mode)";
    }
    else
    {
        environment = "WM (White Model) - simulation";
    }
#else
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "HW (on EMULATOR)";
    }
    else
    {
        environment = "HW (Hardware)";
    }
#endif

    cpssOsPrintf("%s Board Type: %s [%s]\n", devName , boardName , environment);
}

extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

extern GT_STATUS mvHwsGW16SerdesAutoStartInit
(
    CPSS_HW_DRIVER_STC *driverPtr,
    GT_U8 devNum,
    GT_U8 ravenIdx
);

extern GT_STATUS mvHwsGW16SerdesAutoStartInitStatusGet
(
    GT_U8 devNum,
    GT_U32 d2dNum
);

CPSS_HW_DRIVER_STC *raven[6];

static GT_BOOL debugPrintFlagEnabled = GT_FALSE;

GT_STATUS ravenAppDemoPrintFlagEnableSet(GT_BOOL enable)
{
    debugPrintFlagEnabled = enable;
    return GT_OK;
}

typedef struct
{
        GT_U32      addr;
        char*       mapStr;
}RAVEN_ADDR_2_STR_STC;

RAVEN_ADDR_2_STR_STC regNamesDb[200]=
{
    {0x33001C04, "RX Control AUTOSTART"},
    {0x34001C04, "RX Control AUTOSTART"},
    {0x33000004, "TX Control APBWAIT"},
    {0x34000004, "TX Control APBWAIT"},
    {0x33000448, "TX0_PATSEL1"},
    {0x33000848, "TX1_PATSEL1"},
    {0x33000C48, "TX2_PATSEL1"},
    {0x33001048, "TX3_PATSEL1"},
    {0x34000448, "TX0_PATSEL1"},
    {0x34000848, "TX1_PATSEL1"},
    {0x34000C48, "TX2_PATSEL1"},
    {0x34001048, "TX3_PATSEL1"},
    {0x330021D4, "RX0_PATRST"},
    {0x330029D4, "RX1_PATRST"},
    {0x330031D4, "RX2_PATRST"},
    {0x330039D4, "RX3_PATRST"},
    {0x340021D4, "RX0_PATRST"},
    {0x340029D4, "RX1_PATRST"},
    {0x340031D4, "RX2_PATRST"},
    {0x340039D4, "RX3_PATRST"},
    {0x330021C8, "RX0_PATPRBS"},
    {0x330029C8, "RX1_PATPRBS"},
    {0x330031C8, "RX2_PATPRBS"},
    {0x330039C8, "RX3_PATPRBS"},
    {0x340021C8, "RX0_PATPRBS"},
    {0x340029C8, "RX1_PATPRBS"},
    {0x340031C8, "RX2_PATPRBS"},
    {0x340039C8, "RX3_PATPRBS"},

    {0x33007000, "PCS Global Control"},
    {0x34007000, "PCS Global Control"},
    {0x33007020, "PCS Alignment Marker BER Count for each lane0"},
    {0x33007024, "PCS Alignment Marker BER Count for each lane1"},
    {0x33007028, "PCS Alignment Marker BER Count for each lane2"},
    {0x3300702C, "PCS Alignment Marker BER Count for each lane3"},
    {0x34007020, "PCS Alignment Marker BER Count for each lane0"},
    {0x34007024, "PCS Alignment Marker BER Count for each lane1"},
    {0x34007028, "PCS Alignment Marker BER Count for each lane2"},
    {0x3400702C, "PCS Alignment Marker BER Count for each lane3"},

    {0x33008008, "PMA Operation Control"},
    {0x34008008, "PMA Operation Control"},
    {0x33008004, "PMA Reset Control Per Lane"},
    {0x34008004, "PMA Reset Control Per Lane"},
    {0x33009044, "KDU Fifo Lane Ctrl - Lane0"},
    {0x3300A044, "KDU Fifo Lane Ctrl - Lane1"},
    {0x3300B044, "KDU Fifo Lane Ctrl - Lane2"},
    {0x3300C044, "KDU Fifo Lane Ctrl - Lane3"},
    {0x34009044, "KDU Fifo Lane Ctrl - Lane0"},
    {0x3400A044, "KDU Fifo Lane Ctrl - Lane1"},
    {0x3400B044, "KDU Fifo Lane Ctrl - Lane2"},
    {0x3400C044, "KDU Fifo Lane Ctrl - Lane3"},

    {0x33009800, "KDU RX PAT CTRL - Lane0"},
    {0x3300A800, "KDU RX PAT CTRL - Lane1"},
    {0x3300B800, "KDU RX PAT CTRL - Lane2"},
    {0x3300C800, "KDU RX PAT CTRL - Lane3"},
    {0x33009C00, "KDU TX PAT CTRL - Lane0"},
    {0x3300AC00, "KDU TX PAT CTRL - Lane1"},
    {0x3300BC00, "KDU TX PAT CTRL - Lane2"},
    {0x3300CC00, "KDU TX PAT CTRL - Lane3"},
    {0x34009800, "KDU RX PAT CTRL - Lane0"},
    {0x3400A800, "KDU RX PAT CTRL - Lane1"},
    {0x3400B800, "KDU RX PAT CTRL - Lane2"},
    {0x3400C800, "KDU RX PAT CTRL - Lane3"},
    {0x34009C00, "KDU TX PAT CTRL - Lane0"},
    {0x3400AC00, "KDU TX PAT CTRL - Lane1"},
    {0x3400BC00, "KDU TX PAT CTRL - Lane2"},
    {0x3400CC00, "KDU TX PAT CTRL - Lane3"},

    {0x33009804, "KDU RX PAT SEL - Lane0"},
    {0x3300A804, "KDU RX PAT SEL - Lane1"},
    {0x3300B804, "KDU RX PAT SEL - Lane2"},
    {0x3300C804, "KDU RX PAT SEL - Lane3"},
    {0x33009C04, "KDU TX PAT SEL - Lane0"},
    {0x3300AC04, "KDU TX PAT SEL - Lane1"},
    {0x3300BC04, "KDU TX PAT SEL - Lane2"},
    {0x3300CC04, "KDU TX PAT SEL - Lane3"},
    {0x34009804, "KDU RX PAT SEL - Lane0"},
    {0x3400A804, "KDU RX PAT SEL - Lane1"},
    {0x3400B804, "KDU RX PAT SEL - Lane2"},
    {0x3400C804, "KDU RX PAT SEL - Lane3"},
    {0x34009C04, "KDU TX PAT SEL - Lane0"},
    {0x3400AC04, "KDU TX PAT SEL - Lane1"},
    {0x3400BC04, "KDU TX PAT SEL - Lane2"},
    {0x3400CC04, "KDU TX PAT SEL - Lane3"},

    {0x33009808, "KDU RX PAT STS - Lane0"},
    {0x3300A808, "KDU RX PAT STS - Lane1"},
    {0x3300B808, "KDU RX PAT STS - Lane2"},
    {0x3300C808, "KDU RX PAT STS - Lane3"},
    {0x33009C08, "KDU TX PAT STS - Lane0"},
    {0x3300AC08, "KDU TX PAT STS - Lane1"},
    {0x3300BC08, "KDU TX PAT STS - Lane2"},
    {0x3300CC08, "KDU TX PAT STS - Lane3"},
    {0x34009808, "KDU RX PAT STS - Lane0"},
    {0x3400A808, "KDU RX PAT STS - Lane1"},
    {0x3400B808, "KDU RX PAT STS - Lane2"},
    {0x3400C808, "KDU RX PAT STS - Lane3"},
    {0x34009C08, "KDU TX PAT STS - Lane0"},
    {0x3400AC08, "KDU TX PAT STS - Lane1"},
    {0x3400BC08, "KDU TX PAT STS - Lane2"},
    {0x3400CC08, "KDU TX PAT STS - Lane3"},

    {0x33009004, "KDU Fifo Frzctl Lane0"},
    {0x3300A004, "KDU Fifo Frzctl Lane1"},
    {0x3300B004, "KDU Fifo Frzctl Lane2"},
    {0x3300C004, "KDU Fifo Frzctl Lane3"},
    {0x34009004, "KDU Fifo Frzctl Lane0"},
    {0x3400A004, "KDU Fifo Frzctl Lane1"},
    {0x3400B004, "KDU Fifo Frzctl Lane2"},
    {0x3400C004, "KDU Fifo Frzctl Lane3"},

    {0x33009820, "KDU RX PAT PERR CNT - Lane0"},
    {0x3300A820, "KDU RX PAT PERR CNT - Lane1"},
    {0x3300B820, "KDU RX PAT PERR CNT - Lane2"},
    {0x3300C820, "KDU RX PAT PERR CNT - Lane3"},
    {0x33009C20, "KDU TX PAT PERR CNT - Lane0"},
    {0x3300AC20, "KDU TX PAT PERR CNT - Lane1"},
    {0x3300BC20, "KDU TX PAT PERR CNT - Lane2"},
    {0x3300CC20, "KDU TX PAT PERR CNT - Lane3"},
    {0x34009820, "KDU RX PAT PERR CNT - Lane0"},
    {0x3400A820, "KDU RX PAT PERR CNT - Lane1"},
    {0x3400B820, "KDU RX PAT PERR CNT - Lane2"},
    {0x3400C820, "KDU RX PAT PERR CNT - Lane3"},
    {0x34009C20, "KDU TX PAT PERR CNT - Lane0"},
    {0x3400AC20, "KDU TX PAT PERR CNT - Lane1"},
    {0x3400BC20, "KDU TX PAT PERR CNT - Lane2"},
    {0x3400CC20, "KDU TX PAT PERR CNT - Lane3"},

    {0x33009830, "KDU RX PAT PUSER MASK - Lane0"},
    {0x3300A830, "KDU RX PAT PUSER MASK - Lane1"},
    {0x3300B830, "KDU RX PAT PUSER MASK - Lane2"},
    {0x3300C830, "KDU RX PAT PUSER MASK - Lane3"},
    {0x33009C30, "KDU TX PAT PUSER MASK - Lane0"},
    {0x3300AC30, "KDU TX PAT PUSER MASK - Lane1"},
    {0x3300BC30, "KDU TX PAT PUSER MASK - Lane2"},
    {0x3300CC30, "KDU TX PAT PUSER MASK - Lane3"},
    {0x34009830, "KDU RX PAT PUSER MASK - Lane0"},
    {0x3400A830, "KDU RX PAT PUSER MASK - Lane1"},
    {0x3400B830, "KDU RX PAT PUSER MASK - Lane2"},
    {0x3400C830, "KDU RX PAT PUSER MASK - Lane3"},
    {0x34009C30, "KDU TX PAT PUSER MASK - Lane0"},
    {0x3400AC30, "KDU TX PAT PUSER MASK - Lane1"},
    {0x3400BC30, "KDU TX PAT PUSER MASK - Lane2"},
    {0x3400CC30, "KDU TX PAT PUSER MASK - Lane3"},

    {0x33009810, "KDU RX PAT PUSER PAT - Lane0"},
    {0x3300A810, "KDU RX PAT PUSER PAT - Lane1"},
    {0x3300B810, "KDU RX PAT PUSER PAT - Lane2"},
    {0x3300C810, "KDU RX PAT PUSER PAT - Lane3"},
    {0x33009C10, "KDU TX PAT PUSER PAT - Lane0"},
    {0x3300AC10, "KDU TX PAT PUSER PAT - Lane1"},
    {0x3300BC10, "KDU TX PAT PUSER PAT - Lane2"},
    {0x3300CC10, "KDU TX PAT PUSER PAT - Lane3"},
    {0x34009810, "KDU RX PAT PUSER PAT - Lane0"},
    {0x3400A810, "KDU RX PAT PUSER PAT - Lane1"},
    {0x3400B810, "KDU RX PAT PUSER PAT - Lane2"},
    {0x3400C810, "KDU RX PAT PUSER PAT - Lane3"},
    {0x34009C10, "KDU TX PAT PUSER PAT - Lane0"},
    {0x3400AC10, "KDU TX PAT PUSER PAT - Lane1"},
    {0x3400BC10, "KDU TX PAT PUSER PAT - Lane2"},
    {0x3400CC10, "KDU TX PAT PUSER PAT - Lane3"},

    {0xFFFFFFFF, NULL}
};

void ravenRegisterInfo
(
    GT_U32 regAddr
)
{
    GT_U32 i;

    for(i = 0; regNamesDb[i].mapStr != NULL; i++)
    {
        if(regAddr == regNamesDb[i].addr)
        {
            osPrintf("    %s\n", regNamesDb[i].mapStr);
            return;
        }
    }

    osPrintf("\n");
    return;
}

GT_STATUS ravenExactDelay
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 mils
)
{
    devNum = devNum;
    portGroup = portGroup;

    return cpssOsTimerWkAfter(mils);
}

GT_STATUS ravenDeviceInfoGet
(
    GT_U8  devNum,
    GT_U32 *devId,
    GT_U32 *revNum
)
{
    CPSS_HW_DRIVER_STC *drv;
    GT_STATUS rc;
    GT_U32 data;

    if (devNum >= 6)
    {
        return GT_BAD_PARAM;
    }

    drv = raven[devNum];
    rc = drv->read(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, 0x4c, &data, 1);
    if(rc == GT_OK)
    {
        *devId = (data & 0xFFFF0) >> 4;;
        *revNum = (data & 0xF);
    }
    return rc;
}


GT_STATUS ravenDfxWrite
(
    GT_U8 devNum,
    GT_U32 addr,
    GT_U32 data
)
{
    CPSS_HW_DRIVER_STC *drv;

    if (devNum >= 6)
    {
        return GT_BAD_PARAM;
    }
    drv = raven[devNum];

    if(debugPrintFlagEnabled == GT_TRUE)
    {
        cpssOsPrintf("WR  %d  0  TSK  DFX  ADDR  0x%08x  0x%08x  0x%08x\n", devNum, addr, data);
    }

    return drv->writeMask(drv, CPSS_HW_DRIVER_AS_DFX_E, addr, &data, 1, 0xffffffff);
}

GT_STATUS ravenDfxFieldSet
(
    GT_U8   devNum,
    GT_U32  addr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  fieldData
)
{
    GT_U32 mask, value;
    CPSS_HW_DRIVER_STC *drv;

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    value = ((fieldData << fieldOffset) & mask);

    if (devNum >= 6)
    {
        return GT_BAD_PARAM;
    }
    drv = raven[devNum];
    return drv->writeMask(drv, CPSS_HW_DRIVER_AS_DFX_E, addr, &value, 1, mask);
}

GT_STATUS ravenDfxRead
(
    GT_U8 devNum,
    GT_U32 addr,
    GT_U32 *data
)
{
    CPSS_HW_DRIVER_STC *drv;

    if (devNum >= 6)
    {
        return GT_BAD_PARAM;
    }
    drv = raven[devNum];
    return drv->read(drv, CPSS_HW_DRIVER_AS_DFX_E, addr, data, 1);
}

GT_STATUS ravenDfxFieldGet
(
    GT_U8   devNum,
    GT_U32  addr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  *fieldData
)
{
    CPSS_HW_DRIVER_STC *drv;
    GT_STATUS rc;
    GT_U32 data, mask;

    if (devNum >= 6)
    {
        return GT_BAD_PARAM;
    }
    drv = raven[devNum];
    rc = drv->read(drv, CPSS_HW_DRIVER_AS_DFX_E, addr, &data, 1);
    if(rc == GT_OK)
    {
        CALC_MASK_MAC(fieldLength, fieldOffset, mask);
        *fieldData = (GT_U32)((data & mask) >> fieldOffset);
    }
    return rc;
}


GT_STATUS ravenPpWrite
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 addr,
    IN GT_U32 data,
    IN GT_U32 mask
)
{
    CPSS_HW_DRIVER_STC *drv;

    portGroup = portGroup;

    if (devNum >= 6)
    {
        return GT_BAD_PARAM;
    }
    drv = raven[devNum];
    if (!drv)
    {
        osPrintf("driver (devNum=%d) not found\n", devNum);
        return GT_FAIL;
    }

    if(debugPrintFlagEnabled == GT_TRUE)
    {
        cpssOsPrintf("WR  %d  0  TSK  REG  ADDR  0x%08x  0x%08x  0x%08x", devNum, addr, data, mask);
        ravenRegisterInfo(addr);
    }

    addr &= 0xFFFFFFF;

    return drv->writeMask(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, addr, &data, 1, mask);
}


GT_STATUS ravenPpRead
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 addr,
    IN GT_U32 *data,
    IN GT_U32 mask
)
{
    CPSS_HW_DRIVER_STC *drv;
    GT_STATUS rc;
    GT_U32 regValue;

    if(data == NULL)
    {
        return GT_BAD_PTR;
    }

    portGroup = portGroup;

    if (devNum >= 6)
    {
        return GT_BAD_PARAM;
    }

    if(mask == 0)
    {
        mask = 0xFFFFFFFF;
    }

    addr &= 0xFFFFFFF;

    drv = raven[devNum];
    if (!drv)
    {
        osPrintf("driver (devNum=%d) not found\n", devNum);
        return GT_FAIL;
    }

    rc = drv->read(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, addr, &regValue, 1);
    if(rc == GT_OK)
    {
        *data = regValue & mask;
    }
    if(debugPrintFlagEnabled == GT_TRUE)
    {
        cpssOsPrintf("RD  %d  0  TSK  REG  ADDR  0x%08x  0x%08x\n", devNum, addr, *data);
    }
    return rc;
}

GT_STATUS ravenSerdesReadReg
(
    GT_U8  devNum,
    GT_U8  portGroup,
    GT_U8  regType,
    GT_U8  serdesNum,
    GT_U32 regAddr,
    GT_U32 *dataPtr,
    GT_U32 mask
)
{
    GT_U32  realRegAddr;
    GT_U32  baseAddr;

    if(regType > 1)
    {
        return GT_BAD_PARAM;
    }

    baseAddr = (0 == regType) ? 0x3E000000 : 0x3E000800;
    realRegAddr = baseAddr + regAddr + 0x1000 * serdesNum;

    return ravenPpRead(devNum,portGroup,realRegAddr,dataPtr,mask);
}

GT_STATUS ravenSerdesWriteReg
(
    IN  GT_U8   devNum,
    IN  GT_U8   portGroup,
    IN  GT_U8   regType,
    IN  GT_U8   serdesNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  data,
    IN  GT_U32  mask
)
{
    GT_U32  realRegAddr;
    GT_U32  baseAddr;

    if(regType > 1)
    {
        return GT_BAD_PARAM;
    }

    baseAddr = (0 == regType) ? 0x3E000000 : 0x3E000800;
    realRegAddr = baseAddr + regAddr + 0x1000 * serdesNum;

    return ravenPpWrite(devNum,portGroup,realRegAddr,data,mask);
}

/*******************************************************************************
* ravenInitSystem
*
* DESCRIPTION:
*       This is the 'simple' board initialization function for Raven device.
*
* INPUTS:
*       startDevNum - First device number to initialized
*       endDevNum   - Last  device number to initialized
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       This is a simplified version of cpssInitSystem.
*
*******************************************************************************/
GT_STATUS ravenInitSystem(GT_U8 startDevNum, GT_U8 endDevNum)
{
    GT_STATUS   rc;
    GT_U32      devIndex, data, counter;
    CPSS_HW_DRIVER_STC *smi;
    HWS_OS_FUNC_PTR func;

    func.osExactDelayPtr            = ravenExactDelay;
    func.osMicroDelayPtr            = NULL;
    func.osTimerWkPtr               = cpssOsTimerWkAfter;
    func.osMemSetPtr                = cpssOsMemSet;
    func.osFreePtr                  = cpssOsFree;
    func.osMallocPtr                = cpssOsMalloc;
    func.sysDeviceInfo              = ravenDeviceInfoGet;
    func.serdesRegGetAccess         = (MV_SERDES_REG_ACCESS_GET)ravenSerdesReadReg;
    func.serdesRegSetAccess         = (MV_SERDES_REG_ACCESS_SET)ravenSerdesWriteReg;
    func.serverRegGetAccess         = ravenDfxRead;
    func.serverRegSetAccess         = ravenDfxWrite;
    func.osMemCopyPtr               = (MV_OS_MEM_COPY_FUNC)cpssOsMemCpy;
    func.serverRegFieldSetAccess    = ravenDfxFieldSet;
    func.serverRegFieldGetAccess    = ravenDfxFieldGet;
    func.timerGet                   = cpssOsTimeRT;
    func.registerGetAccess          = ravenPpRead;
    func.registerSetAccess          = ravenPpWrite;

    if ((endDevNum > 5) || (startDevNum > endDevNum))
    {
        return GT_BAD_PARAM;
    }

    /* Call to fatal_error initialization, use default fatal error call_back - supplied by mainOs */
    rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
    if (rc != GT_OK)
    {
        cpssOsPrintf("osFatalErrorInit fail\n");
        return rc;
    }

    boardTypePrint("DB" /*boardName*/, "HEX-RAVEN" /*devName*/);

    extDrvSoCInit();
    if (cpssHwDriverLookup("/SoC/internal-regs/smi@0x72004") != NULL)
    {
        CPSS_HW_DRIVER_STC *soc = cpssHwDriverLookup("/SoC/internal-regs");
        if (soc != NULL)
        {
            /* 0 - normal speed(/128), 1 - fast mode(/16), 2 - accel (/8) */
            GT_U32 data = 1;
            soc->writeMask(soc, 0, 0x72014, &data, 1, 0x3);
        }
    }

    smi = cpssHwDriverLookup("/smi0");
    if(smi == NULL)
    {
        osPrintf("ERROR: SMI driver not found\n\n");
        return GT_NOT_FOUND;
    }

    for(devIndex = startDevNum; devIndex <= endDevNum; devIndex++)
    {
        CPSS_HW_INFO_STC   hwInfo;
        /*************************************/
        /*     SMI Driver initialization     */
        /*************************************/
        int phy=devIndex+2;
        smi->read(smi, phy, SMI_DEVICE_IDENTIFIER1_REG_ADDR_CNS, &data, 1);
        if (data != 0x0141) /* not marvell slave smi device */
        {
            cpssOsPrintf("Not MARVELL slave SMI device found, SMI slave addr = 0x%x, id=0x%x\n", phy, data);
            return GT_NOT_APPLICABLE_DEVICE;
        }
        else
        {
            cpssOsPrintf("MARVELL slave SMI device found, SMI slave addr = 0x%x\n", phy);
        }

#if 0
        raven[devIndex] = cpssHwDriverSip5SlaveSMICreateDrv(smi, phy);
#endif
        rc = cpssHwDriverSlaveSmiConfigure("/smi0", phy, &hwInfo);
        /* TEMP, CPSS will do it later */
        if (rc == GT_OK)
        {
            char buf[64];
            cpssOsSprintf(buf,"/raven%d", devIndex);
            cpssHwDriverAddAlias(hwInfo.driver, buf);
            raven[devIndex] = hwInfo.driver;
        }

        if (raven[devIndex] == NULL)
        {
            cpssOsPrintf("cpssHwDriverSip5SlaveSMICreateDrv returned with NULL pointer for devNum = %d\n", devIndex);
            return GT_BAD_PTR;
        }
#if 0
        cpssOsSprintf(buf, "ssmi@%d", phy);
        cpssHwDriverRegister(raven[devIndex], buf);
        sprintf(buf, "raven%d", devIndex);
        cpssHwDriverAddAlias(raven[devIndex], buf);
#endif

        /*********************************/
        /*     Device initialization     */
        /*********************************/
        /*
            Init stage polling
            This polling is on the init_stage field (bits 5:4) of the Server
            Status register. Polling is continued until this field is equal
            2'b11 (Fully Functional), or until the timeout expires (100ms).
        */
        data = 0;
        counter = 0;
        while(data != 3)
        {
            raven[devIndex]->read(raven[devIndex], CPSS_HW_DRIVER_AS_DFX_E, DFX_SERVER_STATUS_REG_ADDR_CNS, &data, 1);
            data = (data >> 4) & 3;
            if(counter == 100)
            {
                cpssOsPrintf("ERROR: Device not ready, Slave Smi Addr = 0x%x\n", devIndex+0x10);
                return GT_FAIL;
            }
            counter++;
            cpssOsTimerWkAfter(1);
        }
        cpssOsPrintf("Device %d is ready\n", devIndex);

        /*******************************************/
        /*     HWS (Avago 16nm) initialization     */
        /*******************************************/
        rc = hwsRavenIfPreInit(devIndex, &func);
        if(rc != GT_OK)
        {
            cpssOsPrintf("hwsRavenIfPreInit failed: devNum = %d, rc = %d\n", devIndex, rc);
            return rc;
        }

        rc = hwsRavenCpllConfig(devIndex);
        if(rc != GT_OK)
        {
            cpssOsPrintf("hwsRavenCpllConfig failed: devNum = %d, rc = %d\n", devIndex, rc);
            return rc;
        }
    }

    for(devIndex = startDevNum; devIndex <= endDevNum; devIndex++)
    {
        /***********************************************************/
        /*     D2D auto start (autonegotiation) initialization     */
        /***********************************************************/

        /* In order to perform D2D/PHY configurations it is needed to setup links
           on all 6 devices and only then start to run basic init sequence */
        rc = mvHwsGW16SerdesAutoStartInit(NULL, devIndex, devIndex);
        if(rc != GT_OK)
        {
            cpssOsPrintf("mvHwsGW16SerdesAutoStartInit failed: devNum = %d, rc = %d\n", devIndex, rc);
            return rc;
        }
    }


    for(devIndex = startDevNum; devIndex <= endDevNum; devIndex++)
    {
        /***********************************************************/
        /*     D2D PHY wait for auto start init                    */
        /***********************************************************/
        rc = mvHwsGW16SerdesAutoStartInitStatusGet(devIndex, 0);
        if(rc != GT_OK)
        {
            cpssOsPrintf("mvHwsGW16SerdesAutoStartInitStatusGet failed: devNum = %d, rc = %d\n", devIndex, rc);
            return rc;
        }
    }

    for(devIndex = startDevNum; devIndex <= endDevNum; devIndex++)
    {
#if 0
        rc = hwsRavenCpllConfig(devIndex);
        if(rc != GT_OK)
        {
            cpssOsPrintf("hwsRavenCpllConfig failed: devNum = %d, rc = %d\n", devIndex, rc);
            return rc;
        }
#endif
        rc = hwsRavenIfInit(devIndex, &func);
        if(rc != GT_OK)
        {
            cpssOsPrintf("hwsRavenIfInit failed: devNum = %d, rc = %d\n", devIndex, rc);
            return rc;
        }
    }

    /*cpssHwDriverList();*/

    systemInitialized = GT_TRUE;

    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(50);

    return rc;
}

/*******************************************************************************
* ravenInitSystem
*
* DESCRIPTION:
*       This is the 'simple' board initialization function for Raven device.
*
* INPUTS:
*       startDevNum - First device number to initialized
*       endDevNum   - Last  device number to initialized
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       This is a simplified version of cpssInitSystem.
*
*******************************************************************************/
GT_STATUS setRavenInitSystem(GT_U8 startDevNum, GT_U8 endDevNum)
{
    GT_STATUS rc;

    rc = ravenInitSystem(startDevNum, endDevNum);
    if (rc != GT_OK)
    {
        cpssOsPrintf("setRavenInitSystem failed: startDevNum=%d, endDevNum=%d, rc = %d\n",
                     startDevNum, endDevNum, rc);
        return rc;
    }

    return GT_OK;
}


