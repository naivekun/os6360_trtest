/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
********************************************************************************
*/
/**
********************************************************************************
* @file sample.c
*
* @brief This file contains sample functions code for driver
*  initialization and demo use of other MCD API's
*   For Reference Only.
*
***********************************************************************/

#ifndef CHX_FAMILY
#define _GNU_SOURCE
#endif

#include "../src/mcdApiTypes.h"
#include "../src/mcdAPI.h"
#include "../src/mcdUtils.h"
#include "../src/mcdFwDownload.h"
#include "../src/mcdInitialization.h"
#include "../src/mcdHwSerdesCntl.h"
#include "../src/mcdHwCntl.h"
#include "../src/mcdAPIInternal.h"
#include "../src/mcdRsFEC.h"

#ifndef ASIC_SIMULATION
#include "../src/mcdServiceCpuCm3BvFw.h"
#endif
#include "../src/mcdInternalCtrlApInitIf.h"
#include "../src/mcdInternalIpcApis.h"
#include "mcd5123Sim.h"


#ifdef CHX_FAMILY

#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsTimer.h>
#include <gtExtDrv/drivers/gtSmiDrvCtrl.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>
#include "stdarg.h"

#define MCD_MALLOC  osMalloc
#define MCD_FREE    osFree
#define MCD_PRINTF  osPrintf
#define MCD_SLEEP   osTimerWkAfter

#else
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#ifdef _WIN32
#include <Windows.h>
#define MCD_SLEEP  Sleep

unsigned int osDelay
(
    unsigned int delay
)
{
  return 0;
}

#else /* !_WIN32 */

#include <unistd.h>
#include <time.h>
#include <sys/times.h>

#define MCD_SLEEP   usleep


unsigned int osDelay
(
    unsigned int delay /* in nanoseconds */
)
{
  struct timespec sleep, remain ;
  if (1000000000 < delay)
  {
     /* sleep time to big */
    return 4;
  }

  sleep.tv_nsec = delay;
  sleep.tv_sec  = 0 ;
  if ( nanosleep(&sleep, &remain) )
  {
    return 1;
  }

  return 0;
}

#endif /* _WIN32 */


/*****************************/
/*****   DEFINE SECTION  *****/
/*****************************/

#define MCD_MALLOC  malloc
#define MCD_FREE    free
#define MCD_PRINTF  printf
#define GT_U32      MCD_U32 /* for compatibility with extDrv */



/*******************************/
/*****   Extern Functions  *****/
/*******************************/

unsigned int extDrvXSmiInit
(
    void
);

unsigned int extDrvXSmiReadReg
(
    IN  MCD_U32  phyId,
    IN  MCD_U32  devAddr,
    IN  MCD_U32  regAddr,
    OUT MCD_U32 *dataPtr
);

unsigned int extDrvXSmiWriteReg
(
    IN MCD_U32 phyId,
    IN MCD_U32 devAddr,
    IN MCD_U32 regAddr,
    IN MCD_U32 value
);

unsigned int extDrvI2cRead(
    IN  MCD_PVOID drv,
    IN  MCD_U8    bus_id,
    IN  MCD_U8    slave_address,
    IN  MCD_U32   offset_type,
    IN  MCD_U32   offset,
    IN  MCD_U32   buffer_size,
    OUT MCD_U8   *buffer
);

unsigned int extDrvI2cWrite(
    IN MCD_PVOID drv,
    IN MCD_U8    bus_id,
    IN MCD_U8    slave_address,
    IN MCD_U32   offset_type,
    IN MCD_U32   offset,
    IN MCD_U32   buffer_size,
    IN MCD_U8   *buffer
);

MCD_PVOID extDrvI2cCreateDrv(
    IN MCD_PVOID parent
);

#endif /* MCD_RUN_WITH_CPSS */

/* List of MCD_OP_MODE */
static const MCD_8* portModeList[] =   {"UNKNOWN    ", "P100_40_C  ", "P100L      ", "P100S      ", "P100C      ",
                                        "P100K      ", "P40L       ", "P40C       ", "P40K       ", "P100_40_K  ",
                                        "R100L      ", "R100C      ", "R25L       ", "R25C       ", "BM21L      ",
                                        "BM21C      ", "R10L       ", "R10K       ", "R40L       ", "R40C       ",
                                        "G10L       ", "G10S       ", "G10C       ", "G10K       ", "G14L       ",
                                        "G14S       ", "G14C       ", "G14K       ", "G41L       ", "G41S       ",
                                        "G41C       ", "G41K       ", "G44L       ", "G44S       ", "G44C       ",
                                        "G44K       ", "G100L      ", "G100S      ", "G100C      ", "G100K      ",
                                        "P50R2L     ", "P50R2S     ", "P40R2L     ", "P40R2S     ", "P25L       ",
                                        "P25S       ", "P10L       ", "P10S       ", "P2_5       ", "P1         ",
                                        "G21L       ", "P40S       ", "P2_5S      ", "P1_SGMII   ", "R1L        ",
                                        "R1C        ", "P1_Bx_SGMII", "G21SK      ", "MODE_NUM   "};

/* List of config status */
static const MCD_8* configStsList[] =  {"FALSE", "TRUE "};

/* List of autoNegAdvEnable */
static const MCD_8* anEnList[] =       {"NO  ", "YES "};

/* List calibration mode */
static const char* calibrationList[] = {"DEFAULT_CALIBRATION", "OPTICAL_CALIBRATION", "CALIBRATION_W/O_DFE", "REDUCE_CALIBRATION"};

/* List of MCD_FEC_TYPE */
static const MCD_8* fecTypeList[] =    {"NO_FEC     ", "RS_FEC     ", "FC_FEC     ", "RS_FEC_HOST", "FC_FEC_HOST",
                                        "RS_FEC_LINE", "NA         ", "FC_FEC_LINE", "NA         "};

/* List of MCD_NO_PPM_MODE */
static const MCD_8* noPpmList[] =      {"OFF_MODE       ", "NORMAL_MODE    ", "RX_SQUELCH_MODE", "SEND_FAULT_MODE"};

/* List of Mode vector */
static char*  mcdSampleConvertModevector2String
(
    IN MCD_U32  modeVector
)
{
    switch (modeVector)
    {
        case 0x0001:
            return "1000B_KX        ";
        case 0x0002:
            return "10GB_KX4        ";
        case 0x0004:
            return "10GB_KR         ";
        case 0x0008:
            return "40GB_KR4        ";
        case 0x0010:
            return "40GB_CR4        ";
        case 0x0020:
            return "100GB_CR10      ";
        case 0x0040:
            return "100GB_KP4       ";
        case 0x0080:
            return "100GB_KR4       ";
        case 0x0100:
            return "100GB_CR4       ";
        case 0x0200:
            return "25GB_KR1S       ";
        case 0x0400:
            return "25GB_KCR        ";
        case 0x0800:
            return "25GB_KR1_CONSRTM";
        case 0x1000:
            return "25GB_CR1_CONSRTM";
        case 0x2000:
            return "50GB_KR2_CONSRTM";
        case 0x4000:
            return "50GB_CR2_CONSRTM";
        default:
            return "NONE            ";
    }
}


int mcdSerdesAacsServerExec(MCD_DEV_PTR pDev);

MCD_STATUS mcdApPortCtrlStatusShow
(
    MCD_DEV_PTR             pDev,
    MCD_U32  apPortNum
);

/*****************************/
/*****   ENUM   SECTION  *****/
/*****************************/



/***********************************/
/*****   STRUCTURES   SECTION  *****/
/***********************************/

typedef struct {
    MCD_BOOL  hwPhyConnected;
} MCD_XSMI_CONNECT_STC;



/******************************/
/*****   GLOBALS SECTION  *****/
/******************************/
/**
* @enum MCD_PORT_OPERATION_MODE_ENT
 *
 * @brief operation mode of a port: PCS, RITEMER, etc.
*/
typedef enum{
    /** @brief Operation mode does not matter.
     */
    MCD_PORT_AGNOSTIC_OPERATION_MODE_E,
    /** @brief PCS Operation mode.
        */
    MCD_PORT_PCS_OPERATION_MODE_E,
    /** @brief Retimer Operation mode with single lane on port.
        */
    MCD_PORT_RETIMER_SINGLE_LANE_OPERATION_MODE_E,
    /** @brief Retimer Operation mode with multiple lanes on port.
        */
    MCD_PORT_RETIMER_MULTI_LANE_OPERATION_MODE_E
} MCD_PORT_OPERATION_MODE_ENT;


/*#ifdef MCD_RUN_WITH_CPSS*/
/*#define SERDES_FW_DECLARATION_ONLY*/
/*#endif*/
MCD_SERDES_CONFIG_DATA serdesElectricalParamsMv[16]=
{
   { 0, 0, 10, 0, 0, 31}, /* bitMapEnable=31, means override rxPolarity, txPolarity, preCursor, attenuation, postCursor */
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31},
   { 0, 0, 10, 0, 0, 31}

/* MCD_U8 bitMapEnable; bit position description:
    rxPolarity = 0x01;
    txPolartiy = 0x02;
    preCursor  = 0x04;
    attenuation= 0x08;
    postCursor = 0x10;

   Example 1: To override rxPolarity alone, set bitMapEnable = 1 (in hex, bitMapEnable = 0x1);
   Example 2: To override rxPolarity and txPolarity alone, set bitMapEnable = 3 (in hex, bitMapEnable = 0x3);
   Example 3: To override preCursor, attenuation, postCursor, set bitMapEnable = 28 (in hex, bitMapEnable = 0x1C);
   Example 4: To override all parameter under MCD_SERDES_CONFIG_DATA, set bitMapEnable = 31 (in hex, bitMapEnable = 0x1F);
*/
};

/* the values of the matrix are examples of how to
 initialize the matrix, they should be modifed according
 to the SerDes baud rate */
MCD_PRE_DEFINED_CTLE_DATA serdesCtleParamsMv[16]=
{
   { 99, 4, 14, 12, 1, 31}, /* override DC,LF,HF,BW,LB*/
   { 99, 4, 14, 12, 1, 15}, /* override DC,LF,HF,BW */
   { 99, 4, 14, 12, 1, 7},  /* override DC,LF,HF */
   { 99, 4, 14, 12, 1, 3},  /* override DC,LF */
   { 99, 4, 14, 12, 1, 1},  /* override DC */
   { 100, 3, 15, 13, 0, 0}, /* override nothing */
   { 99, 4, 14, 12, 1, 31},
   { 99, 4, 14, 12, 1, 31},
   { 99, 4, 14, 12, 1, 31},
   { 99, 4, 14, 12, 1, 31},
   { 99, 4, 14, 12, 1, 0},
   { 99, 4, 14, 12, 1, 1},
   { 99, 4, 14, 12, 1, 3},
   { 99, 4, 14, 12, 1, 7},
   { 99, 4, 14, 12, 1, 15},
   { 99, 4, 14, 12, 1, 31},
};

MCD_SERDES_TXRX_LANE_REMAP serdesTxRxLaneRemapMv[8][2] =
{
   { {3, 3}, {3, 3}},     /* Line port mapping - remap0 */
   { {2, 2}, {2, 2}},     /* Line port mapping - remap1 */
   { {1, 1}, {1, 1}},     /* Line port mapping - remap2 */
   { {0, 0}, {0, 0}},     /* Line port mapping - remap3 */
   { {3, 3}, {3, 3}},     /* Host port mapping - remap0 */
   { {2, 2}, {2, 2}},     /* Host port mapping - remap1 */
   { {1, 1}, {1, 1}},     /* Host port mapping - remap2 */
   { {0, 0}, {0, 0}}      /* Host port mapping - remap3 */
};

MCD_LANE_STEERING_OVERRIDE_CFG laneSteeringOverrideMv[MCD_MAX_PORT_NUM] =
{
    { MCD_LANE_STEERING_NO_LPB, MCD_LANE_STEERING_NO_LPB, 9, 8},    /* port0, PCS8 */
    { MCD_LANE_STEERING_NO_LPB, MCD_LANE_STEERING_NO_LPB, 10, 11},    /* port1, PCS9 */
    { 15, 14, 8, 9},    /* port2, PCS10 */
    { 12, 13, 11, 10},    /*port3, PCS11 */
    { MCD_LANE_STEERING_NO_LPB, MCD_LANE_STEERING_NO_LPB, 13, 12},    /* port4, PCS12 */
    { MCD_LANE_STEERING_NO_LPB,MCD_LANE_STEERING_NO_LPB , 14, 15},    /* port5, PCS13 */
    { 11, 10, 12, 13},    /* port6, PCS14 */
    { 8, 9, 15, 14}     /* port7, PCS15 */
};

#define AVAGO_FW_SWAP_IMAGE_EXIST
#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 && !defined ASIC_SIMULATION
/*================================================*/
#include "../src/mcd_serdes.0x2464_0245.h"
#include "../src/mcd_sbus_master.0x101A_0001.h"
#include "../src/mcd_serdes.0x2464_0245swap.h"
/*================================================*/

extern  const short *serdesFwPtr;
extern  const short *sbusMasterFwPtr;
extern  const short *serdesFwDataSwapPtr;
#endif /* !defined MV_HWS_REDUCED_BUILD_EXT_CM3 */

static MCD_PVOID i2cDrv;


#ifndef ASIC_SIMULATION
const MCD_FIRMWARE_CONFIG  fwConfigInfo = { mcdServiceCpuCm3BvFw_INFO};
#endif

MCD_U16   *mcdHwSerdesCntlBusMasterImage     = NULL;
MCD_U16  mcdHwSerdesCntlBusMasterImageSize        = 0;
MCD_U16   *mcdHwSerdesCntlBusMasterSwapImage = NULL;
MCD_U16  mcdHwSerdesCntlBusMasterSwapImageSize    = 0;
MCD_U16   *mcdHwSerdesCntlSerdesImage        = NULL;
MCD_U16  mcdHwSerdesCntlSerdesImageSize           = 0;

/********************************/
/*****   FUNCTIONS SECTION  *****/
/********************************/

/*******************************************************************************
* mcdHwSerdesCntlInitEmbededFwLoadData
*
* DESCRIPTION:
*
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
void mcdHwSerdesCntlInitEmbededFwLoadData()
{
#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 && !defined ASIC_SIMULATION/**/
    mcdHwSerdesCntlBusMasterImage           = (MCD_U16  *)mcd_sbusMaster01x101A_0001Data/*sbusMasterFwPtr*/;
    mcdHwSerdesCntlBusMasterImageSize       = AVAGO_SBUS_MASTER_FW_IMAGE_SIZE;
    mcdHwSerdesCntlBusMasterSwapImage       = (MCD_U16  *)mcd_serdes0x2464_0245Swap/*serdesFwDataSwapPtr*/;
    mcdHwSerdesCntlBusMasterSwapImageSize   = AVAGO_SERDES_FW_SWAP_IMAGE_SIZE;
    mcdHwSerdesCntlSerdesImage              = (MCD_U16  *)mcd_serdes0x2464_0245Data/*serdesFwPtr*/;
    mcdHwSerdesCntlSerdesImageSize          = AVAGO_SERDES_FW_IMAGE_SIZE;
#endif /*!defined MV_HWS_REDUCED_BUILD_EXT_CM3 && !defined ASIC_SIMULATION */
}



/* MACRO to fix warnings */
#define UNUSED_PARAM(x) x = x

/* only 16 device may be on XSMI bus */
#define MCD_MAX_DEVICES 16

/* 32 PHY devices on 2 XSMI buses - 16 per bus  */
#define MCD_MAX_PHY     32

/* Device handlers DB */
static MCD_DEV_PTR  pDevArray[MCD_MAX_DEVICES] = {NULL};

/* DB per XSMI PHY address */
static MCD_XSMI_CONNECT_STC x5123HwPhyInfo[MCD_MAX_PHY];

/* SMI/XSMI access tracing constants and flag */
/* there is no print outs */
#define MCD_DEBUG_SMI_OFF 0

/* print out indirect access transactions only */
#define MCD_DEBUG_SMI_ON 1
/* print out SMI/XSMI bus access and indirect access transactions */
#define MCD_DEBUG_SMI_DETAILS 2
/* XSMI subsystem init state */
MCD_BOOL mcdGlobalXSmiInitDone = MCD_FALSE;

static int mcdDebugSmi = MCD_DEBUG_SMI_OFF;


/*******************************************************************************
 mcdDebugSmiSet

 Inputs:
   newMode - SMI/XSMI tracing print-outs mode
             (MCD_DEBUG_SMI_OFF, MCD_DEBUG_SMI_ON, MCD_DEBUG_SMI_DETAILS)

 Outputs:
       none

 Returns:
       old mode

 Description:
   Set SMI/XSMI tracing print-outs mode

 Notes/Warnings:

*******************************************************************************/
int mcdDebugSmiSet(int newMode)
{
    int oldMode = mcdDebugSmi;
    mcdDebugSmi = newMode;
    return oldMode;
}



MCD_STATUS mcdSampleSerdesGetEom
(
    IN  MCD_U32                           phyId,
    IN  MCD_U16                           serdesNum,
    IN  MCD_BOOL                          noeye,
    IN  MCD_U32                           min_dwell_bits,
    IN  MCD_U32                           max_dwell_bits
);

/* X5123 and EC808 device simulation */

/* Slave SMI register number */
#define MCD_SMI_REGS_NUM  32

#define MCD_X5123_SIM_RUNIT_MAX_REG (1 + (0xFFFC / 4)) /* maximal address is 0xFFFC, baseaddress 0x00010000 */
#define MCD_X5123_SIM_DFX_MAX_REG   (1 + (0xC9C / 4))  /* maximal address is 0xC9C,  baseaddress 0x800F8000 */
#define MCD_X5123_SIM_UMAC_MAX      (2)          /* there are 2 UMAC instances */
#define MCD_X5123_SIM_UMAC_MAX_REG  (1 + (0xD364 / 4))  /* maximal address is 0xD364, baseaddress 0x00020000, 0x00040000 */
#define MCD_X5123_SIM_MG_MAX_REG    (1 + (0xAFFFF / 4))    /* maximal address is 0xAFFFF,  baseaddress 0x30000000 */
#define MCD_X5123_SIM_AP_MAX_REG    (1 + (0x1FFC / 4))    /* maximal address is 0x1FFC,  baseaddress 0x31180000 */
#define MCD_X5123_SIM_SBC_MAX_REG    (1 + (0x7FBFC / 4))    /* maximal address is 0x7FBFC,  baseaddress 0x33000000 */
#define MCD_X5123_SIM_SERDES_MAX_REG (1 + (0x8FFFC / 4))   /* maximal address is 0x8FFFC,  baseaddress 0x32000000 */


typedef struct{
    /* DB to hold Slave SMI registers values */
    MCD_U16  s_smi_reg_db[MCD_SMI_REGS_NUM];
    MCD_U32  serverStatusReadCount;
    MCD_U32  runit_regs[MCD_X5123_SIM_RUNIT_MAX_REG];
    MCD_U32  dfx_unit_regs[MCD_X5123_SIM_DFX_MAX_REG];
    MCD_U32  umac_unit_regs[MCD_X5123_SIM_UMAC_MAX][MCD_X5123_SIM_UMAC_MAX_REG];
    MCD_U32  mg_unit_regs[MCD_X5123_SIM_MG_MAX_REG];
    MCD_U32  ap_unit_regs[MCD_X5123_SIM_AP_MAX_REG];
    MCD_U32  sbc_unit_regs[MCD_X5123_SIM_SBC_MAX_REG];
    MCD_U32  serdes_unit_regs[MCD_X5123_SIM_SERDES_MAX_REG];
} MCD_X5123_SIM_DEV;

/* Device Simulations handlers DB */
static MCD_X5123_SIM_DEV  *pSimDevArray[MCD_MAX_DEVICES] = {NULL};

typedef struct{
    MCD_U32  regAddr;
    MCD_U32  regData;
} MCD_X5123_SIM_DEFAULT_REG_STC;

static MCD_X5123_SIM_DEFAULT_REG_STC x5123UmacDefaults[] = MCD_5123_UMAC_DEFAULT;
static MCD_X5123_SIM_DEFAULT_REG_STC x5123SbcDefaults[] = MCD_5123_SBC_DEFAULT;

/*******************************************************************************
 mcd5123SimInit

 Inputs:
   phyAddrBitmap - bitmap of PHYADDR (address of PHY on SMI/XSMI bus) of 5123 devices.
                   only even bits may be set (0, 2, 4, e.t.c)
   simTraceMode - SMI/XSMI tracing print-outs mode
             (MCD_DEBUG_SMI_OFF, MCD_DEBUG_SMI_ON, MCD_DEBUG_SMI_DETAILS)

 Outputs:
       none

 Returns:
       MCD_OK - on success
       MCD_FAIL - on wrong parameters or other errors

 Description:
   Create simulated X5123 and EC808 devices according to phyAddrBitmap.
   Up to 15 devices may be created.
   Function sets simulation to default state if it was already created.
   Set SMI/XSMI tracing print-outs mode

 Notes/Warnings:

*******************************************************************************/
MCD_STATUS mcd5123SimInit
(
    IN MCD_U32 phyAddrBitmap,
    IN int     simTraceMode
)
{
    MCD_U32 phyAddr, simDbIdx;
    MCD_X5123_SIM_DEV  * pSimDev;
    MCD_U32  defRegsArraySize, ii, regIdx;

    /* check bitmap - only even bits must be set */
    if (phyAddrBitmap & 0xAAAAAAAA)
    {
        MCD_PRINTF("mcd5123SimInit: wrong phyAddrBitmap, odd bits are set 0x%X\n", phyAddrBitmap);
        return MCD_FAIL;
    }

    MCD_PRINTF("mcd5123SimInit: check porting sizeof(MCD_U32) = %d sizeof(MCD_U16) = %d\n", sizeof(MCD_U32), sizeof(MCD_U16));

    for (phyAddr = 0; phyAddr < 32; )
    {
        if (phyAddrBitmap & (1 << phyAddr))
        {
            /* init simulated device */
            simDbIdx = phyAddr / 2;
            pSimDev = pSimDevArray[simDbIdx];

            if (pSimDev == NULL)
            {
                /* allocate memory and init simulation */
                pSimDevArray[simDbIdx] = (MCD_X5123_SIM_DEV*)MCD_MALLOC(sizeof(MCD_X5123_SIM_DEV));
                if (pSimDevArray[simDbIdx] == NULL)
                {
                    MCD_PRINTF("mcd5123SimInit: FAIL out of CPU memory\n");
                    return MCD_FAIL;
                }

                MCD_PRINTF("X5123 Simulation: create device on PHYADDR %d\n", phyAddr);
            }
            else
            {
                MCD_PRINTF("X5123 Simulation: re-init of device on PHYADDR %d\n", phyAddr);
            }

            /* reset memory */
            mcdMemSet(pSimDevArray[simDbIdx], 0, sizeof(MCD_X5123_SIM_DEV));

            /* set default values of  SMI registers */
            pSimDevArray[simDbIdx]->s_smi_reg_db[2] = pSimDevArray[simDbIdx]->s_smi_reg_db[0xF] = 0x0141;
            pSimDevArray[simDbIdx]->s_smi_reg_db[3] = 0x0FF0;

            /* set default values for MG registers */
            pSimDevArray[simDbIdx]->mg_unit_regs[(0x4c >> 2)] = (MCD_U32)(0x0328 << 4);

            /* set defaults of UMAC units */
            defRegsArraySize = sizeof(x5123UmacDefaults) / sizeof(x5123UmacDefaults[0]);
            if (defRegsArraySize >= MCD_X5123_SIM_UMAC_MAX_REG)
            {
                MCD_PRINTF("mcd5123SimInit: UMAC Default regs array size[%d] more than DB[%d]\n", defRegsArraySize, MCD_X5123_SIM_UMAC_MAX_REG);
                return MCD_FAIL;
            }

            for (ii = 0; ii < defRegsArraySize; ii++)
            {
                /* the DB use HW spec address of unit with baseaddress.
                   Remove baseaddress to calculate register's index in DB.
                   Address step is 4. Delete DB value to 4 to get index. */
                regIdx = (x5123UmacDefaults[ii].regAddr & 0xFFFF) >> 2;

                pSimDevArray[simDbIdx]->umac_unit_regs[0][regIdx] =
                pSimDevArray[simDbIdx]->umac_unit_regs[1][regIdx] = x5123UmacDefaults[ii].regData;
            }


            /* set defaults of SBC unit */
            defRegsArraySize = sizeof(x5123SbcDefaults) / sizeof(x5123SbcDefaults[0]);
            if (defRegsArraySize >= MCD_X5123_SIM_SBC_MAX_REG)
            {
                MCD_PRINTF("mcd5123SimInit: SBC Default regs array size[%d] more than DB[%d]\n", defRegsArraySize, MCD_X5123_SIM_SBC_MAX_REG);
                return MCD_FAIL;
            }

            for (ii = 0; ii < defRegsArraySize; ii++)
            {
                /* the DB use HW spec address of unit with baseaddress.
                   Remove baseaddress to calculate register's index in DB.
                   Address step is 4. Delete DB value to 4 to get index. */
                regIdx = (x5123SbcDefaults[ii].regAddr & 0xFFFFF) >> 2;

                pSimDevArray[simDbIdx]->sbc_unit_regs[regIdx] = x5123SbcDefaults[ii].regData;
            }
        }

        phyAddr += 2;
    }
    mcdDebugSmiSet(simTraceMode);
    return MCD_OK;
}

/* get pointer to simulation device
   allocate and init memory for device */
static MCD_X5123_SIM_DEV  * x5123SimDevGet
(
    IN MCD_U16 mdioPort
)
{
    MCD_U32  simDbIdx =  mdioPort / 2; /* each device has 2 SMI ids */
    MCD_X5123_SIM_DEV  * pSimDev;

    if (simDbIdx >= MCD_MAX_DEVICES)
    {
        return NULL;
    }

    pSimDev = pSimDevArray[simDbIdx];
    if (pSimDev == NULL)
    {
        if (MCD_OK != mcd5123SimInit((1 << mdioPort), mcdDebugSmi))
        {
            return NULL;
        }

        pSimDev = pSimDevArray[simDbIdx];
    }

    return pSimDev;
}

/* X5123 and EC808 SMI Slave simulation. Device has only 13 SMI registers. */
/* And it implement indirect access to all registers by them.              */
#ifdef ASIC_SIMULATION
static int mcdSmiSimulEnable = 1;
#else
static int mcdSmiSimulEnable = 0;
#endif

int mcdSmiSimulEnableSet(int newMode)
{
    int old = mcdSmiSimulEnable;
    mcdSmiSimulEnable = newMode;
    return old;
}

/* simulation of Slave SMI read transaction */
static void readSsmi(
    IN  MCD_X5123_SIM_DEV  *pSimDev,
    IN  MCD_U16             smiReg,
    OUT MCD_U16            *valuePtr
)
{
    switch (smiReg)
    {
        /* readable registers */
        case 2:
        case 3:
        case 6:
        case 7:
        case 0xA:
        case 0xF:
        case 0x1F:
            *valuePtr = pSimDev->s_smi_reg_db[smiReg];
            break;
        /* other register are write only or does not exist */
        default: *valuePtr = 0; break;
    }
    return;
}

/* check full address got by indirect transaction */
static MCD_STATUS  ssmiCheckFullAdd
(
    IN  MCD_X5123_SIM_DEV   *pSimDev,
    IN  MCD_U32              fullAddress,
    OUT MCD_U32            **simRegPtrPtr
)
{
    MCD_U32 regIdx;

    *simRegPtrPtr = NULL;

    if (((fullAddress >= 0x00020000) && (fullAddress <= 0x0002FFFF))  ||
        ((fullAddress >= 0x00040000) && (fullAddress <= 0x0004FFFF))  ||
        ((fullAddress >= 0x00010000) && (fullAddress <= 0x0001FFFF)))
    {
        /* internal access units bits 0:1 of address must be 0 */
        if (fullAddress & 0x3)
        {
            MCD_PRINTF("SSMI Address Check: wrong bits 0:1, must be 0 for internal access address 0x%08X\n", fullAddress);
            return MCD_FAIL;
        }

        regIdx = (fullAddress & 0xFFFF) / 4;

        if(fullAddress >= 0x00040000)
        {
            if (regIdx >= MCD_X5123_SIM_UMAC_MAX_REG)
            {
                MCD_PRINTF("SSMI Address Check: UMAC 1 address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            *simRegPtrPtr = &(pSimDev->umac_unit_regs[1][regIdx]);
        }
        else if(fullAddress >= 0x00020000)
        {
            if (regIdx >= MCD_X5123_SIM_UMAC_MAX_REG)
            {
                MCD_PRINTF("SSMI Address Check: UMAC 0 address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            *simRegPtrPtr = &(pSimDev->umac_unit_regs[0][regIdx]);
        }
        else
        {
            if (regIdx >= MCD_X5123_SIM_RUNIT_MAX_REG)
            {
                MCD_PRINTF("SSMI Address Check: RUNIT address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            *simRegPtrPtr = &(pSimDev->runit_regs[regIdx]);
        }

        return MCD_OK;
    }
    else if (((fullAddress >= 0x80000000) && (fullAddress <= 0x800FFFFF))  ||
             ((fullAddress >= 0x30000000) && (fullAddress <= 0x30FFFFFF))  ||
             ((fullAddress >= 0x31000000) && (fullAddress <= 0x31FFFFFF))  ||
             ((fullAddress >= 0x32000000) && (fullAddress <= 0x32FFFFFF))  ||
             ((fullAddress >= 0x33000000) && (fullAddress <= 0x33FFFFFF)))
    {
        /* external access units bits 0:1 of address must be 1 */
        if ((fullAddress & 0x3) != 2)
        {
            MCD_PRINTF("SSMI Address Check: wrong bit 1, must be 1 for external access address 0x%08X\n", fullAddress);
            return MCD_FAIL;
        }

        if(fullAddress >= 0x80000000)
        {
            regIdx = (fullAddress - 0x800F8000) / 4;

            if (regIdx >= MCD_X5123_SIM_DFX_MAX_REG)
            {
                MCD_PRINTF("SSMI Address Check: DFX address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            *simRegPtrPtr = &(pSimDev->dfx_unit_regs[regIdx]);
        }
        else if (fullAddress >= 0x33000000)
        {
            if ((fullAddress >= 0x33000100) && (fullAddress < 0x33040400))
            {
                MCD_PRINTF("SSMI Address Check: SBC unit address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            if ((fullAddress >= 0x33044800) && (fullAddress < 0x3307F400))
            {
                MCD_PRINTF("SSMI Address Check: SBC Client address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            regIdx = (fullAddress - 0x33000000) / 4;
            if (regIdx >= MCD_X5123_SIM_SBC_MAX_REG)
            {
                MCD_PRINTF("SSMI Address Check: SBC address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            *simRegPtrPtr = &(pSimDev->sbc_unit_regs[regIdx]);
        }
        else if (fullAddress >= 0x32000000)
        {
            regIdx = (fullAddress - 0x32000000) / 4;
            if (regIdx >= MCD_X5123_SIM_SERDES_MAX_REG)
            {
                MCD_PRINTF("SSMI Address Check: SERDES address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            *simRegPtrPtr = &(pSimDev->serdes_unit_regs[regIdx]);
        }
        else if (fullAddress >= 0x31000000)
        {
            regIdx = (fullAddress - 0x31180000) / 4;
            if (regIdx >= MCD_X5123_SIM_AP_MAX_REG)
            {
                MCD_PRINTF("SSMI Address Check: AP address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            *simRegPtrPtr = &(pSimDev->ap_unit_regs[regIdx]);
        }
        else
        {
            regIdx = (fullAddress - 0x30000000) / 4;
            if (regIdx >= MCD_X5123_SIM_MG_MAX_REG)
            {
                MCD_PRINTF("SSMI Address Check: MG address out of range 0x%08X\n", fullAddress);
                return MCD_FAIL;
            }

            *simRegPtrPtr = &(pSimDev->mg_unit_regs[regIdx]);
        }

        return MCD_OK;
    }

    MCD_PRINTF("SSMI Address Check: out of ranges address 0x%08X\n", fullAddress);
    return MCD_FAIL;
}

/* simulation of Slave SMI write transaction to trigger register for indirect write */
static void writeSsmiIndirecWriteTrigger
(
    IN MCD_X5123_SIM_DEV  *pSimDev
)
{
    MCD_U32 fullAddress;
    MCD_U32 *simRegPtr;
    MCD_U32 fullData;
    MCD_STATUS status;

    fullAddress = pSimDev->s_smi_reg_db[1] | (pSimDev->s_smi_reg_db[0] << 16);
    fullData    = pSimDev->s_smi_reg_db[9] | (pSimDev->s_smi_reg_db[8] << 16);

    status = ssmiCheckFullAdd(pSimDev, fullAddress, &simRegPtr);
    if ((status != MCD_OK) || (NULL == simRegPtr))
    {
        MCD_PRINTF("SSMI Indirect Write: fail on wrong address 0x%08X\n", fullAddress);
    }
    else
    {
        fullAddress &= 0xFFFFFFFC;
        if (mcdDebugSmi >= MCD_DEBUG_SMI_ON)
        {
            MCD_PRINTF("SSMI Indirect Write: address 0x%08X, data 0x%08X\n", fullAddress, fullData);
        }

        /* store value in simulation's memory */
        *simRegPtr = fullData;
    }

    return;
}

/* simulation of Slave SMI write transaction to trigger register for indirect read */
static void writeSsmiIndirecReadTrigger
(
    IN MCD_X5123_SIM_DEV  *pSimDev
)
{
    MCD_U32 fullAddress;
    MCD_U32 offset;
    MCD_U32 interruptCode;
    MCD_U32 *simRegPtr;
    MCD_U32 fullData = 0;
    MCD_STATUS status;

    fullAddress = (MCD_U32)pSimDev->s_smi_reg_db[5] | ((MCD_U32)pSimDev->s_smi_reg_db[4] << 16);

    status = ssmiCheckFullAdd(pSimDev, fullAddress, &simRegPtr);
    if ((status != MCD_OK) || (NULL == simRegPtr))
    {
        MCD_PRINTF("SSMI Indirect Read: fail on wrong address 0x%08X\n", fullAddress);
    }
    else
    {
        fullAddress &= 0xFFFFFFFC;

        /* take care special case */
        if (fullAddress == 0x800F8010)
        {
            /* check that driver do pooling of the bit at least 5 times */
            if (pSimDev->serverStatusReadCount++ > 5)
            {
                /* device is ready */
                fullData = 0x3F;
            }
        }
        else if ((fullAddress >= 0x33040400) && (fullAddress <= 0x33044400))
        {
            /* catch SPICO Interrupts OUT Data read request */
            offset = fullAddress & 0x3FF;
            if (offset == 0x010)
            {
                interruptCode = (*(simRegPtr - 1)) >> 16;

                if (interruptCode == 0x3C)
                {
                    /* FW CRC check should return 0 - CRC OK */
                    fullData = 0;
                }
                else if (interruptCode == 0x00004027)
                    /* delete training in progress bit*/
                    fullData = 0x00004025;
                else
                {
                    /* 16 LSB of Interrupt status is 16 MSB of
                       previose INTERRUPT_ISSUE register in most of cases */
                    fullData = interruptCode;
                }
            }
            else
            {

                /* get data of simulated register */
                fullData = *simRegPtr;
            }
        }
        else if (fullAddress == 0x3009FC34)
        {
            /* IPC message ready */
            fullData = 1;
        }
        else if (fullAddress == 0x3009FC38)
        {
            /* data size */
            fullData = 0x00000030;
        }
        else if (fullAddress == 0x3009FC40)
        {
            int position;
            position = 77 - ((*(simRegPtr - 0x51)) * 18);

            /* message type  */
            fullData = 0xFF & ((*(simRegPtr - position)) >> 8);
            if (fullData == 0xB)
            {
                /* AAPL data */
                *(simRegPtr + 1) = 0x0001A650;
                *(simRegPtr + 2) = 0x000012B8;
            }
            (*(simRegPtr - 0x51))++;
        }
        else
        {
            /* get data of simulated register */
            fullData = *simRegPtr;
        }

        if (mcdDebugSmi >= MCD_DEBUG_SMI_ON)
        {
            MCD_PRINTF("SSMI Indirect Read: address 0x%08X, data 0x%08X\n", fullAddress, fullData);
        }

        pSimDev->s_smi_reg_db[7] = (MCD_U16)(fullData & 0xFFFF);
        pSimDev->s_smi_reg_db[6] = (MCD_U16)((fullData >> 16) & 0xFFFF);
    }

    return;
}

/* simulation of Slave SMI write transaction */
static void writeSsmi(
    IN MCD_X5123_SIM_DEV  *pSimDev,
    IN MCD_U16             smiReg,
    IN MCD_U16             value
)
{
    switch (smiReg)
    {
        /* writable registers */
        case 0:
        case 1:
        case 4:
        case 5:
        case 8:
        case 9:
        case 0xA:
            if ((smiReg == 0) || (smiReg == 4))
            {
                /* same memory is used to store address */
                pSimDev->s_smi_reg_db[0] = value;
                pSimDev->s_smi_reg_db[4] = value;
            }
            else if((smiReg == 1) || (smiReg == 5))
            {
                /* same memory is used to store address */
                pSimDev->s_smi_reg_db[1] = value;
                pSimDev->s_smi_reg_db[5] = value;
            }
            else
            {
                pSimDev->s_smi_reg_db[smiReg] = value;
            }

            if (smiReg == 9)
            {
                writeSsmiIndirecWriteTrigger(pSimDev);
            }
            else if (smiReg == 5)
            {
                writeSsmiIndirecReadTrigger(pSimDev);
            }

            break;
        /* other register are read only or does not exist */
        default: break;
    }
    return;
}

static MCD_U32 mcdSampleUsePpForXsmi = 0;

#ifdef CHX_FAMILY
static MCD_U8 mcdSampleUsePpDevNum = 0;
#endif

/* MCD callback for XSMI Read operation */
MCD_STATUS readMdio(    MCD_DEV_PTR pDev,
                        MCD_U16 mdioPort,
                        MCD_U16 mmd,
                        MCD_U16 reg,
                        MCD_U16 *valuePtr)
{
    MCD_STATUS rc;
    GT_U32     val;
    MCD_XSMI_CONNECT_STC * pXSmiPhyDbPtr = x5123HwPhyInfo;

    if (pDev->firstMdioPort == 0x1F)
    {
        MCD_PRINTF(
            "READ MDIO: reading broadcast, firstMdioPort 0x%X, mmd 0x%X, reg 0x%X\n",
            pDev->firstMdioPort, mmd, reg);
        MCD_PRINTF(
            "READ MDIO: shadowValidBmp 0x%X, regShadow[0] 0x%X, regShadow[1] 0x%X\n",
            pDev->shadowValidBmp, pDev->regShadow[0], pDev->regShadow[1]);

        *valuePtr = 0;
        return MCD_FAIL;
    }

    if (pDev->firstMdioPort != mdioPort)
    {
        MCD_PRINTF("READ MDIO: wrong mdioPort[%d], must be [%d]\n", mdioPort, pDev->firstMdioPort);
        *valuePtr = 0;
        return MCD_FAIL;
    }

    if ((mmd > 0xF) && (mmd < 0x1F))
    {
        MCD_PRINTF("READ MDIO: wrong mmd[%d], must be in ranges 0..0xF, 0x1F\n", mmd, pDev->firstMdioPort);
        *valuePtr = 0;
        return MCD_FAIL;
    }

    if(pXSmiPhyDbPtr[pDev->firstMdioPort].hwPhyConnected == MCD_TRUE)
    {
#ifdef CHX_FAMILY
        if (mcdSampleUsePpForXsmi)
        {
            rc = cpssDxChPhyPort10GSmiRegisterRead(mcdSampleUsePpDevNum, 0,(MCD_U8)(pDev->firstMdioPort), MCD_TRUE, reg, (MCD_U8)mmd, valuePtr);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("readMdio: cpssDxChPhyPort10GSmiRegisterRead register[%d] fails for pDev->firstMdioPort[%d]\n", reg, pDev->firstMdioPort);
                return MCD_FAIL;
            }
        }
        else
#endif
        {
            rc = extDrvXSmiReadReg(pDev->firstMdioPort, mmd, reg, &val);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("readMdio: READ XSMI register[%d] fails for pDev->firstMdioPort[%d]\n", reg, pDev->firstMdioPort);
                *valuePtr = 0;
                return MCD_FAIL;
            }

            *valuePtr = (MCD_U16)val;
        }
    }
    else
    {
        if (mcdSmiSimulEnable)
        {
            MCD_X5123_SIM_DEV  * pSimDev = x5123SimDevGet(mdioPort);
            if (pSimDev == NULL)
            {
                MCD_PRINTF("READ MDIO: simulation fails for mdioPort[%d]\n", pDev->firstMdioPort);
                *valuePtr = 0;
                return MCD_FAIL;
            }

            readSsmi(pSimDev, mmd, valuePtr);
        }
    }

    if (mcdDebugSmi == MCD_DEBUG_SMI_DETAILS)
    {
        MCD_PRINTF("READ MDIO: PRTADD %d, DEVADD %d, ADDR %d, OUT DATA 0x%04X\n", mdioPort, mmd, reg,*valuePtr);
    }

    return MCD_OK;
}

/* MCD callback for XSMI Write operation */
MCD_STATUS writeMdio(
                        MCD_DEV_PTR pDev,
                        MCD_U16 mdioPort,
                        MCD_U16 mmd,
                        MCD_U16 reg,
                        MCD_U16 value)
{
    MCD_STATUS rc;
    MCD_U32 val = (MCD_U32)value;
    MCD_XSMI_CONNECT_STC * pXSmiPhyDbPtr = x5123HwPhyInfo;

    if (pDev->firstMdioPort != mdioPort)
    {
        MCD_PRINTF("WRITE MDIO: wrong mdioPort[%d], must be [%d]\n", mdioPort, pDev->firstMdioPort);
        return MCD_FAIL;
    }

    if ((mmd > 0xF) && (mmd < 0x1F))
    {
        MCD_PRINTF("WRITE MDIO: wrong mmd[%d], must be in ranges 0..0xF, 0x1F\n", mmd, pDev->firstMdioPort);
        return MCD_FAIL;
    }

    if (mcdDebugSmi == MCD_DEBUG_SMI_DETAILS)
    {
        MCD_PRINTF("WRITE MDIO: PRTADD %d, DEVADD %d, ADDR %d, OUT DATA 0x%04X\n", mdioPort, mmd, reg, value);
    }

    /* 0x1F is a broadcast address, always connected */
    /* it has no entry in pXSmiPhyDbPtr array        */
    if ((pDev->firstMdioPort == 0x1F) ||
        (pXSmiPhyDbPtr[pDev->firstMdioPort].hwPhyConnected == MCD_TRUE))
    {
#ifdef CHX_FAMILY
        if (mcdSampleUsePpForXsmi)
        {
            rc = cpssDxChPhyPort10GSmiRegisterWrite(mcdSampleUsePpDevNum, 0, (MCD_U8)(pDev->firstMdioPort), MCD_TRUE, reg, (MCD_U8)mmd, val);
            if (rc != MCD_OK)
            {
                MCD_PRINTF(
                    "writeMdio: cpssDxChPhyPort10GSmiRegisterWrite register[%d] fails for pDev->firstMdioPort[%d]\n",
                    reg, pDev->firstMdioPort);
                return MCD_FAIL;
            }
        }
        else
#endif
        {
            rc = extDrvXSmiWriteReg(pDev->firstMdioPort, mmd, reg, val);
            if (rc != MCD_OK)
            {
                MCD_PRINTF(
                    "writeMdio: WRITE XSMI register[%d] fails for pDev->firstMdioPort[%d]\n",
                    reg, pDev->firstMdioPort);
                return MCD_FAIL;
            }
        }
    }
    else
    {
        if (mcdSmiSimulEnable)
        {
            MCD_X5123_SIM_DEV  * pSimDev = x5123SimDevGet(mdioPort);
            if (pSimDev == NULL)
            {
                MCD_PRINTF("WRITE MDIO: simulation fails for mdioPort[%d]\n", pDev->firstMdioPort);
                return MCD_FAIL;
            }

            writeSsmi(pSimDev, mmd, value);
        }
    }

    return MCD_OK;
}


MCD_STATUS mcdSampleXSmiDump(MCD_U32 xsmiPhyAddr,
                             MCD_U32 mmd,
                             MCD_U32 reg)
{
    MCD_STATUS rc;
    GT_U32     val;

    /* Init XSMI subsystem */
    if (mcdGlobalXSmiInitDone == MCD_FALSE)
    {
        rc = extDrvXSmiInit();
        if (rc != MCD_OK)
        {
            MCD_PRINTF("mcdSampleInitDrv: extDrvXSmiInit fail retVal = %d\n", rc);
        }
        else
        {
            mcdGlobalXSmiInitDone = MCD_TRUE;
        }
    }

    rc = extDrvXSmiReadReg(xsmiPhyAddr, mmd, reg, &val);
    if (rc != MCD_OK)
    {
        MCD_PRINTF("extDrvXSmiReadReg fails for PRTADD %d DEVADD %d, ADDR %d\n", xsmiPhyAddr, mmd, reg);
        return MCD_FAIL;
    }

    MCD_PRINTF("READ MDIO: PRTADD %d, DEVADD %d, ADDR 0x%04X, OUT DATA 0x%04X\n", xsmiPhyAddr, mmd, reg,val);

    return MCD_OK;
}

/******************************************************************************
 MCD_STATUS mcdSampleXSmiVendorIdHit

   Sample code to scan PHY devices and check vendorID registers
*******************************************************************************/
static MCD_STATUS mcdSampleXSmiVendorIdHit
(
    MCD_U32     phyId,
    MCD_U32     mmd,
    MCD_U32     mmd1,
    MCD_U16    *reg3DataPtr
)
{
    MCD_STATUS  rc;
    GT_U32      val;

#ifdef CHX_FAMILY
    if (mcdSampleUsePpForXsmi)
    {
        MCD_U16                              data_u16;
        rc = cpssDxChPhyPort10GSmiRegisterRead(mcdSampleUsePpDevNum, 0, phyId, MCD_TRUE, 0x0002, mmd, &data_u16);
        if (rc != MCD_OK)
        {
            MCD_PRINTF("mcdSampleXSmiVendorIdHit: cpssDxChPhyPort10GSmiRegisterRead fails - register[0x0002] ,phyId[%d]\n", phyId);
            return MCD_FAIL;
        }
        val = data_u16;
    }
    else
#endif
    {
        rc = extDrvXSmiReadReg(phyId, mmd, 0x0002, &val);
        if (rc != MCD_OK)
        {
            MCD_PRINTF("mcdSampleXSmiVendorIdHit: extDrvXSmiReadReg fails - register[0x0002] ,phyId[%d]\n", phyId);
            return MCD_FAIL;
        }
    }

    /* Marvell vendor ID bits[15..0] */
    if (val == 0x0141)
    {
#ifdef CHX_FAMILY
        if (mcdSampleUsePpForXsmi)
        {
            MCD_U16                              data_u16;
            rc = cpssDxChPhyPort10GSmiRegisterRead(mcdSampleUsePpDevNum, 0, phyId, MCD_TRUE, 0x0003, mmd1, &data_u16);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("mcdSampleXSmiVendorIdHit: cpssDxChPhyPort10GSmiRegisterRead fails - register[0x0003] ,phyId[%d]\n", phyId);
                return MCD_FAIL;
            }
            val = data_u16;
        }
        else
#endif
        {
            rc = extDrvXSmiReadReg(phyId, mmd1, 0x0003, &val);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("mcdSampleXSmiVendorIdHit: extDrvXSmiReadReg fails - register[0x0003], phyId[%d]\n", phyId);
                return MCD_FAIL;
            }
        }

        *reg3DataPtr = (MCD_16)(val & 0xFFFF);
        return MCD_OK;
    }

    return MCD_STATUS_NOT_FOUND;
}

/******************************************************************************
 MCD_STATUS mcdSampleXSmiScan

   Sample code to scan PHY devices and check vendor ID registers
*******************************************************************************/
static MCD_STATUS prvMcdSampleXSmiScan
(
    void
)
{
    MCD_U32 phyId;
    MCD_STATUS rc;
    MCD_U32 mmd, mmd1;
    MCD_BOOL deviceFound;
    MCD_U16  reg3Data;
    MCD_XSMI_CONNECT_STC * pXSmiPhyDbPtr = x5123HwPhyInfo;

    /* Init DB */
    mcdMemSet(pXSmiPhyDbPtr, 0, sizeof(x5123HwPhyInfo));
    if (mcdGlobalXSmiInitDone == MCD_FALSE)
    {
        /* Nothing to find before initialization is done */
        MCD_PRINTF("mcdSampleXSmiScan: XSMI subsysten not initialized\n");
        return MCD_STATUS_NOT_INITIALIZED;
    }

    deviceFound = MCD_FALSE;

    for (phyId = 0; phyId < MCD_MAX_PHY; phyId++)
    {
        reg3Data = 0;
        mmd1 = mmd = 1;
        rc = mcdSampleXSmiVendorIdHit(phyId, mmd, mmd1, &reg3Data);
        if (rc != MCD_OK)
        {
            mmd = 2;
            mmd1 = 3;
            rc = mcdSampleXSmiVendorIdHit(phyId, mmd, mmd1, &reg3Data);
            if (rc == MCD_OK)
            {
                if (deviceFound == MCD_FALSE)
                {
                    /* first device found */
                    MCD_PRINTF("XSMI PHY Addr | Register 3\n");
                    MCD_PRINTF("==========================\n");
                }
                /* Vendor ID found */
                deviceFound = MCD_TRUE;
                /* Set DB relevant field */
                pXSmiPhyDbPtr[phyId].hwPhyConnected = MCD_TRUE;
                MCD_PRINTF(" %2d           | 0x%04X\n", phyId, reg3Data);
            }
        }
        else
        {
            if (deviceFound == MCD_FALSE)
            {
                /* first device found */
                MCD_PRINTF("XSMI PHY Addr | Register 3\n");
                MCD_PRINTF("==========================\n");
            }

            deviceFound = MCD_TRUE;
            /* Set DB relevant field */
            pXSmiPhyDbPtr[phyId].hwPhyConnected = MCD_TRUE;
            MCD_PRINTF(" %2d           | 0x%04X\n", phyId, reg3Data);
        }
    }

    if (deviceFound == MCD_FALSE)
    {
        MCD_PRINTF("mcdSampleXSmiScan: Marvell vendor ID not found\n");
        return MCD_STATUS_NOT_FOUND;
    }
    else
    {
        MCD_PRINTF("\n");
    }

    return MCD_OK;
}

/*******************************************************************************
 mcdSampleInitDrv

 Inputs:
   phyId         - logical number of PHY device, 0..15
   pHostContext  - user specific data for host to pass to the low layer
   loadImage     - MCD_TRUE load all firmware images, MCD_FALSE don't load
   firstMDIOPort - PHY address of first port on XSMI bus

 Outputs:
       none

 Returns:
       MCD_OK               - on success
       MCD_FAIL             - on error

 Description:
   Marvell X5123 and EC808 Driver Initialization Routine.

 Side effects:
   None

 Notes/Warnings:

*******************************************************************************/
MCD_STATUS mcdSampleInitDrv
(
    IN MCD_U32   phyId,
    IN MCD_PVOID pHostContext,   /* optional host context */
    IN MCD_BOOL  loadImage,      /* 1:load image; 0:do not load */
    IN MCD_U16   mdioFirstPort
)
{
#ifndef ASIC_SIMULATION
    const MCD_U8    *pCm3Image;
    MCD_U32          cm3ImageSize;
    const MCD_U16    *pBusMasterImage;
    MCD_U16          busMasterImageSize;
    const MCD_U16    *pBusMasterSwapImage;
    MCD_U16          busMasterSwapImageSize;
    const MCD_U16    *pSerdesImage;
    MCD_U16          serdesImageSize;
    MCD_U16          serdesRevision;
    MCD_U16          sbmRevision;
    MCD_U8           major;
    MCD_U8           minor;
    MCD_U8           patch;
    MCD_U32          devId;
    MCD_U32          pDevIdx = 0;
#endif
    MCD_STATUS       retVal = MCD_OK;
    MCD_DEV_PTR      mcdDeviceStructPtr=NULL;

    /* check porting of MCD types */
    if ((sizeof(MCD_U8) != 1))
    {
        MCD_PRINTF("mcdSampleInitDrv: wrong porting of MCD_U8, must be 1 but not %d\n", sizeof(MCD_U8));
        return MCD_FAIL;
    }

    if ((sizeof(MCD_U16) != 2))
    {
        MCD_PRINTF("mcdSampleInitDrv: wrong porting of MCD_U16, must be 2 but not %d\n", sizeof(MCD_U16));
        return MCD_FAIL;
    }

    if ((sizeof(MCD_U32) != 4))
    {
        MCD_PRINTF("mcdSampleInitDrv: wrong porting of MCD_U32, must be 4 but not %d\n", sizeof(MCD_U32));
        return MCD_FAIL;
    }

    if (sizeof(long) == 8)
    {
        MCD_PRINTF("\n Run on 64 Bit CPU.\n");
    }

        /* init global variables related to Firmware included in the this Application */
        mcdHwSerdesCntlInitEmbededFwLoadData();

    if (phyId >= MCD_MAX_DEVICES)
    {
        MCD_PRINTF("mcdSampleInitDrv: phyId [%d] out of range 0..%d\n", phyId, (MCD_MAX_DEVICES - 1));
        return MCD_FAIL;
    }

    if (pDevArray[phyId])
    {
        MCD_PRINTF("mcdSampleInitDrv: phyId [%d] already initialized\n", phyId);
        return MCD_FAIL;
    }

    /* Init XSMI subsystem */
    if (mcdGlobalXSmiInitDone == MCD_FALSE)
    {
        if (mcdSampleUsePpForXsmi)
        {
            mcdGlobalXSmiInitDone = MCD_TRUE;
        }
        else
        {
            retVal = extDrvXSmiInit();
            if (retVal != MCD_OK)
            {
                MCD_PRINTF("mcdSampleInitDrv: extDrvXSmiInit fail retVal = %d\n", retVal);
            }
            else
            {
                mcdGlobalXSmiInitDone = MCD_TRUE;
            }
        }
        /* XSMI first time scan sets HW PHY Connected' flag in DB.*/
        prvMcdSampleXSmiScan();
    }

#ifndef ASIC_SIMULATION
    if (loadImage)
    {
        /* values below defined in included H-files of avago */
        pCm3Image = mcdServiceCpuCm3BvFw;
        cm3ImageSize = fwConfigInfo.fwSize;
        pBusMasterImage    = mcdHwSerdesCntlBusMasterImage;
        busMasterImageSize = mcdHwSerdesCntlBusMasterImageSize;
        pBusMasterSwapImage    = mcdHwSerdesCntlBusMasterSwapImage;
        busMasterSwapImageSize = mcdHwSerdesCntlBusMasterSwapImageSize;
        pSerdesImage    = mcdHwSerdesCntlSerdesImage;
        serdesImageSize = mcdHwSerdesCntlSerdesImageSize;

        /* allocate MCD_DEV */
        for (devId = phyId; devId < MCD_MAX_PHY; devId++)
        {
            if (devId % 2)
            {
                continue;
            }
            else if (x5123HwPhyInfo[devId].hwPhyConnected == MCD_TRUE)
            {
                pDevArray[pDevIdx] = (MCD_DEV_PTR) MCD_MALLOC(sizeof(MCD_DEV));
                mcdDeviceStructPtr = pDevArray[pDevIdx];
                pDevIdx++;
                if (!mcdDeviceStructPtr)
                {
                    MCD_PRINTF("mcdSampleInitDrv: out of CPU memory\n");
                    return MCD_FAIL;
                }
                mcdMemSet(mcdDeviceStructPtr, 0, (sizeof(MCD_DEV)));
                retVal = mcdInitDriver(
                &readMdio, &writeMdio, mdioFirstPort+devId,
                pCm3Image, cm3ImageSize,
                pBusMasterImage, busMasterImageSize,
                pBusMasterSwapImage, busMasterSwapImageSize,
                pSerdesImage, serdesImageSize,
                pHostContext, mcdDeviceStructPtr);
                if (retVal != MCD_OK)
                {
                    MCD_PRINTF("mcdSampleInitDrv: mcdInitDriver failed retVal = %d\n", retVal);
                    return retVal;
                }
            }
        }

        if (pDevArray[0] != NULL)
        {
            mcdDeviceStructPtr = pDevArray[0];
        }

        /* Get MCD software release version  */
        retVal = mcdGetAPIVersion(&major,&minor,&patch);
        if (retVal != MCD_OK)
        {
           MCD_PRINTF("mcdInitDriver: mcdGetAPIVersion Failed.\n");
           return retVal;
        }
        MCD_PRINTF("MCD version %d.%d.%d\n",major,minor,patch);

        serdesRevision = 0;
        sbmRevision    = 0;
        if (mcdDeviceStructPtr != NULL)
        {
            retVal = mcdSerdesGetRevision(
                mcdDeviceStructPtr, &serdesRevision, &sbmRevision);
            if (retVal != MCD_OK)
            {
                MCD_PRINTF("mcdSampleInitDrv: mcdSerdesGetRevision failed retVal = %d\n", retVal);
                return retVal;
            }
            MCD_PRINTF(
                "mcdSampleInitDrv: SERDES FW revision 0x%X BUS MASTER FW revision 0x%X\n",
                serdesRevision, sbmRevision);
        }
    }
    else
#endif /* !defined(ASIC_SIMULATION) */
    {
        /* allocate MCD_DEV */
        pDevArray[phyId] = (MCD_DEV_PTR) MCD_MALLOC(sizeof(MCD_DEV));
        mcdDeviceStructPtr = pDevArray[phyId];
        if (!mcdDeviceStructPtr)
        {
            MCD_PRINTF("mcdSampleInitDrv: out of CPU memory\n");
            return MCD_FAIL;
        }
        mcdMemSet(mcdDeviceStructPtr, 0, (sizeof(MCD_DEV)));
#ifdef ASIC_SIMULATION
        mcdDeviceStructPtr->serdesDev = MCD_MALLOC(0x12b8);
#endif /* !defined(ASIC_SIMULATION) */

        /* initialized the dev structure without loading the ROMs and firmware */
        retVal = mcdInitDriver(
            &readMdio, &writeMdio, mdioFirstPort,
            NULL/*pCm3Image*/, 0/*cm3ImageSize*/,
            NULL/*pBusMasterImage*/, 0/*busMasterImageSize*/,
            NULL/*pBusMasterSwapImage*/, 0/*pBusMasterSwapImage*/,
            NULL/*pSerdesImage*/, 0/*serdesImageSize*/,
            pHostContext, mcdDeviceStructPtr);
        if (retVal != MCD_OK)
        {
            MCD_PRINTF("mcdSampleInitDrv: mcdInitDriver failed retVal = %d\n", retVal);
            return retVal;
        }
    }

    i2cDrv = extDrvI2cCreateDrv(NULL);
    if( NULL == i2cDrv)
    {
        return  MCD_FAIL;
    }

    return MCD_OK;
}

/*******************************************************************************
 mcdSampleInitDrvLoadAllDevices

 Inputs:
   phyIdBmp         - bitmap of logical Ids
                      logical Id range is 0..15
                      device with logical Id == logId has  firstMDIOPort == (logId * 2)
                      pHostContext wil be NULL for each device
   imagesAmount     - what FW images to load:
                      0 - nothing
                      1 - serdes only
                      2 - serdes and master
                      3 - serdes, master and swap
                      4 - (yet not supported) serdes, master, swap and CM3

 Outputs:
       none

 Returns:
       MCD_OK               - on success
       MCD_FAIL             - on error

 Description:
   Marvell X5123 and EC808 Driver Initialization Routine.

 Side effects:
   None

 Notes/Warnings:

*******************************************************************************/
MCD_STATUS mcdSampleInitDrvLoadAllDevices
(
    IN MCD_U32   phyIdBmp,
    IN MCD_U32   imagesAmount
)
{
#ifdef ASIC_SIMULATION
    (void)phyIdBmp;
    (void)phyIdBmp;
    return GT_FAIL;
#else /* !defined(ASIC_SIMULATION) */
    MCD_STATUS retVal;
    MCD_U16 i, numOfDevs;
    MCD_DEV_PTR     pDevArr[MCD_MAX_DEVICES];
    MCD_U16         firstMDIOPortArr[MCD_MAX_DEVICES];
    MCD_PVOID       pHostContextArr[MCD_MAX_DEVICES];
    const MCD_U8    *pCm3Image = NULL;
    MCD_U32         cm3ImageSize =0;
    const MCD_U16   *pBusMasterImage = NULL;
    MCD_U16         busMasterImageSize = 0;
    const MCD_U16   *pBusMasterSwapImage = NULL;
    MCD_U16         busMasterSwapImageSize = 0;
    const MCD_U16   *pSerdesImage = NULL;
    MCD_U16         serdesImageSize = 0;

    /* Init XSMI subsystem */
    if (mcdGlobalXSmiInitDone == MCD_FALSE)
    {
        retVal = extDrvXSmiInit();
        if (retVal != MCD_OK)
        {
            MCD_PRINTF("mcdSampleInitDrv: extDrvXSmiInit fail retVal = %d\n", retVal);
        }
        else
        {
            mcdGlobalXSmiInitDone = MCD_TRUE;
        }
        /* XSMI first time scan sets HW PHY Connected' flag in DB.*/
        prvMcdSampleXSmiScan();
    }

    numOfDevs = 0;
    for (i = 0; (i < MCD_MAX_DEVICES); i++)
    {
        if (phyIdBmp & (1 << i))
        {
            MCD_PRINTF("mcdSampleInitDrvLoadAllDevices: dev = %d\n", i);
            /* allocate MCD_DEV */
            pDevArray[i] = (MCD_DEV_PTR) MCD_MALLOC(sizeof(MCD_DEV));
            if (pDevArray[i] == NULL)
            {
                MCD_PRINTF("mcdSampleInitDrvLoadAllDevices: out of CPU memory\n");
                return MCD_FAIL;
            }
            mcdMemSet(pDevArray[i], 0, (sizeof(MCD_DEV)));

            pDevArr[i]          = pDevArray[i];
            firstMDIOPortArr[i] = (i * 2);
            pHostContextArr[i]  = NULL;
            numOfDevs ++;
        }
    }

    MCD_PRINTF("mcdSampleInitDrvLoadAllDevices: numOfDevs = %d\n", numOfDevs);

    /* update*/
    if (imagesAmount >= 1)
    {
        pSerdesImage           = mcdHwSerdesCntlSerdesImage;
        serdesImageSize        = mcdHwSerdesCntlSerdesImageSize;
    }
    if (imagesAmount >= 2)
    {
        pBusMasterImage        = mcdHwSerdesCntlBusMasterImage;
        busMasterImageSize     = mcdHwSerdesCntlBusMasterImageSize;
    }
    if (imagesAmount >= 3)
    {
        pBusMasterSwapImage    = mcdHwSerdesCntlBusMasterSwapImage;
        busMasterSwapImageSize = mcdHwSerdesCntlBusMasterSwapImageSize;
    }
    if (imagesAmount >=4)
    {
        pCm3Image = mcdServiceCpuCm3BvFw;
        cm3ImageSize = fwConfigInfo.fwSize;
    }

    retVal = mcdInitDriverAllDevices(
        &readMdio, &writeMdio,
        numOfDevs, firstMDIOPortArr,
        pCm3Image, cm3ImageSize,
        pBusMasterImage, busMasterImageSize,
        pBusMasterSwapImage, busMasterSwapImageSize,
        pSerdesImage, serdesImageSize,
        pHostContextArr, pDevArr);

    if (retVal != MCD_OK)
    {
        MCD_PRINTF(
            "mcdSampleInitDrvLoadAllDevices: mcdInitDriverAllDevices failed retVal = %d\n", retVal);
    }

    return retVal;
#endif /* !defined(ASIC_SIMULATION) */
}

/*******************************************************************************
 mcdSampleGetDrvDev

 Inputs:
   phyId         - logical number of PHY device, 0..15

 Outputs:
       none

 Returns:
       pointer to device handler

 Description:
   Get pointer to device handler for logical PHY number.
   NULL - in case of error or not initialized

 Notes/Warnings:

*******************************************************************************/
MCD_DEV_PTR mcdSampleGetDrvDev
(
    IN MCD_U32   phyId
)
{
    if (phyId >= MCD_MAX_DEVICES)
    {
        MCD_PRINTF("mcdSampleGetDrvDev: phyId [%d] out of range 0..%d\n", phyId, (MCD_MAX_DEVICES - 1));
        return NULL;
    }

    return pDevArray[phyId];
}

/******************************************************************************
  mcdSampleUnloadDrv

 Inputs:
   phyId         - logical number of PHY device, 0..15

 Outputs:
       none

 Returns:
       MCD_OK               - on success
       MCD_FAIL             - on error

 Description:
   Unload MCD Device driver. Free allocated memory.

 Notes/Warnings:

*******************************************************************************/
MCD_STATUS mcdSampleUnloadDrv
(
    IN MCD_U32   phyId
)
{
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleUnloadDrv: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    mcdUnloadDriver(pDev);

    MCD_FREE(pDev);
    pDevArray[phyId] = NULL;

    return MCD_OK;
}



/******************************************************************************
 MCD_STATUS mcdSampleSerdesTxEnableDisable

   Sample code to enable/disable TX serdes
*******************************************************************************/
MCD_STATUS mcdSampleSerdesTxEnableDisable
(
    IN MCD_U32      phyId,
    IN MCD_U16      numOfLoops,
    IN MCD_U16      timeout,
    IN MCD_U8       numOfPorts,
    ...
)
{
    va_list  ap;                 /* arguments list pointer */
    MCD_U32   port,loop;
    MCD_STATUS status;
    MCD_U16 portNum;
    MCD_DEV_PTR pDev;
    MCD_OP_MODE portMode;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSerdesTxEnableDisable: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    va_start(ap, numOfPorts);

    MCD_PRINTF("mcdSampleSerdesTxEnableDisable: numOfLoops [%d]\n", numOfLoops);
    MCD_PRINTF("mcdSampleSerdesTxEnableDisable: numOfPorts [%d]\n", numOfPorts);

    for(port = 0; port < numOfPorts; port++)
    {
        portNum = (MCD_U16)va_arg(ap, MCD_U32);
        /* check port is valid */
        if (mcdIsPortValid(pDev, portNum) != MCD_OK)
        {
            MCD_PRINTF("mcdSampleSerdesTxEnableDisable: incorrect portNum [%d]\n", portNum);
            va_end(ap);
            return MCD_FAIL;
        }

        portMode = pDev->portConfig[portNum].portMode;
        if (portMode == MCD_MODE_UNKNOWN)
        {
            MCD_PRINTF("mcdSampleSerdesTxEnableDisable: portNum [%d] umknown port mode\n", portNum);
            continue;
        }

        MCD_PRINTF("mcdSampleSerdesTxEnableDisable: portNum [%d]\n", portNum);
        MCD_PRINTF("mcdSampleSerdesTxEnableDisable: portMode [%d]\n", portMode);

        for (loop = 0; loop < numOfLoops; loop++)
        {
            status = mcdPortSerdesTxEnable(pDev, portNum, MCD_TRUE);
            if (status != MCD_OK)
            {
                MCD_PRINTF("mcdSampleSerdesTxEnableDisable failed on port TX enable:[%d]\n", portNum);
            }

            if (timeout)
            {
                MCD_SLEEP(timeout);
            }

            status = mcdPortSerdesTxEnable(pDev, portNum, MCD_FALSE);
            if (status != MCD_OK)
            {
                MCD_PRINTF("mcdSampleSerdesTxEnableDisable failed on port TX disable:[%d]\n", portNum);
            }
        }
    }

    va_end(ap);

    return MCD_OK;
}

/******************************************************************************
 MCD_STATUS mcdSampleXSmiScan

   Sample code to scan PHY devices and check vendor ID registers
*******************************************************************************/
MCD_STATUS mcdSampleXSmiScan
(
    void
)
{
    MCD_STATUS rc;

    /* Init XSMI subsystem */
    if (mcdGlobalXSmiInitDone == MCD_FALSE)
    {
        rc = extDrvXSmiInit();
        if (rc != MCD_OK)
        {
            MCD_PRINTF("mcdSampleXSmiScan: extDrvXSmiInit fail retVal = %d\n", rc);
        }
        else
        {
            mcdGlobalXSmiInitDone = MCD_TRUE;
        }
    }

    return prvMcdSampleXSmiScan();
}

/******************************************************************************
 MCD_STATUS mcdSampleXSmiWriteTest

   XSMI Write test.
*******************************************************************************/
MCD_STATUS mcdSampleXSmiWriteTest
(
    MCD_U32     phyId,
    MCD_U32     devAddr,
    MCD_U32     regAddr,
    MCD_U32     regMask,
    MCD_U32     iterations
)
{
    MCD_STATUS  rc;
    MCD_U32      testDataArr[] = {0xaaaa, 0x0000, 0x5555, 0xffff};
    MCD_U32      testCasesNum = sizeof(testDataArr) / sizeof(testDataArr[0]);
    GT_U32       ii, kk, val;


    for (ii = 0; ii < iterations; ii++)
    {
        for (kk = 0; kk < testCasesNum; kk++)
        {
            rc = extDrvXSmiWriteReg(phyId, devAddr, regAddr, testDataArr[kk]);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("extDrvXSmiWriteReg fails rc[%d]\n", rc);
                return MCD_FAIL;
            }

            rc = extDrvXSmiReadReg(phyId, devAddr, regAddr, &val);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("extDrvXSmiReadReg fails rc[%d]\n", rc);
                return MCD_FAIL;
            }

            if ((val & regMask) != testDataArr[kk])
            {
                MCD_PRINTF("Test fails on iterator %d, write 0x%x, read 0x%x\n", ii, testDataArr[kk], val);
                return MCD_FAIL;
            }
        }
    }

    MCD_PRINTF("Test OK: regAddr 0x%x, devAddr 0x%x iterations %d\n", regAddr, devAddr,iterations);
    return MCD_OK;
}

#ifndef MCD_RUN_WITH_CPSS
static void mcdSampleAppHelp(void)
{
    MCD_PRINTF(" mcdSample command format:\n");
    MCD_PRINTF(" mcdSample [mode] [FEC] [debug]\n");
    MCD_PRINTF(" modes (optional):\n");
    MCD_PRINTF(" 100G   - 100G LR/SR mode - this mode used when mode ommited\n");
    MCD_PRINTF(" 40G-R4 - 40G-R4  LR/SR mode\n");
    MCD_PRINTF(" 40G-R2 - 40G-R2  LR/SR mode\n");
    MCD_PRINTF(" 50G    - 50G-R2  LR/SR mode\n");
    MCD_PRINTF(" 25G    - 25G-R1  LR/SR mode\n");
    MCD_PRINTF(" R-25G  - 25G retimer mode\n");
    MCD_PRINTF("\n FEC(optional):\n");
    MCD_PRINTF(" RS   - RS FEC\n");
    MCD_PRINTF(" FC   - FC FEC\n");
    MCD_PRINTF(" FEC Disabled otherwise\n");
    MCD_PRINTF("\n debug (optional) - enable debug printing\n");
}

int main(int argc, char *argv[])
{
    MCD_STATUS  rc;
    MCD_U32     phyId = 0, ii;
    MCD_U16     phyPort, lane;
    MCD_OP_MODE  portMode = MCD_MODE_P100L;
    MCD_FEC_TYPE fecCorrect = MCD_NO_FEC;
    MCD_U32     portStep = 4;  /* port number increment        */
    char       *portModeStr = "100G";
    char       *fecModeStr = "NO FEC";
    MCD_DEV_PTR pDev;       /* pointer to MCD Device structure */
    MCD_U8      serdesNum;  /* number of SERDES */
    MCD_RX_TRAINING_MODES           rxTraining;
    MCD_TX_TRAINING_MODES           txTraining;
    MCD_U16                         currentStatus;
    MCD_U16                         latchedStatus;
    DETAILED_STATUS_TYPE            statusDetail;
    MCD_U64                         txRxBitCount_not_used;
    MCD_U64                         rxBitErrorCount;


    MCD_PRINTF("++++++++++++++++++++++++++++\n");
    MCD_PRINTF("Start MCD Sample Application\n");
    MCD_PRINTF("++++++++++++++++++++++++++++\n");

    sleep(1);

    if ((argc > 1) && (strcmp("help", argv[1]) == 0))
    {
        mcdSampleAppHelp();
    }
    else if (argc > 4)
    {
        MCD_PRINTF("Too many parameters\n");
        mcdSampleAppHelp();
        sleep(1);
        return 1;
    }
    else
    {
        for (ii = 1; ii < argc; ii++)
        {
            if ((strcmp("100G", argv[ii]) == 0))
            {
            }
            else if ((strcmp("40G-R4", argv[ii]) == 0))
            {
                portMode = MCD_MODE_P40L;
                portModeStr = "40G-R4";
            }
            else if ((strcmp("40G-R2", argv[ii]) == 0))
            {
                portMode = MCD_MODE_P40R2L;
                portModeStr = "40G-R2";
                portStep = 2;
            }
            else if ((strcmp("50G", argv[ii]) == 0))
            {
                portMode = MCD_MODE_P50R2L;
                portModeStr = "50G-R2";
                portStep = 2;
            }
            else if ((strcmp("25G", argv[ii]) == 0))
            {
                portMode = MCD_MODE_P25L;
                portModeStr = "25G";
                portStep = 1;
            }
            else if ((strcmp("R-25G", argv[ii]) == 0))
            {
                portMode = MCD_MODE_R25L;
                portModeStr = "Retimer 25G";
                portStep = 1;
            }
            else if ((strcmp("RS", argv[ii]) == 0))
            {
                fecCorrect = MCD_RS_FEC;
                fecModeStr = "RS FEC";
            }
            else if ((strcmp("FC", argv[ii]) == 0))
            {
                fecCorrect = MCD_FC_FEC;
                fecModeStr = "FC FEC";
            }
            else if ((strcmp("debug", argv[ii]) == 0))
            {
                mcdDebugLevelSet(MCD_DBG_ALL_LVL);
            }
            else
            {
                MCD_PRINTF("Wrong option %s\n",argv[ii]);
                mcdSampleAppHelp();
                sleep(1);
                return 1;
            }
        }

        rc = mcdSampleInitDrv( phyId/* PHY Logical ID */, 0 /* just not used number*/,
                   MCD_TRUE /* load FW */, 0 /* PHY XSMI Address */);
        if (rc != MCD_OK)
        {
            MCD_PRINTF("mcdSampleInitDrv fails rc[%d]\n", rc);
            return MCD_FAIL;
        }

        MCD_PRINTF("\n PHY %d init done successfully\n", phyId);
        sleep(1);

        for (phyPort = 0; phyPort < 8; )
        {
            rc = mcdSampleSetModeSelectionExt(phyId, phyPort, portMode, MCD_FALSE, fecCorrect, MCD_FALSE, MCD_FALSE,
                                                    MCD_FALSE, MCD_FALSE, MCD_FALSE);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("mcdSampleSetModeSelectionExt fails rc[%d] for port %d\n", rc, phyPort);
                return MCD_FAIL;
            }

            MCD_PRINTF("\n Port [%d] %s %s config done successfully\n", phyPort, portModeStr, fecModeStr);

            phyPort += portStep;
        }

        sleep(10);
        pDev = mcdSampleGetDrvDev(phyId);
        if(pDev == NULL)
        {
            MCD_PRINTF("\n mcdSampleGetDrvDev fails for PHY id %d\n", phyId);
            return 1;
        }

        for (phyPort = 0; phyPort < 8; )
        {
            rc = mcdCheckLinkStatus(pDev, phyPort,&currentStatus,&latchedStatus,&statusDetail);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("mcdCheckLinkStatus fails rc[%d] for port %d\n", rc, phyPort);
                return rc;
            }

            MCD_PRINTF("\n Port [%d] Current Status [%d] Latched Status [%d]\n", phyPort, currentStatus, latchedStatus);

            phyPort += portStep;
        }

        /* print eye of first 8 SERDES */
        for (lane = 0; lane < 8 ; lane++)
        {
            rc = mcdSampleSerdesGetEom(phyId, lane,MCD_FALSE,0,0);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("mcdSampleSerdesGetEom fails rc[%d] for port %d\n", rc, phyPort);
                return rc;
            }
        }

        /* run PRBS on Line side - use default pattern */
        for (phyPort = 0; phyPort < 8; )
        {
            for (lane = 0; lane < portStep; lane++)
            {
                rc = mcdSetPRBSEnableTxRx(pDev, phyPort, MCD_LINE_SIDE, lane, MCD_TRUE, MCD_TRUE);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdSetPRBSEnableTxRx fails rc[%d] for port %d lane %d\n", rc, phyPort, lane);
                    return rc;
                }
            }
            phyPort += portStep;
        }

        MCD_PRINTF("\n Check PRBS Errors.\n");
        sleep(1);
        /* read PRBS counters */
        for (ii = 0; ii < 10 ; ii++)
        {
            for (phyPort = 0; phyPort < 8;)
            {
                for (lane = 0; lane < portStep; lane++)
                {
                    rc = mcdGetPRBSCounts(pDev, phyPort, MCD_LINE_SIDE, lane,
                                          &txRxBitCount_not_used, &txRxBitCount_not_used,
                                          &rxBitErrorCount);
                    if (rc != MCD_OK)
                    {
                        MCD_PRINTF("mcdSetPRBSEnableTxRx fails rc[%d] for port %d lane %d\n", rc, phyPort, lane);
                        return rc;
                    }

                    {
                        MCD_U32 errorToPrint = (MCD_U32)rxBitErrorCount;
                        MCD_PRINTF(" Port [%d] Lane [%d] PRBS Errors [%d]\n", phyPort, lane, errorToPrint);
                    }
                }
                phyPort += portStep;
            }
            sleep(1);
        }

        /* stop PRBS on Line side */
        for (phyPort = 0; phyPort < 8; )
        {
            for (lane = 0; lane < portStep; lane++)
            {
                rc = mcdSetPRBSEnableTxRx(pDev, phyPort, MCD_LINE_SIDE, lane, MCD_FALSE, MCD_FALSE);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdSetPRBSEnableTxRx fails rc[%d] for port %d lane %d\n", rc, phyPort, lane);
                    return rc;
                }
            }
            phyPort += portStep;
        }

        rxTraining = MCD_START_CONTINUE_ADAPTIVE_TUNING;
        txTraining = MCD_IGNORE_TX_TRAINING;

        for (serdesNum = 0; serdesNum < 16; serdesNum++)
        {
            rc = mcdSerdesAutoTuneStartExt(pDev, serdesNum, rxTraining, txTraining);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("mcdSerdesAutoTuneStartExt fails rc[%d] for SERDES %d\n", rc, serdesNum);
                return MCD_FAIL;
            }
        }

        MCD_PRINTF("\n SERDES Auto Tune - RX Adaptive - done successfully\n");

        rxTraining = MCD_ONE_SHOT_DFE_TUNING;

        for (serdesNum = 0; serdesNum < 16; serdesNum++)
        {
            rc = mcdSerdesAutoTuneStartExt(pDev, serdesNum, rxTraining, txTraining);
            if (rc != MCD_OK)
            {
                MCD_PRINTF("mcdSerdesAutoTuneStartExt fails rc[%d] for SERDES %d\n", rc, serdesNum);
                return MCD_FAIL;
            }
        }

        MCD_PRINTF("\n SERDES Auto Tune - RX One Shot DFE - done successfully\n");
    }

    MCD_PRINTF("+++++++++++++++++++++++++++++++++++++++++++++\n");
    MCD_PRINTF(" MCD Sample Application finished successfully\n");
    MCD_PRINTF("+++++++++++++++++++++++++++++++++++++++++++++\n");
    sleep(3);

    return 0;
}
#else
#ifdef CHX_FAMILY
GT_STATUS appDemoDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);

GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);

/*******************************************************************************
 cpssAldrinMcdInit

 Inputs:
   phyId         - logical number of PHY device, 0..15

 Outputs:
       none

 Returns:
       MCD_OK               - on success
       MCD_FAIL             - on error

 Description:
   Marvell X5123 Driver Initialization Routine for Marvell Aldrin DB board.
   X5123 device is connected to Aldrin PP (packet processor) for XSMI management.
   CPSS SDK is used for Aldrin PP initialization and XSMI API.

 Side effects:
   None

 Notes/Warnings:

*******************************************************************************/
int cpssAldrinMcdInit(MCD_U32 phyId)
{
    MCD_STATUS rc;

    appDemoDbEntryAdd( "bc2BoardType", 0x70);

    rc = cpssInitSystem(29,1,0);
    if (rc != MCD_OK)
    {
        MCD_PRINTF("cpssInitSystem fails rc[%d]\n", rc);
        return MCD_FAIL;
    }

    mcdSampleUsePpForXsmi = 1;

    rc = mcdSampleInitDrv( phyId/* PHY Logical ID */, 0 /* just not used number*/,
               MCD_TRUE /* load FW */, 0 /* PHY XSMI Address */);
    if (rc != MCD_OK)
    {
        MCD_PRINTF("mcdSampleInitDrv fails rc[%d]\n", rc);
        return MCD_FAIL;
    }
    return MCD_OK;
}
#endif /* CHX_FAMILY */
#endif

#ifdef AAPL_ENABLE_EYE_MEASUREMENT
/**
* @internal mcdSampleSerdesGetEom function
* @endinternal
*
* @brief   Displays on the screen serdes vbtc and hbtc calculation and eye matrix.
*
* @param[in] phyId                    - PHY ID
* @param[in] serdesNum                - local serdes number
* @param[in] noeye                    - if 1 - Displays on the screen serdes vbtc and hbtc calculation only and not displays eye matrix.
* @param[in] min_dwell_bits           - minimum dwell bits - It is lower limit on how long to sample at each data point.
*                                      valid range: 100000 --- 100000000 .If 0 - transform to default 100000.
* @param[in] max_dwell_bits           - maximum dwell bits - It is the upper limit on how long to sample at each data point.
*                                      valid range: 100000 --- 100000000 .If 0 - transform to default 100000000.
*                                      should be min_dwell_bits <= max_dwell_bits
*
* @retval MCD_OK                   - on success
* @retval MCD_STATUS_BAD_PARAM     - on bad parameter
* @retval else                     - on error
*
* @note use like mcdSampleSerdesGetEom 0,0,0,0,0
*/
MCD_STATUS mcdSampleSerdesGetEom
(
    IN  MCD_U32                           phyId,
    IN  MCD_U16                           serdesNum,
    IN  MCD_BOOL                          noeye,
    IN  MCD_U32                           min_dwell_bits,
    IN  MCD_U32                           max_dwell_bits
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    MCD_U32    len_matrix;
    MCD_U32    i;
    MCD_U16  sAddr;
    MCD_SERDES_EYE_GET_RESULT eye_results;
    MCD_SERDES_EYE_GET_INPUT eye_input;
    MCD_U32 dwell_bits_bottom;
    MCD_U32 dwell_bits_top;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mmcdSampleSerdesGetEom: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    MCD_PRINTF("mcdSampleSerdesGetEome:serdesNum %d\n", serdesNum);
    MCD_PRINTF("mcdSampleSerdesGetEome: min_dwell_bits %d\n", min_dwell_bits);
    MCD_PRINTF("mcdSampleSerdesGetEom: max_dwell_bits %d\n", max_dwell_bits);

    sAddr = serdesNum + 1;
    dwell_bits_bottom = 100000;
    dwell_bits_top = 100000000;
    if(0 == min_dwell_bits)
    {
        min_dwell_bits = dwell_bits_bottom;
    }
    if(0 == max_dwell_bits)
    {
        max_dwell_bits = dwell_bits_top;
    }
    if((min_dwell_bits < dwell_bits_bottom) || (min_dwell_bits > dwell_bits_top))
    {
         MCD_PRINTF("minimum dwell bits = %d is out of range:   %d - %d\n", min_dwell_bits, dwell_bits_bottom, dwell_bits_top);
         return MCD_STATUS_BAD_PARAM;
    }
    if((max_dwell_bits < dwell_bits_bottom) || (max_dwell_bits > dwell_bits_top))
    {
         MCD_PRINTF("maximum dwell bits = %d is out of range:   %d - %d\n", max_dwell_bits, dwell_bits_bottom, dwell_bits_top);
         return MCD_STATUS_BAD_PARAM;
    }
    if(max_dwell_bits < min_dwell_bits)
    {
         MCD_PRINTF("maximum dwell bits = %d is less then minimum dwell bits = %d\n", max_dwell_bits, min_dwell_bits);
         return MCD_STATUS_BAD_PARAM;
    }

    eye_input.min_dwell_bits = min_dwell_bits;
    eye_input.max_dwell_bits = max_dwell_bits;
    eye_results.matrixPtr = NULL;
    eye_results.vbtcPtr = NULL;
    eye_results.hbtcPtr = NULL;
    eye_results.height_mv = 0;
    eye_results.width_mui = 0;
    rc=mcdSerdesEyeGetExt(pDev, sAddr, &eye_input, &eye_results);
    if(MCD_OK == rc)
    {
        MCD_PRINTF("\nSerDes No. %d: has an eye height of %d mV.\n", serdesNum, eye_results.height_mv);
        MCD_PRINTF("SerDes No. %d: has an eye width of  %d mUI.\n", serdesNum, eye_results.width_mui);
        if(NULL != eye_results.vbtcPtr)
        {
            MCD_PRINTF("%s", eye_results.vbtcPtr);
        }
        if(NULL != eye_results.hbtcPtr)
        {
            MCD_PRINTF("%s\n", eye_results.hbtcPtr);
        }
        if(noeye == MCD_FALSE)
        {
            if(NULL != eye_results.matrixPtr)
            {
                len_matrix = mcdStrlen(eye_results.matrixPtr);
                for(i = 0; i < len_matrix; i++)
                {
                    MCD_PRINTF("%c", eye_results.matrixPtr[i]);
                }
            }
        }
    }
    else
    {
        MCD_PRINTF("Can't get eye data\n");
        return rc;
    }

    if(NULL != eye_results.matrixPtr)
    {
        MCD_FREE(eye_results.matrixPtr);
        eye_results.matrixPtr = NULL;
    }
    if(NULL != eye_results.vbtcPtr)
    {
        MCD_FREE(eye_results.vbtcPtr);
        eye_results.vbtcPtr = NULL;
    }
    if(NULL != eye_results.hbtcPtr)
    {
        MCD_FREE(eye_results.hbtcPtr);
        eye_results.hbtcPtr = NULL;
    }
    MCD_PRINTF("\n");
    return MCD_OK;
}

/**
* @internal mcdSampleSerdesShiftSamplePoint function
* @endinternal
*
* @brief   Displays on the screen serdes vbtc and hbtc calculation and eye matrix.
*
* @param[in] phyId                    - PHY ID
* @param[in] serdesNum                - local serdes number
* @param[in] delay                    - value the delay (0..31) -the middle point of eye measurement
*
* @retval MCD_OK                   - on success
* @retval MCD_STATUS_BAD_PARAM     - on bad parameter
*
* @note use like mcdSampleSerdesShiftSamplePoint 0,0,20
*/
MCD_STATUS mcdSampleSerdesShiftSamplePoint
(
    IN  MCD_U32                           phyId,
    IN  MCD_U16                           serdesNum,
    IN  MCD_U32                           delay
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSerdesShiftSamplePoint: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    /* call  api function */
    rc = mcdSerdesShiftSamplePoint(pDev, serdesNum, delay);
    return rc;
}

/**
* @internal mcdSampleSerdesSetRetimerClock function
* @endinternal
*
* @brief   Configure retimer clockk
*
* @param[in] phyId
* @param[in] serdesNum                - SERDES number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mcdSampleSerdesSetRetimerClock
(
    IN MCD_U32 phyId,
    IN MCD_U32 serdesNum,
    MCD_SERDES_SPEED    baudRate
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mmcdSampleSerdesGetEom: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    rc = mcdSerdesSetRetimerClock(pDev, serdesNum, baudRate);
    MCD_PRINTF("\nmmcdSerdesSetRetimerClock returns %d\n",rc);
    return MCD_OK;


}

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

/* AAPL AACS Server debug utility used with CPSS SDK only */
#ifdef MCD_RUN_WITH_CPSS
#include <serdes/avago/aapl/include/aacs_server.h>

#if AAPL_ENABLE_AACS_SERVER

/* Avago server process Id */
static MCD_U32 avagoAACS_ServerTid;

extern unsigned int osTaskCreate(const char *name, unsigned int prio, unsigned int stack,
    unsigned (__TASKCONV *start_addr)(void*), void *arglist, MCD_U32 *tid);

/**
* @internal mcdSerdesDebugInit function
* @endinternal
*
* @brief   Create Avago AACS Server Process
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSerdesDebugInit
(
    char *name,
    unsigned (__TASKCONV *taskRoutine)(void*),
    MCD_SER_DEV_PTR pSerdesDev
)
{
    void * params = (void *)pSerdesDev;

    if((osTaskCreate(name, 250, 0x2000, taskRoutine, params, &avagoAACS_ServerTid)) != MCD_OK)
    {
        return MCD_FAIL;
    }

    return MCD_OK;
}

/*******************************************************************************
* mcdAACS_ServerRoutine
*
* DESCRIPTION:
*       Initialize Avago AACS Server
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*******************************************************************************/
#ifndef ASIC_SIMULATION
static unsigned __TASKCONV mcdAACS_ServerRoutine
(
    IN void * param
)
{
    unsigned int tcpPort = 90;

    mcdWrapperSerdesAacsServer((MCD_SER_DEV_PTR) param, tcpPort);

    return MCD_OK;
}
#endif /* ASIC_SIMULATION */
/**
* @internal mcdSerdesAacsServerExec function
* @endinternal
*
* @brief   Initialize the Avago AACS Server
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mcdSerdesAacsServerExec(MCD_DEV_PTR pDev)
{
#ifndef ASIC_SIMULATION
    int res;
    MCD_SER_DEV_PTR pSerdesDev = (MCD_SER_DEV_PTR)pDev->serdesDev;

    res = mcdSerdesDebugInit("mcdSerdesAACS_Server", mcdAACS_ServerRoutine, pSerdesDev);
    if (res != MCD_OK)
    {
        MCD_DBG_ERROR("Failed to init AACS Server\n");
        return res;
    }
#endif /* ASIC_SIMULATION */
    return MCD_OK;
}
#endif /* AAPL_ENABLE_AACS_SERVER */
#endif /* MCD_RUN_WITH_CPSS       */
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

#ifdef MCD_RUN_WITH_CPSS
int mcdSampleAacsServerExec
(
    IN MCD_U32 phyId
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mmcdSampleSerdesGetEom: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    rc = mcdSerdesAacsServerExec( pDev);
    MCD_PRINTF("\nmcdSampleAacsServerExec returns %d\n",rc);
    return MCD_OK;
}
#endif
MCD_STATUS mcdSampleSquelchRxClockSet
(
    IN MCD_U32 phyId,
    IN MCD_U16 mdioPort,
    IN MCD_U32 laneNum,
    IN MCD_RX_CLOCK_CTRL_TYPE clockCtrl
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSquelchRxClockSet: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    rc =  mcdSquelchRxClockSet( pDev,  mdioPort, laneNum, clockCtrl);
    MCD_PRINTF("\nmcdSampleSquelchRxClockSet returns %d\n",rc);
    return MCD_OK;

}


/**
* @internal mcdSampleSerdesSetRetimerClockLaneSteeringMode function
* @endinternal
*
* @brief   Configure retimer clock
*
* @param[in] phyId
* @param[in] serdesNum                - SERDES number
* @param[in] partnerSerdesNum
*                                      baudrate
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mcdSampleSerdesSetRetimerClockLaneSteeringMode
(
    IN MCD_U32 phyId,
    IN MCD_U32 serdesNum,
    IN MCD_U32 partnerSerdesNum,
    MCD_SERDES_SPEED    baudRate
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mmcdSampleSetRetimerClockLaneSteeringMode: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    rc = mcdSerdesSetRetimerClockLaneSteeringMode(pDev, serdesNum, partnerSerdesNum, baudRate);
    MCD_PRINTF("\nmcdSerdesSetRetimerClockLaneSteeringMode returns %d\n",rc);
    return MCD_OK;


}


/**
* @internal mcdSampleGetSerdesAutoTuneStatus function
* @endinternal
*
* @brief  Get Auto-tune status of RX and TX training and print it
*
*
* @param[in] phyId          - device number
* @param[in] serdesNum  -  serdes number  (0..15)
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleGetSerdesAutoTuneStatus 0,8
*
*/

MCD_STATUS mcdSampleGetSerdesAutoTuneStatus
(
    IN MCD_U32 phyId,
    IN MCD_U8  serdesNum
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    MCD_AUTO_TUNE_STATUS            rxStatus;
    MCD_AUTO_TUNE_STATUS            txStatus;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("\nmcdSampleSerdesAutoTuneStatus: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }

    rc = mcdSerdesAutoTuneStatus( pDev, serdesNum, &rxStatus, &txStatus);
    if(rc != 0)
    {
        MCD_PRINTF("\nmcdSampleSerdesAutoTuneStatus on lane %d failed \n", serdesNum);
        return rc;
    }

    MCD_PRINTF("\nmcdSampleSerdesAutoTuneStatus on lane %d: \n", serdesNum);
    switch(rxStatus)
    {
    case MCD_TUNE_PASS:
        MCD_PRINTF(" RX training PASSED\n");
        break;

    case MCD_TUNE_FAIL:
        MCD_PRINTF(" RX training FAILED\n");
        break;

    case MCD_TUNE_NOT_COMPLITED:
        MCD_PRINTF(" RX training NOT COMPLETED\n");
        break;

    case MCD_TUNE_READY:
        MCD_PRINTF(" RX training READY\n");
        break;

    case MCD_TUNE_NOT_READY:
        MCD_PRINTF(" RX training NOT READY\n");
        break;

    case MCD_TUNE_INTERRUPTED:
        MCD_PRINTF(" RX training INTERRUPTED\n");
        break;

    default:
        MCD_PRINTF("Urecognized RX training result %d\n", rxStatus);
        break;

    }


    switch(txStatus)
    {
    case MCD_TUNE_PASS:
        MCD_PRINTF(" TX training PASSED\n");
        break;

    case MCD_TUNE_FAIL:
        MCD_PRINTF(" TX training FAILED\n");
        break;

    case MCD_TUNE_NOT_COMPLITED:
        MCD_PRINTF(" TX training NOT COMPLETED\n");
        break;

    case MCD_TUNE_READY:
        MCD_PRINTF(" TX training READY\n");
        break;

    case MCD_TUNE_NOT_READY:
        MCD_PRINTF(" TX training NOT READY\n");
        break;

    case MCD_TUNE_INTERRUPTED:
        MCD_PRINTF(" TX training INTERRUPTED\n");
        break;

    default:
        MCD_PRINTF("Urecognized TX traing result %d\n", txStatus);
        break;

    }

    MCD_PRINTF("mcdSampleSerdesAutoTuneStatus  rxStatus = %d txStatus = %d returns %d\n", rxStatus, txStatus, rc);
    return MCD_OK;
}

/**
* @internal mcdSamplePortStop function
* @endinternal
*
* @brief   Release all port resources, calls to serdes power down
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSamplePortStop
(
    IN MCD_U32 phyId,
    IN MCD_U16 mdioPort
 )
{

    MCD_DEV_PTR pDev;
    MCD_STATUS rc;

    pDev = mcdSampleGetDrvDev(phyId);

    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSamplePortStop: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    /* used one serdes lane mode to stop overlapped port */
    rc = mcdPortStop( pDev,mdioPort,MCD_MODE_P1);
    MCD_PRINTF("\nmmcdSamplePortStop returns %d\n",rc);
    return MCD_OK;

}



/**
* @internal mcdSampleCheckLinkStatus function
* @endinternal
*
* @brief   Print the status of a port - UP/DOWN/LATCHED/NOT LATCHED.
*
*
* @param[in] phyId                - device number
* @param[in] portNum            - port number (0..8).
* @param[out] portOpMode     - port operation mode.
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note portOpMode needed because link status information is a union depending on port type (PCS, retimer, etc), and we don't know the
    port type from inside MCD (no API)
*
*/

MCD_STATUS mcdSampleCheckLinkStatus
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_PORT_OPERATION_MODE_ENT  portOpMode
)
{
    MCD_U16 currentStatus,latchedStatus, i;
    DETAILED_STATUS_TYPE statusDetail;
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleCheckLinkStatus: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    rc = mcdCheckLinkStatus(pDev,portNum, &currentStatus, &latchedStatus, &statusDetail);
    if(rc != MCD_OK)
    {
        MCD_PRINTF("mcdSampleCheckLinkStatus: mcdCheckLinkStatus returns %d\n", rc);
        return MCD_FAIL;
    }

    if((portOpMode == MCD_PORT_AGNOSTIC_OPERATION_MODE_E) || (portOpMode == MCD_PORT_PCS_OPERATION_MODE_E))
    {
        MCD_PRINTF("\nmcdSampleCheckLinkStatus: PCS:  host current %d hostLatched %d lineCurrent %d lineLatched %d"
                   " hostLinkChanged %d lineLinkChanged %d\n",
            statusDetail.P100_40_status.hostCurrent,
            statusDetail.P100_40_status.hostLatched,
            statusDetail.P100_40_status.lineCurrent,
            statusDetail.P100_40_status.lineLatched,
            statusDetail.P100_40_status.hostLinkChanged,
            statusDetail.P100_40_status.lineLinkChanged);
    }
    if((portOpMode == MCD_PORT_AGNOSTIC_OPERATION_MODE_E) || (portOpMode == MCD_PORT_RETIMER_SINGLE_LANE_OPERATION_MODE_E))
    {
        MCD_PRINTF("\nmcdSampleCheckLinkStatus:Retimer single lane: \n");
        MCD_PRINTF("lane %d: host current %d lineCurrent %d \n",portNum,
        statusDetail.R25_10_status.hostCurrent[portNum],
        statusDetail.R25_10_status.lineCurrent[portNum]);
    }

    if((portOpMode == MCD_PORT_AGNOSTIC_OPERATION_MODE_E) || (portOpMode == MCD_PORT_RETIMER_MULTI_LANE_OPERATION_MODE_E))
    {
        MCD_PRINTF("\nmcdSampleCheckLinkStatus:Retimer multi lane: \n");
        for(i=0; i<8; i++)
        {
            MCD_PRINTF("lane %d: host current %d lineCurrent %d \n",i,
            statusDetail.R100_40_status.hostCurrent[i],
            statusDetail.R100_40_status.lineCurrent[i]);
        }
    }

    MCD_PRINTF("\nmcdSampleCheckLinkStatus current status %d\n",currentStatus);
    MCD_PRINTF("\nmcdSampleCheckLinkStatus returns %d\n",rc);
    return MCD_OK;
}

/**
* @internal mcdSampleCheckPcsLinkStability function
* @endinternal
*
* @brief   Checking PCS link stability by reading the current
*          PCS status register and PCS interrupt changed
*          register for both host and line.
*
* @param[in] phyId                - device number
* @param[in] portNum            - port number (0..8).
* @param[out] numOfCycles     - number of test cycles.
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note in this sample code we are only printing the amount of
*       link flaps without returning any errors, however we are
*       recomending that in case of host or line flaps, run
*       this test again and not enable the MAC until the
*       stability check will pass.
*
*/
MCD_STATUS mcdSampleCheckPcsLinkStability
(
    MCD_U32 phyId,
    MCD_U16 portNum,
    MCD_U32 numOfCycles
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    MCD_U32 i; /* itterator*/
    MCD_U16 currentStatus,latchedStatus;
    DETAILED_STATUS_TYPE statusDetail;
    MCD_U16 hostFlaps = 0;
    MCD_U16 lineFlaps = 0;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleCheckPcsLinkStability: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }

    MCD_PRINTF("mcdSampleCheckPcsLinkStability: Running PCS Stability check in blocking mode, number "
               "of cycles %d\n", numOfCycles);

    /* first calling mcdCheckLinkStatus API to clear the interrupt cause bit */
    rc = mcdCheckLinkStatus(pDev,portNum, &currentStatus, &latchedStatus, &statusDetail);
    if(rc != MCD_OK)
    {
        MCD_PRINTF("mcdSampleCheckLinkStatus: mcdCheckLinkStatus returns %d\n", rc);
        return MCD_FAIL;
    }

    for (i=0; i< numOfCycles; i++)
    {
            rc = mcdCheckLinkStatus(pDev,portNum, &currentStatus, &latchedStatus, &statusDetail);
            if(rc != MCD_OK)
            {
                MCD_PRINTF("mcdSampleCheckLinkStatus: mcdCheckLinkStatus returns %d\n", rc);
                return MCD_FAIL;
            }

            if (statusDetail.P100_40_status.hostLinkChanged)
            {
                hostFlaps++;
            }
            if (statusDetail.P100_40_status.lineLinkChanged)
            {
                lineFlaps++;
            }
    }

    if (statusDetail.P100_40_status.hostCurrent && statusDetail.P100_40_status.lineCurrent &&
        !hostFlaps && !lineFlaps)
    {
        MCD_PRINTF(" PCS Stability Check successed\n");
    }
    else
    {
        MCD_PRINTF(" PCS Stability Check Failed\n");
    }

    MCD_PRINTF("\nmcdSampleCheckLinkStatus: PCS:  host current %d lineCurrent %d"
           " hostFlaps %d lineFlaps %d\n",
    statusDetail.P100_40_status.hostCurrent,
    statusDetail.P100_40_status.lineCurrent,
    hostFlaps,
    lineFlaps);

    return MCD_OK;
}


/**
* @internal mcdSampleSetLoopback function
* @endinternal
*
* @brief   Sets loopback of various kinds.
*
*
* @param[in] phyId                - device number
* @param[in] portSerdesNum  - represents port  (0..8) or serdes  (0..15) number depending on  serdesLoopback value.
* @param[in] hostSide     - host or lane side of a port, relevant for serdesLoopback == MCD_FALSE case only.
* @param[in] serdesLoopback     - if a loopback is port(PCS) or serdes type
* @param[in] serialLoopback     - if MCD_TRUE - TX->RX, aka Serial, Deep or Near-side Loopback
*               if  MCD_FALSE, it is RX->TX, aka Parallel, Shallow or Far-side Loopback
* @param[in] enable     - 1 - Loopback enable, 0  - Loopback disable.
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleSetLoopback 0,0, 0, 1, 1, 1
*
*/

MCD_STATUS mcdSampleSetLoopback
(
    IN MCD_U32 phyId,
    IN MCD_U16 portSerdesNum,
    IN MCD_BOOL hostSide,
    IN MCD_BOOL serdesLoopback,
    IN MCD_BOOL serialLoopback,
    IN MCD_BOOL enable
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    MCD_U16 lbType;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetLoopback: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }
    if(serdesLoopback == MCD_TRUE)
    {
        if(portSerdesNum >= MCD_SERDES_LANE_COUNT)
        {
            MCD_PRINTF("mcdSampleSetLoopback: serdes number cannot exceed 15\n");
            return MCD_FAIL;
        }

        if(enable == MCD_FALSE)
        {
            rc = mcdSerdesSetLoopback(pDev, portSerdesNum+1, MCD_CLEAR_SERDES_LOOPBACK);
            return rc;
        }

        if(serialLoopback == MCD_TRUE)
        {
            rc = mcdSerdesSetLoopback(pDev, portSerdesNum+1, MCD_DEEP_SERDES_LOOPBACK);
            return rc;
        }
        else
        {
            rc = mcdSerdesSetLoopback(pDev, portSerdesNum+1, MCD_SHALLOW_SERDES_LOOPBACK);
            return rc;
        }

    }
    else
    {
        if(serialLoopback == MCD_TRUE)
            lbType = MCD_DEEP_PCS_LB;
        else
        {
            MCD_PRINTF("mcdSampleSetLoopback: MCD_SHALLOW_PCS_LB not supported\n");
            return MCD_FAIL;
        }

        if(portSerdesNum >= MCD_MAX_PORT_NUM)
        {
            MCD_PRINTF("mcdSampleSetLoopback: port number cannot exceed 7\n");
            return MCD_FAIL;
        }

        if(enable == MCD_FALSE)
        {
            if(hostSide == MCD_TRUE)
            {
                rc = mcdSetHostLoopback(pDev, portSerdesNum, lbType, MCD_DISABLE);
            }
            else
            {
                rc = mcdSetLineLoopback(pDev, portSerdesNum, lbType, MCD_DISABLE);
            }
            return rc;
        }

        if(hostSide == MCD_TRUE)
        {
            rc = mcdSetHostLoopback(pDev, portSerdesNum, lbType, MCD_ENABLE);
        }
        else
        {
            rc = mcdSetLineLoopback(pDev, portSerdesNum, lbType, MCD_ENABLE);
        }
        return rc;

    }

    return MCD_OK;
}


MCD_STATUS mcdSampleSetPortLoopback
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_OP_MODE portMode,
    IN MCD_U16 host_or_line,
    IN MCD_U32  lbMode
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }
    rc = mcdSetPortLoopback(pDev, portNum, portMode, host_or_line, lbMode);
    return rc;
}


/**
* @internal mcdSampleSerdesAutoTuneStartExt function
* @endinternal
*
* @brief  Run types of RX and TX training on a SERDES.
*
* @param[in] phyId                - device number
* @param[in] serdesNum  - SERDES lane number (0..15)
* @param[in] rxTraining     -RX training type, including START/STOP for continiuos training
* @param[in] txTraining     -TX training type, including START/STOP for continiuos training
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleSerdesAutoTuneStartExt 0,2,4,0
*  for "no training" use MCD_IGNORE_RX_TRAINING or MCD_IGNORE_TX_TRAINING
*/
MCD_STATUS mcdSampleSerdesAutoTuneStartExt
(
    MCD_U32                         phyId,
    MCD_U8                          serdesNum,
    MCD_RX_TRAINING_MODES           rxTraining,
    MCD_TX_TRAINING_MODES           txTraining
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSerdesAutoTuneStartExt: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    rc = mcdSerdesAutoTuneStartExt(pDev, serdesNum, rxTraining, txTraining);
    MCD_PRINTF("\nmcdSampleSerdesAutoTuneStartExt  returns %d\n",rc);
    return MCD_OK;
}

MCD_STATUS mcdSampleEnhanceTuneLite
(
    MCD_U32       phyId,
    MCD_U8      serdesNum,
    MCD_SERDES_SPEED    serdesSpeed,
    MCD_U8                   min_dly,
    MCD_U8                   max_dly
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleEnhanceTuneLite: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
     MCD_PRINTF("\nmmcdSampleEnhanceTuneLite  wait starts\n");

     rc = mcdSerdesEnhanceTuneLite(pDev, &serdesNum, 1, serdesSpeed, min_dly, max_dly);
     MCD_PRINTF("\nmmcdSampleEnhanceTuneLite  returns %d\n",rc);
    return MCD_OK;
}

MCD_STATUS mcdSampleEnhanceTune
(
    IN MCD_U32 phyId,
    IN MCD_U8 serdesNum,
    MCD_U8  minLF,
    MCD_U8 maxLF
)
{
     MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleEnhanceTunes: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
     MCD_PRINTF("\nmmcdSampleEnhanceTune  wait starts\n");

     /*   mcdSerdesTxEnable(pDev,serdesNum,MCD_TRUE); */
    /*   avago_serdes_initialize_signal_ok(pDev->serdesDev,(serdesNum + 1), 2);*/
    rc = mcdSerdesEnhanceTune(pDev, &serdesNum, 1, minLF, maxLF);
    MCD_PRINTF("\nmmcdSampleEnhanceTune  returns %d\n",rc);
    return MCD_OK;
}
MCD_STATUS mcdSampleCm3PortPollingCntrl
(
    IN MCD_U32 phyId,
    IN MCD_U8 portNum,
    IN MCD_BOOL action
)
{
     MCD_DEV_PTR pDev;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleCm3PortPollingCntrl: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    if (!action)
    {
        /* stop CM3 polling on PCS link */
        ATTEMPT(mcdPortPollingLinkStatusStop(pDev, portNum));
        MCD_PRINTF("\nmcdSampleCm3PortPollingCntrl  STOP CM3 polling for port %d \n", portNum);

    }
    else
    {
        ATTEMPT(mcdPortPollingLinkStatusStart(pDev, portNum));
        MCD_PRINTF("\nmcdSampleCm3PortPollingCntrl  START CM3 polling for port %d \n", portNum);

    }
    return MCD_OK;
}
MCD_AP_SERDES_CONFIG_DATA serdesApParameters[4] =
{
    {0,0},
    {0,0},
    {0,0},
    {0,0}
};

/*******************************************************************************
 MCD_STATUS mmcdSetApParameters
    Set Auto-neg advertising speeds
*******************************************************************************/
MCD_STATUS mcdSampleApPortCtrlStart
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_U32 serdesNum,
    IN MCD_U32 polarityVector,
    IN MCD_U32 modesVector,
    IN MCD_BOOL fcPause,
    IN MCD_BOOL fcAsmDir,
    IN MCD_FEC_TYPE fecSup,
    IN MCD_FEC_TYPE fecReq,
    IN MCD_BOOL nonceDis,
    IN MCD_BOOL apEnable
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    MCD_CONFIG_SERDES_AP_PARAM config;
    MCD_U32 i;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleApPortCtrlStart: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    MCD_PRINTF("Parameters:\n");
    MCD_PRINTF("-----------\n\n");
    MCD_PRINTF("\tPort Num:      %d\n", portNum);
    MCD_PRINTF("\tLane Num:      %d\n", serdesNum);
    MCD_PRINTF("\tPolarity Mask: 0x%x\n", polarityVector);
    MCD_PRINTF("\tMode Mask:     0x%x\n", modesVector);
    MCD_PRINTF("\tLoopback:      %s\n", (nonceDis) ? "Enabled" : "Disabled");
    MCD_PRINTF("\tFECSup:        %d\n", fecSup);
    MCD_PRINTF("\tFECReq:        %d\n", fecReq);
    MCD_PRINTF("\tFC:            %s\n", (fcPause) ? "Enabled" : "Disabled");
    MCD_PRINTF("\tFC Asymmetric: %s\n", (fcAsmDir) ? "Enabled" : "Disabled");
    MCD_PRINTF("\tAP enable: %s\n", (apEnable) ? "Enabled" : "Disabled");

    config.numOfLanes = 4;
    for(i = 0; i < 4; i++)
    {
        serdesApParameters[i].txPolarity = (polarityVector  >> (2*i+1)) & 1;
        serdesApParameters[i].rxPolarity = (polarityVector  >> (2*i)) & 1;
        MCD_PRINTF("txPolarity: %d\n", serdesApParameters[i].txPolarity);
        MCD_PRINTF("rxPolarity: %d\n", serdesApParameters[i].rxPolarity);
    }
    config.params = (MCD_AP_SERDES_CONFIG_DATA* )&serdesApParameters;

    rc = mcdSetAPModeSelection( pDev, portNum, serdesNum, modesVector, fcPause, fcAsmDir, fecSup, fecReq, nonceDis, &config, MCD_TRUE);
    MCD_PRINTF("\nmcdSampleApPortCtrlStart returns %d\n", rc);
    return MCD_OK;
}

MCD_STATUS mcdSampleApManualRxParameters
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_U16 laneNum,
    IN MCD_SERDES_SPEED baudRate,
    IN MCD_U16  bitMap
)
{
    MCD_DEV_PTR pDev;
    MCD_PORT_AP_SERDES_RX_CONFIG rxCfg;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleApManualRxParmaeters: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    rxCfg.bandWidth = 14;
    rxCfg.dcGain = 99;
    rxCfg.highFrequency = 14;
    rxCfg.loopBandwidth = 0;
    rxCfg.lowFrequency = 4;
    rxCfg.serdesSpeed = (MCD_SERDES_SPEED)baudRate;
    rxCfg.squelch = 70;
    rxCfg.etlMaxDelay = 0;
    rxCfg.etlMinDelay = 0;
    rxCfg.etlEnable = MCD_TRUE;
    rxCfg.fieldOverrideBmp = bitMap;

    ATTEMPT(mcdSetApManualRxParameters(pDev,portNum,laneNum,&rxCfg));

    return MCD_OK;

}

MCD_STATUS mcdSampleApManualTxParameters
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_U16 laneNum,
    IN MCD_SERDES_SPEED baudRate,
    IN MCD_U8  bitMap
)
{
    MCD_DEV_PTR pDev;
    MCD_PORT_AP_SERDES_TX_CONFIG txCfg;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleApManualTxParmaeters: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    txCfg.txAmp = 8;
    txCfg.emph0 = 2;
    txCfg.emph1 = 3;
    txCfg.serdesSpeed = (MCD_SERDES_SPEED)baudRate;
    txCfg.txOverrideBmp = bitMap;

    ATTEMPT(mcdSetApManualTxParameters(pDev, portNum, laneNum, &txCfg));

    return MCD_OK;
}

MCD_STATUS mcdSampleSetLaneSteering
(
    IN MCD_U32 phyId,
    IN  MCD_U16      mdioPort,
    IN  MCD_MASTER_SLICE  masterSlice,
    IN  MCD_LANE_STEERING_MODE  laneSteeringMode,
    IN  MCD_FEC_TYPE  fecCorrect,
    IN  MCD_U32 modeVector,
    IN  MCD_BOOL autoNegAdvEnable,
    IN  MCD_U32 laneNumber,
    IN  MCD_BOOL laneSteeringOverride
)
{
    MCD_DEV_PTR pDev;
    MCD_MODE_CONFIG_PARAM   config;
    MCD_CONFIG_AP_PARAM     configAp;
    MCD_CONFIG_SERDES_AP_PARAM configApSerdes;
    MCD_U32 i, laneNum;
    MCD_U32 polarityVector = 0;
    MCD_BOOL reducedRxTraining = MCD_FALSE;
    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));
    memset(&configAp, 0, sizeof(MCD_CONFIG_AP_PARAM));
    memset(&configApSerdes, 0, sizeof(MCD_CONFIG_SERDES_AP_PARAM));

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetLaneSteering: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    laneNum = laneNumber;

    if (autoNegAdvEnable)
    {
        config.configApPtr = (MCD_CONFIG_AP_PARAM *)&configAp;
        config.configApPtr->laneNum = laneNum;
        configApSerdes.params = (MCD_AP_SERDES_CONFIG_DATA* )&serdesApParameters;
        configApSerdes.numOfLanes = 4;
        for(i = 0; i < 4; i++)
        {
            serdesApParameters[i].txPolarity = (polarityVector  >> (2*i+1)) & 1;
            serdesApParameters[i].rxPolarity = (polarityVector  >> (2*i)) & 1;
        }
        config.configApPtr->configApSerdesPtr = (MCD_CONFIG_SERDES_AP_PARAM*)&configApSerdes;
        if ((laneSteeringMode == MCD_4P_P1G_1000BaseX_STEERING) ||
            (laneSteeringMode == MCD_4P_P1G_SGMII_STEERING))
        {
            config.configApPtr->apEnable = MCD_TRUE;
        }
        if ((autoNegAdvEnable == MCD_TRUE) && (modeVector != 0))
        {
            config.configApPtr->apEnable = MCD_TRUE;
            config.configApPtr->fcAsmDir = MCD_FALSE;
            config.configApPtr->fcPause = MCD_FALSE;
            config.configApPtr->fecReq = fecCorrect;
            config.configApPtr->fecSup = fecCorrect;
            config.configApPtr->fecReqConsortium = fecCorrect;
            config.configApPtr->fecSupConsortium = fecCorrect;
            config.configApPtr->modeVector = modeVector;
            config.configApPtr->nonceDis = MCD_TRUE;
            config.configApPtr->retimerEnable = MCD_FALSE;
            config.configApPtr->ctleBiasVal = MCD_CTLE_BIAS_NORMAL;
            if (laneSteeringMode == MCD_G21L_P10G_NONE_STEERING_AP)
            {
                config.configApPtr->g21Mode = 1;
            }
        }
    }

    config.ctleParamsPtr = NULL;
    config.electricalParamsPtr = NULL;
    config.noPpmMode = MCD_NO_PPM_OFF_MODE;
    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_NORMAL;
    config.refClk.lsRefClkSel = MCD_PRIMARY_CLK;
    config.refClk.hsRefClkSel = MCD_PRIMARY_CLK;
    config.refClk.lsRefClkDiv = MCD_REF_CLK_NO_DIV;
    config.refClk.hsRefClkDiv = MCD_REF_CLK_NO_DIV;

    if (laneSteeringOverride)
    {
        config.laneSteerCfgPtr = (MCD_LANE_STEERING_OVERRIDE_CFG*)&laneSteeringOverrideMv;
    }
    else
    {
        config.laneSteerCfgPtr = NULL;
    }

    ATTEMPT(mcdSetLaneSteering(pDev, mdioPort, masterSlice,
                               laneSteeringMode, fecCorrect, reducedRxTraining, &config));

    return MCD_OK;

}


/*******************************************************************************
 MCD_STATUS mcdSampleOneSideModeSet
*******************************************************************************/
MCD_STATUS mcdSampleOneSideModeSet
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_OP_MODE portMode,
    IN MCD_FEC_TYPE fec,
    IN MCD_BOOL noPpmMode
)
{
    MCD_DEV_PTR pDev;
    MCD_MODE_CONFIG_PARAM config;

    config.noPpmMode = noPpmMode;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleOneSideModeSet: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    ATTEMPT(mcdOneSideSetMode(pDev, portNum, portMode, MCD_HOST_SIDE, fec, &config));
    MCD_PRINTF("\nmmcdSampleOneSideModeSet mode %d\n", portMode);

    return MCD_OK;

}

/*******************************************************************************
 MCD_STATUS mcdSampleSetModeSelectionExt
*******************************************************************************/
MCD_STATUS mcdSampleSetModeSelectionExt
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_OP_MODE portMode,
    IN MCD_BOOL autoNegAdvEnable,
    IN MCD_FEC_TYPE fec,
    IN MCD_U32 modeVector,
    IN MCD_BOOL retimerEnable,
    IN MCD_U32 laneNum,
    IN MCD_U32 polarityVector,
    IN MCD_BOOL setElecticalParams

)
{
    MCD_DEV_PTR             pDev;
    MCD_MODE_CONFIG_PARAM   config;
    MCD_CONFIG_AP_PARAM     configAp;
    MCD_CONFIG_SERDES_AP_PARAM configApSerdes;
    MCD_U32 i;
    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));
    memset(&configAp, 0, sizeof(MCD_CONFIG_AP_PARAM));
    memset(&configApSerdes, 0, sizeof(MCD_CONFIG_SERDES_AP_PARAM));
    config.noPpmMode = MCD_FALSE ;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetModeSelectionExt: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    config.configApPtr = (MCD_CONFIG_AP_PARAM*)&configAp;
    config.configApPtr->laneNum = laneNum;
    configApSerdes.params = (MCD_AP_SERDES_CONFIG_DATA* )&serdesApParameters;
    configApSerdes.numOfLanes = 4;
    for(i = 0; i < 4; i++)
    {
        serdesApParameters[i].txPolarity = (polarityVector  >> (2*i+1)) & 1;
        serdesApParameters[i].rxPolarity = (polarityVector  >> (2*i)) & 1;
    }
    config.configApPtr->configApSerdesPtr = (MCD_CONFIG_SERDES_AP_PARAM*)&configApSerdes;
    if ((autoNegAdvEnable == MCD_TRUE) && (modeVector != 0))
    {
        config.configApPtr->apEnable = MCD_TRUE;
        config.configApPtr->fcAsmDir = MCD_FALSE;
        config.configApPtr->fcPause = MCD_FALSE;
        config.configApPtr->fecReq = fec;
        config.configApPtr->fecSup = fec;
        config.configApPtr->fecReqConsortium = fec;
        config.configApPtr->fecSupConsortium = fec;
        config.configApPtr->modeVector = modeVector;
        config.configApPtr->nonceDis = MCD_TRUE;
        config.configApPtr->retimerEnable = retimerEnable;
        config.configApPtr->ctleBiasVal = MCD_CTLE_BIAS_HIGH;
        config.configApPtr->enSdTuningApRes = MCD_FALSE;
    }

    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_NORMAL;
    config.refClk.lsRefClkSel = MCD_PRIMARY_CLK;
    config.refClk.hsRefClkSel = MCD_PRIMARY_CLK;
    config.refClk.lsRefClkDiv = MCD_REF_CLK_NO_DIV;
    config.refClk.hsRefClkDiv = MCD_REF_CLK_NO_DIV;

    config.ctleParamsPtr = NULL;
    if (setElecticalParams)
    {
        config.electricalParamsPtr = (MCD_PORT_PER_SERDES_CONFIG_DATA*)&serdesElectricalParamsMv;
    }
    else
    {
        config.electricalParamsPtr = NULL;
    }
    ATTEMPT(mcdSetModeSelectionExt(pDev, portNum, portMode, autoNegAdvEnable, fec, &config));

    return MCD_OK;

}

/*******************************************************************************
 MCD_STATUS mcdSampleSetMultiPortSelectionExt
*******************************************************************************/
MCD_STATUS mcdSampleSetMultiPortSelectionExt
(
    IN MCD_U32 phyId,
    IN MCD_U8 portBitMap,
    IN MCD_OP_MODE portMode,
    IN MCD_BOOL autoNegAdvEnable,
    IN MCD_FEC_TYPE fec,
    IN MCD_U32 modeVector,
    IN MCD_BOOL retimerEnable,
    IN MCD_U32 iterrationCount,
    IN MCD_U32 delayTimeMs,
    IN MCD_BOOL multiLaneEnable
)
{
    MCD_U16 i,j,laneNum;
    for (j=0; j< iterrationCount; j++)
    {
        if (j)
        {
            for (i=0; i< 8; i++)
            {
                if (0x1 & (portBitMap>>i))
                {
                    ATTEMPT(mcdPortStop(pDevArray[phyId],i,portMode));
                }
            }
        }
        for (i = 0; i < 8; i++)
        {
            if ((portBitMap >> i) & 0x1)
            {
                if (multiLaneEnable && autoNegAdvEnable && (i == 4))
                {
                    laneNum = 7;
                }
                else
                {
                    laneNum = i;
                }
                ATTEMPT(mcdSampleSetModeSelectionExt(phyId, i, portMode, autoNegAdvEnable, fec, modeVector, retimerEnable, (MCD_U32)laneNum, 0, MCD_FALSE));
            }
        }
        mcdWrapperMsSleep(delayTimeMs);
    }

    return MCD_OK;
}


/*******************************************************************************
 MCD_STATUS mcdSampleSetModeSelectionExt
*******************************************************************************/
MCD_STATUS mcdSampleSetModeSelectionExt2
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_OP_MODE portMode,
    IN MCD_BOOL autoNegAdvEnable,
    IN MCD_FEC_TYPE fec,
    IN MCD_U32 modeVector,
    IN MCD_U32 noPpmMode,
    IN MCD_U32 laneNum,
    IN MCD_U32 biasVal,
    IN MCD_U32 g21Mode

)
{
    MCD_DEV_PTR             pDev;
    MCD_MODE_CONFIG_PARAM   config;
    MCD_CONFIG_AP_PARAM     configAp;
    MCD_CONFIG_SERDES_AP_PARAM configApSerdes;
    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));
    memset(&configAp, 0, sizeof(MCD_CONFIG_AP_PARAM));
    memset(&configApSerdes, 0, sizeof(MCD_CONFIG_SERDES_AP_PARAM));
    config.noPpmMode = noPpmMode ;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetModeSelectionExt: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    config.configApPtr = (MCD_CONFIG_AP_PARAM*)&configAp;
    config.configApPtr->laneNum = laneNum;
    configApSerdes.params = (MCD_AP_SERDES_CONFIG_DATA* )&serdesApParameters;
    configApSerdes.numOfLanes = 4;

    config.configApPtr->configApSerdesPtr = (MCD_CONFIG_SERDES_AP_PARAM*)&configApSerdes;
    if ((autoNegAdvEnable == MCD_TRUE) && (modeVector != 0))
    {
        config.configApPtr->apEnable = MCD_TRUE;
        config.configApPtr->fcAsmDir = MCD_FALSE;
        config.configApPtr->fcPause = MCD_FALSE;
        config.configApPtr->fecReq = fec;
        config.configApPtr->fecSup = fec;
        config.configApPtr->fecReqConsortium = fec;
        config.configApPtr->fecSupConsortium = fec;
        config.configApPtr->modeVector = modeVector;
        config.configApPtr->nonceDis = MCD_TRUE;
        config.configApPtr->retimerEnable = MCD_FALSE;
        config.configApPtr->ctleBiasVal = biasVal;
        config.configApPtr->g21Mode = g21Mode;
    }

    config.ctleBiasParams.host = biasVal;
    config.ctleBiasParams.line = biasVal;

    config.ctleParamsPtr = NULL;
    config.electricalParamsPtr = NULL;
    ATTEMPT(mcdSetModeSelectionExt(pDev, portNum, portMode, autoNegAdvEnable, fec, &config));

    return MCD_OK;

}

MCD_STATUS mcdSampleSetModeSelectionCtle
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_OP_MODE portMode,
    IN MCD_FEC_TYPE fec,
    IN MCD_U32 dc,
    IN MCD_U32 hf,
    IN MCD_U32 lf,
    IN MCD_U32 bw,
    IN MCD_U32 calibrationMode
)
{
    MCD_DEV_PTR             pDev;
    MCD_MODE_CONFIG_PARAM   config;
    MCD_PORT_PER_SERDES_CTLE_CONFIG_DATA   ctleParams;
    MCD_U32 numLanes,i;

    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));
    memset(&ctleParams, 0, sizeof(ctleParams));
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetModeSelectionExt: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    config.noPpmMode = MCD_NO_PPM_OFF_MODE ;
    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_NORMAL;
    /*
    dcGain - 0x1
    lowFrequency - 0x2
    highFrequency - 0x4
    bandWitdh - 0x8
    loopBandWith - 0x10
    */
    numLanes = mcdPortModeGetLanesNum(portMode,MCD_LINE_SIDE);
    for (i=0; i < numLanes; i++) {
        ctleParams.ctleParams[portNum+i].dcGain = dc;
        ctleParams.ctleParams[portNum+i].highFrequency = hf;
        ctleParams.ctleParams[portNum+i].lowFrequency = lf;
        ctleParams.ctleParams[portNum+i].bandWidth = bw;
        ctleParams.ctleParams[portNum+i].bitMapEnable = 0xF;
    }

    config.ctleParamsPtr = (MCD_PORT_PER_SERDES_CTLE_CONFIG_DATA*)&ctleParams;
    config.calibrationMode = calibrationMode;
    config.electricalParamsPtr = NULL;
    ATTEMPT(mcdSetModeSelectionExt(pDev, portNum, portMode, MCD_FALSE, fec, &config));

    return MCD_OK;

}

/**
* @internal mcdSampleSetModeSelectionEoOverride function
* @endinternal
*
* @brief   set manually EO range for line side
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSampleSetModeSelectionEoOverride
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_OP_MODE portMode,
    IN MCD_FEC_TYPE fec,
    IN MCD_U32 minEye,
    IN MCD_U32 maxEye,
    IN MCD_U32 calibrationMode,
    IN MCD_CALIBRATION_ALGO_E calAlgorithm
)
{
    MCD_DEV_PTR             pDev;
    MCD_MODE_CONFIG_PARAM   config;
    MCD_LINE_SIDE_EO_TH_OVERRIDE  eoThresholdCfg;

    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));
    memset(&eoThresholdCfg, 0, sizeof(eoThresholdCfg));
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetModeSelectionEoOverride: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    config.noPpmMode = MCD_NO_PPM_OFF_MODE ;
    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_NORMAL;


    eoThresholdCfg.minEyeThreshold = minEye;
    eoThresholdCfg.maxEyeThreshold = maxEye;
    config.eyeThresholdPtr = (MCD_LINE_SIDE_EO_TH_OVERRIDE*)&eoThresholdCfg;


    config.ctleParamsPtr = NULL;
    config.calibrationMode = calibrationMode;
    config.calibrationAlgo = calAlgorithm;
    config.electricalParamsPtr = NULL;
    ATTEMPT(mcdSetModeSelectionExt(pDev, portNum, portMode, MCD_FALSE, fec, &config));

    return MCD_OK;

}


/**
* @internal mcdSampleSetModeSelectionConfInterval function
* @endinternal
*
* @brief   set manually EO range for line side
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if 0
MCD_STATUS mcdSampleSetModeSelectionConfInterval
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_OP_MODE portMode,
    IN MCD_FEC_TYPE fec,
    IN MCD_CTLE_CALIBRATION_MODE_E calibrationMode,
    IN MCD_CALIBRATION_ALGO_E calAlgorithm
)
{
    MCD_DEV_PTR             pDev;
    MCD_MODE_CONFIG_PARAM   config;

    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetModeSelectionConfInterval: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    config.noPpmMode = MCD_NO_PPM_OFF_MODE ;
    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_NORMAL;

    config.confidenceParamsPtr = NULL;
    config.ctleParamsPtr = NULL;
    config.calibrationMode = calibrationMode;
    config.calibrationAlgo = calAlgorithm;
    config.electricalParamsPtr = NULL;
    ATTEMPT(mcdSetModeSelectionExt(pDev, portNum, portMode, MCD_FALSE, fec, &config));

    return MCD_OK;

}
#else
MCD_STATUS mcdSampleSetModeSelectionConfInterval
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum,
    IN MCD_OP_MODE portMode,
    IN MCD_FEC_TYPE fec,
    IN MCD_U32 lfLowThreshold,
    IN MCD_U32 lfHighThreshold,
    IN MCD_U32 hfThreshold,
    IN MCD_U32 calibrationMode,
    IN MCD_CALIBRATION_ALGO_E calAlgorithm
)
{
    MCD_DEV_PTR             pDev;
    MCD_MODE_CONFIG_PARAM   config;
    MCD_CONFIDENCE_INTERVAL_PARAMS_OVERRIDE  confidenceParams;

    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));
    memset(&confidenceParams, 0, sizeof(MCD_CONFIDENCE_INTERVAL_PARAMS_OVERRIDE));
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetModeSelectionConfInterval: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    config.noPpmMode = MCD_NO_PPM_OFF_MODE ;
    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_NORMAL;


    confidenceParams.lfLowThreshold = lfLowThreshold;
    confidenceParams.lfHighThreshold = lfHighThreshold;
    confidenceParams.hfThreshold = hfThreshold;
    config.confidenceParamsPtr = (MCD_CONFIDENCE_INTERVAL_PARAMS_OVERRIDE *)&confidenceParams;


    config.ctleParamsPtr = NULL;
    config.calibrationMode = calibrationMode;
    config.calibrationAlgo = calAlgorithm;
    config.electricalParamsPtr = NULL;
    ATTEMPT(mcdSetModeSelectionExt(pDev, portNum, portMode, MCD_FALSE, fec, &config));

    return MCD_OK;

}
#endif
/**
* @internal mcdSampleSetLaneRemapping function
* @endinternal
*
* @brief   Configure all serdeses in LaneRemap mode
*
* @param[in] phyId
* @param[in] host_or_line                 -  serdes side
* @param[in] sliceNum           - slice number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSampleSetLaneRemapping
(
    IN  MCD_U32      phyId,
    IN  MCD_U16      host_or_line,
    IN  MCD_U16      sliceNum
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetLaneRemapping: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }


    rc = mcdSetLaneRemapping(pDev, host_or_line, sliceNum, serdesTxRxLaneRemapMv);

    MCD_PRINTF("mcdSetLaneRemapping returns %d\n",rc);

    return MCD_OK;
}
/**
*
* @internal mcdSampleAutoNegCheckCompleteExt function
* @endinternal
* @brief   Complete host side configuration for AP mode, after neagotiation and training on AP side has been completed.
*
*
* @param[in] phyId                - device number
* @param[in] mdioPort  -  port  (0..8)
* @param[in] set_speed     - if TRUE, the speed found in AP nogotiation on line side is configured on host side,
*                                           if FALSE, the funtion serves to check if line side speed negotiation was resolved -
*                                           resulting speed is printed out, and if it is not 0, it means resoltion was successful.
* @param[in] polarityVector     -polarity vector to be used on host side lanes
* @param[in] setElecticalParams     -TX and CTLE SERDES values to be used in training on host side of port,
*                                                    0 means - use default parameters
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleAutoNegCheckCompleteExt 0, 0, 1, 0, 0
**/
MCD_STATUS mcdSampleAutoNegCheckCompleteExt
(
    IN MCD_U32 phyId,
    IN MCD_U16 mdioPort,
    IN MCD_BOOL set_speed,
    IN MCD_U32 polarityVector,
    IN MCD_BOOL setElecticalParams
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;
    MCD_U16 speed;
    MCD_MODE_CONFIG_PARAM   config;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleAutoNegCheckCompleteExt: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }

    memset(&config, 0, sizeof(MCD_MODE_CONFIG_PARAM));

    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_NORMAL;

    config.ctleParamsPtr = NULL;
    if (setElecticalParams)
    {
        config.electricalParamsPtr = (MCD_PORT_PER_SERDES_CONFIG_DATA*)&serdesElectricalParamsMv;
    }
    else
    {
        config.electricalParamsPtr = NULL;
    }

    rc = mcdAutoNegCheckCompleteExt( pDev,  mdioPort,MCD_LINE_SIDE,set_speed,&config, &speed);

    MCD_PRINTF("\nmcdSampleAutoNegCheckCompleteExt  speed = %x returns %d\n",speed,rc);
    return MCD_OK;

}

/**
* @internal mcdSamplePostInitSetBootMode function
* @endinternal
*
* @brief   set boot mode after Driver was initalized
*
* @param[in] mode                 -  Boot Mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSamplePostInitSetBootMode
(
    IN MCD_BOOT_MODE mode
)
{
    MCD_U32 i;
    for (i=0; i< MCD_MAX_DEVICES; i++)
    {
        if (pDevArray[i] != NULL)
        {
            ATTEMPT(mcdPostInitSetBootMode(pDevArray[i],mode));
        }
    }

    return MCD_OK;
}

/**
* @internal mcdSamplePreInitSetBootMode function
* @endinternal
*
* @brief   set boot mode before driver initalization
*
* @param[in] mode                 -  Boot Mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSamplePreInitSetBootMode
(
    IN MCD_BOOT_MODE mode
)
{
    ATTEMPT(mcdPreInitSetBootMode(mode));
    return MCD_OK;
}


/*******************************************************************************
 MCD_STATUS mcdSampleApPortCtrlStatsShow
*******************************************************************************/
MCD_STATUS mcdSampleApPortCtrlStatsShow
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum

)
{
    MCD_DEV_PTR pDev;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleApPortCtrlStatsShow: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    mcdApPortCtrlStatsShow(pDev, portNum);

    return MCD_OK;
}

/*******************************************************************************
 MCD_STATUS mcdSamplePollingStatRetimerPerPortValuesClear
*******************************************************************************/
MCD_STATUS mcdSamplePollingStatRetimerPerPortValuesClear
(
    IN  MCD_U32 phyId,
    IN  MCD_U16 portNum,
    IN  MCD_U16 lineSide
)
{
    MCD_DEV_PTR pDev;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF(
            "mcdSamplePollingStatRetimerPerPortValuesClear: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    return mcdPollingStatRetimerPerPortValuesClear(
        pDev, portNum, lineSide);
}

/*******************************************************************************
 MCD_STATUS mcdSampleApPortCtrlStatsShow
*******************************************************************************/
MCD_STATUS mcdSampleApPortCtrlStatusShow
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum
)
{
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleApPortCtrlStatusShow: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    mcdApPortCtrlStatusShow(pDev, portNum);

    return MCD_OK;
}

/*******************************************************************************
 MCD_STATUS mcdSampleApPortCtrlDebugInfoShow
*******************************************************************************/
MCD_STATUS mcdSampleApPortCtrlDebugInfoShow
(
    IN MCD_U32 phyId,
    IN MCD_U16 portNum

)
{
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleApPortCtrlStatusShow: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    mcdApPortCtrlDebugInfoShow(pDev, portNum, MCD_TRUE);

    return MCD_OK;
}

/**
* @internal mcdSamplePollingDebugInfoShow function
* @endinternal
*
* @brief   Print FW Hws real-time polling statistic information stored in system
*
* @param[in] pDev                     - pointer to device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSamplePollingDebugInfoShow
(
    IN MCD_U32 phyId
)
{
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSamplePollingDebugInfoShow: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    mcdPollingStatShow(pDev);

    return MCD_OK;
}

/**
* @internal mcdPollingStatClear function
* @endinternal
*
* @brief   Clear FW Hws real-time polling statistic information stored in system
*
* @param[in] pDev                     - pointer to device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSamplePollingStatClear
(
    IN MCD_U32 phyId
)
{
    MCD_DEV_PTR      pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("%s: return fail\n", __func__);
        return MCD_FAIL;
    }

    return (mcdPollingStatClear(pDev));
}

/**
* @internal mcdSampleSerdesPMDTrainingLog function
* @endinternal
*
* @brief   Print the log of AP negotiation.on a SERDES lane
*
*
* @param[in] phyId                - device number
* @param[in] serdesNum  -  serdes number  (0..15)
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleSerdesPMDTrainingLog 0, 0
*
*/

MCD_STATUS mcdSampleSerdesPMDTrainingLog
(
    IN MCD_U32 phyId,
    IN MCD_U16 serdesNum
)
{
    MCD_TRAINING_INFO traingInfo;
    MCD_DEV_PTR pDev;
    MCD_U16 sAddr = serdesNum+1, i;
    MCD_STATUS rc = MCD_OK;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleCheckLinkStatus: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }

    rc = mcdSerdesPMDTrainingLog(pDev,sAddr,&traingInfo);
    if(rc != MCD_OK)
    {
        MCD_PRINTF("mcdSampleSerdesPMDTrainingLog: mcdSerdesPMDTrainingLog returns %d\n", rc);
        return MCD_FAIL;
    }


    MCD_PRINTF("reset = %d\n rx_metric = %d\n local_preset = %d\n local_initialize %d\n",
        traingInfo.reset,
        traingInfo.rx_metric,
        traingInfo.local.preset,
        traingInfo.local.initialize);
    for(i=0;i<3;i++)
    {
        MCD_PRINTF("local.tap[%d].inc = %d local.tap[%d].dec = %d local.tap[%d].max = %d local.tap[%d].min = %d\n",
            i, traingInfo.local.tap[i].inc,
            i, traingInfo.local.tap[i].dec,
            i, traingInfo.local.tap[i].max,
            i, traingInfo.local.tap[i].min);
    }

    MCD_PRINTF("remote_preset = %d\n remote_initialize %d\n",
        traingInfo.remote.preset,
        traingInfo.remote.initialize);

    for(i=0;i<3;i++)
    {
        MCD_PRINTF("remote.tap[%d].inc = %d remote.tap[%d].dec = %d remote.tap[%d].max = %d remote.tap[%d].min = %d\n",
            i, traingInfo.remote.tap[i].inc,
            i, traingInfo.remote.tap[i].dec,
            i, traingInfo.remote.tap[i].max,
            i, traingInfo.remote.tap[i].min);
    }

    for(i=0;i<8;i++)
    {
        MCD_PRINTF("last_remote_request_%d = %d\n",
            i, traingInfo.last_remote_request[i]);
    }
    MCD_PRINTF("last_local_request = %d\n",
        traingInfo.last_local_request);

    return rc;
}



MCD_STATUS mcdSampleUnMaskCm3ToHostDoorBellInt
(
    unsigned int phyIndex,
    MCD_BOOL enable
)
{
    #ifndef ASIC_SIMULATION
    MCD_DEV_PTR          pDev;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        MCD_PRINTF("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }

    if (enable == MCD_TRUE)
    {
        ATTEMPT(mcdHwXmdioWrite32bit(pDev, 0, REG_ADDR_GLOBAL_INTERRUPT_SUMMARY_MASK, 0x80 /*bit 7*/));

        ATTEMPT(mcdHwXmdioWrite32bit(pDev, 0, REG_ADDR_MG_INTERNAL_INTERRUPT_MASK, 0x4000 /*bit 14*/));
    }
    else
    {
        ATTEMPT(mcdHwXmdioWrite32bit(pDev, 0, REG_ADDR_GLOBAL_INTERRUPT_SUMMARY_MASK, 0x0 ));

        ATTEMPT(mcdHwXmdioWrite32bit(pDev, 0, REG_ADDR_MG_INTERNAL_INTERRUPT_MASK , 0x0 ));
    }

    #endif
    return MCD_OK;
}


MCD_STATUS mcdSampleCm3ToHostSignalReceive
(
    unsigned int        phyIndex
)
{
#ifndef ASIC_SIMULATION
    MCD_DEV_PTR         pDev;
    MCD_STATUS          rc;
    MCD_U32             signalCode;
    MCD_U32             globalCauseData;
    MCD_U32             internalCauseData;
    MCD_U32             portNum;

    pDev = mcdSampleGetDrvDev(phyIndex);
    if (pDev == NULL)
    {
        MCD_PRINTF("no device with given ID %d\n", phyIndex);
        return MCD_OK;
    }

    ATTEMPT(mcdHwXmdioRead32bit(pDev, 0, REG_ADDR_GLOBAL_INTERRUPT_SUMMARY_CAUSE, &globalCauseData))
    if (((globalCauseData >> 7) & 0x1) == 0)
    {
        MCD_PRINTF("mcdSampleCm3ToHostSignalReceive: doorbell interrupt was not occured, globalCauseData 0x%08x \n", globalCauseData);
        return MCD_FAIL;
    }

    ATTEMPT(mcdHwXmdioRead32bit(pDev, 0, REG_ADDR_MG_INTERNAL_INTERRUPT_CAUSE, &internalCauseData))
    if (((internalCauseData >> 14) & 0x1) == 0)
    {
        MCD_PRINTF("mcdSampleCm3ToHostSignalReceive: doorbell interrupt was not occured, internalCauseData 0x%08x \n", internalCauseData);
        return MCD_FAIL;
    }

    rc = mcdGetCm3ToHostSignal(pDev, &signalCode);
    if (rc != MCD_OK)
    {
        MCD_PRINTF("mcdGetCm3ToHostSignal failed, rc %d\n", rc);
        return rc;
    }

    for (portNum = 0; portNum < 8; portNum++)
    {
       /* check if one of the following events occured
        MCD_DOORBELL_EVENT_PORT_0_802_3_AP_E  0x00000001
        MCD_DOORBELL_EVENT_PORT_1_802_3_AP_E  0x00000002
        MCD_DOORBELL_EVENT_PORT_2_802_3_AP_E  0x00000004
        MCD_DOORBELL_EVENT_PORT_3_802_3_AP_E  0x00000008
        MCD_DOORBELL_EVENT_PORT_4_802_3_AP_E  0x00000010
        MCD_DOORBELL_EVENT_PORT_5_802_3_AP_E  0x00000020
        MCD_DOORBELL_EVENT_PORT_6_802_3_AP_E  0x00000040
        MCD_DOORBELL_EVENT_PORT_7_802_3_AP_E  0x00000080 */
        if ((signalCode >> portNum) & 0x1)
        {
            MCD_PRINTF("AP resolution success, MCD_DOORBELL_EVENT_PORT_%d_802_3_AP_E\n", portNum);
        }
    }


    /* clean DORBELL INTERRUPT REGISTER */
    ATTEMPT(mcdHwXmdioWrite32bit(pDev, 0, REG_ADDR_MG_CM3_TO_HOST_DOORBELL, 0x0));

#endif
    return MCD_OK;
}

/* Used for mcdSampleCm3Test function */
#define R1_MG_LOCATION              0x30000480
#define R2_MG_LOCATION              0x30000484
#define R3_MG_LOCATION              0x3000048C

#define RUNIT_1_LOCATION            0x13100  /* SSMI_Xbar_port_0 [16:0]*/
#define RUNIT_2_LOCATION            0x13200  /* TWSI_Xbar_port_0 [16:0]*/

#define DFX_1_LOCATION              0x800F8000  /* PIPE_Select [16:0]*/
#define DFX_2_LOCATION              0x800F848C  /* Server_XBAR_Target_Port_15_Config [9:0]*/

#define COM0_1_LOCATION             0x20004  /* Chenable_com0 */
#define COM0_2_LOCATION             0x201C0  /* Force_interrupt0 [15:0]*/

#define COM1_1_LOCATION             0x40004  /* Chenable_com1  [3:0]*/
#define COM1_2_LOCATION             0x401C0  /* Force_interrupt0 [15:0]*/

#define SBC_1_LOCATION              0x3300001C  /* SBC_Interrupt_1_Mask [31:1]*/
#define SBC_2_LOCATION              0x33000068  /* SBC_Metal_Fix [15:0]*/

#define SD_0_0_LOCATION             0x3200002c  /* SerDes 0_0 interrupt_0_mask_out*/
#define SD_0_1_LOCATION             0x3200102c  /* SerDes 0_1 interrupt_0_mask_out */
#define SD_0_2_LOCATION             0x3200202c  /* SerDes 0_2 interrupt_0_mask_out */
#define SD_0_3_LOCATION             0x3200302c  /* SerDes 0_3 interrupt_0_mask_out */
#define SD_0_4_LOCATION             0x3200402c  /* SerDes 0_4 interrupt_0_mask_out */
#define SD_0_5_LOCATION             0x3200502c  /* SerDes 0_5 interrupt_0_mask_out */
#define SD_0_6_LOCATION             0x3200602c  /* SerDes 0_6 interrupt_0_mask_out */
#define SD_0_7_LOCATION             0x3200702c  /* SerDes 0_7 interrupt_0_mask_out */
#define SD_1_0_LOCATION             0x3208002c  /* SerDes 1_0 interrupt_0_mask_out */
#define SD_1_1_LOCATION             0x3208102c  /* SerDes 1_1 interrupt_0_mask_out */
#define SD_1_2_LOCATION             0x3208202c  /* SerDes 1_2 interrupt_0_mask_out */
#define SD_1_3_LOCATION             0x3208302c  /* SerDes 1_3 interrupt_0_mask_out */
#define SD_1_4_LOCATION             0x3208402c  /* SerDes 1_4 interrupt_0_mask_out */
#define SD_1_5_LOCATION             0x3208502c  /* SerDes 1_5 interrupt_0_mask_out */
#define SD_1_6_LOCATION             0x3208602c  /* SerDes 1_6 interrupt_0_mask_out */
#define SD_1_7_LOCATION             0x3208702c  /* SerDes 1_7 interrupt_0_mask_out */

#define SD_0_0_INT_LOCATION         0x33040420  /* SerDes 0_0_int Interrupt_enable [7:0]*/
#define SD_0_1_INT_LOCATION         0x33040820  /* SerDes 0_1_int Interrupt_enable [7:0]*/
#define SD_0_2_INT_LOCATION         0x33040C20  /* SerDes 0_2_int Interrupt_enable [7:0]*/
#define SD_0_3_INT_LOCATION         0x33041020  /* SerDes 0_3_int Interrupt_enable [7:0]*/
#define SD_0_4_INT_LOCATION         0x33041420  /* SerDes 0_4_int Interrupt_enable [7:0]*/
#define SD_0_5_INT_LOCATION         0x33041820  /* SerDes 0_5_int Interrupt_enable [7:0]*/
#define SD_0_6_INT_LOCATION         0x33041C20  /* SerDes 0_6_int Interrupt_enable [7:0]*/
#define SD_0_7_INT_LOCATION         0x33042020  /* SerDes 0_7_int Interrupt_enable [7:0]*/
#define SD_1_0_INT_LOCATION         0x33042420  /* SerDes 1_0_int Interrupt_enable [7:0] */
#define SD_1_1_INT_LOCATION         0x33042820  /* SerDes 1_1_int Interrupt_enable [7:0]*/
#define SD_1_2_INT_LOCATION         0x33042C20  /* SerDes 1_2_int Interrupt_enable [7:0]*/
#define SD_1_3_INT_LOCATION         0x33043020  /* SerDes 1_3_int Interrupt_enable [7:0]*/
#define SD_1_4_INT_LOCATION         0x33043420  /* SerDes 1_4_int Interrupt_enable [7:0]*/
#define SD_1_5_INT_LOCATION         0x33043820  /* SerDes 1_5_int Interrupt_enable [7:0]*/
#define SD_1_6_INT_LOCATION         0x33043C20  /* SerDes 1_6_int Interrupt_enable [7:0]*/
#define SD_1_7_INT_LOCATION         0x33044020  /* SerDes 1_7_int Interrupt_enable [7:0]*/


#define AP_1_INT_LOCATION           0x311802A0  /* AP ANEG_Spare_register_0 */
#define AP_2_INT_LOCATION           0x311806A0  /* AP ANEG_Spare_register_0*/
#define AP_3_INT_LOCATION           0x31180AA0  /* AP ANEG_Spare_register_0*/
#define AP_4_INT_LOCATION           0x31180EA0  /* AP ANEG_Spare_register_0*/
#define AP_5_INT_LOCATION           0x311812A0  /* AP ANEG_Spare_register_0*/
#define AP_6_INT_LOCATION           0x311816A0  /* AP ANEG_Spare_register_0*/
#define AP_7_INT_LOCATION           0x31181AA0  /* AP ANEG_Spare_register_0*/
#define AP_8_INT_LOCATION           0x31181EA0  /* AP ANEG_Spare_register_0*/

#define AP_1_EXT_LOCATION           0x31180150  /* AP  */
#define AP_2_EXT_LOCATION           0x31180550  /* AP */
#define AP_3_EXT_LOCATION           0x31180950  /* AP */
#define AP_4_EXT_LOCATION           0x31180D50  /* AP */
#define AP_5_EXT_LOCATION           0x31181150  /* AP */
#define AP_6_EXT_LOCATION           0x31181550  /* AP */
#define AP_7_EXT_LOCATION           0x31181950  /* AP */
#define AP_8_EXT_LOCATION           0x31181D50  /* AP */

#define MEM_CM3_LOCATION            0x00000000  /* CM3 memory - address 0*/
#define MEM_CM3_1_LOCATION          0x00000004  /* CM3 memory - address 1*/

#define SBC_SPICO_LOCATION          0x3307F418  /* SPICO_BP [31:0]*/
#define SBC_CTRL_LOCATION           0x3307Fa08  /* Generic Write Page0 [7:0]*/
#define AVAGO_TSEN_LOCATION         0x33044410  /* Spare_reg [15:0]*/

#define RFU_1_LOCATION              0x00018100 /* GPIO_0_31_Datout*/
#define RFU_2_LOCATION              0x0001823C /* LED_Mix_Register*/

/* test access to all 16 Serdes units  and AP blocks
   after special CM3 Load with Design test is run on CM3*/

int mcdSampleCm3Test(
            IN MCD_U32 phyId
)
{
    MCD_DEV_PTR pDev;
    MCD_U32 data_read;
    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF(" mcdSampleCm3Test phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

     MCD_PRINTF(" mcdSampleCm3Test start\n");
     mcdHwXmdioRead32bit (pDev, 0,RUNIT_1_LOCATION,&data_read);
     if (data_read != 0x00011234)  MCD_PRINTF("wrong data in reg RUNIT_1_LOCATION %d \n");

     mcdHwXmdioRead32bit (pDev, 0,RUNIT_2_LOCATION,&data_read);
     if (data_read != 0x00010321) MCD_PRINTF("wrong data in reg RUNIT_2_LOCATION %d \n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,DFX_1_LOCATION,&data_read);
     if (data_read != 0x00015467) MCD_PRINTF("wrong data in reg DFX_1_LOCATION %d \n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,DFX_2_LOCATION,&data_read);
     if (data_read != 0x000002ad) MCD_PRINTF("wrong data in reg DFX_2_LOCATION %d \n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,COM0_1_LOCATION,&data_read);
     if (data_read != 0x0006) MCD_PRINTF("wrong data in reg COM0_1_LOCATION %d \n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,COM0_2_LOCATION,&data_read);
     if (data_read != 0x8765) MCD_PRINTF("wrong data in reg COM0_2_LOCATION% %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,COM1_1_LOCATION,&data_read);
     if (data_read != 0x0004) MCD_PRINTF("wrong data in reg COM1_1_LOCATION %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,COM1_2_LOCATION,&data_read);
     if (data_read != 0x6543) MCD_PRINTF("wrong data in reg COM1_2_LOCATION %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SBC_1_LOCATION,&data_read);
     if (data_read != 0x00005412) MCD_PRINTF("wrong data in reg SBC_1_LOCATION %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SBC_2_LOCATION,&data_read);
     if (data_read != 0x9632) MCD_PRINTF("wrong data in reg SBC_2_LOCATION%d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_1_0_LOCATION,&data_read);
     if (data_read != 0x00000012) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_1_1_LOCATION,&data_read);
     if (data_read != 0x00000022) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_1_2_LOCATION,&data_read);
     if (data_read != 0x00000032) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_1_3_LOCATION,&data_read);
     if (data_read != 0x00000042) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_1_4_LOCATION,&data_read);
     if (data_read != 0x00000052) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_1_5_LOCATION,&data_read);
     if (data_read != 0x00000062) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_1_6_LOCATION,&data_read);
     if (data_read != 0x00000072) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_1_7_LOCATION, &data_read);
     if (data_read != 0x00000094) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_0_0_LOCATION,&data_read);
     if (data_read != 0x000000a2) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_0_1_LOCATION,&data_read);
     if (data_read != 0x000000b2) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_0_2_LOCATION,&data_read);
     if (data_read != 0x000000c2) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_0_3_LOCATION,&data_read);
     if (data_read != 0x000000d2) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_0_4_LOCATION,&data_read);
     if (data_read != 0x000000e2) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_0_5_LOCATION,&data_read);
     if (data_read != 0x000000f2) MCD_PRINTF("wrong data in reg %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_0_6_LOCATION,&data_read);
     if (data_read != 0x00000092) MCD_PRINTF("wrong data in reg SD_0_6_LOCATION %d\n",data_read);

     mcdHwXmdioRead32bit (pDev, 0,SD_0_7_LOCATION,&data_read);
     if (data_read != 0x00000082) MCD_PRINTF("wrong data in reg SD_0_7_LOCATION%d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_1_INT_LOCATION,&data_read);
      if (data_read != 0x0000abcd)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_2_INT_LOCATION,&data_read);
      if (data_read != 0x0000abca)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_3_INT_LOCATION,&data_read);
      if (data_read != 0x0000abcb)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_4_INT_LOCATION,&data_read);
      if (data_read != 0x0000abcc)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_5_INT_LOCATION,&data_read);
      if (data_read != 0x0000abce)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_6_INT_LOCATION,&data_read);
      if (data_read != 0x0000abcf)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_7_INT_LOCATION,&data_read);
      if (data_read != 0x0000abc1)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_8_INT_LOCATION,&data_read);
      if (data_read != 0x0000abc2)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SBC_SPICO_LOCATION,&data_read);
      if (data_read != 0x85214796)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SBC_CTRL_LOCATION,&data_read);
      if (data_read != 0x00000046)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AVAGO_TSEN_LOCATION,&data_read);
      if (data_read != 0x00007532)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_1_0_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_1_1_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_1_2_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_1_3_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_1_4_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_1_5_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_1_6_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_1_7_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_0_0_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_0_1_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_0_2_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_0_3_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_0_4_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_0_5_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_0_6_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,SD_0_7_INT_LOCATION,&data_read);
      if (data_read != 0x00000030) MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_1_EXT_LOCATION,&data_read);
      if (data_read != 0x0000abcd)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_2_EXT_LOCATION,&data_read);
      if (data_read != 0x0000abca)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_3_EXT_LOCATION,&data_read);
      if (data_read != 0x0000abcb)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_4_EXT_LOCATION,&data_read);
      if (data_read != 0x0000abcc)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_5_EXT_LOCATION,&data_read);
      if (data_read != 0x0000abce)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_6_EXT_LOCATION,&data_read);
      if (data_read != 0x0000abcf)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_7_EXT_LOCATION,&data_read);
      if (data_read != 0x0000abc1)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,AP_8_EXT_LOCATION,&data_read);
      if (data_read != 0x0000abc2)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,RFU_1_LOCATION,&data_read);
      if (data_read != 0x12345678)        MCD_PRINTF("wrong data in reg %d\n",data_read);

      mcdHwXmdioRead32bit (pDev, 0,RFU_2_LOCATION,&data_read);
      if (data_read != 0x00000169)        MCD_PRINTF("wrong data in reg %d\n",data_read);

     MCD_PRINTF(" mcdSampleCm3Test end\n");
       return MCD_OK;
}

#endif

MCD_STATUS mcdSampleSerdesPolarityTest
(
    IN  MCD_U32  phyId,
    IN  MCD_U32  portBitmap,
    IN  MCD_U32  serdesBitmap, /* serdes lanes to configure and check */
    IN  MCD_U16  swReset,
    IN  MCD_U32  loopsAmount,
    IN  MCD_U32  valuesAmount,
    ...
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS  rc;
    va_list     args;
    MCD_U32     loopIndex;
    MCD_U32     valuesBitmap;
    MCD_U32     valuesIndex;
    MCD_U16     i;
    MCD_U16     port;
    MCD_U16     polarity;
    MCD_U16     polarityGet;
    MCD_U16     serdesToPortArr[8];
    MCD_U32     bitmapBase;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF(" mcdSampleCm3Test phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }
    MCD_PRINTF(
        "parameters phyId portBitmap serdesBitmap swReset loopsAmount valuesAmount valuesBitmap0 ... \n");
    MCD_PRINTF(
        "bitmaps format 0xOOPPQQRR - OO - HOST TX, PP - PP HOST RX, QQ - LINE TX, RR - LINE RX\n");

    MCD_PRINTF(
        "Loops: %d valuesAmount %d\n", loopsAmount, valuesAmount);
    va_start(args, valuesAmount);
    for (valuesIndex = 0; (valuesIndex < valuesAmount); valuesIndex++)
    {
        valuesBitmap = va_arg(args, MCD_U32);
        MCD_PRINTF("0x%08X ", valuesBitmap);
    }
    va_end(args);
    MCD_PRINTF("\n");

    port = 0xFF;
    for (i = 0; (i < 8); i++)
    {
        if (portBitmap & (1 << i)) port = i;
        serdesToPortArr[i] = port;
    }

    /* check param and print */
    /* set host TX polarity */
    bitmapBase = 24;
    for (i = 0; (i < 8); i++)
    {
        if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
        port = serdesToPortArr[i];
        MCD_PRINTF("set host TX polarity serdes %d \n", i);
        if (port >= 8)
        {
            MCD_PRINTF("no port found\n");
            return MCD_FAIL;
        }
    }
    /* set host RX polarity */
    bitmapBase = 16;
    for (i = 0; (i < 8); i++)
    {
        if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
        port = serdesToPortArr[i];
        MCD_PRINTF("set host RX polarity serdes %d \n", i);
        if (port >= 8)
        {
            MCD_PRINTF("no port found\n");
            return MCD_FAIL;
        }
    }
    /* set line TX polarity */
    bitmapBase = 8;
    for (i = 0; (i < 8); i++)
    {
        if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
        port = serdesToPortArr[i];
        MCD_PRINTF("set line TX polarity serdes %d \n", i);
        if (port >= 8)
        {
            MCD_PRINTF("no port found\n");
            return MCD_FAIL;
        }
    }
    /* set line RX polarity */
    bitmapBase = 0;
    for (i = 0; (i < 8); i++)
    {
        if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
        port = serdesToPortArr[i];
        MCD_PRINTF("set line RX polarity serdes %d \n", i);
        if (port >= 8)
        {
            MCD_PRINTF("no port found\n");
            return MCD_FAIL;
        }
    }

    for (loopIndex = 0; (loopIndex < loopsAmount); loopIndex++)
    {
        va_start(args, valuesAmount);
        for (valuesIndex = 0; (valuesIndex < valuesAmount); valuesIndex++)
        {
            valuesBitmap = va_arg(args, MCD_U32);
            /* set host TX polarity */
            bitmapBase = 24;
            for (i = 0; (i < 8); i++)
            {
                if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
                port = serdesToPortArr[i];
                polarity = ((valuesBitmap >> (i + bitmapBase)) & 1);
                rc = mcdSetTxPolarity(
                    pDev, port, MCD_HOST_SIDE,
                    (i - port)/*laneOffset*/, polarity, swReset);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdSetTxPolarity HOST serdes %d failed rc %d\n", i, rc);
                    return MCD_FAIL;
                }
            }
            /* set host RX polarity */
            bitmapBase = 16;
            for (i = 0; (i < 8); i++)
            {
                if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
                port = serdesToPortArr[i];
                polarity = ((valuesBitmap >> (i + bitmapBase)) & 1);
                rc = mcdSetRxPolarity(
                    pDev, port, MCD_HOST_SIDE,
                    (i - port)/*laneOffset*/, polarity, swReset);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdSetRxPolarity HOST serdes %d failed rc %d\n", i, rc);
                    return MCD_FAIL;
                }
            }
            /* set line TX polarity */
            bitmapBase = 8;
            for (i = 0; (i < 8); i++)
            {
                if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
                port = serdesToPortArr[i];
                polarity = ((valuesBitmap >> (i + bitmapBase)) & 1);
                rc = mcdSetTxPolarity(
                    pDev, port, MCD_LINE_SIDE,
                    (i - port)/*laneOffset*/, polarity, swReset);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdSetTxPolarity LINE serdes %d failed rc %d\n", i, rc);
                    return MCD_FAIL;
                }
            }
            /* set line RX polarity */
            bitmapBase = 0;
            for (i = 0; (i < 8); i++)
            {
                if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
                port = serdesToPortArr[i];
                polarity = ((valuesBitmap >> (i + bitmapBase)) & 1);
                rc = mcdSetRxPolarity(
                    pDev, port, MCD_LINE_SIDE,
                    (i - port)/*laneOffset*/, polarity, swReset);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdSetRxPolarity LINE serdes %d failed rc %d\n", i, rc);
                    return MCD_FAIL;
                }
            }
            /* check host TX polarity */
            bitmapBase = 24;
            for (i = 0; (i < 8); i++)
            {
                if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
                port = serdesToPortArr[i];
                polarity = ((valuesBitmap >> (i + bitmapBase)) & 1);
                rc = mcdGetTxPolarity(
                    pDev, port, MCD_HOST_SIDE,
                    (i - port)/*laneOffset*/, &polarityGet);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdGetTxPolarity HOST serdes %d failed rc %d\n", i, rc);
                    return MCD_FAIL;
                }
                if (polarity != polarityGet)
                {
                    MCD_PRINTF(
                        "mcdGetTxPolarity HOST serdes %d set %d get %d\n",
                        i, polarity, polarityGet);
                    return MCD_FAIL;
                }
            }
            /* check host RX polarity */
            bitmapBase = 16;
            for (i = 0; (i < 8); i++)
            {
                if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
                port = serdesToPortArr[i];
                polarity = ((valuesBitmap >> (i + bitmapBase)) & 1);
                rc = mcdGetRxPolarity(
                    pDev, port, MCD_HOST_SIDE,
                    (i - port)/*laneOffset*/, &polarityGet);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdGetRxPolarity HOST serdes %d failed rc %d\n", i, rc);
                    return MCD_FAIL;
                }
                if (polarity != polarityGet)
                {
                    MCD_PRINTF(
                        "mcdGetRxPolarity HOST serdes %d set %d get %d\n",
                        i, polarity, polarityGet);
                    return MCD_FAIL;
                }
            }
            /* check line TX polarity */
            bitmapBase = 8;
            for (i = 0; (i < 8); i++)
            {
                if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
                port = serdesToPortArr[i];
                polarity = ((valuesBitmap >> (i + bitmapBase)) & 1);
                rc = mcdGetTxPolarity(
                    pDev, port, MCD_LINE_SIDE,
                    (i - port)/*laneOffset*/, &polarityGet);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdGetTxPolarity LINE serdes %d failed rc %d\n", i, rc);
                    return MCD_FAIL;
                }
                if (polarity != polarityGet)
                {
                    MCD_PRINTF(
                        "mcdGetTxPolarity LINE serdes %d set %d get %d\n",
                        i, polarity, polarityGet);
                    return MCD_FAIL;
                }
            }
            /* check line RX polarity */
            bitmapBase = 0;
            for (i = 0; (i < 8); i++)
            {
                if (((1 << (i + bitmapBase)) & serdesBitmap) == 0) continue;
                port = serdesToPortArr[i];
                polarity = ((valuesBitmap >> (i + bitmapBase)) & 1);
                rc = mcdGetRxPolarity(
                    pDev, port, MCD_LINE_SIDE,
                    (i - port)/*laneOffset*/, &polarityGet);
                if (rc != MCD_OK)
                {
                    MCD_PRINTF("mcdGetRxPolarity LINE serdes %d failed rc %d\n", i, rc);
                    return MCD_FAIL;
                }
                if (polarity != polarityGet)
                {
                    MCD_PRINTF(
                        "mcdGetRxPolarity LINE serdes %d set %d get %d\n",
                        i, polarity, polarityGet);
                    return MCD_FAIL;
                }
            }
        }
        va_end(args);
    }
    return MCD_OK;
}

/**
* @internal mcdSampleSerdesTxEnable function
* @endinternal
*
* @brief   Enable TX on a SEDES lane.
*
*
* @param[in] phyId                   device number
* @param[in] serdesNum           serdes  (0..15) number
* @param[in] enable                1 - TX enable, 0  - TX disable.
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleSerdesTxEnable 0,0,1
*
*/
MCD_STATUS mcdSampleSerdesTxEnable
(
    IN MCD_U32 phyId,
    IN MCD_U8 serdesNum,
    IN MCD_BOOL enable
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSerdesTxEnable: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }

    rc = mcdSerdesTxEnable(pDev, serdesNum, enable);
    return rc;

}

/**
* @internal mcdSampleSerdesRxPowerCtrl function
* @endinternal
*
* @brief   power up/down SerDes Rx direction
*
* @param[in] pDev                     - phy Id
* @param[in] serdesNum                - serdes number
* @param[in] powerUp                  - power up/down
* @param[in] baudRate                 - serdes baud rate
* @param[in] retimerMode              - retimer enable/disable
*
* @retval MCD_OK                   - on success
* @retval MCD_FAIL                 - on error
*/
MCD_STATUS mcdSampleSerdesRxPowerCtrl
(
    IN MCD_U32 phyId,
    IN MCD_U32 serdesNum,
    IN unsigned char powerUp,
    IN MCD_SERDES_SPEED baudRate,
    IN MCD_BOOL retimerMode
)
{
    MCD_DEV_PTR pDev;

        MCD_REF_CLK_SEL refClkSel = MCD_PRIMARY_CLK;
    MCD_REF_CLK_DIV refClkDiv = MCD_REF_CLK_NO_DIV;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSerdesRxEnable: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }



    ATTEMPT(mcdSerdesRxPowerCtrl(pDev,serdesNum,powerUp,baudRate,retimerMode,refClkSel,refClkDiv));

    return MCD_OK;
}

/**
* @internal mcdSampleSerdesTxPowerCtrl function
* @endinternal
*
* @brief   power up/down SerDes Tx direction
*
* @param[in] phyId                     - phy Id
* @param[in] serdesNum                - serdes number
* @param[in] powerUp                  - power up/down
* @param[in] baudRate                 - serdes baud rate
* @param[in] retimerMode              - retimer enable/disable
*
* @retval MCD_OK                   - on success
* @retval MCD_FAIL                 - on error
*/
MCD_STATUS mcdSampleSerdesTxPowerCtrl
(
    IN MCD_U32 phyId,
    IN MCD_U32 serdesNum,
    IN unsigned char powerUp,
    IN MCD_SERDES_SPEED baudRate,
    IN MCD_BOOL retimerMode
)
{
    MCD_DEV_PTR pDev;

        MCD_REF_CLK_SEL refClkSel = MCD_PRIMARY_CLK;
    MCD_REF_CLK_DIV refClkDiv = MCD_REF_CLK_NO_DIV;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSerdesTxEnable: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }



    ATTEMPT(mcdSerdesTxPowerCtrl(pDev,serdesNum,powerUp,baudRate,retimerMode,refClkSel,refClkDiv));

    return MCD_OK;
}

/**
* @internal mcdSampleSerdesResetImpl function
* @endinternal
*
* @brief   set SERDES reset of different types
*
*
* @param[in] phyId                   device number
* @param[in] serdesNum           serdes  (0..15) number
* @param[in]  analogReset -     Analog Reset  1= On 0 = Off
* @param[in]  analogReset -     digital Reset  1= On 0 = Off
* @param[in]  analogReset -     SyncE Reset  1= On 0 = Off
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleSerdesResetImpl 0,0,1,1,0
*
*/
MCD_STATUS mcdSampleSerdesResetImpl
(
    IN MCD_U32 phyId,
    IN MCD_U8 serdesNum,
    IN MCD_U8     analogReset,
    IN MCD_U8     digitalReset,
    IN MCD_U8     syncEReset
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSerdesResetImpl: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }

    rc = mcdSerdesResetImpl(pDev, serdesNum, analogReset, digitalReset, syncEReset);
    return rc;
}


MCD_STATUS mcdSampleSetSerdesElectricalParams
(
    IN  MCD_U8  serdesNum,
    IN  MCD_U8  rxPolarity,
    IN  MCD_U8  txPolarity,
    IN  MCD_U16  attenuation,
    IN  MCD_U16  preCursor,
    IN  MCD_U16  postCursor)

{
   serdesElectricalParamsMv[serdesNum].rxPolarity  = rxPolarity;
   serdesElectricalParamsMv[serdesNum].txPolarity  = txPolarity ;
   serdesElectricalParamsMv[serdesNum].attenuation = attenuation;
   serdesElectricalParamsMv[serdesNum].preCursor   = preCursor  ;
   serdesElectricalParamsMv[serdesNum].postCursor  = postCursor ;
   return MCD_OK;
}

MCD_STATUS mcdSampleSetExtSelectionMode
(
    IN MCD_U32      phyId,
    IN MCD_U16      mdioPort,
    IN MCD_OP_MODE  portMode,
    IN MCD_BOOL     autoNegAdvEnable,
    IN MCD_BOOL setElecticalParams,
    IN MCD_FEC_TYPE fecCorrect,
    IN MCD_REF_CLK_SEL hsRefClkSel,
    IN MCD_REF_CLK_SEL lsRefClkSel,
    IN MCD_REF_CLK_DIV hsRefClkDiv,
    IN MCD_REF_CLK_DIV lsRefClkDiv
)
{
    MCD_MODE_CONFIG_PARAM config;
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSetExtSelectionMode: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    /* Initialization of DB to default */
    ATTEMPT(mcdSetDefaultInitDB(pDev, mdioPort, &config));

    if (setElecticalParams)
    {
        config.electricalParamsPtr = (MCD_PORT_PER_SERDES_CONFIG_DATA*)&serdesElectricalParamsMv;
    }
    else
    {
        config.electricalParamsPtr = NULL;
    }

    config.noPpmMode = MCD_NO_PPM_OFF_MODE;
    config.refClk.lsRefClkSel = lsRefClkSel;
    config.refClk.hsRefClkSel = hsRefClkSel;
    config.refClk.lsRefClkDiv = lsRefClkDiv;
    config.refClk.hsRefClkDiv = hsRefClkDiv;
    config.ctleBiasParams.host = MCD_CTLE_BIAS_NORMAL;
    config.ctleBiasParams.line = MCD_CTLE_BIAS_HIGH;

    ATTEMPT(mcdSetModeSelectionExt(pDev, mdioPort, portMode, autoNegAdvEnable, fecCorrect, &config));
    MCD_PRINTF("\nmcdSampleSetExtModeSelection mode %d\n", portMode);

    return MCD_OK;

}

MCD_STATUS mcdSampleSyncEClkConfig
(
    IN  MCD_U32          phyId,
    IN  MCD_U16          mdioPort,
    IN  MCD_U32          laneNum,
    IN  MCD_RCLK_DIV_VAL divider,
    IN  MCD_BOOL         squelchEnable,
    OUT MCD_RCLK_OUT_PIN outPin
)
{
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSyncEClkConfig: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    ATTEMPT(mcdSyncEClkConfig(pDev, mdioPort, laneNum, divider, squelchEnable, outPin));

    return MCD_OK;
}

/**
* @internal mcdSampleGetBootModeState function
* @endinternal
*
* @brief   get the current boot mode
*
* @param[in] phyId
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSampleGetBootModeState
(
    IN  MCD_U32  phyId
)
{
    MCD_DEV_PTR pDev;
    MCD_BOOT_MODE bootMode;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleGetBootModeState: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    ATTEMPT(mcdGetBootModeState(pDev, &bootMode));
    if (bootMode)
    {
        MCD_PRINTF("boot mode: High-Availability, phyId %d \n", phyId);
    }
    else
    {
        MCD_PRINTF("boot mode: Regular, phyId %d \n", phyId);
    }
    return MCD_OK;
}

MCD_STATUS mcdSampleSerdesClockRegular
(
    IN  MCD_U32  phyId,
    IN  MCD_U16  serdesNum,
    IN  MCD_SERDES_SPEED baudRate       /*  Retimer mode from the list             */
)
{

    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSyncEClkConfig: phyId [%d] not exist\n", phyId);
        return MCD_FAIL;
    }

    ATTEMPT(mcdSerdesClockRegular(pDev, serdesNum, baudRate));
    return MCD_OK;
}

/*******************************************************************************
*  mcdSampleSerdesAutoTuneResult
*
* DESCRIPTION:
*       Print the output of the Serdes auto-tune procedure in a sequence list of serdes number
*
* INPUTS:
*       phyId       - phy number
*       serdesStart - The fist Serdes # to start the print
*       serdesNum   - number of Serdes lanes to be printed
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
MCD_STATUS mcdSampleSerdesAutoTuneResult
(
    IN  MCD_U32     phyId,
    IN  MCD_U8      serdesStart,
    IN  MCD_U8      serdesNum
)
{
    MCD_STATUS   rc;
    MCD_U32     i;
    MCD_DEV_PTR pDev;
    MCD_AUTO_TUNE_RESULTS results;
    MCD_U8        serdesIdx;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("\nMCD_STATUS_NOT_INITIALIZED\n");
        return MCD_OK;
    }

    /*serdesStart++;*/

    for (serdesIdx = serdesStart; serdesIdx < serdesNum ;serdesIdx++)
    {
      rc = mcdSerdesAutoTuneResult(pDev,(MCD_U8)serdesIdx,&results);
      if (rc != MCD_OK)
      {
          MCD_PRINTF("\nexecute mvHwsSerdesAutoTuneResult fail\n");
          return MCD_OK;
      }

      MCD_PRINTF("\nSERDES # %d\n", serdesIdx);

      for (i = 0; i < 13; i++)
      {
          MCD_PRINTF("DFE%d is %d   ", i, results.DFE[i]);
      }
      MCD_PRINTF("\nHF is %d  \n", results.HF);
      MCD_PRINTF("LF is %d  \n", results.LF);
      MCD_PRINTF("DC is %d  \n", results.DC);
      MCD_PRINTF("BW is %d  \n", results.BW);
      MCD_PRINTF("LB is %d  \n", results.LB);
      MCD_PRINTF("EO is %d  \n", results.EO);
      MCD_PRINTF("\nTX Amp is %d  \n", results.txAmp);
      MCD_PRINTF("TX Emph0 is %d  \n", results.txEmph0);
      MCD_PRINTF("TX Emph1 is %d\n", results.txEmph1);
    }
    return MCD_OK;
}

/**
* @internal mcdSampleSerdesSpicoInterrupt function
* @endinternal
*
* @brief   execute Spico interrupts.
*
* @param[in] phyId                   device number
* @param[in] serdesNum           SERDES number (0..15)
* @param[in] interruptCode       interrupt code
* @param[in] interruptData        interrupt data
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleSerdesSpicoInterrupt 0,1,1,5
*
*/
MCD_STATUS mcdSampleSerdesSpicoInterrupt
(
    IN MCD_U32    phyId,
    IN MCD_U32    serdesNum,
    IN MCD_U32    interruptCode,
    IN MCD_U32    interruptData
)
{
    MCD_DEV_PTR    pDev;
    MCD_STATUS     rc;
    int            result;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleSerdesSpicoInterrupt: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }

    rc = mcdSerdesSpicoInterrupt(pDev, serdesNum, interruptCode, interruptData, &result);
    if (rc != MCD_OK)
    {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
        return rc;
    }

    MCD_PRINTF("\nInterrupt Code: 0x%-08X data: 0x%-08X result: 0x%-08X\n", interruptCode, interruptData, result);

    return rc;

}

/*******************************************************************************
* mcdSampleHwGetPhyRegMask32bit
*
* DESCRIPTION:
*       Sample Wrapper for BV Read register
*
* INPUTS:
*     phyId    - phy number
*     mdioPort - mdio port
*     reg      - Register address
*     mask     - Mask bits
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
MCD_STATUS mcdSampleHwGetPhyRegMask32bit(
    IN  MCD_U32     phyId,
    IN  MCD_U32     mdioPort,
    IN  MCD_U32     reg,
    IN  MCD_U32     mask )
{

    MCD_U32  value;
    MCD_STATUS rc;
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF(" MCD_STATUS_NOT_INITIALIZED\n");
        return MCD_OK;
    }

    if (mask == 0) mask = 0xFFFFFFFF;

    rc = mcdHwGetPhyRegMask32bit(pDev, mdioPort, reg, mask, &value);
    if (rc == MCD_OK)
    {
      MCD_PRINTF("Address: 0x%X  Data = 0x%X\n", reg, value);
    } else {
      MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}


/*******************************************************************************
* mcdSampleHwSetPhyRegMask32bit
*
* DESCRIPTION:
*       Sample Wrapper for BV Write register
*
* INPUTS:
*     phyId    - phy number
*     mdioPort - mdio port
*     reg      - Register address
*     mask     - Mask bits
*     value    - Value to be set in the register
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
MCD_STATUS mcdSampleHwSetPhyRegMask32bit(
    IN  MCD_U32     phyId,
    IN  MCD_U32     mdioPort,
    IN  MCD_U32     reg,
    IN  MCD_U32     mask,
    IN  MCD_U32     value)
{

    MCD_STATUS rc;
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF(" MCD_STATUS_NOT_INITIALIZED\n");
        return MCD_OK;
    }

    if (mask == 0) mask = 0xFFFFFFFF;

    rc = mcdHwSetPhyRegMask32bit(pDev, mdioPort, reg, mask, value);
    if (rc == MCD_OK)
    {
      MCD_PRINTF("Address: 0x%X  Data = 0x%X\n", reg, value);
    } else {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/*******************************************************************************
* mcdSampleSetLineSidePRBSPattern
*
* DESCRIPTION:
*       Sample Wrapper for setting Line Side PRBS pattern
*
* INPUTS:
*     phyId    - phy number
*     mdioPort - mdio port
*     reg      - Register address
*     mask     - Mask bits
*     value    - Value to be set in the register
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
MCD_STATUS mcdSampleSetLineSidePRBSPattern
(
    IN  MCD_U32   phyId,
    IN  MCD_U32   mdioPort,
    IN  MCD_U16   laneOffset,
    IN  MCD_PRBS_LINE_SELECTOR_TYPE pattSel,
    IN  MCD_PATTERN_LINE_AB_SELECTOR_TYPE pattSubSel
)
{
    MCD_STATUS    rc;

    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc = mcdSetLineSidePRBSPattern(pDev, mdioPort, laneOffset, pattSel, pattSubSel);

    if (rc != MCD_OK) {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/*******************************************************************************
* mcdSampleSetPRBSEnableTxRx
*
* DESCRIPTION:
*       This function is used to start or stop the PRBS transmit and/or
*       receiver.
*
* INPUTS:
*       pDev     - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       mdioPort - MDIO port address, 0-31
*       host_or_line - which interface is being set:
*                      MCD_HOST_SIDE
*                      MCD_LINE_SIDE
*       laneOffset - 0..3 for lanes 0-3
*       txEnable - MCD_ENABLE or MCD_DISABLE to start or stop the transmit
*       rxEnable - MCD_ENABLE or MCD_DISABLE to start or stop the receiver
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
MCD_STATUS mcdSampleSetPRBSEnableTxRx
(
    IN  MCD_U32  phyId,
    IN  MCD_U16  mdioPort,
    IN  MCD_U16  host_or_line,
    IN  MCD_U16  laneOffset,
    IN  MCD_U16  txEnable,
    IN  MCD_U16  rxEnable
)
{
    MCD_STATUS  rc;
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc= mcdSetPRBSEnableTxRx(pDev, mdioPort, host_or_line, laneOffset, txEnable, rxEnable);

    if (rc != MCD_OK) {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/*******************************************************************************
* mcdSamplePRBSCounterReset
*
* DESCRIPTION:
*       This function is used to reset the counters when the PRBS has been
*       setup for manual clearing instead of clear-on-read. Default
*       is to use manual clearing. Call mcdSetPRBSEnableClearOnRead() to
*       enable clearing the counters when reading the registers.
*
* INPUTS:
*       pDev     - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       mdioPort - MDIO port address, 0-31
*       host_or_line - which interface is being set:
*                      MCD_HOST_SIDE
*                      MCD_LINE_SIDE
*       laneOffset - 0..3 for lanes 0-3
* RETURNS:
*       MCD_OK if query was successful, MCD_FAIL if not
*
* COMMENTS:
*       Assumes the PRBS has not been setup for clear-on-read.
*
*******************************************************************************/
MCD_STATUS mcdSamplePRBSCounterReset
(
    IN  MCD_U32  phyId,
    IN  MCD_U16  mdioPort,
    IN  MCD_U16  host_or_line,
    IN  MCD_U16  laneOffset
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc= mcdPRBSCounterReset(pDev, mdioPort, host_or_line, laneOffset);

    if (rc != MCD_OK) {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/******************************************************************************
* mcdSampleSetPRBSWaitForLock
*
* DESCRIPTION:
*       TConfigures PRBS to wait for locking before counting, or to wait for
*       locking before counting.
*
* INPUTS:
*       pDev     - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       mdioPort - MDIO port address, 0-31
*       host_or_line - which interface is being set:
*                      MCD_HOST_SIDE
*                      MCD_LINE_SIDE
*       laneOffset - 0..3 for lanes 0-3
*       disableWaitforLock - 1 counts PRBS before locking, 0 waits
*           for locking before counting
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MCD_OK if query was successful, MCD_FAIL if not
*
* COMMENTS:
*       Should be called before starting the receiver.
*
*******************************************************************************/
MCD_STATUS mcdSampleSetPRBSWaitForLock
(
    IN  MCD_U32  phyId,
    IN  MCD_U16  mdioPort,
    IN  MCD_U16  host_or_line,
    IN  MCD_U16  laneOffset,
    IN  MCD_U16  disableWaitforLock
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc = mcdSetPRBSWaitForLock(pDev, mdioPort, host_or_line, laneOffset, disableWaitforLock);

    if (rc != MCD_OK) {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/******************************************************************************
* mcdSampleSetPRBSClearOnRead
*
* DESCRIPTION:
*       The default for the PRBS counters is to be cleared manually by
*       calling mcdPRBSCounterReset(). This configures either to read-clear,or
*       manual clear (by setting a register bit).
*
* APPLICABLE DEVICES:
*       ???
* INPUTS:
*       pDev     - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       mdioPort - MDIO port address, 0-31
*       host_or_line - which interface is being set:
*                      MCD_HOST_SIDE
*                      MCD_LINE_SIDE
*       laneOffset - 0..3 for lanes 0-3
*       enableReadClear - 1 enables clear-on-read, 0 enables manual clear
*           (by setting register bit).
*
* OUTPUTS:
*       disableWaitforLock - 1 counts PRBS before locking, 0 waits
*           for locking before counting
*
* RETURNS:
*       MCD_OK if query was successful, MCD_FAIL if not
*
* COMMENTS:
*       Should be called before starting the receiver.
*
*******************************************************************************/
MCD_STATUS mcdSampleSetPRBSClearOnRead
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   host_or_line,
    IN  MCD_U16   laneOffset,
    IN  MCD_U16   enableReadClear
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc = mcdSetPRBSClearOnRead(pDev, mdioPort, host_or_line, laneOffset, enableReadClear);

    /* pack output arguments to galtis string */
    if (rc != MCD_OK) {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/******************************************************************************
* mcdSampleGetPRBSClearOnRead
*
* DESCRIPTION:
*       Checks whether the PRBS is configured to clear-on-read (1) or manual
*       cleared (0).
*
* APPLICABLE DEVICES:
*       ???
* INPUTS:
*       pDev     - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       mdioPort - MDIO port address, 0-31
*       host_or_line - which interface is being set:
*                      MCD_HOST_SIDE
*                      MCD_LINE_SIDE
*       laneOffset - 0..3 for lanes 0-3
*
* OUTPUTS:
*       enableReadClear - 1 clear-on-read enabled, 0 manual clear is enabled
*
* RETURNS:
*       MCD_OK if query was successful, MCD_FAIL if not
*
* COMMENTS:
*       None
*
*******************************************************************************/
MCD_STATUS mcdSampleGetPRBSClearOnRead
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   host_or_line,
    IN  MCD_U16   laneOffset
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;
    MCD_U16       enableReadClear = 0;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc = mcdGetPRBSClearOnRead(pDev, mdioPort, host_or_line, laneOffset, &enableReadClear);

    if (rc == MCD_OK) {
        MCD_PRINTF("Enable Read On Clear = %d\n", enableReadClear);
    } else {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/******************************************************************************
* mcdSampleGetPRBSLocked
*
* DESCRIPTION:
*       Returns the indicator if the PRBS receiver is locked or not.
*
* APPLICABLE DEVICES:
*       ???
* INPUTS:
*       pDev     - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       mdioPort - MDIO port address, 0-31
*       host_or_line - which interface is being set:
*                      MCD_HOST_SIDE
*                      MCD_LINE_SIDE
*       laneOffset - 0..3 for lanes 0-3
*
* OUTPUTS:
*       prbsLocked - MTD_TRUE if the PRBS receiver is locked, MTD_FALSE otherwise
*
* RETURNS:
*       MCD_OK if query was successful, MCD_FAIL if not
*
* COMMENTS:
*       Should be called after starting the receiver.
*
*******************************************************************************/
MCD_STATUS mcdSampleGetPRBSLocked
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   host_or_line,
    IN  MCD_U16   laneOffset
)
{
    MCD_STATUS  rc;
    MCD_DEV_PTR pDev;
    MCD_BOOL prbsLocked = MCD_FALSE;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc = mcdGetPRBSLocked(pDev, mdioPort, host_or_line, laneOffset, &prbsLocked);

    if (rc == MCD_OK) {
        MCD_PRINTF("PRBS Locked = %d\n", prbsLocked);
    } else {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/******************************************************************************
* mcdSampleGetPRBSCounts
*
* DESCRIPTION:
*       Returns the 48-bit results in the output parameters above. If the PRBS
*       control has been set to clear-on-read, the registers will clear. If
*       not, they must be cleared manually by calling mcdPRBSCounterReset()
*
* APPLICABLE DEVICES:
*       ???
* INPUTS:
*       pDev     - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       mdioPort - MDIO port address, 0-31
*       host_or_line - which interface is being set:
*                      MCD_HOST_SIDE
*                      MCD_LINE_SIDE
*       laneOffset - 0..3 for lanes 0-3
*
* OUTPUTS:
*       txBitCount - number of bits transmitted
*       rxBitCount - number of bits received
*       rxBitErrorCount - number of bits in error
*
* RETURNS:
*       MCD_OK if query was successful, MCD_FAIL if not
*
* COMMENTS:
*       None
*
*******************************************************************************/
MCD_STATUS mcdSampleGetPRBSCounts
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   host_or_line,
    IN  MCD_U16   laneOffset
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;

    MCD_U64  txBitCount = 0;
    MCD_U64  rxBitCount = 0;
    MCD_U64  rxBitErrorCount = 0;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call cpss api function */
    rc = mcdGetPRBSCounts(pDev, mdioPort, host_or_line, laneOffset, &txBitCount,&rxBitCount, &rxBitErrorCount);

    if (rc == MCD_OK) {
        MCD_PRINTF("txBitCount = %llu\n"     , txBitCount);
        MCD_PRINTF("rxBitCount = %llu\n"     , rxBitCount);
        MCD_PRINTF("rxBitErrorCount = %llu\n", rxBitErrorCount);
    } else {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/******************************************************************************
* mcdSampleSetHostSidePRBSPattern
*
* DESCRIPTION:
*       This function is used to select the type of PRBS pattern desired in
*       for the host side PRBS in registers 4.AX30.3:0 (X depends on Port/Lane).
*
* APPLICABLE DEVICES:
*       ???
* INPUTS:
*       pDev - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       mdioPort - MDIO port address, 0-31
*       laneOffset - 0..3 for lanes 0-3
*       pattSel - one of the following:
*            MCD_HOST_PRBS31
*            MCD_HOST_PRBS7
*            MCD_HOST_PRBS9
*            MCD_HOST_PRBS23
*            MCD_HOST_IPRBS31
*            MCD_HOST_IPRBS7
*            MCD_HOST_PRBS15
*            MCD_HOST_IPRBS15
*            MCD_HOST_IPRBS9
*            MCD_HOST_IPRBS23
*            MCD_HOST_PRBS58
*            MCD_HOST_IPRBS58
*            MCD_HOST_HI_FREQ
*            MCD_HOST_LO_FREQ
*            MCD_HOST_MX_FREQ
*            MCD_HOST_SQ_WAV
*
*
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* COMMENTS:
*       None
*
*******************************************************************************/
MCD_STATUS mcdSampleSetHostSidePRBSPattern
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   laneOffset,
    IN  MCD_PRBS_HOST_SELECTOR_TYPE pattSel
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc = mcdSetHostSidePRBSPattern(pDev, mdioPort, laneOffset, pattSel);

    if (rc != MCD_OK) {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/******************************************************************************
* mcdSampleGetPRBSWaitForLock
*
* DESCRIPTION:
*       Returns configuration for PRBS whether it is set to wait for locking
*       or not before counting.
*
* APPLICABLE DEVICES:
*       ???
* INPUTS:
*       pDev - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       mdioPort - MDIO port address, 0-31
*       host_or_line - which interface is being set:
*                      MCD_HOST_SIDE
*                      MCD_LINE_SIDE
*       laneOffset - 0..3 for lanes 0-3
*
*
*
* OUTPUTS:
*       disableWaitforLock - 1 counts PRBS before locking, 0 waits
*       for locking before counting
*
*
* RETURNS:
*       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* COMMENTS:
*       None
*
*******************************************************************************/
MCD_STATUS mcdSampleGetPRBSWaitForLock
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   host_or_line,
    IN  MCD_U16   laneOffset
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;
    MCD_U16       disableWaitforLock = 0;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc = mcdGetPRBSWaitForLock(pDev, mdioPort, host_or_line, laneOffset, &disableWaitforLock);

    if (rc == MCD_OK) {
        MCD_PRINTF("Disable Wait For Lock = %d\n", disableWaitforLock);
    } else {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/******************************************************************************
* mcdSampleSerdesSetTxPRBS
*
* DESCRIPTION:
*       This function sets the Tx PRBS data source on an individual SerDes lane.
*
* APPLICABLE DEVICES:
*       ???
* INPUTS:
*       pDev - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       sAddr - SerDes lane. Range [1 to 16]; Refer to MCD_GET_SERDES_ID for sAddr
*       txDataType - Tx data selection. Refer to MCD_SERDES_TX_DATA_TYPE for selections
*           MCD_SERDES_TX_DATA_SEL_PRBS7    = 0, < PRBS7 (x^7+x^6+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS9    = 1, < PRBS9 (x^9+x^7+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS11   = 2, < PRBS11 (x^11+x^9+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS15   = 3, < PRBS15 (x^15+x^14+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS23   = 4, < PRBS23 (x^23+x^18+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS31   = 5, < PRBS31 (x^31+x^28+1)
*           MCD_SERDES_TX_DATA_SEL_USER     = 7, < User pattern generator
*           MCD_SERDES_TX_DATA_SEL_CORE     = 8, < External data
*           MCD_SERDES_TX_DATA_SEL_LOOPBACK = 9  < Parallel loopback from receiver
*
* OUTPUTS:
*       None
*
*
* RETURNS:
*       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* COMMENTS:
*       Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*******************************************************************************/
MCD_STATUS mcdSampleSerdesSetTxPRBS
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   sAddr,
    IN  MCD_SERDES_TX_DATA_TYPE txDataType
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc = mcdSerdesSetTxPRBS(pDev, sAddr, txDataType);

    if (rc != MCD_OK) {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/******************************************************************************
* mcdSampleSerdesGetTxPRBS
*
* DESCRIPTION:
*       This function gets the Tx PRBS data source on an individual SerDes lane.
*
* APPLICABLE DEVICES:
*       ???
* INPUTS:
*       pDev - pointer to MCD_DEV initialized by mcdLoadDriver() call
*       sAddr - SerDes lane. Range [1 to 16]; Refer to MCD_GET_SERDES_ID for sAddr
*
* OUTPUTS:
*       txDataType - Tx data selection. Refer to MCD_SERDES_TX_DATA_TYPE for selections
*           MCD_SERDES_TX_DATA_SEL_PRBS7    = 0, < PRBS7 (x^7+x^6+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS9    = 1, < PRBS9 (x^9+x^7+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS11   = 2, < PRBS11 (x^11+x^9+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS15   = 3, < PRBS15 (x^15+x^14+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS23   = 4, < PRBS23 (x^23+x^18+1)
*           MCD_SERDES_TX_DATA_SEL_PRBS31   = 5, < PRBS31 (x^31+x^28+1)
*           MCD_SERDES_TX_DATA_SEL_USER     = 7, < User pattern generator
*           MCD_SERDES_TX_DATA_SEL_CORE     = 8, < External data
*           MCD_SERDES_TX_DATA_SEL_LOOPBACK = 9  < Parallel loopback from receiver
*
*
* RETURNS:
*       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* COMMENTS:
*       Calling this function requires blocking all other caller from issuing
*       interrupt read/write calls to the SerDes and SBus master.
*
*******************************************************************************/
MCD_STATUS mcdSampleSerdesGetTxPRBS
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   sAddr
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;
    MCD_SERDES_TX_DATA_TYPE txDataType = MCD_SERDES_TX_DATA_SEL_PRBS7;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call api function */
    rc = mcdSerdesGetTxPRBS(pDev, sAddr, &txDataType);

    if (rc == MCD_OK) {
        switch (txDataType) {
            case  0:
              MCD_PRINTF("PRBS7\n");
              break;
            case  1:
              MCD_PRINTF("PRBS9\n");
              break;
            case  2:
              MCD_PRINTF("PRBS11\n");
              break;
            case  3:
              MCD_PRINTF("PRBS15\n");
              break;
            case  4:
              MCD_PRINTF("PRBS23\n");
              break;
            case  5:
              MCD_PRINTF("PRBS31\n");
              break;
            case  7:
              MCD_PRINTF("User Pattern\n");
              break;
            case  8:
              MCD_PRINTF("Core - External data\n");
              break;
            case  9:
              MCD_PRINTF("PLoopback\n");
              break;
            default:
              MCD_PRINTF("%s failed, Unknown txDataType=%d\n", __func__, txDataType);
              break;
        }
    } else {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    return rc;
}

/**
* @internal mcdSampleGetPortStatistics function
* @endinternal
*
* @brief  Print Statistics (MIB) counters of a port.
*
* @param[in] phyId                - device number
* @param[in] portNum  - represents port  (0..8) number
* @param[in] host_or_line     - host or lane side of a port, values MCD_LINE_SIDE or MCD_HOST_SIDE.
* @param[in] reducedPrint     - if true prints only non-zero counters
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleGetPortStatistics 0,0,3,0
*
*/
MCD_STATUS mcdSampleGetPortStatistics
(
    IN MCD_U32   phyId,
    IN MCD_U16  portNum,
    IN MCD_U16 host_or_line,
    IN MCD_BOOL   reducedPrint
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call  api function */
    rc = mcdGetPortStatistics(pDev, portNum, host_or_line, reducedPrint);
    return rc;
}

/**
* @internal mcdSampleGetPortStatisticsAll function
* @endinternal
*
* @brief  Print Statistics (MIB) counters of all ports and all sides.
*
* @param[in] phyId                - device number
* @param[in] reducedPrint     - if true prints only non-zero counters
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleGetPortStatisticsAll 0,1
*
*/
MCD_STATUS mcdSampleGetPortStatisticsAll
(
    IN MCD_U32   phyId,
    IN MCD_BOOL   reducedPrint
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;
    MCD_U16       i;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* call  api function */
    for(i=0;i<8;i++)
    {
         printf("\nPort %i Line side\n",i);
         rc = mcdGetPortStatistics(pDev, i, MCD_LINE_SIDE, reducedPrint);
         printf("\nPort %i Host side\n",i);
         rc = mcdGetPortStatistics(pDev, i, MCD_HOST_SIDE, reducedPrint);
    }
    return rc;
}

/**
* @internal mcdSampleGetPortConfig function
* @endinternal
*
* @brief  Display port configuration for specific port number.
*         Feature starting with APxx is for AP config and
*                               Prtxx is for Port config.
*
* @param[in] phyId                - device number
* @param[in] portNumber           - represents port number 0-7
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleGetPortConfig 0, 0
*
*/
MCD_STATUS mcdSampleGetPortConfig
(
    IN MCD_U32    phyId,
    IN MCD_U16    portNum
)
{
    MCD_DEV_PTR     pDev;
    MCD_OP_MODE     portModeId;
    MCD_U32         apLanesNum, apModeVectorID;
    MCD_U32         tmpData, resModeVector, bitPos;
    MCD_U16         apFecSupId, apFecReqId;
    MCD_U16         fecCorrectId;
    MCD_U8          apAnEnId, apNoPpmModeId, calibrationModeId;
    MCD_U8          portNoPpmMode;
    MCD_8           *apModeVector, *advSpeedMode, *acqSpeedMode;
    MCD_BOOL        apRetimerModeId, apFcPauseId, apFcAsmDirId;
    MCD_BOOL        apNonceDisId, haModeId;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleGetPortConfig: Device phyId not initialized\n");
        return MCD_FAIL;
    }

    /* Check for Port number */
    if (portNum >= MCD_MAX_PORT_NUM)
    {
        MCD_PRINTF("mcdSampleGetPortConfig: Invalid portNum# %d, should be in the range of 0-7", portNum);
        return MCD_FAIL;
    }

    /* Get AP configuration */
    apAnEnId = ((pDev->apCfg[portNum].options >> 2) & 0x1);
    apRetimerModeId = pDev->apCfg[portNum].retimerMode;
    apFcPauseId = pDev->apCfg[portNum].fcPause;
    apFcAsmDirId = pDev->apCfg[portNum].fcAsmDir;
    apLanesNum = pDev->apCfg[portNum].apLaneNum;
    apNonceDisId = pDev->apCfg[portNum].nonceDis;
    apNoPpmModeId = pDev->apCfg[portNum].noPpmMode;

    apModeVectorID = pDev->apCfg[portNum].modesVector;
    apModeVector = mcdSampleConvertModevector2String(apModeVectorID);

    /* fecSup parameter */
    if ((pDev->apCfg[portNum].fecSup) >= MCD_RS_FEC_HOST)
    {
        apFecSupId = ((pDev->apCfg[portNum].fecSup >> 3) + 3);
    } else {
        apFecSupId = pDev->apCfg[portNum].fecSup;
    }

    /* fecReq parameter */
    if (pDev->apCfg[portNum].fecReq == 0xFF)
    {
        apFecReqId = 8;
    } else if (((pDev->apCfg[portNum].fecReq) >= MCD_RS_FEC_HOST) && ((pDev->apCfg[portNum].fecReq) <= MCD_FC_FEC_LINE)) {
        apFecReqId = ((pDev->apCfg[portNum].fecReq >> 3) + 3);
    } else {
        apFecReqId = pDev->apCfg[portNum].fecReq;
    }

    /* Get port configuration */
    portModeId = pDev->portConfig[portNum].portMode;
    portNoPpmMode = pDev->portConfig[portNum].noPpmMode;
    /* autoNegAdvSpeed & autoNegAcquSpeed are not supportd in AP mode */
    if (pDev->portConfig[portNum].autoNegAdvSpeed == 0)
    {
        advSpeedMode = "NOT SUPPORTED!";
    }
    else
    {
        /* if found autoNegAdvSpeed non-zero. (Need to be implemented) */
        MCD_PRINTF("mcdSampleGetPortConfig: AutoNegAdvSpeed value found is 0x%X, Note: TBD",
                        pDev->portConfig[portNum].autoNegAdvSpeed);
    }
    if (pDev->portConfig[portNum].autoNegAcquSpeed == 0)
    {
        acqSpeedMode = "NOT SUPPORTED!";
    }
    else
    {
        /* if found autoNegAcquSpeed non-zero. (Need to be implemented) */
        MCD_PRINTF("mcdSampleGetPortConfig: AutoNegAcquSpeed value found is 0x%X, Note: TBD",
                        pDev->portConfig[portNum].autoNegAcquSpeed);
    }

    /* fecCorrect parameter */
    if (pDev->portConfig[portNum].fecCorrect >= MCD_RS_FEC_HOST)
    {
        fecCorrectId = ((pDev->portConfig[portNum].fecCorrect >> 3) + 3);
    } else {
        fecCorrectId = pDev->portConfig[portNum].fecCorrect;
    }

    /* Get calibration mode */
    calibrationModeId = pDev->calibrationCfg[portNum].calibrationMode;

    /* Display status of AP, non-AP, CalibrationMode, HAmode*/
    MCD_PRINTF("\n Port# \t\t: %2d\n PortMode \t: %s\n AP_enable \t: %s\n AP_retimer \t: %s\n AP_noPpm \t: %s\n"
                " Ap_Lane# \t: %X\n",
                        portNum,
                        portModeList[portModeId],
                        anEnList[apAnEnId],
                        configStsList[apRetimerModeId],
                        noPpmList[apNoPpmModeId],
                        apLanesNum);
    /* Bitwise operation to check if modesVector is having more than one mode */
    if (apAnEnId)
    {
        resModeVector = 0;
        bitPos = 1;
        tmpData = 0;
        tmpData |= apModeVectorID;
        MCD_PRINTF(" AP_modevector  : ");
        while (tmpData)
        {
             /* If AN is Enabled and tmpData is non zero, check bit wise */
             resModeVector = apModeVectorID & bitPos;
             bitPos = bitPos << 1;
             tmpData = tmpData >> 1;
             if (resModeVector) {
                 apModeVector = mcdSampleConvertModevector2String(resModeVector);
                 MCD_PRINTF("%s ", apModeVector);
            }
        }
        MCD_PRINTF("\n");
    }
    else
    {
        /* This API prints info for both AP/non-AP mode */
        apModeVector = mcdSampleConvertModevector2String(apModeVectorID);
        MCD_PRINTF(" AP_modevector \t: %s\n", apModeVector);
    }

    MCD_PRINTF(" AP_fcPause \t: %s\n AP_fcAsmDir \t: %s\n AP_fecSupID \t: %s\n"
                " AP_fecReqID \t: %s\n AP_nonceDisId \t: %s\n Prt_FecCorrect : %s\n Prt_NoPpmMode \t: %s\n"
                " Prt_Adv_speed \t: %s\n Prt_Acq_speed \t: %s\n CalibrationMod : %s\n",
                        configStsList[apFcPauseId],
                        configStsList[apFcAsmDirId],
                        fecTypeList[apFecSupId],
                        fecTypeList[apFecReqId],
                        configStsList[apNonceDisId],
                        fecTypeList[fecCorrectId],
                        noPpmList[portNoPpmMode],
                        advSpeedMode,
                        acqSpeedMode,
                        calibrationList[calibrationModeId]);

    /* Get HA mode, its per system */
    haModeId = pDev->highAvailabilityMode;
    MCD_PRINTF("\n NOTE 1: HA mode = %s on Phy device address = %d\n", configStsList[haModeId], pDev->firstMdioPort);

    /* Keyword definition for Portmode, starting with Pxxxxx = PCS, Rxxxxx = Retimer*/
    MCD_PRINTF("\n NOTE 2: \"Portmode\" Keywords Definition : ");
    MCD_PRINTF("\n         P = PCS,  L = No-FEC No-AN,  S= W/FEC & No-AN,  C = W/FEC & AN,  K = W/FEC & AN,"
               "\n         R = RETIMER,  G = GEARBOX\n");

    return MCD_OK;
}

/**
* @internal mcdSampleGetPortConfigAll function
* @endinternal
*
* @brief  Display port configuration for ALL ports.
*         Feature starting with APxx is for AP config and
*                               Prtxx is for Port config.
*
* @param[in] phyId                - device number
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleGetPortConfigAll 0
*
*/
MCD_STATUS mcdSampleGetPortConfigAll
(
    IN MCD_U32    phyId
)
{
    MCD_DEV_PTR     pDev;
    MCD_OP_MODE     portModeId;
    MCD_U32         apLanesNum, apModeVectorID;
    MCD_U32         tmpData, resModeVector, bitPos;
    MCD_U16         portNum, apFecSupId, fecCorrectId;
    MCD_U8          apAnEnId, calibrationModeId;
    MCD_8           *apModeVector;
    MCD_BOOL        haModeId;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleGetPortConfigAll: Device phyId not initialized\n");
        return MCD_FAIL;
    }

    MCD_PRINTF("\n++----------------------------------------------------------------------------------"
               "-----------------------++\n");
    MCD_PRINTF("| Prt | Portmode    | Prt_FecCorrt| AP_En | AP_Lane# | AP_ModeVector    | AP_FecSup   "
               "| CalibrationMode     |");
    MCD_PRINTF("\n++----------------------------------------------------------------------------------"
               "-----------------------++\n");
    apModeVector = mcdSampleConvertModevector2String(0); /* Init to NONE */
    bitPos = 0;
    tmpData = 0;

    /* Looping through all Ports */
    for (portNum =0; portNum < MCD_MAX_PORT_NUM; portNum++)
    {
        /* Get AP configuration */
        apAnEnId = ((pDev->apCfg[portNum].options >> 2) & 0x1);
        apLanesNum = pDev->apCfg[portNum].apLaneNum;

        /* fecSup parameter */
        if ((pDev->apCfg[portNum].fecSup) >= MCD_RS_FEC_HOST)
        {
            apFecSupId = ((pDev->apCfg[portNum].fecSup >> 3) + 3);
        } else {
            apFecSupId = pDev->apCfg[portNum].fecSup;
        }

        apModeVectorID = pDev->apCfg[portNum].modesVector;
        /* Bitwise operation to check if modesVector is having more than one mode */
        if (apAnEnId)
        {
            resModeVector = 0;
            bitPos = 1;
            tmpData = 0;
            tmpData |= apModeVectorID;
            while (tmpData)
            {
                /* If AN is Enabled and tmpData is non zero, check bit wise */
                resModeVector = apModeVectorID & bitPos;
                bitPos = bitPos << 1;
                tmpData = tmpData >> 1;
                /* Exiting for first match */
                if (resModeVector) {
                    apModeVector = mcdSampleConvertModevector2String(resModeVector);
                    break;
                }
            }
        }
        else
        {
            /* This API prints info for both AP/non-AP mode */
            apModeVector = mcdSampleConvertModevector2String(apModeVectorID);
        }

        /* Get port configuration */
        portModeId = pDev->portConfig[portNum].portMode;

         /* fecCorrect parameter */
         if (pDev->portConfig[portNum].fecCorrect >= MCD_RS_FEC_HOST)
         {
             fecCorrectId = ((pDev->portConfig[portNum].fecCorrect >> 3) + 3);
         } else {
             fecCorrectId = pDev->portConfig[portNum].fecCorrect;
         }

        /* Get calibration mode */
        calibrationModeId = pDev->calibrationCfg[portNum].calibrationMode;

        /* Display status of AP, non-AP, Calibration mode, HA mode */
        MCD_PRINTF("| %2d  | %s | %s | %s  | %-8X | %s | %s | %s |\n",
                            portNum,
                            portModeList[portModeId],
                            fecTypeList[fecCorrectId],
                            anEnList[apAnEnId],
                            apLanesNum,
                            apModeVector,
                            fecTypeList[apFecSupId],
                            calibrationList[calibrationModeId]);

        /* Continuing to check if any other bits set for modesVector */
        if (apAnEnId)
        {
            while (tmpData)
            {
                resModeVector = apModeVectorID & bitPos;
                bitPos = bitPos << 1;
                tmpData = tmpData >> 1;
                if (resModeVector) {
                    apModeVector = mcdSampleConvertModevector2String(resModeVector);
                    /* if modesVector is found again, then displaying in new line */
                    MCD_PRINTF("|     |             |             |       |          | %s |             "
                               "|                     |\n", apModeVector);
                }
            }
        }
        MCD_PRINTF("|-----------------------------------------------------------------------------------"
                   "------------------------|\n");
    }

    /* Get HA mode, its per system */
    haModeId = pDev->highAvailabilityMode;
    MCD_PRINTF("\n NOTE 1: HA mode = %s on Phy device address = %d\n", configStsList[haModeId], pDev->firstMdioPort);

    /* Keyword definition for Portmode, starting with Pxxxxx = PCS, Rxxxxx = Retimer*/
    MCD_PRINTF("\n NOTE 2: \"Portmode\" Keywords Definition : ");
    MCD_PRINTF("\n         P = PCS,  L = No-FEC No-AN,  S= W/FEC & No-AN,  C = W/FEC & AN,  K = W/FEC & AN,"
               "\n         R = RETIMER,  G = GEARBOX\n");

    return MCD_OK;
}


/******************************************************************************
* mcdSamplePortApIntropSetIpc
*
* DESCRIPTION:
*       Wrapper function for changing the AP Link timeout
*
* INPUTS:
*       phyId    - phy system device id
*
*
*
*
* RETURNS:
*       MCD_OK or MCD_FAIL if io error or bad parameter passed in (out of range)
*
* COMMENTS:
*
*******************************************************************************/
MCD_STATUS mcdSamplePortApIntropSetIpc
(
    IN MCD_U32   phyId,
    IN MCD_U32   phyPortNum,
    IN MCD_U32   apLinkDuration,
    IN MCD_U32   apLinkMaxInterval
)
{
    GT_STATUS                   rc;
    MCD_DEV_PTR                 pDev;

    MCD_IPC_PORT_AP_INTROP_STRUCT apIntropIpc;

    /* reset memory */
    mcdMemSet(&apIntropIpc, 0, sizeof(MCD_IPC_PORT_AP_INTROP_STRUCT));
    apIntropIpc.phyPortNum = phyPortNum;
    apIntropIpc.portMode = MCD_MODE_NUM;
    apIntropIpc.attrBitMask = (0x10 | 0x20);/*AP_PORT_INTROP_AP_LINK_DUR | AP_PORT_INTROP_AP_LINK_MAX_INT*/
    apIntropIpc.apLinkDuration = apLinkDuration;
    apIntropIpc.apLinkMaxInterval = apLinkMaxInterval;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_FAIL;
    }

    rc = mcdPortApIntropSetIpc(pDev, phyPortNum, (MCD_U32*)(&apIntropIpc));
    if (rc != MCD_OK) {
        MCD_PRINTF("%s failed. rc=%d\n", __func__, rc);
    }

    return rc;
}

/**
* @internal mcdSampleLowSpeedAutoNegEnable function
* @endinternal
*
* @brief   To Enable/Disable Auto Neg for 1G port mode
*
* @param[in] phyId                  - device number
* @param[in] mdioPort               - MDIO port, 0-7
* @param[in] host_or_line           - Line=3 and Host=4
* @param[in] enable                 - 1 for AN enable, 0 for disable
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleLowSpeedAutoNegEnable 0, 0, 3, 1
*
*/
MCD_STATUS mcdSampleLowSpeedAutoNegEnable
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   host_or_line,
    IN  MCD_BOOL  enable
)
{
    MCD_DEV_PTR                 pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("%s: Device phyId not initialized\n", __func__);
        return MCD_FAIL;
    }

    /* call to modify AN - API function */
    ATTEMPT(mcdLowSpeedAutoNegEnable(pDev, mdioPort, host_or_line, enable));

    return MCD_OK;
}

/**
* @internal mcdSampleLowSpeedAutoNegEnableGet function
* @endinternal
*
* @brief   Get status of ANenable for 1G port mode
*
* @param[in] phyId                  - device number
* @param[in] mdioPort               - MDIO port, 0-7
* @param[in] host_or_line           - Line=3 and Host=4
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleLowSpeedAutoNegEnableGet 0, 0, 3
*
*/
MCD_STATUS mcdSampleLowSpeedAutoNegEnableGet
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   host_or_line
)
{
    MCD_DEV_PTR                 pDev;
    MCD_BOOL                    anEnSts;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("%s: Device phyId not initialized\n", __func__);
        return MCD_FAIL;
    }

    /* call to Get status of ANenable - API function */
    ATTEMPT(mcdLowSpeedAutoNegEnableGet(pDev, mdioPort, host_or_line, &anEnSts));
    if (anEnSts == MCD_TRUE)
    {
        MCD_PRINTF("AutoNeg for Low Speed (1G) is ENABLED\n");
    }
    else
    {
        MCD_PRINTF("AutoNeg for Low Speed (1G) is DISABLED!\n");
    }

    return MCD_OK;
}

/**
* @internal mcdSampleLowSpeedAutoNegRestart function
* @endinternal
*
* @brief   Restart Auto Neg for 1G port mode
*
* @param[in] phyId                  - device number
* @param[in] mdioPort               - MDIO port, 0-7
* @param[in] host_or_line           - Line=3 and Host=4
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleLowSpeedAutoNegRestart 0, 0, 3
*
*/
MCD_STATUS mcdSampleLowSpeedAutoNegRestart
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   host_or_line
)
{
    MCD_DEV_PTR                 pDev;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("%s: Device phyId not initialized\n", __func__);
        return MCD_FAIL;
    }

    /* call to Restart AN - API function */
    ATTEMPT(mcdLowSpeedAutoNegRestart(pDev, mdioPort, host_or_line));

    return MCD_OK;
}

/**
* @internal mcdSampleLowSpeedAutoNegDoneGet function
* @endinternal
*
* @brief   Get status of AN Done for 1G port mode
*
* @param[in] phyId                  - device number
* @param[in] mdioPort               - MDIO port, 0-7
* @param[in] host_or_line           - Line=3 and Host=4
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleLowSpeedAutoNegDoneGet 0, 0, 3
*
*/
MCD_STATUS mcdSampleLowSpeedAutoNegDoneGet
(
    IN  MCD_U32   phyId,
    IN  MCD_U16   mdioPort,
    IN  MCD_U16   host_or_line
)
{
    MCD_DEV_PTR                 pDev;
    MCD_BOOL                    anDoneStsGet;

    pDev = mcdSampleGetDrvDev(phyId);
    if(pDev == NULL)
    {
        MCD_PRINTF("%s: Device phyId not initialized\n", __func__);
        return MCD_FAIL;
    }

    /* call to Get status of AN Done - API function */
    ATTEMPT(mcdLowSpeedAutoNegDoneGet(pDev, mdioPort, host_or_line, &anDoneStsGet));
    if (anDoneStsGet == MCD_TRUE)
    {
        MCD_PRINTF("AutoNeg Done bit for Low Speed (1G) is SET\n");
    }
    else
    {
        MCD_PRINTF("AutoNeg Done bit for Low Speed (1G) is NOT-SET!\n");
    }

    return MCD_OK;
}


#ifdef MCD_DEBUG

/**
* @internal mcdSampleDbgPrintAllRegisters function
* @endinternal
*
* @brief   prints all registers.
*
*
* @param[in] phyId                   device number
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleDbgPrintAllRegisters 0
*
*/
MCD_STATUS mcdSampleDbgPrintAllRegisters
(
    IN MCD_U32 phyId
)
{
    MCD_DEV_PTR pDev;
    MCD_STATUS rc;

    pDev = mcdSampleGetDrvDev(phyId);
    if (pDev == NULL)
    {
        MCD_PRINTF("mcdSampleDbgPrintAllRegisters: phyId [%d] does not exist\n", phyId);
        return MCD_FAIL;
    }
    rc = mcdDbgPrintAllRegisters(pDev);

    return rc;
}
#endif


/**
* @internal mcdSampleGetFecCorrectedCwCntr function
* @endinternal
*
* @brief   Reading the FEC corrected counter
*
* @param[in] devNum
* @param[in] mdio_port - port number
* @param[in] host_or_line
* @param[in] fecCorrect type, FC or RS
* @param[out] codeWordCounter - the return value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
MCD_STATUS mcdSampleGetFecCorrectedCwCntr
(
    IN  MCD_U32     phyId,
    IN  MCD_U16     portNum,
    IN  MCD_U16     host_or_line,
    IN MCD_FEC_TYPE fecCorrect
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;
    MCD_U32       codeWordCounter;

    pDev = mcdSampleGetDrvDev(phyId);

    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    rc  = mcdGetFecCorrectedCwCntr( pDev, portNum, host_or_line, fecCorrect, &codeWordCounter);

    if (rc != MCD_OK){
        MCD_PRINTF("%s - mcdGetFecCorrectedCwCntr failed. rc=%d\n", __func__, rc);
        return MCD_FAIL;
    }

     MCD_PRINTF("codeWordCounter = %d\n", codeWordCounter);

     return rc;
}

/**
* @internal mcdSampleGetFecUnCorrectedCwCntr function
* @endinternal
*
* @brief   Reading the FEC un-corrected counter
*
* @param[in] devNum
* @param[in] mdio_port - port number
* @param[in] host_or_line
* @param[in] fecCorrect type, FC or RS
* @param[out] codeWordCounter - the return value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/

MCD_STATUS mcdSampleGetFecUnCorrectedCwCntr
(
    IN  MCD_U32     phyId,
    IN  MCD_U16     portNum,
    IN  MCD_U16     host_or_line,
    IN MCD_FEC_TYPE fecCorrect
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;
    MCD_U32       codeWordCounter;

    pDev = mcdSampleGetDrvDev(phyId);

    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

         rc  = mcdGetFecUnCorrectedCwCntr( pDev, portNum, host_or_line, fecCorrect, &codeWordCounter);

         if (rc != MCD_OK){
                 MCD_PRINTF("%s - mcdGetFecUnCorrectedCwCntr failed. rc=%d\n", __func__, rc);
                 return MCD_FAIL;
         }

          MCD_PRINTF("codeWordCounter = %d\n", codeWordCounter);

     return rc;
}


/**
* @internal mcdSampleGetFecSymbolErrorCntr function
* @endinternal
*
* @brief   Reading the FEC symbol error counter
*
* @param[in] devNum
* @param[in] mdio_port - port number
* @param[in] lane - within the port #
* @param[in] host_or_line
* @param[in] fecCorrect type, FC or RS
* @param[out] errorCounter - the return value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/

MCD_STATUS mcdSampleGetFecSymbolErrorCntr
(
    IN  MCD_U32      phyId,
    IN  MCD_U16      portNum,
    IN  MCD_U16      lane,
    IN  MCD_U16      host_or_line,
    IN  MCD_FEC_TYPE fecCorrect
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;
    MCD_U32       errorCounter;

    pDev = mcdSampleGetDrvDev(phyId);

    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    rc = mcdGetFecSymbolErrorCntr( pDev, portNum, lane, host_or_line, fecCorrect, &errorCounter);

    if (rc != MCD_OK){
        MCD_PRINTF("%s - mcdGetFecSymbolErrorCntr failed. rc=%d\n", __func__, rc);
        return MCD_FAIL;
    }

     MCD_PRINTF("errorCounter = %d\n", errorCounter);

     return rc;
}

/**
 * * @internal mcdSampleSerdesTxInjectError function
 * * @endinternal
 * *
 * * @brief   Reading the FEC symbol error counter
 * *
 * * @param[in] devNum
 * * @param[in] laneNum
 * * @param[in] numErrBit
 * *
 * * @retval 0                        - on success
 * * @retval 1                        - on error
 * */
MCD_STATUS mcdSampleSerdesTxInjectError
(
     IN  MCD_U32    phyId,
     IN  MCD_U16    laneNum,
     IN  MCD_U16    numErrBit
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;

    pDev = mcdSampleGetDrvDev(phyId);

    if(pDev == NULL)
    {
         MCD_PRINTF("Device phyId not initialized\n");
         return MCD_OK;
    }

    rc = mcdSerdesTxInjectError(pDev, laneNum + 1, numErrBit);

    if (rc != MCD_OK){
        MCD_PRINTF("%s - mcdSerdesTxInjectError failed. rc=%d\n", __func__, rc);
        return MCD_FAIL;
    }

     return rc;
}




MCD_STATUS mcdI2cRead(
    IN  MCD_U8    bus_id,
    IN  MCD_U8    slave_address,
    IN  MCD_U32   offset_type,
    IN  MCD_U32   offset,
    IN  MCD_U32   buffer_size,
    OUT MCD_U8   *buffer
)
{
    if( extDrvI2cRead(i2cDrv,
                      bus_id,
                      slave_address,
                      offset_type,
                      offset,
                      buffer_size,
                      buffer) !=0 )
    {
        return MCD_FAIL;
    }

    return MCD_OK;
}

MCD_STATUS mcdI2cWrite(
    IN MCD_U8    bus_id,
    IN MCD_U8    slave_address,
    IN MCD_U32   offset_type,
    IN MCD_U32   offset,
    IN MCD_U32   buffer_size,
    IN MCD_U8   *buffer
)
{
    if( extDrvI2cWrite(i2cDrv,
                       bus_id,
                       slave_address,
                       offset_type,
                       offset,
                       buffer_size,
                       buffer) !=0 )
    {
        return MCD_FAIL;
    }

    return MCD_OK;
}

MCD_STATUS mcdI2cReadSampleTest(
    IN MCD_U8    bus_id,
    IN MCD_U8    slave_address,
    IN MCD_U32   offset_type,
    IN MCD_U32   offset,
    IN MCD_U32   buffer_size
)
{
    MCD_STATUS rc;
    MCD_U8  buffer[256];
    MCD_U32 ii;

    if(buffer_size > 256)
    {
        MCD_PRINTF("buffer_size (%d) > 256\n", buffer_size);
        return MCD_FAIL;
    }

    MCD_PRINTF("mcdI2cReadSampleTest: bus_id 0x%x, slave_address 0x%x, offset_type 0x%x, offset 0x%x, buffer_size 0x%x\n",
               bus_id, slave_address, offset_type, offset, buffer_size);

    rc = mcdI2cRead(bus_id,
                    slave_address,
                    offset_type,
                    offset,
                    buffer_size,
                    buffer);

    MCD_PRINTF("mcdI2cRead returns with rc %x\n", rc);

    if( MCD_OK == rc )
    {
        for ( ii = 0; ii < buffer_size ; ii++)
        {
            MCD_PRINTF("0x%x: 0x%x\n", ii, buffer[ii]);
        }
    }

    return rc;
}

MCD_STATUS mcdI2cWriteSampleTest(
    IN MCD_U8    bus_id,
    IN MCD_U8    slave_address,
    IN MCD_U32   offset_type,
    IN MCD_U32   offset,
    IN MCD_U32   buffer_size,
    IN MCD_U8    data0,
    IN MCD_U8    data1,
    IN MCD_U8    data2
)
{
    MCD_STATUS rc;
    MCD_U8  buffer[256];

    MCD_PRINTF("mcdI2cWriteSampleTest: bus_id 0x%x, slave_address 0x%x, offset_type 0x%x, offset 0x%x, buffer_size 0x%x\n"
               "                           data0 0x%x, data1 0x%x, data2 0x%x\n",
               bus_id, slave_address, offset_type, offset, buffer_size, data0, data1, data2);

    buffer[0] = data0;
    buffer[1] = data1;
    buffer[2] = data2;

    if(buffer_size > 3)
    {
        MCD_PRINTF("buffer_size (%d) > 3\n", buffer_size);
        return MCD_FAIL;
    }

    rc = mcdI2cWrite(bus_id,
                     slave_address,
                     offset_type,
                     offset,
                     buffer_size,
                     buffer);

    MCD_PRINTF("mcdI2cWrite returns with rc %x\n", rc);

    return rc;
}



/**
* @internal mcdSampleSerdesGetTemperature function
* @endinternal
*
* @brief   prints temperature of SERDES and core of the device
*
* @param[in] phyId                - device number
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleSerdesGetTemperature 0
*
*******************************************************************************/
MCD_STATUS mcdSampleSerdesGetTemperature
(
    IN  MCD_U32   phyId
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;
    MCD_32 coreTemperature = 0;
    MCD_32 serdesTemperature = 0;

    pDev = mcdSampleGetDrvDev(phyId);

    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }
    rc = mcdSerdesGetTemperature(pDev, &serdesTemperature , &coreTemperature);

    if (rc == MCD_OK)
    {
        MCD_PRINTF("SERDES Temperature = %d\n", serdesTemperature);
        MCD_PRINTF("core Temperature = %d\n", coreTemperature);
    }
    else
    {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

     return rc;
}

/**
* @internal mcdSampleGetSerdesSignalDetectAndDspLock function
* @endinternal
*
* @brief   Return value of signal detect and DSP lock for corresponding lane.
*          Signal_detect is idle detect from SERDES.
*          Dsp_lock is signal ok from SERDES.
*
* @param[in] phyId                - device number
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleSerdesGetTemperature 0
*
*******************************************************************************/
MCD_STATUS mcdSampleGetSerdesSignalDetectAndDspLock
(
    IN  MCD_U32    phyId,
    IN  MCD_U16    portNum,
    IN  MCD_U16    laneOffset
)
{
    MCD_STATUS    rc;
    MCD_DEV_PTR   pDev;
    MCD_U16       hostSignalDetect = 0;
    MCD_U16       hostDspLock = 0;
    MCD_U16       lineSignalDetect = 0;
    MCD_U16       lineDspLock = 0;
    MCD_U32       lastPort = portNum;

    pDev = mcdSampleGetDrvDev(phyId);

    if(pDev == NULL)
    {
        MCD_PRINTF("Device phyId not initialized\n");
        return MCD_OK;
    }

    /* Printing all ports status in one function call */
    if (portNum == 255) {
        lastPort = 7;
        portNum = 0;
    }

    for ( ; portNum <= lastPort; portNum++) {
        rc = mcdGetSerdesSignalDetectAndDspLock( pDev, portNum, MCD_HOST_SIDE, laneOffset, &hostSignalDetect, &hostDspLock);
        rc = mcdGetSerdesSignalDetectAndDspLock( pDev, portNum, MCD_LINE_SIDE, laneOffset, &lineSignalDetect, &lineDspLock);
        if (rc == MCD_OK)
        {
            MCD_PRINTF("Port-%d[%d]:  Host Side: SignalDetect=%d  dspLock=%d\n", portNum, laneOffset, hostSignalDetect, hostDspLock);
            MCD_PRINTF("Port-%d[%d]:  Line Side: SignalDetect=%d  dspLock=%d\n", portNum, laneOffset, lineSignalDetect, lineDspLock);
        }
        else
        {
            MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
        }
    }
     return rc;
}



/******************************************************************************/
/*       For internal use, please keep this function last in this file        */
/******************************************************************************/

/**
* @internal mcdSampleShowVersion function
* @endinternal
*
* @brief   prints MCD version
*
* @param[in] none
*
* @retval MCD_OK                    - on success.
* @retval MCD_FAIL                  - on failure
*
* @note use like mcdSampleShowVersion
*
*******************************************************************************/
MCD_STATUS mcdSampleShowVersion(void)
{
    MCD_U8 major;
    MCD_U8 minor;
    MCD_U8 patch;
    MCD_STATUS rc;

    rc = mcdGetAPIVersion(&major, &minor, &patch);
    if (rc != MCD_OK)
    {
        MCD_PRINTF("%s failed.  rc=%d\n", __func__, rc);
    }

    MCD_PRINTF("\nMCD version:\n Major %d\n Minor %d\n patch %d\n", major, minor,patch);
    return MCD_OK;
}



