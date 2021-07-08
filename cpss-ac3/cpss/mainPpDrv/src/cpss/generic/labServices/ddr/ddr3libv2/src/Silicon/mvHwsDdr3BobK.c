/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsDdr3BobK.c
*
* DESCRIPTION: DDR3 BobK configuration
*
* DEPENDENCIES:
******************************************************************************/

#define DEFINE_GLOBALS

#include "mvDdr3TrainingIpStatic.h"
#include "mvDdr3TrainingIpFlow.h"
#include "mvDdrTrainingIpDb.h"
#include "mvDdr3TrainingIp.h"
#include "mvDdr3TrainingIpDef.h"
#include "mvDdr3TrainingIpPrvIf.h"
#include "mvDdr3LoggingDef.h"
#include "mvHwsDdr3BobK.h"
#if !defined(CPSS_BUILD)
#include "ddr3_msys_bobk.h"
#endif

/************************** definitions ******************************/
#define BOBK_NUM_BYTES                   (3)

#define BOBK_CLIENT_FIELD(client)       (client << 15)
#define BOBK_GET_CLIENT_FIELD(interfaceAccess,interfaceId) ((interfaceAccess == ACCESS_TYPE_MULTICAST) ? BOBK_CLIENT_FIELD(MULTICAST_ID):BOBK_CLIENT_FIELD(interfaceId))
#define BOBK_XSB_MAPPING(interfaceId, interfaceAccess, regaddr)   (regaddr+ XSB_BASE_ADDR + BOBK_GET_CLIENT_FIELD(interfaceAccess,interfaceMap[interfaceId].client))

#if defined(CPSS_BUILD)

#define REG_DEVICE_SAR0_ADDR                        0xF8200
#define REG_DEVICE_SAR1_ADDR                        0xF8204
#define REG_DEVICE_SAR1_OVERRIDE_ADDR               0xF82D8
#define PLL0_CNFIG_OFFSET                           21
#define PLL0_CNFIG_MASK                             0x7
#define PLL1_CNFIG_OFFSET                           18
#define PLL1_CNFIG_MASK                             0x7

#define CS_CBE_VALUE(csNum)   (csCbeReg[csNum])
#endif

#define BOBK_CLIENT_MAPPING(interfaceId, regaddr) ((regaddr << 2)+ CLIENT_BASE_ADDR + BOBK_CLIENT_FIELD(interfaceMap[interfaceId].client))

#define TM_PLL_REG_DATA(a,b,c)  ((a << 12) + (b << 8) + (c << 2))
#define R_MOD_W(writeData,readData,mask) ((writeData & mask) | (readData & (~mask)))

#define CETUS_DEV_ID   0xBE00
#define CAELUM_DEV_ID  0xBC00

/************************** pre-declaration ******************************/

static GT_STATUS ddr3TipBobKGetMediumFreq
(
    GT_U32          devNum,
    GT_U32          interfaceId,
    MV_HWS_DDR_FREQ *freq
);

GT_STATUS    ddr3TipBobKExtRead
(
    GT_U32      devNum,
    GT_U32      interfaceId,
    GT_U32      regAddr,
    GT_U32      numOfBursts,
    GT_U32      *data
);

GT_STATUS    ddr3TipBobKExtWrite
(
    GT_U32      devNum,
    GT_U32      interfaceId,
    GT_U32      regAddr,
    GT_U32      numOfBursts,
    GT_U32      *data
);

GT_STATUS ddr3TipBobKSetDivider
(
    GT_U8                           devNum,
    GT_U32                          interfaceId,
    MV_HWS_DDR_FREQ                 freq
);

static GT_STATUS ddr3TipTmSetDivider
(
    GT_U8                           devNum,
    GT_U32                          interfaceId,
    MV_HWS_DDR_FREQ                 frequency
);

static GT_STATUS ddr3TipCpuSetDivider
(
    GT_U8                           devNum,
    GT_U32                          interfaceId,
    MV_HWS_DDR_FREQ                 frequency
);

GT_STATUS ddr3BobKUpdateTopologyMap
(
    GT_U32 devNum,
    MV_HWS_TOPOLOGY_MAP* topologyMap
);

GT_STATUS ddr3TipBobKGetInitFreq
(
    GT_U8           devNum,
    GT_U32          interfaceId,
    MV_HWS_DDR_FREQ* freq
);

static GT_STATUS ddr3TipTmGetInitFreq
(
    GT_STATUS       devNum,
    MV_HWS_DDR_FREQ *freq
);

static GT_STATUS ddr3TipCpuGetInitFreq
(
    GT_STATUS       devNum,
    MV_HWS_DDR_FREQ *freq
);

GT_STATUS    ddr3TipBobKRead
(
    GT_U8                 devNum,
    GT_U32                regAddr,
    GT_U32                *data,
    GT_U32                mask
);

GT_STATUS    ddr3TipBobKWrite
(
    GT_U8                 devNum,
    GT_U32                regAddr,
    GT_U32                dataValue,
    GT_U32                mask
);
GT_STATUS    ddr3TipBobKIFRead
(
    GT_U8                 devNum,
    MV_HWS_ACCESS_TYPE    interfaceAccess,
    GT_U32                interfaceId,
    GT_U32                regAddr,
    GT_U32                *data,
    GT_U32                mask
);

GT_STATUS    ddr3TipBobKIFWrite
(
    GT_U8                 devNum,
    MV_HWS_ACCESS_TYPE    interfaceAccess,
    GT_U32                interfaceId,
    GT_U32                regAddr,
    GT_U32                dataValue,
    GT_U32                mask
);

GT_STATUS ddr3TipBobKIsfAccessDone
(
    GT_U32 devNum,
    GT_U32 interfaceId
);

GT_STATUS ddr3TipBobKGetDeviceInfo
(
    GT_U8      devNum,
    MV_DDR3_DEVICE_INFO * infoPtr
);

GT_STATUS    ddr3TipBobKTMWrite
(
    GT_U8                 devNum,
    MV_HWS_ACCESS_TYPE    interfaceAccess,
    GT_U32                interfaceId,
    GT_U32                regAddr,
    GT_U32                dataValue,
    GT_U32                mask
);
/************************** Globals ******************************/

extern GT_U32 maskTuneFunc;
extern GT_U32 freqVal[];
extern MV_HWS_DDR_FREQ mediumFreq;
extern MV_HWS_TOPOLOGY_MAP *topologyMap;
extern GT_U32 firstActiveIf;
extern MV_HWS_DDR_FREQ initFreq;
extern GT_U32 dfsLowFreq;
extern GT_U32 dfsLowPhy1;
extern GT_U32  PhyReg1Val;
extern GT_U8  isPllBeforeInit;
extern GT_U32 useBroadcast;
extern GT_U32 isCbeRequired;
extern GT_U32 delayEnable, ckDelay,caDelay;
extern GT_U8 debugTrainingAccess;
extern GT_U8 calibrationUpdateControl; /*2 external only, 1 is internal only*/
extern GT_U32 dfsLowFreq;

GT_U32 debugBobK = 0;
GT_U32  pipeMulticastMask;

#if defined(CPSS_BUILD)
static GT_U32 csCbeReg[]=
{
    0xE , 0xD, 0xB, 0x7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

GT_U32 devId, hwDdrIfMask, hwMaxDdrIfNum;

static MV_DFX_ACCESS interfaceMap[] =
{
    /* Pipe Client*/
    {   0,   16 },
    {   3,   18 },
    {   1,   10 },
    {   3,   1  },
    {   0,   0  } /* in BOBK interface 4 doesn't belong to TM*/
};
#endif
static GT_U8 bobKBwPerFreq[DDR_FREQ_LIMIT] =
{
    0x3, /*DDR_FREQ_LOW_FREQ*/
    0x4, /*DDR_FREQ_400*/
    0x4, /*DDR_FREQ_533*/
    0x5, /*DDR_FREQ_667*/
    0x5, /*DDR_FREQ_800*/
    0x5, /*DDR_FREQ_933,*/
    0x5, /*DDR_FREQ_1066*/
    0x3, /*DDR_FREQ_311*/
    0x3, /*DDR_FREQ_333*/
    0x4,  /*DDR_FREQ_467*/
    0x5, /*DDR_FREQ_850*/
    0x5,  /*DDR_FREQ_600*/
    0x3,  /*DDR_FREQ_300*/
    0x5, /*DDR_FREQ_900*/
    0x3 /*DDR_FREQ_360*/
};

static GT_U8 bobKRatePerFreq[DDR_FREQ_LIMIT] =
{
   0x1, /*DDR_FREQ_LOW_FREQ,*/
   0x2, /*DDR_FREQ_400,*/
   0x2, /*DDR_FREQ_533,*/
   0x2, /*DDR_FREQ_667,*/
   0x2, /*DDR_FREQ_800,*/
   0x3, /*DDR_FREQ_933,*/
   0x3, /*DDR_FREQ_1066,*/
   0x1, /*DDR_FREQ_311*/
   0x1, /*DDR_FREQ_333*/
   0x2, /*DDR_FREQ_467*/
   0x2, /*DDR_FREQ_850*/
   0x2, /*DDR_FREQ_600*/
   0x1, /*DDR_FREQ_300*/
   0x3,  /*DDR_FREQ_900*/
   0x1  /*DDR_FREQ_360*/
};

/* Bit mapping (for PBS) */
GT_U32 bobKDQbitMap2Phypin[] =
{
/*#warning "DQ mapping is updated for Interface 0 and 4 only!" !!!*/
        /* Interface 0 */
        2,0,3,9,7,8,6,1 , /* dq[0:7]   */
        3,2,1,6,0,9,7,8 , /* dq[8:15]  */
        9,6,3,2,1,7,0,8 , /* dq[16:23] */
        3,1,7,2,0,8,9,6 , /* dq[24:31] */
        0,0,0,0,0,0,0,0 , /* dq[ECC]   */

        /* Interface 1 */
        9,0,1,6,8,3,2,7 , /* dq[0:7]   */
        9,0,7,2,8,6,3,1 , /* dq[8:15]  */
        9,0,8,3,7,6,2,1 , /* dq[16:23] */
        0,7,1,9,6,2,3,8 , /* dq[24:31] */
        0,0,0,0,0,0,0,0 , /* dq[ECC]   */

        /* Interface 2 */
        0,0,0,0,0,0,0,0 , /* dq[0:7]   */
        0,0,0,0,0,0,0,0 , /* dq[8:15]  */
        0,0,0,0,0,0,0,0 , /* dq[16:23] */
        0,0,0,0,0,0,0,0 , /* dq[24:31] */
        0,0,0,0,0,0,0,0 , /* dq[ECC]   */

        /* Interface 3 */
        6,2,8,9,0,1,7,3 , /* dq[0:7]   */
        1,3,7,9,0,2,8,6 , /* dq[8:15]  */
        2,0,7,1,8,9,6,3 , /* dq[16:23] */
        1,6,8,2,0,7,3,9 , /* dq[24:31] */
        0,0,0,0,0,0,0,0 , /* dq[ECC]   */

        /* Interface 4 */
        1,9,8,3,0,7,2,6 , /* dq[0:7]   */
        3,2,8,1,9,6,7,0 , /* dq[8:15]  */
        0,6,2,1,9,3,8,7 , /* dq[16:23] */
        0,6,1,8,3,9,7,2 , /* dq[24:31] */
        0,1,2,3,6,7,8,9   /* dq[ECC]   */
};

#if defined(CPSS_BUILD)

MV_HWS_TOPOLOGY_MAP bobKTopologyMap[] =
{
    /* 1st board  - CETUS*/
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                         speed_bin           memory_width  mem_size  frequency  casL casWL      temperature */
 {  {{{0x1,0,0,0}, {0x1,0,0,0}, {0x2,1,0,0}, {0x2,1,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} ,
    {{{0x1,1,0,0}, {0x1,1,0,0}, {0x2,0,0,0}, {0x2,0,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} ,
    {{{0x1,1,0,0}, {0x1,1,0,0}, {0x2,0,0,0}, {0x2,0,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} ,
    {{{0x1,1,0,0}, {0x1,1,0,0}, {0x2,0,0,0}, {0x2,0,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} ,
    {{{0x1,1,0,0}, {0x1,1,0,0}, {0x2,0,0,0}, {0x2,0,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} },
    0xF  /* Buses mask */
    },
    /* 2nd board  - Caelum*/
    {
    0xB, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                         speed_bin           memory_width  mem_size  frequency  casL casWL      temperature */
 {  {{{0x2,0,0,0}, {0x2,0,0,0}, {0x1,1,0,0}, {0x1,1,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} ,
    {{{0x2,0,0,0}, {0x2,0,0,0}, {0x1,1,0,0}, {0x1,1,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} ,
    {{{0x1,1,0,0}, {0x1,1,0,0}, {0x2,0,0,0}, {0x2,0,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} ,
    {{{0x2,0,0,0}, {0x2,0,0,0}, {0x1,1,0,0}, {0x1,1,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} ,
    {{{0x1,1,0,0}, {0x1,1,0,0}, {0x2,0,0,0}, {0x2,0,0,0}, {0,0,0,0}}, SPEED_BIN_DDR_1866M, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_667, 0 ,   0 , MV_HWS_TEMP_HIGH} },
    0xF  /* Buses mask */
    }
};
#endif

static GT_U8    ddr3TipClockMode( GT_U32 frequency );

/************************** Server Access ******************************/
GT_STATUS ddr3TipBobKServerRegWrite
(
    GT_U32 devNum,
    GT_U32 regAddr,
    GT_U32 dataValue
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    regAddr = regAddr;
    dataValue = dataValue;

    return GT_OK;
#else
    return hwsServerRegSetFuncPtr(devNum, regAddr, dataValue);
#endif
}

GT_STATUS ddr3TipBobKServerRegRead
(
    GT_U32 devNum,
    GT_U32 regAddr,
    GT_U32 *dataValue,
    GT_U32 regMask
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    regAddr = regAddr;
    regMask = regMask;

    *dataValue = 0;
#else
    hwsServerRegGetFuncPtr(devNum, regAddr, dataValue);


    *dataValue  = *dataValue & regMask;
#endif
    return GT_OK;
}

/**********************************************************************************/

GT_STATUS ddr3TipBobKGetFreqConfig
(
    GT_U8                           devNum,
    MV_HWS_DDR_FREQ                 freq,
    MV_HWS_TIP_FREQ_CONFIG_INFO     *freqConfigInfo
)
{
    devNum = devNum; /* avoid warnings */

    if (bobKBwPerFreq[freq] == 0xff)
    {
        return GT_NOT_SUPPORTED;
    }

    if (freqConfigInfo == NULL)
    {
        return GT_BAD_PARAM;
    }

    freqConfigInfo->bwPerFreq = bobKBwPerFreq[freq];
    freqConfigInfo->ratePerFreq = bobKRatePerFreq[freq];

    freqConfigInfo->isSupported = GT_TRUE;

    return GT_OK;
}
#if defined(CPSS_BUILD)
/*****************************************************************************
Enable Pipe
******************************************************************************/
static GT_STATUS    ddr3TipPipeEnable
(
    GT_U8                 devNum,
    MV_HWS_ACCESS_TYPE    interfaceAccess,
    GT_U32                interfaceId,
    GT_BOOL               enable
)
{
    GT_U32 dataValue=0, pipeEnableMask = 0;


    if (enable == GT_FALSE)
    {
        pipeEnableMask = 0;
    }
    else
    {
       if (interfaceAccess == ACCESS_TYPE_MULTICAST)
        {
            pipeEnableMask = pipeMulticastMask;
        }
        else
        {
            pipeEnableMask = (1 << interfaceMap[interfaceId].pipe);
        }
    }
    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, PIPE_ENABLE_ADDR, &dataValue, MASK_ALL_BITS));
    dataValue = (dataValue & (~0xFF)) | pipeEnableMask;
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, PIPE_ENABLE_ADDR, dataValue));
    return GT_OK;
}

static void ddr3TipIsUnicastAccess( GT_U8 devNum,GT_U32* interfaceId, MV_HWS_ACCESS_TYPE* interfaceAccess)
{
    GT_U32 indexCnt, totalCnt = 0 , interfaceTmp = 0;
    MV_HWS_TOPOLOGY_MAP *topologyMap = ddr3TipGetTopologyMap(devNum);

    if (*interfaceAccess == ACCESS_TYPE_MULTICAST)
    {
        for(indexCnt = 0; indexCnt < MAX_INTERFACE_NUM; indexCnt++)
        {
            if (IS_INTERFACE_ACTIVE(topologyMap->interfaceActiveMask, indexCnt) !=  0)
            {
                totalCnt++;
                interfaceTmp = indexCnt;
                continue;
            }
        }
        if (totalCnt == 1) {
            *interfaceAccess = ACCESS_TYPE_UNICAST;
            *interfaceId = interfaceTmp;
        }
    }
}

/******************************************************************************
* Name:     ddr3TipBobKSelectTMDdrController.
* Desc:     Enable/Disable access to Marvell's server.
* Args:     devNum     - device number
*           enable        - whether to enable or disable the server
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipBobKSelectTMDdrController
(
    GT_U8    devNum,
    GT_BOOL  enable
)
{
    GT_U32 interfaceId = 0, dataValue = 0;

    if (devId == CETUS_DEV_ID)
    {
        /* IN Cetus TM has only 1 IF #0*/
        hwMaxDdrIfNum = 1;
        hwDdrIfMask   = 1;
    }
    else if (devId == CAELUM_DEV_ID)
    {
        /* in Caelum TM may use 3 IFs 0,1,and 3*/
        hwMaxDdrIfNum = 4;
        hwDdrIfMask   = 0xB;
    }
    else
          mvPrintf ("Unsupported device ID %x\n", devId);

    for (interfaceId = 0; interfaceId < hwMaxDdrIfNum; interfaceId++)
    {
        if(IS_INTERFACE_ACTIVE(topologyMap->interfaceActiveMask, interfaceId) ==  0)/* if the interface doesnt exixst in HW :*/
            continue;
        else /* interface exist in HW*/
        {
            ddr3TipPipeEnable(devNum, ACCESS_TYPE_UNICAST, interfaceId, GT_TRUE);
            if (IS_INTERFACE_ACTIVE(topologyMap->interfaceActiveMask, interfaceId) ==  0)
            {
                 /*disable in-active interfaces (so they won't be accessed upon multicast)*/
                CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, BOBK_CLIENT_MAPPING(interfaceId, CLIENT_CTRL_REG), &dataValue, MASK_ALL_BITS));
                dataValue &= ~0x40;
                CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_CLIENT_MAPPING(interfaceId, CLIENT_CTRL_REG), dataValue));
                continue;
            }
                /* Enable relevant Pipe */
            pipeMulticastMask |= (1 << interfaceMap[interfaceId].pipe);
            /* multicast Macro is subscribed to Multicast 0x1D */
            CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, BOBK_CLIENT_MAPPING(interfaceId, CLIENT_CTRL_REG), &dataValue, MASK_ALL_BITS));
            dataValue |= 0x40;
            CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_CLIENT_MAPPING(interfaceId, CLIENT_CTRL_REG), dataValue));

            ddr3TipPipeEnable(devNum, ACCESS_TYPE_UNICAST, interfaceId, GT_FALSE);

        }
    }
        /* enable access to relevant pipes (according to the pipe mask) */
    ddr3TipPipeEnable(devNum, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, GT_TRUE);

   if (enable)
    {
       /* Enable DDR Tuning */

        CHECK_STATUS(ddr3TipBobKTMWrite(devNum, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, READ_BUFFER_SELECT_REG,  0x8000, MASK_ALL_BITS));
        /* configure duplicate CS */
        CHECK_STATUS(ddr3TipBobKTMWrite(devNum, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, CS_ENABLE_REG,  0x4A/*Haim 0x42*/, MASK_ALL_BITS));
    }
    else
    {
        /* Disable DDR Tuning Select */
        CHECK_STATUS(ddr3TipBobKTMWrite(devNum, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, CS_ENABLE_REG,  0x2, MASK_ALL_BITS));
        CHECK_STATUS(ddr3TipBobKTMWrite(devNum, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, READ_BUFFER_SELECT_REG, 0,MASK_ALL_BITS));
   }
   return GT_OK;
}

/*****************************************************************************
Check if Dunit access is done
******************************************************************************/

GT_STATUS    ddr3TipBobKIsAccessDone
(
    GT_U8 devNum,
    GT_32 interfaceId
)
{
    GT_U32 rdData = 1, cnt = 0;

    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, BOBK_XSB_MAPPING(interfaceId, ACCESS_TYPE_UNICAST, XSB_CMD_REG), &rdData, MASK_ALL_BITS));
    rdData &= 1;

    while(rdData != 0)
    {
        if (cnt++ >= MAX_POLLING_ITERATIONS)
            break;
        CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, BOBK_XSB_MAPPING(interfaceId, ACCESS_TYPE_UNICAST, XSB_CMD_REG), &rdData, MASK_ALL_BITS));
        rdData &= 1;
#ifdef ASIC_SIMULATION
        rdData = 0;/* no more iterations needed */
#endif /*ASIC_SIMULATION*/
    }
    if (cnt < MAX_POLLING_ITERATIONS)
    {
        return GT_OK;
    }
    else
    {
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO,("AccessDone fail: if = %d\n",interfaceId));
        return GT_FAIL;
    }
}

/******************************************************************************/
GT_STATUS    ddr3TipBobKTMRead
(
    GT_U8                 devNum,
    MV_HWS_ACCESS_TYPE    interfaceAccess,
    GT_U32                interfaceId,
    GT_U32                regAddr,
    GT_U32                *data,
    GT_U32                mask
)
{
    GT_U32 dataValue = 0, startIf, endIf;
    MV_HWS_TOPOLOGY_MAP *topologyMap = ddr3TipGetTopologyMap(devNum);

    ddr3TipIsUnicastAccess(devNum, &interfaceId, &interfaceAccess);

    if (interfaceAccess == ACCESS_TYPE_UNICAST)
    {
        startIf = interfaceId;
        endIf = interfaceId;
    }
    else
    {
        startIf = 0;
        endIf = MAX_INTERFACE_NUM-1;
    }

    for(interfaceId = startIf; interfaceId <= endIf; interfaceId++)
    {
        if (IS_INTERFACE_ACTIVE(topologyMap->interfaceActiveMask, interfaceId) ==  0)
        {
            continue;
        }
        ddr3TipPipeEnable(devNum, ACCESS_TYPE_UNICAST, interfaceId, GT_TRUE);
        CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, ACCESS_TYPE_UNICAST, XSB_CTRL_1_REG) ,TRAINING_ID));

        /*working with XSB client InterfaceNum  Write Interface ADDR as data to XSB address C*/
        CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, ACCESS_TYPE_UNICAST, XSB_ADDRESS_REG), regAddr));
        dataValue = ((GT_U32)(TARGET_INT << 19)) + (BYTE_EN << 11) + (BOBK_NUM_BYTES << 4) + (CMD_READ << 2) +(INTERNAL_ACCESS_PORT  << 1) + EXECUTING;
        /*Write Interface COMMAND as data to XSB address 8 */
        CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, ACCESS_TYPE_UNICAST, XSB_CMD_REG), dataValue));

        CHECK_STATUS(ddr3TipBobKIsAccessDone(devNum, interfaceId));

        /* consider that input data is always a vector, and for unicast the writing will be in the interface index in vector */
        CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, BOBK_XSB_MAPPING(interfaceId, ACCESS_TYPE_UNICAST, XSB_DATA_REG), &data[interfaceId], mask));
        if (debugBobK >= 2)
        {
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("DunitRead   0x%x: 0x%x (mask 0x%x)\n",  regAddr,  *data, mask));
        }
        /*ddr3TipPipeEnable(devNum, ACCESS_TYPE_UNICAST, interfaceId, GT_FALSE);*/
    }
    return GT_OK;
}

/******************************************************************************/
GT_STATUS    ddr3TipBobKTMWrite
(
    GT_U8                 devNum,
    MV_HWS_ACCESS_TYPE    interfaceAccess,
    GT_U32                interfaceId,
    GT_U32                regAddr,
    GT_U32                dataValue,
    GT_U32                mask
)
{
    GT_U32 dataCmd = 0, uiDataRead = 0, uiReadInterfaceId=0;
    GT_U32 dataRead[MAX_INTERFACE_NUM];
    GT_U32 startIf, endIf;
    MV_HWS_TOPOLOGY_MAP *topologyMap = ddr3TipGetTopologyMap(devNum);

    ddr3TipIsUnicastAccess(devNum, &interfaceId, &interfaceAccess);
    if (mask != MASK_ALL_BITS)
    {
        /* if all bits should be written there is no need for read-modify-write */
        if (interfaceAccess == ACCESS_TYPE_MULTICAST)
        {
            CHECK_STATUS(ddr3TipGetFirstActiveIf(devNum, topologyMap->interfaceActiveMask, &uiReadInterfaceId));
        }
        else
        {
            uiReadInterfaceId = interfaceId;
        }
        if (debugBobK >= 2)
        {
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("ddr3TipBobKTMWrite active interface = %d\n",uiReadInterfaceId));
        }

        CHECK_STATUS(ddr3TipBobKTMRead(devNum, ACCESS_TYPE_UNICAST, uiReadInterfaceId,regAddr,dataRead,MASK_ALL_BITS));
        uiDataRead = dataRead[uiReadInterfaceId];
        dataValue = (uiDataRead & (~mask)) | (dataValue & mask);
    }
    ddr3TipPipeEnable(devNum, interfaceAccess, interfaceId, GT_TRUE);
    /* set the ID */
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, interfaceAccess, XSB_CTRL_1_REG) , TRAINING_ID));

    /*working with XSB multicast client , Write Interface ADDR as data to XSB address C */
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, interfaceAccess, XSB_ADDRESS_REG), regAddr));
    /*Write Interface DATA as data to XSB address 0x10.*/
    if (debugBobK >= 1)
    {
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("Dunit Write: interface %d  access %d Address 0x%x Data 0x%x\n", interfaceId, interfaceAccess, regAddr, dataValue));
    }
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, interfaceAccess, XSB_DATA_REG),  dataValue));
    dataCmd = ((GT_U32)(TARGET_INT << 19)) + (BYTE_EN << 11) + (BOBK_NUM_BYTES << 4) +  (CMD_WRITE << 2) + (INTERNAL_ACCESS_PORT << 1);
    /*Write Interface COMMAND as data to XSB address 8
      Execute an internal write to xBar port1 */
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, interfaceAccess, XSB_CMD_REG), dataCmd));
    dataCmd |= EXECUTING;
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, interfaceAccess, XSB_CMD_REG), dataCmd));

     /*check that write is completed; the test should be done per interface */
    if (interfaceAccess == ACCESS_TYPE_MULTICAST)
    {
        startIf = 0;
        endIf = hwMaxDdrIfNum-1;
    }
    else
    {
        startIf = interfaceId;
        endIf = interfaceId;
    }

    for(interfaceId = startIf; interfaceId <= endIf; interfaceId++)
    {
        VALIDATE_IF_ACTIVE(topologyMap->interfaceActiveMask, interfaceId)
        ddr3TipPipeEnable(devNum, ACCESS_TYPE_UNICAST, interfaceId, GT_TRUE);
        CHECK_STATUS(ddr3TipBobKIsAccessDone(devNum, interfaceId));
    }
   /* ddr3TipPipeEnable(devNum, interfaceAccess, interfaceId, GT_FALSE);*/
    return GT_OK;
}
#else /*!defined(CPSS_BUILD)*/
/******************************************************************************
* Name:     ddr3TipBobKWrite.
* Desc:
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipBobKWrite
(
    GT_U8                 devNum,
    GT_U32                regAddr,
    GT_U32                dataValue,
    GT_U32                mask
)
{
    GT_U32 uiDataRead;
#ifdef WIN32
    return GT_OK;
#endif
    if (mask != MASK_ALL_BITS)
    {
        CHECK_STATUS(ddr3TipBobKRead(devNum, regAddr,  &uiDataRead, MASK_ALL_BITS));
        dataValue = (uiDataRead & (~mask)) | (dataValue & mask);
    }

    if (debugBobK >= 1)
    {
       mvPrintf("Dunit Write: Address 0x%x Data 0x%x\n", regAddr, dataValue);
    }

    MV_REG_WRITE(regAddr, dataValue);
    if (debugBobK >= 2)
        mvPrintf("write: 0x%x = 0x%x\n",regAddr,dataValue);

    return GT_OK;
}



/******************************************************************************
* Name:     ddr3TipBobKRead.
* Desc:
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipBobKRead
(
    GT_U8                 devNum,
    GT_U32                regAddr,
    GT_U32                *data,
    GT_U32                mask
)
{
#ifdef WIN32
     return GT_OK;
#endif

    *data = MV_REG_READ(regAddr) & mask;
    if (debugBobK >= 2)
    {
        mvPrintf("DunitRead   0x%x: 0x%x (mask 0x%x)\n",  regAddr,  *data, mask);
    }

    return GT_OK;
}
/******************************************************************************
* Name:     ddr3TipBobKIFWrite.
* Desc:
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipBobKIFWrite
(
    GT_U8                 devNum,
    MV_HWS_ACCESS_TYPE    interfaceAccess,
    GT_U32                interfaceId,
    GT_U32                regAddr,
    GT_U32                dataValue,
    GT_U32                mask
)
{
    if (interfaceAccess == ACCESS_TYPE_MULTICAST)
        interfaceId = 4;
 
    return(ddr3TipBobKWrite(devNum,regAddr,dataValue,mask));
}



/******************************************************************************
* Name:     ddr3TipBobKIFRead.
* Desc:
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipBobKIFRead
(
    GT_U8                 devNum,
    MV_HWS_ACCESS_TYPE    interfaceAccess,
    GT_U32                interfaceId,
    GT_U32                regAddr,
    GT_U32                *data,
    GT_U32                mask
)
{
    if (interfaceAccess == ACCESS_TYPE_MULTICAST)
        interfaceId = 4;
    return ddr3TipBobKRead(devNum, regAddr, &data[interfaceId], mask);
}

#endif

/******************************************************************************
* Name:     ddr3TipBobKSelectCpuDdrController.
* Desc:     Enable/Disable access to Marvell's server.
* Args:     devNum     - device number
*           enable        - whether to enable or disable the server
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS    ddr3TipBobKSelectCPUDdrController
(
    GT_U8    devNum,
    GT_BOOL  enable
)
{
    /* avoid warnings */
    devNum =devNum;
    enable = enable;
    return GT_OK;
}

/******************************************************************************
* Name:     ddr3TipInitBobKSilicon.
* Desc:     init Training SW DB.
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
static GT_STATUS ddr3TipInitBobKSilicon
(
    GT_U32  devNum,
    GT_U32  boardId
)
{
    MV_HWS_TIP_CONFIG_FUNC_DB   configFunc;
    MV_HWS_TOPOLOGY_MAP*        topologyMap = ddr3TipGetTopologyMap(devNum);

#if defined(CPSS_BUILD)
    GT_TUNE_TRAINING_PARAMS tuneParams;
#endif

    GT_U8 numOfBusPerInterface;

    if(topologyMap == NULL)
    {
        return GT_NOT_INITIALIZED;
    }

#if 0
    ddr3TipBobKGetInterfaceMap((GT_U8)devNum);
#endif

    boardId = boardId; /* avoid warnings */

#if defined(CPSS_BUILD)
    numOfBusPerInterface = 4; /* TM has only 4 buses*/
    configFunc.tipDunitReadFunc = ddr3TipBobKTMRead;
    configFunc.tipDunitWriteFunc = ddr3TipBobKTMWrite;
    configFunc.tipDunitMuxSelectFunc = ddr3TipBobKSelectTMDdrController;

#else
    numOfBusPerInterface = 5; /*MSYS may have 5 buses*/
    configFunc.tipDunitReadFunc = ddr3TipBobKIFRead;
    configFunc.tipDunitWriteFunc = ddr3TipBobKIFWrite;
    configFunc.tipDunitMuxSelectFunc = ddr3TipBobKSelectCPUDdrController;
#endif

    configFunc.tipExternalRead  = ddr3TipBobKExtRead;
    configFunc.tipExternalWrite = ddr3TipBobKExtWrite;

    configFunc.tipGetFreqConfigInfoFunc = ddr3TipBobKGetFreqConfig;
    configFunc.tipSetFreqDividerFunc = ddr3TipBobKSetDivider;
    configFunc.tipGetDeviceInfoFunc = ddr3TipBobKGetDeviceInfo;
    configFunc.tipGetTemperature = NULL;
    configFunc.tipGetClockRatio = ddr3TipClockMode;

    mvHwsDdr3TipInitConfigFunc(devNum, &configFunc);

    /* register bit mapping (for PBS) */
    ddr3TipRegisterDqTable(devNum, bobKDQbitMap2Phypin);

    /*Set device attributes*/
    ddr3TipDevAttrInit(devNum);
#if defined(CPSS_BUILD)
    ddr3TipDevAttrSet(devNum, MV_ATTR_TIP_REV, MV_TIP_REV_2);   /* TM DDR (Non-ECC) TIP version 2*/
#else
    ddr3TipDevAttrSet(devNum, MV_ATTR_TIP_REV, MV_TIP_REV_3);   /* BOBK MSYS DDR TIP version 3  */
#endif
    ddr3TipDevAttrSet(devNum, MV_ATTR_PHY_EDGE, MV_DDR_PHY_EDGE_NEGATIVE);
    ddr3TipDevAttrSet(devNum, MV_ATTR_OCTET_PER_INTERFACE, numOfBusPerInterface);
    ddr3TipDevAttrSet(devNum, MV_ATTR_INTERLEAVE_WA, GT_FALSE);

#if defined(CPSS_BUILD)
    maskTuneFunc = ( INIT_CONTROLLER_MASK_BIT |
                    SET_MEDIUM_FREQ_MASK_BIT |
                    WRITE_LEVELING_MASK_BIT |
                    LOAD_PATTERN_2_MASK_BIT |
                    READ_LEVELING_MASK_BIT |
                    PBS_RX_MASK_BIT |
                    PBS_TX_MASK_BIT |
                    SET_TARGET_FREQ_MASK_BIT |
                    WRITE_LEVELING_TF_MASK_BIT |
                    READ_LEVELING_TF_MASK_BIT |
                    WRITE_LEVELING_SUPP_TF_MASK_BIT |
                    CENTRALIZATION_RX_MASK_BIT |
                    CENTRALIZATION_TX_MASK_BIT);
#else
   maskTuneFunc =     (/*SET_LOW_FREQ_MASK_BIT |
                        LOAD_PATTERN_MASK_BIT |*/
                        SET_MEDIUM_FREQ_MASK_BIT |
                        WRITE_LEVELING_MASK_BIT |
                        LOAD_PATTERN_2_MASK_BIT |
                        WRITE_LEVELING_SUPP_MASK_BIT |
                        READ_LEVELING_MASK_BIT |
                        PBS_RX_MASK_BIT |
                        PBS_TX_MASK_BIT |
                        SET_TARGET_FREQ_MASK_BIT |
                        WRITE_LEVELING_TF_MASK_BIT |
                        WRITE_LEVELING_SUPP_TF_MASK_BIT |
                        READ_LEVELING_TF_MASK_BIT |
                        CENTRALIZATION_RX_MASK_BIT |
                        CENTRALIZATION_TX_MASK_BIT
                        );

    /*Supplementary not supported for ECC modes*/
    if( ddr3IfEccEnabled())
    {
        maskTuneFunc &= ~WRITE_LEVELING_SUPP_TF_MASK_BIT;
        maskTuneFunc &= ~WRITE_LEVELING_SUPP_MASK_BIT;
    }

#endif
    /*Skip mid freq stages for 400Mhz DDR speed*/
    if( (topologyMap->interfaceParams[firstActiveIf].memoryFreq == DDR_FREQ_400) ){
        maskTuneFunc = ( WRITE_LEVELING_MASK_BIT |
                        LOAD_PATTERN_2_MASK_BIT |
                        READ_LEVELING_MASK_BIT |
                        CENTRALIZATION_RX_MASK_BIT |
                        CENTRALIZATION_TX_MASK_BIT);
    }

    delayEnable = 1;
    caDelay = 0;

#if defined(CPSS_BUILD)
    if (ckDelay == MV_PARAMS_UNDEFINED)
        ckDelay = 150;

    /* update DGL parameters */
    tuneParams.ckDelay = ckDelay;
    tuneParams.PhyReg3Val = 0xA;
    tuneParams.gRttNom = 0x44;
    tuneParams.gDic = 0x2;
    tuneParams.uiODTConfig = 0x120012;
    tuneParams.gZpriData = 123;
    tuneParams.gZnriData = 123;
    tuneParams.gZpriCtrl = 74;
    tuneParams.gZnriCtrl = 74;
    tuneParams.gZpodtData = 45;
    tuneParams.gZnodtData = 45;
    tuneParams.gZpodtCtrl = 45;
    tuneParams.gZnodtCtrl = 45;
    tuneParams.gRttWR = 0; /*0x400;0x0;*/

    CHECK_STATUS(ddr3TipTuneTrainingParams(devNum, &tuneParams));
#endif /* CPSS_BUILD */

    /* frequency and general parameters */
    CHECK_STATUS(ddr3TipBobKGetMediumFreq(devNum, firstActiveIf, &mediumFreq));
    initFreq = topologyMap->interfaceParams[firstActiveIf].memoryFreq;
    freqVal[DDR_FREQ_LOW_FREQ] = dfsLowFreq = 130;
    dfsLowPhy1 = PhyReg1Val;
    calibrationUpdateControl = 1;

    return GT_OK;
}

/******************************************************************************
* Name:     ddr3BobKUpdateTopologyMap.
* Desc:
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS ddr3BobKUpdateTopologyMap(GT_U32 devNum, MV_HWS_TOPOLOGY_MAP* topologyMap)
{
    GT_U32 interfaceId;
    MV_HWS_DDR_FREQ freq;

    DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("topologyMap->interfaceActiveMask is 0x%x\n", topologyMap->interfaceActiveMask));

    for(interfaceId = 0; interfaceId < MAX_INTERFACE_NUM; interfaceId++) {
        if (IS_INTERFACE_ACTIVE(topologyMap->interfaceActiveMask, interfaceId) ==  GT_FALSE)
            continue;
        CHECK_STATUS(ddr3TipBobKGetInitFreq((GT_U8)devNum, interfaceId, &freq));
        topologyMap->interfaceParams[interfaceId].memoryFreq = freq;
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("Update topology frequency for interface %d to %d\n",interfaceId, freq));
    }

    /* re-calc topology parameters according to topology updates (if needed) */
    CHECK_STATUS(mvHwsDdr3TipLoadTopologyMap(devNum, topologyMap));

    return GT_OK;
}

/******************************************************************************
* Name:     ddr3TipInitBobK.
* Desc:     init Training SW DB and updates DDR topology.
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS ddr3TipInitBobK
(
    GT_U32  devNum,
    GT_U32  boardId
)
{
    MV_HWS_TOPOLOGY_MAP* topologyMap = ddr3TipGetTopologyMap(devNum);

    if(NULL == topologyMap)
    {
#if defined(CPSS_BUILD)
#ifndef ASIC_SIMULATION
        CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, 0x000F8240 ,  &devId, MASK_ALL_BITS));
        devId &= 0xFF00;
#else
        devId = 0xBC00; /* CAELUM_DEV_ID */
#endif

        if (devId == CETUS_DEV_ID)
        {
            boardId = 0;
        }
        else if (devId == CAELUM_DEV_ID)
        {
            boardId = 1;
        }
        else
            return GT_NOT_INITIALIZED;

        /* for CPSS, since topology is not always initialized, it is
           needed to set it to default topology */
        topologyMap = &bobKTopologyMap[boardId];
#else
        return GT_FAIL;
#endif
    }

    CHECK_STATUS(ddr3BobKUpdateTopologyMap(devNum, topologyMap));

    ddr3TipInitBobKSilicon(devNum, boardId);
    return GT_OK;
}

#if defined(CPSS_BUILD)
/*****************************************************************************
Data Reset
******************************************************************************/
static GT_STATUS    ddr3TipDataReset
(
    GT_U32                  devNum,
    MV_HWS_ACCESS_TYPE      interfaceAccess,
    GT_U32                  interfaceId
)
{
    GT_STATUS retVal;
    GT_U32  uiWordCnt;

    for(uiWordCnt = 0; uiWordCnt < 8 ; uiWordCnt++)
    {
        /*Write Interface DATA as data to XSB address 0x10.*/
        retVal = ddr3TipBobKServerRegWrite(devNum,     BOBK_XSB_MAPPING(interfaceId, interfaceAccess, XSB_DATA_REG+(uiWordCnt * 4)), 0xABCDEF12);
        if (retVal != GT_OK)
        {
            return retVal;
        }
    }
    return GT_OK;
}

/*****************************************************************************
XSB External read
******************************************************************************/
GT_STATUS    ddr3TipBobKExtRead
(
    GT_U32      devNum,
    GT_U32      interfaceId,
    GT_U32      regAddr,
    GT_U32      numOfBursts,
    GT_U32      *data
)
{
    GT_U32 burstNum, wordNum , dataValue;
    GT_U32 cntPoll;
    MV_HWS_ACCESS_TYPE accessType = ACCESS_TYPE_UNICAST;
    /*DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("=== EXTERNAL READ ==="));*/

    ddr3TipPipeEnable((GT_U8)devNum, accessType, interfaceId, GT_TRUE);
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, accessType, XSB_CTRL_0_REG) ,EXT_TRAINING_ID));

    for(burstNum=0 ; burstNum < numOfBursts; burstNum++)
    {
        ddr3TipDataReset(devNum, ACCESS_TYPE_UNICAST, interfaceId);
        /*working with XSB client InterfaceNum  Write Interface ADDR as data to XSB address C*/
        CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, accessType, XSB_ADDRESS_REG + (burstNum * 32)), regAddr));
        if (isCbeRequired ==  GT_TRUE)
        {
            /*CS_CBE_VALUE(0)*/
            dataValue = CS_CBE_VALUE(0) << 19;
        }
        else
        {
            dataValue = (GT_U32)(TARGET_EXT << 19);
        }
        dataValue |= (BYTE_EN << 11) + (NUM_BYTES_IN_BURST << 4) + (ACCESS_EXT << 3);
        /*Write Interface COMMAND as data to XSB address 8 */
        CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING( interfaceId, accessType, XSB_CMD_REG),   dataValue));
        dataValue |= EXECUTING;
        CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING( interfaceId, accessType, XSB_CMD_REG),   dataValue));

        for(cntPoll=0; cntPoll < MAX_POLLING_ITERATIONS; cntPoll++)
        {
            CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, BOBK_XSB_MAPPING( interfaceId, accessType, XSB_CMD_REG), &dataValue, 0x1));

            if (dataValue == 0)
                break;
        }

        if (cntPoll >= MAX_POLLING_ITERATIONS)
        {
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR, ("No Done indication for DDR Read\n", dataValue));
            return GT_NOT_READY;
        }

        for(wordNum = 0; wordNum < EXT_ACCESS_BURST_LENGTH /*s_uiNumOfBytesInBurst/4*/; wordNum++)
        {
            CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, BOBK_XSB_MAPPING(interfaceId, accessType, XSB_DATA_REG + (wordNum * 4)), &data[wordNum], MASK_ALL_BITS));
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("ddr3TipExtRead data 0x%x \n",data[wordNum]));
        }
    }
    return GT_OK;
}



/*****************************************************************************
XSB External write
******************************************************************************/
GT_STATUS    ddr3TipBobKExtWrite
(
    GT_U32      devNum,
    GT_U32      interfaceId,
    GT_U32      regAddr,
    GT_U32      numOfBursts,
    GT_U32      *addr
)
{
    GT_U32        wordNum = 0,  dataCmd = 0, burstNum=0, cntPoll = 0, dataValue  = 0;
    MV_HWS_ACCESS_TYPE accessType = ACCESS_TYPE_UNICAST;

    ddr3TipPipeEnable((GT_U8)devNum, accessType, interfaceId, GT_TRUE);
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, accessType, XSB_CTRL_0_REG) ,EXT_TRAINING_ID  ));
    for(burstNum=0 ; burstNum < numOfBursts; burstNum++)
    {
        /*working with XSB multicast client , Write Interface ADDR as data to XSB address C */
        CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, accessType, XSB_ADDRESS_REG), regAddr + (burstNum * EXT_ACCESS_BURST_LENGTH * 4)));
        for(wordNum = 0; wordNum < 8 ; wordNum++)
        {
            /*Write Interface DATA as data to XSB address 0x10.*/
            CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, accessType, XSB_DATA_REG + (wordNum * 4)), addr[wordNum]));
        }
        if (isCbeRequired ==  GT_TRUE)
        {
            dataCmd =  CS_CBE_VALUE(0) << 19;
        }
        else
        {
            dataCmd = (GT_U32)(TARGET_EXT << 19);
        }
        dataCmd |= (BYTE_EN << 11) + (NUM_BYTES_IN_BURST << 4) + (ACCESS_EXT << 3) + EXT_MODE;
        CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId,  accessType, XSB_CMD_REG), dataCmd));
        /* execute xsb write */
        dataCmd |= 0x1;
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("ddr3TipExtWrite dataCmd 0x%x \n", dataCmd));
        CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, BOBK_XSB_MAPPING(interfaceId, accessType, XSB_CMD_REG), dataCmd));

        for(cntPoll=0; cntPoll < MAX_POLLING_ITERATIONS; cntPoll++)
        {
            CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, BOBK_XSB_MAPPING( interfaceId, accessType, XSB_CMD_REG), &dataValue, 0x1));

            if (dataValue == 0)
                break;
        }

        if (cntPoll >= MAX_POLLING_ITERATIONS)
        {
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR, ("No Done indication for DDR Write External\n", dataValue));
            return GT_NOT_READY;
        }

    }
    return GT_OK;
}
#else
/******************************************************************************
* external read from memory
*/
GT_STATUS    ddr3TipBobKExtRead
(
    GT_U32      devNum,
    GT_U32      interfaceId,
    GT_U32      regAddr,
    GT_U32      numOfBursts,
    GT_U32      *data
)
{
    GT_U32 burstNum;

    for(burstNum=0 ; burstNum < numOfBursts*8; burstNum++)
    {
        data[burstNum] = * (volatile unsigned int *) (regAddr + 4* burstNum);
    }

    return GT_OK;
}


/******************************************************************************
* external write to memory
*/
GT_STATUS    ddr3TipBobKExtWrite
(
    GT_U32      devNum,
    GT_U32      interfaceId,
    GT_U32      regAddr,
    GT_U32      numOfBursts,
    GT_U32      *data
)
{
    GT_U32 burstNum;

    for(burstNum=0 ; burstNum < numOfBursts*8; burstNum++)
    {
        *(volatile unsigned int *) (regAddr+4*burstNum) = data[burstNum];
    }

    return GT_OK;
}

#endif
/******************************************************************************/
/*   PLL/Frequency Functionality                                              */
/******************************************************************************/

/******************************************************************************
* Name:     ddr3TipBonKSetDivider.
* Desc:     Pll Divider
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS ddr3TipBobKSetDivider
(
    GT_U8                           devNum,
    GT_U32                          interfaceId,
    MV_HWS_DDR_FREQ                 frequency
)
{
    if(interfaceId < 4)
    {
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("TM set divider for interface %d\n",interfaceId));
        return ddr3TipTmSetDivider(devNum, interfaceId, frequency);
    }

    else
    {
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("CPU set divider for interface 4\n"));
        return ddr3TipCpuSetDivider(devNum, interfaceId, frequency);
    }
}

/******************************************************************************
* return 1 of core/DUNIT clock ration is 1 for given freq, 0 if clock ratios is 2:1
*/
static GT_U8 ddr3TipClockMode( GT_U32 frequency )
{
    if(frequency == DDR_FREQ_LOW_FREQ){
        return 1;
    }
    else{
        return 2;
    }
}

/******************************************************************************
* Name:     ddr3TipTmSetDivider.
* Desc:     Pll Divider of The Trafic Manager Unit
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
static GT_STATUS ddr3TipTmSetDivider
(
    GT_U8                           devNum,
    GT_U32                          interfaceId,
    MV_HWS_DDR_FREQ                 frequency
)
{
    GT_U32 data = 0, writeData, divider = 0;
    MV_HWS_DDR_FREQ sarFreq;

    DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("TM PLL Config\n"));

    /* avoid warnings */
    interfaceId = interfaceId;
    /* Calc SAR */
   /*CHECK_STATUS(ddr3TipTmGetInitFreq(devNum, &sarFreq));*/
    /* calc SAR */
    ddr3TipBobKGetInitFreq(devNum, 0, &sarFreq);
    divider = freqVal[sarFreq]/freqVal[frequency];

    DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("\tSAR value %d divider %d (freqVal[%d] %d  freqVal[%d] %d\n",
                      sarFreq, divider, sarFreq, freqVal[sarFreq], frequency, freqVal[frequency]));
    switch (divider)
    {
    case 1:
        writeData  = TM_PLL_REG_DATA(2,1,3);
        break;
    case 2:
        writeData  = TM_PLL_REG_DATA(4,2,3);
        break;
    case 3:
        writeData  = TM_PLL_REG_DATA(6,3,3);
        break;
    case 4:
        writeData  = TM_PLL_REG_DATA(8,4,3);
        break;

    default:
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR, ("Error: ddr3TipTmSetDivider: %d divider is not supported\n", divider));
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, 0x000F8264,  &data, MASK_ALL_BITS ));
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0x000F8264,  R_MOD_W(writeData,data,0xFF0C)));

    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, 0x000F8264,  &data, MASK_ALL_BITS));
    data |= (1<<16);
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0x000F8264,  data));

    /*hwsOsExactDelayPtr(devNum, 0, 10); */
    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, 0x000F8264,  &data, MASK_ALL_BITS));
    data &= ~(1<<16);
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0x000F8264,  R_MOD_W(0, data, (1<<16))));

    return GT_OK;
}

/******************************************************************************
* Name:     ddr3TipCpuSetDivider.
* Desc:     Pll Divider of the CPU(msys) Unit
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
static GT_STATUS ddr3TipCpuSetDivider
(
    GT_U8                           devNum,
    GT_U32                          interfaceId,
    MV_HWS_DDR_FREQ                 frequency
)
{
    GT_U32 data = 0, divider = 0;
#if 0
    GT_U32 value, divRatio;
#endif
    MV_HWS_DDR_FREQ sarFreq;
    GT_U32 writeData;

    DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("CPU PLL Config\n"));
#if 1
    /* calc SAR */
    ddr3TipBobKGetInitFreq(devNum, interfaceId, &sarFreq);
    divider = freqVal[sarFreq]/freqVal[frequency];

    DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("\nSAR value %d divider %d freqVal[%d] %d  freqVal[%d] %d\n",
                      sarFreq, divider, sarFreq, freqVal[sarFreq], frequency, freqVal[frequency]));

    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, 0x000F82ec,  &data, MASK_ALL_BITS ));
    writeData = (0x1 << 9);
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0x000F82ec,  R_MOD_W(writeData,data, (0x1 << 9))));

    switch (divider)
    {
        case 1:
            /*Not 800 is a 667 only*/
            writeData = (sarFreq==DDR_FREQ_800)?(0x2):(0x1);
            break;
        case 2:
            /*Not 800 is a 667 only*/
            writeData = (sarFreq==DDR_FREQ_800)?(0x3):(0x2);
            break;
        default:
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("Error: Wrong divider %d\n", divider));
            return GT_BAD_PARAM;
            break;
    }

    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, 0x000F82e8,  &data, MASK_ALL_BITS ));
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0x000F82e8,  R_MOD_W(writeData,data, (0x7 << 0))));
#else
    /* calc SAR */
    ddr3TipBobKGetInitFreq(devNum, interfaceId, &sarFreq);
    divider = freqVal[sarFreq]/freqVal[frequency];

    DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("\nSAR value %d divider %d freqVal[%d] %d  freqVal[%d] %d\n",
                      sarFreq, divider, sarFreq, freqVal[sarFreq], frequency, freqVal[frequency]));

    /* Configure Dunit to 1:1 in case of DLL off mode else 2:1*/
    value = (ddr3TipClockMode(frequency) == 1)? 0 : 1;
    CHECK_STATUS(ddr3TipBobKWrite(devNum, 0x1524, (value << 15), (1 << 15)));

    /* Dunit training clock + 1:1/2:1 mode */
    divRatio = (ddr3TipClockMode(frequency) << 16);
    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum,  0xF8298,  &data, MASK_ALL_BITS ));
    /*ddr_phy_clk_divider*/
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0xF8298,  R_MOD_W(divRatio, data, (0x3 << 16))));
    divRatio = (frequency==initFreq)?(0):(1);
    /*sel_pll_ddr_clk_div2*/
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0xF8298,  R_MOD_W( divRatio<<15, data, (0x1 << 15))));

    /*cpu_pll_clkdiv_reload_smooth*/
    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum,  0xF8270,  &data, MASK_ALL_BITS ));
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0xF8270,  R_MOD_W((/*0x7F<*/0x6f<11), data, (0x7F << 11))));

    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum,  0xF8268,  &data, MASK_ALL_BITS ));
    /*cpu_pll_clkdiv_relax_en*/
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0xF8268,  R_MOD_W(/*0x7F<*/0x6f, data, 0x7F)));
    /*cpu_pll_clkdiv_reset_mask*/
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0xF8268,  R_MOD_W((/*0x7F<*/0x6f<<7), data, (0x7F << 7))));

    /*cpu_pll_ddr_clkdiv_ratio_full*/
    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum,  0xF826C,  &data, MASK_ALL_BITS ));
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0xF826C,  R_MOD_W((divider<<12), data, (0x3F << 12))));

    /*cpu_pll_clkdiv_reload_force*/
    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum,  0xF8268,  &data, MASK_ALL_BITS ));
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0xF8268,  R_MOD_W((0x2<<21), data, (0x7F << 21))));

    /*cpu_pll_clkdiv_reload_ratio*/
    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum,  0xF8270,  &data, MASK_ALL_BITS ));
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0xF8270,  R_MOD_W((0x1<<10), data, (0x1 << 10))));
    CHECK_STATUS(ddr3TipBobKServerRegWrite(devNum, 0xF8270,  R_MOD_W(0, data, (0x1 << 10))));

    DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("\tCPU PLL config Done\n"));
#endif
    return GT_OK;
}

/******************************************************************************
* Name:     ddr3TipBobKGetInitFreq.
* Desc:     choose from where to extract the frequency (TM or CPU)
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
GT_STATUS ddr3TipBobKGetInitFreq
(
    GT_U8               devNum,
    GT_U32              interfaceId,
    MV_HWS_DDR_FREQ*    sarFreq
)
{
    GT_STATUS res;

    if(interfaceId < 4)
    {
        res = ddr3TipTmGetInitFreq(devNum, sarFreq);
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("TM frequency for interface %d is %d\n",interfaceId, *sarFreq));
    }
    else
    {
        res = ddr3TipCpuGetInitFreq(devNum, sarFreq);
    }
    return res;
}

/*****************************************************************************
TM interface frequency Get
******************************************************************************/
static GT_STATUS ddr3TipTmGetInitFreq
(
    GT_STATUS       devNum,
    MV_HWS_DDR_FREQ *freq
)
{
    GT_U32 data;

    /* calc SAR */
    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, 0x000F8204 ,  &data, MASK_ALL_BITS));
    data = (data >> 15) & 0x7;
#ifdef ASIC_SIMULATION
    data = 2;
#endif
    switch(data)
    {
    case 0:
        /* TM is disabled */
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("TM Disabled\n"));
        *freq = DDR_FREQ_LIMIT;
        return GT_NOT_INITIALIZED;

    case 1:
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("TM DDR_FREQ_800\n"));
        *freq = DDR_FREQ_800;
        break;

    case 2:
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("TM DDR_FREQ_933\n"));
        *freq = DDR_FREQ_933;
        break;

    case 3:
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("TM DDR_FREQ_667\n"));
        *freq = DDR_FREQ_667;
        break;

    default:
        DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR, ("Error: ddr3TipTmGetInitFreq: Unknown Freq SAR value 0x%x\n", data));
        *freq = DDR_FREQ_LIMIT;
        return GT_BAD_PARAM;
    }
    return GT_OK;
}

/*****************************************************************************
CPU interface frequency Get
******************************************************************************/
static GT_STATUS ddr3TipCpuGetInitFreq
(
    GT_STATUS       devNum,
    MV_HWS_DDR_FREQ *freq
)
{
    GT_U32 data;

    /* calc SAR */
    CHECK_STATUS(ddr3TipBobKServerRegRead(devNum, REG_DEVICE_SAR1_ADDR,&data, MASK_ALL_BITS ));
    DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("SAR1 is 0x%X\n", data));

    data = (data >> PLL1_CNFIG_OFFSET) & PLL1_CNFIG_MASK;

    switch(data)
    {
        case 0:
        case 5:
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("DDR_FREQ_400\n"));
            *freq = DDR_FREQ_400;
            break;
        case 1:
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("DDR_FREQ_533\n"));
            *freq = DDR_FREQ_533;
            break;
        case 2:
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("DDR_FREQ_667\n"));
            *freq = DDR_FREQ_667;
            break;
        case 3:
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("DDR_FREQ_800\n"));
            *freq = DDR_FREQ_800;
            break;
        default:
            DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_INFO, ("Freq SAR Unknown\n"));
            *freq = DDR_FREQ_LIMIT;
            return GT_BAD_PARAM;
    }
    return GT_OK;
}

/******************************************************************************
* Name:     ddr3TipBobKGetMediumFreq.
* Desc:
* Args:
* Notes:
* Returns:  GT_OK if success, other error code if fail.
*/
static GT_STATUS ddr3TipBobKGetMediumFreq
(
    GT_U32 devNum,
    GT_U32 interfaceId,
    MV_HWS_DDR_FREQ *freq
)
{
    MV_HWS_DDR_FREQ sarFreq;

      CHECK_STATUS(ddr3TipBobKGetInitFreq((GT_U8)devNum, interfaceId, &sarFreq));

      switch(sarFreq)
      {
          case DDR_FREQ_400:
              DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("No medium freq supported for 400Mhz\n"));
              *freq = DDR_FREQ_400;
              break;

          case DDR_FREQ_667:
              DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("Medium DDR_FREQ_333\n"));
              *freq = DDR_FREQ_333;
              break;

          case DDR_FREQ_800:
              DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("Medium DDR_FREQ_400\n"));
              *freq = DDR_FREQ_400;
              break;

          case DDR_FREQ_933:
              DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_TRACE, ("Medium DDR_FREQ_311\n"));
              *freq = DDR_FREQ_311;
              break;

          default:
              DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR, ("Error: ddr3TipBc2GetMediumFreq: Freq %d is not supported\n", sarFreq));
              return GT_FAIL;
      }
    return GT_OK;
}


GT_STATUS ddr3TipBobKGetDeviceInfo
(
    GT_U8      devNum,
    MV_DDR3_DEVICE_INFO * infoPtr
)
{
    devNum = devNum; /* avoid warnings */

    infoPtr->deviceId = 0xFC00;
    infoPtr->ckDelay = ckDelay;

    return GT_OK;
}


