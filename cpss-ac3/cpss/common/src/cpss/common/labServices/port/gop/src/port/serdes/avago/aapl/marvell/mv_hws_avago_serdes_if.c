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
* mvAvagoSerdesIf.c.c
*
* DESCRIPTION:
*         Avago interface
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifdef MV_HWS_BIN_HEADER
#include "mvSiliconIf.h"
#include <gtGenTypes.h>
#include "mvHighSpeedEnvSpec.h"
#else
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include "cpss/common/labServices/port/gop/port/private/mvHwsPortTypes.h"
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/private/mvPortModeElements.h>
#endif

#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 || defined MV_HWS_BIN_HEADER
#include "sd28firmware/avago_fw_load.h"
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

#include "mv_hws_avago_if.h"
/* Avago include */
#include "aapl.h"
#include "aapl_core.h"
#include "sbus.h"
#include "spico.h"
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include "eye.h"
#endif

#if !defined(ASIC_SIMULATION) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#   if AAPL_ENABLE_AACS_SERVER
#   include <aacs_server.h>

#   endif /* AAPL_ENABLE_AACS_SERVER */
#endif /* !defined(ASIC_SIMULATION) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */
/************************* Globals *******************************************************/

unsigned int avagoConnection = AVAGO_SBUS_CONNECTION;

/* AAPL struct array for each device */
Aapl_t* aaplSerdesDb[HWS_MAX_DEVICE_NUM] = {0};
/* AAPL struct array */
Aapl_t  aaplSerdesDbDef[HWS_MAX_DEVICE_NUM];

EXT void  ms_sleep(uint msec);
extern int recovery_process;

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)
#ifdef PIPE_DEV_SUPPORT
#define MAX_AVG_SIZE 800
#else
#define MAX_AVG_SIZE 3600
#endif
char* hex2bin(GT_U32);
GT_UL64 mvHwsAvagoSerdesCalcVosDfxParams( unsigned char devNum, GT_U32 serdesNum, GT_U32 bitsPerSds, GT_CHAR dataStr[] );
/*GT_CHAR allSdsDataStr[MAX_AVG_SIZE]= {0};*/
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

GT_STATUS mvHwsAvagoSerdesVosDfxParamsSet(GT_U32 devNum, GT_U32 serdesNum);

#if AAPL_ENABLE_AACS_SERVER
#ifndef MV_HWS_BIN_HEADER
static unsigned int aacsServerEnable = 0;

/* Avago server process Id */
static GT_U32 avagoAACS_ServerTid;
#endif /*MV_HWS_BIN_HEADER*/
#endif /* AAPL_ENABLE_AACS_SERVER */

#ifdef MARVELL_AVAGO_DB_BOARD
unsigned int mvAvagoDb = 1;
#else
unsigned int mvAvagoDb = 0;
#endif /* MARVELL_AVAGO_DB_BOARD */

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
HWS_MUTEX avagoAccessMutex;
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */
#ifndef MV_HWS_AVAGO_NO_VOS_WA
#define SERVER_HD_EFUSE_VOS_REGION_BOBK   0x200
#define SERVER_HD_EFUSE_VOS_REGION_ALDRIN 0x100

typedef struct{
    GT_U32  serdes;
    GT_U32  serdesVosOffset;

}HWS_AVAGO_SERDES_VOS_OFFSET;

/* Code for Non CM3 CPU (host) and for Bobk CM3  */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(BOBK_DEV_SUPPORT))
/* Serdeses 24->35 := total 12 */
HWS_AVAGO_SERDES_VOS_OFFSET serdesVosOffsetBobK[]={
    /* Serdes  Vos-offset*/
     {   24,      0xc0    }
    ,{   25,      0xb0    }
    ,{   26,      0xa0    }
    ,{   27,      0x90    }
    ,{   28,      0x40    }
    ,{   29,      0x50    }
    ,{   30,      0x60    }
    ,{   31,      0x70    }
    ,{   32,      0x30    }
    ,{   33,      0x20    }
    ,{   34,      0x10    }
    ,{   35,      0x00    }
};
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(BOBK_DEV_SUPPORT)) */

/* Code for Non CM3 CPU (host) and for Aldrin CM3  */
/* Serdeses 0->31 := total 32 */
HWS_AVAGO_SERDES_VOS_OFFSET serdesVosOffsetAldrin[]={
    /* Serdes  Vos-offset*/
     {   0,      0x100    }
    ,{   1,      0x0f0    }
    ,{   2,      0x110    }
    ,{   3,      0x0e0    }
    ,{   4,      0x120    }
    ,{   5,      0x0d0    }
    ,{   6,      0x130    }
    ,{   7,      0x0c0    }
    ,{   8,      0x0a0    }
    ,{   9,      0x150    }
    ,{   10,     0x0b0    }
    ,{   11,     0x140    }
    ,{   12,     0x160    }
    ,{   13,     0x090    }
    ,{   14,     0x170    }
    ,{   15,     0x080    }
    ,{   16,     0x180    }
    ,{   17,     0x070    }
    ,{   18,     0x190    }
    ,{   19,     0x060    }
    ,{   20,     0x1a0    }
    ,{   21,     0x050    }
    ,{   22,     0x1b0    }
    ,{   23,     0x040    }
    ,{   24,     0x1c0    }
    ,{   25,     0x030    }
    ,{   26,     0x1d0    }
    ,{   27,     0x020    }
    ,{   28,     0x000    }
    ,{   29,     0x1f0    }
    ,{   30,     0x010    }
    ,{   31,     0x1e0    }
};

/* VOS Override values DB. To store the values read from DFX. The values will be stored as following:

       serdesVosParamsDB[0] = vos0-3 base serdes
       serdesVosParamsDB[0] = vos4-6 base serdes
       serdesVosParamsDB[1] = vos0-3 base serdes + 1
       serdesVosParamsDB[1] = vos4-6 base serdes + 1
       .
       .
       .
       serdesVosParamsDB[n] = vos0-3 base serdes + n
       serdesVosParamsDB[n] = vos4-6 base serdes + n

   This DB is sent to service CPU upon init, so its size should be a multiplition of VOS IPC buffer size.
*/
GT_U32  serdesVosParamsDB[72];
GT_BOOL serdesVosParamsDBInitialized = GT_FALSE;
#define VOS_WORDS 72
GT_UL64 serdesVosParams64BitDB[VOS_WORDS];/* array of words to holdt he data of 7 samplers per serdes */
GT_BOOL serdesVos64BitDbInitialized = GT_FALSE;
GT_U32 minVosValue; /* minimum VOS value */
GT_U32 baseVosValue; /* base VOS value */
#endif /* MV_HWS_AVAGO_NO_VOS_WA */

/*
   Tx offset parameters implementation optional compilation flags
*/
/*#define MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_FW_DEBUG*/
/*#define MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG*/
/*#define MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A*/

#if (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG))

#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A

/*
    Implementation A:

        Maximum 1 byte memory waste. This implementation wastes less memory then the next implementation
        but consume more in text (code). Currently, this implementation is more memory-consume then next
        implementation, but it can be changed as serdeses numbers and supported speeds demands will change.

        (Note: array second dimention size of
                (MV_HWS_IPC_MAX_PORT_NUM) + (MV_HWS_IPC_MAX_PORT_NUM/2) + (1)
              implements the calculation-
                ((MV_HWS_IPC_MAX_PORT_NUM * 1.5) +1 )
              The plus 1 is because of the integer down-grade [floor] devision)

       Data is organaized as follows (each letter represent serdes):

       sd0[Tx0] sd0[Tx1]  sd0[Tx2] sd1[Tx0]   sd1[Tx1] sd1[Tx2]    sd2[Tx0] sd2[Tx1]    sd2[Tx2]
         ____________       ____________         ____________        ____________        ____________
        |     |      |     |      |     |       |     |      |      |      |     |      |      |     |
        |_X___|__X___|     |__ X__|__Y__|       |__Y__|___Y__|      |___Z__|__Z__|      |__Z___|_____| ...

*/
GT_U8 mvHwsAvagoSerdesManualTxOffsetDb[2] [(MV_HWS_IPC_MAX_PORT_NUM) + (MV_HWS_IPC_MAX_PORT_NUM/2) + (1)];

#define TX_AMP_RAW_VAL_SHIFT_OP(_sd) (4*(_sd%2))
#define TX_AMP_RAW_VAL_IDX(_sd) ((_sd/2)*3 + (_sd%2))
#define TX_AMP_RAW_VAL_PRV(_sd,_speedIdx) \
    ((GT_8)(((mvHwsAvagoSerdesManualTxOffsetDb[ _speedIdx ][TX_AMP_RAW_VAL_IDX(_sd)]) >> (4*(_sd%2))) & 0xF))
#define TX_AMP_RAW_VAL(_sd,_speedIdx) \
    (((TX_AMP_RAW_VAL_PRV(_sd,_speedIdx) & 0x8)) ?                              \
    ( (GT_8)255 - (GT_8)(((TX_AMP_RAW_VAL_PRV(_sd,_speedIdx) & 0x7) ) - 1) ) :  \
        (TX_AMP_RAW_VAL_PRV(_sd,_speedIdx)))


#define TX_EMPH0_RAW_VAL_SHIFT_OP(_sd) (4*(1-(_sd%2)))
#define TX_EMPH0_RAW_VAL_IDX(_sd) ((_sd/2)*3 + (2*(_sd%2)))
#define TX_EMPH0_RAW_VAL_PRV(_sd,_speedIdx) \
    ((GT_8)((mvHwsAvagoSerdesManualTxOffsetDb[ _speedIdx ][TX_EMPH0_RAW_VAL_IDX(_sd)] >> (4*(1-(_sd%2)))) & 0xF))
#define TX_EMPH0_RAW_VAL(_sd,_speedIdx) \
    (((TX_EMPH0_RAW_VAL_PRV(_sd,_speedIdx) & 0x8))?                                 \
    ( (GT_8)255 - (GT_8)(((TX_EMPH0_RAW_VAL_PRV(_sd,_speedIdx) & 0x7) ) - 1) ) : \
    (TX_EMPH0_RAW_VAL_PRV(_sd,_speedIdx)))


#define TX_EMPH1_RAW_VAL_SHIFT_OP(_sd) (4*(_sd%2))
#define TX_EMPH1_RAW_VAL_IDX(_sd) ((_sd/2)*3 + (1+(_sd%2)))
#define TX_EMPH1_RAW_VAL_PRV(_sd,_speedIdx) \
    ((GT_8)((mvHwsAvagoSerdesManualTxOffsetDb[ _speedIdx ][TX_EMPH1_RAW_VAL_IDX(_sd)] >> (4*(_sd%2)))) & 0xF)
#define TX_EMPH1_RAW_VAL(_sd,_speedIdx) \
    (((TX_EMPH1_RAW_VAL_PRV(_sd,_speedIdx) & 0x8))?                                     \
    ( (GT_8)255 - (GT_8)(((TX_EMPH1_RAW_VAL_PRV(_sd,_speedIdx) & 0x7) ) - 1) ) :    \
    (TX_EMPH1_RAW_VAL_PRV(_sd,_speedIdx)))

#else

/*
   Implementation B:

       this array wastes 4bits per port AND per speed. It could be implemented without any waste (see
       implementation A) but then the code to manage it would consume more space and would be hard to
       maintain. (above example)

       Data is organaized as follows:

       sd0[Tx0] sd0[Tx1] sd0[Tx2]       sd1[Tx0] sd1[Tx1] sd1[Tx2]
         ________________________         ________________________
        |     |      |     |     |       |     |      |      |     |
        |_X___|__X___|_ X__|_____|       |__Y__|___Y__|___Y__|_____|    ...

*/
GT_U16 mvHwsAvagoSerdesManualTxOffsetDb[2][MV_HWS_IPC_MAX_PORT_NUM];
#define TX_PARAMETER_STEP   2
#define TX_AMP_RAW_VAL(_sd, _speedIdx)                                                                      \
    (((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x0010)) ?                                      \
            (GT_8) (255 - ((GT_8)((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x000F)*TX_PARAMETER_STEP - 1 )) :               \
            (GT_8) (mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x000F)*TX_PARAMETER_STEP)

#define TX_EMPH0_RAW_VAL(_sd, _speedIdx)                                                                    \
    ((((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x0200))) ?                                 \
            (GT_8) (255 - ((GT_8)(((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x01E0) >> 5)*TX_PARAMETER_STEP - 1))  :       \
            (GT_8) (((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x01E0) >> 5)*TX_PARAMETER_STEP )

#define TX_EMPH1_RAW_VAL(_sd, _speedIdx)                                                                    \
    ((((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd] & 0x4000))) ?                                 \
            (GT_8) (255 - ((GT_8)(((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x3C00) >> 10)*TX_PARAMETER_STEP - 1) )   :     \
            (GT_8) (((mvHwsAvagoSerdesManualTxOffsetDb[_speedIdx][_sd]) & 0x3C00) >> 10)*TX_PARAMETER_STEP )

#endif /* MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A */

#define INDEX_0_SD_SPEED _10_3125G
#define PERMITTED_SD_SPEED_0 _10_3125G
#define PERMITTED_SD_SPEED_1 _25_78125G
#endif /* (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG)) */

/*************************** Pre-Declarations *******************************************************/
#ifdef MV_HWS_FREE_RTOS
extern GT_STATUS mvPortCtrlAvagoLock(void);
extern GT_STATUS mvPortCtrlAvagoUnlock(void);
#endif /* MV_HWS_FREE_RTOS */

#ifndef ASIC_SIMULATION
extern GT_STATUS mvHwsAvagoInitI2cDriver(unsigned char devNum);
#endif /* ASIC_SIMULATION */

void mvHwsAvagoAccessValidate(unsigned char devNum, uint sbus_addr);
GT_STATUS mvHwsAvagoCheckSerdesAccess(unsigned int devNum, unsigned char portGroup, unsigned char serdesNum);

#ifdef MV_HWS_FREE_RTOS
#define AVAGO_FW_SWAP_IMAGE_EXIST
void *serdesFwPtr = NULL; /*init with pointer to file in SPI*/
int serdesFwSize = 0;
/*replace with file size from file system*/
void *sbusMasterFwPtr = NULL; /*init with pointer to file in SPI*/
int sbusMasterFwSize = 0;
void *serdesFwDataSwapPtr = NULL;
int swapFwSize = 0;
#else
int serdesFwSize = AVAGO_SERDES_FW_IMAGE_SIZE;
int sbusMasterFwSize = AVAGO_SBUS_MASTER_FW_IMAGE_SIZE;
int swapFwSize = AVAGO_SERDES_FW_SWAP_IMAGE_SIZE;
#endif /* MICRO_INIT */

extern MV_HWS_SERDES_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRxTuneParamsArray[];


#if (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG))
/**
* @internal mvHwsAvagoSerdesTxOffsetParamsSet function
* @endinternal
*
* @brief   Set a single Tx parameter (TxAmp, TxEmph0 or TxEmph1) into tx-offsets database.
*
* @param[in] sdIndex                  - serdes index
*                                      speedIndex  - serdes speed index
* @param[in] txValue                  - tx value to store in database
*                                      parameter   - parameter type (TxAmp, TxEmph0 or ExEmph1)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxOffsetParamsSet
(
    GT_U8 sdIndex,
    GT_U8 sdSpeedIndex,
    GT_8  txValue,
    GT_U8 parameterType
)
{
#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A
    if (parameterType==0)
    {
        mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_AMP_RAW_VAL_IDX(sdIndex)] =
        (    (mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_AMP_RAW_VAL_IDX(sdIndex)] & ~(0xF << TX_AMP_RAW_VAL_SHIFT_OP(sdIndex)))
        | (txValue << TX_AMP_RAW_VAL_SHIFT_OP(sdIndex)) );
    }
    else if (parameterType==1)
    {
        mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_EMPH0_RAW_VAL_IDX(sdIndex)] =
         (   (mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_EMPH0_RAW_VAL_IDX(sdIndex)] & ~(0xF << TX_EMPH0_RAW_VAL_SHIFT_OP(sdIndex)))
        | (txValue << TX_EMPH0_RAW_VAL_SHIFT_OP(sdIndex)) );
    }
    else
    {
        mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_EMPH1_RAW_VAL_IDX(sdIndex)] =
          (  (mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][TX_EMPH1_RAW_VAL_IDX(sdIndex)] & ~(0xF << TX_EMPH1_RAW_VAL_SHIFT_OP(sdIndex)))
        | (txValue << TX_EMPH1_RAW_VAL_SHIFT_OP(sdIndex)));
    }
#else
    mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][sdIndex] =
        (mvHwsAvagoSerdesManualTxOffsetDb[sdSpeedIndex][sdIndex] & ~(0x1F<<(5*parameterType)))
            | (txValue << (5*parameterType));
#endif
    return GT_OK;

}

/**
* @internal mvHwsAvagoSerdesTxParametersOffsetSet function
* @endinternal
*
* @brief   Initialize Avago related configurations
*
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] phyPortNum               - port number
* @param[in] serdesSpeed              port speed
* @param[in] serdesNumber             - serdes number
* @param[in] offsets                  - value which stores 12 bits of tx  values
*
* @param[out] baseAddrPtr              - (pointer to) base address of the tx offsets database in FW memory
* @param[out] sizePtr                  - (pointer to) size of the tx offsets database in FW memory
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxParametersOffsetSet
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    MV_HWS_SERDES_SPEED serdesSpeed,
    GT_U8  serdesNumber,
    GT_U16 offsets,
    GT_U32 *baseAddrPtr,
    GT_U32 *sizePtr
)
{
    GT_U8 sdSpeed;

    sdSpeed = serdesSpeed;

    /* supported speeds are currently 10G and 25G */
    if (sdSpeed!=PERMITTED_SD_SPEED_0 && sdSpeed!=PERMITTED_SD_SPEED_1 )
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"serdes speed not supported for serdes tx params offset API");
    }

    /* TxAmp */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesTxOffsetParamsSet(serdesNumber, (sdSpeed == INDEX_0_SD_SPEED )? 0 : 1, offsets&0x1F, 0),
                     LOG_ARG_STRING_MAC("setting tx offsets TxAmp"));
    /* TxEmph0 */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesTxOffsetParamsSet(serdesNumber, (sdSpeed == INDEX_0_SD_SPEED )? 0 : 1, ((offsets&0x3E0) >> 5), 1),
                     LOG_ARG_STRING_MAC("setting tx offsets TxEmph0"));
    /* TxEmph1 */
    CHECK_STATUS_EXT(mvHwsAvagoSerdesTxOffsetParamsSet(serdesNumber, (sdSpeed == INDEX_0_SD_SPEED )? 0 : 1, ((offsets&0x7C00) >> 10), 2),
                     LOG_ARG_STRING_MAC("setting tx offsets TxEmph1"));

    /* provide address and size of database in FW for the host usage */
    *baseAddrPtr = (GT_U32)(&mvHwsAvagoSerdesManualTxOffsetDb[0][0]);
    *sizePtr = sizeof(mvHwsAvagoSerdesManualTxOffsetDb);

    return GT_OK;
}
#endif /* (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG)) */

#if (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG))
    #define MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(_sdNum, _sdSpeed)   \
   ((_sdSpeed == PERMITTED_SD_SPEED_0 || _sdSpeed == PERMITTED_SD_SPEED_1) ? \
        (GT_32)(TX_AMP_RAW_VAL(_sdNum,((_sdSpeed == INDEX_0_SD_SPEED )? 0 : 1))) : 0)

    #define MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(_sdNum, _sdSpeed) \
    ((_sdSpeed == PERMITTED_SD_SPEED_0 || _sdSpeed == PERMITTED_SD_SPEED_1) ? \
        (GT_32)(TX_EMPH0_RAW_VAL(_sdNum,((_sdSpeed == INDEX_0_SD_SPEED )? 0 : 1))) : 0)

    #define MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(_sdNum, _sdSpeed) \
    ((_sdSpeed == PERMITTED_SD_SPEED_0 || _sdSpeed == PERMITTED_SD_SPEED_1) ? \
        (GT_32)(TX_EMPH1_RAW_VAL(_sdNum,((_sdSpeed == INDEX_0_SD_SPEED )? 0 : 1))) : 0)

#else
    #define MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(_sdNum, _sdSpeed)   (0)
    #define MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(_sdNum, _sdSpeed) (0)
    #define MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(_sdNum, _sdSpeed) (0)
#endif /* (defined(MV_HWS_FREE_RTOS) || defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG)) */

#ifndef MICRO_INIT
GT_STATUS hwsAvagoSerdesTxRxTuneParamsSet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_SERDES_SPEED baudRate
)
{
    GT_STATUS   rc;
    MV_HWS_SERDES_SPEED i;

    for (i = SPEED_NA; i < LAST_MV_HWS_SERDES_SPEED; i++)
    {
        if (hwsAvagoSerdesTxRxTuneParamsArray[i].serdesSpeed == baudRate)
        {
            /* debug print */
#if (defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_FW_DEBUG) && defined(MV_HWS_FREE_RTOS))
            hwsOsPrintf("\n sdSpeedIterator=%d \n defaultTxAmp=%d, defaultTxEmph0=%d, defaultTxEmph1=%d \n "
                     " txAmpCalculated=%d, txEmph0Calculated=%d, txEmph1Calculated=%d \n "
                     " [32bit] dbTxAmp=%d, dbTxEmph0=%d, dbTxEmph1=%d \n "
                     " [8bit] dbTxAmp=%d, dbTxEmph0=%d, dbTxEmph1=%d",i,
                     hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txAmp,
                     hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txEmph0,
                     hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txEmph1,
                (hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txAmp+
                    (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(serdesNum, baudRate)),
                (hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txEmph0+
                    (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(serdesNum, baudRate)),
                (hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txEmph1+
                    (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(serdesNum, baudRate)),
                     (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(serdesNum, baudRate),
                     (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(serdesNum, baudRate),
                     (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(serdesNum, baudRate),
                     (GT_8)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(serdesNum, baudRate),
                     (GT_8)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(serdesNum, baudRate),
                     (GT_8)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(serdesNum, baudRate));
#endif /* (defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_FW_DEBUG) && defined(MV_HWS_FREE_RTOS)) */

            rc = mvHwsAvagoSerdesManualTxConfig(devNum,portGroup,serdesNum,
                0xFF&(hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txAmp+
                        MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(serdesNum, baudRate)),
                0, /* txAmpAdj */
                0xFF&(hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txEmph0+
                        MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(serdesNum, baudRate)),
                0xFF&(hwsAvagoSerdesTxRxTuneParamsArray[i].txParams.txEmph1+
                        MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(serdesNum, baudRate)),
                0/* slewRate */, GT_TRUE);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = mvHwsAvagoSerdesManualCtleConfig(devNum,portGroup,serdesNum,
                            hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.dcGain,
                            hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.lowFrequency,
                            hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.highFrequency,
                            hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.bandWidth,
                            0 /* loopbackBandwidth */,
                            hwsAvagoSerdesTxRxTuneParamsArray[i].ctleParams.squelch);
            if (rc != GT_OK)
            {
                return rc;
            }

            break;
        }
    }

    return ((LAST_MV_HWS_SERDES_SPEED == i) ? GT_NOT_SUPPORTED : GT_OK);
}
#else
/* Unique ports table structure typedef from Micro-init FW */
typedef struct UNQ_PORT_TABLE{
    GT_U8   portType;  /* of type UNQ_PORT_TYPE */
    GT_U8   devNum;    /* device number */
    GT_U8   PortNum;   /* port number of PP */
    GT_U8   portState; /* of type UNQ_PORT_STATE */
    GT_U8   portMode;  /* of type MV_HWS_PORT_STANDARD */
    GT_U16   portSpeed; /* of type MV_HWS_SERDES_SPEED */
    GT_U8   Channel;   /* XSMI interface number or I2C channel for SFP*/
    GT_U8   Addr;      /* PHY XSMI address or I2C address of SFP */

} UNQ_PORT_TABLE;
extern UNQ_PORT_TABLE *unqTableArrayGetPortByDevPort( GT_U8 devNum, GT_U8 portNum );
extern MV_HWS_SERDES_TXRX_TUNE_PARAMS miAvagoSerdesTxRxTuneParamsArray[];

GT_STATUS hwsAvagoSerdesTxRxTuneParamsSet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_SERDES_SPEED baudRate
)
{
    GT_STATUS   rc;
    MV_HWS_SERDES_SPEED i = LAST_MV_HWS_SERDES_SPEED;
    MV_HWS_SERDES_TXRX_TUNE_PARAMS *pTuneParamsArray = NULL;

    GT_U8 portMode = unqTableArrayGetPortByDevPort(devNum, serdesNum)->portMode;
    if ( (_10_3125G == baudRate || _3_125G == baudRate) && portMode != NON_SUP_MODE ) {
        pTuneParamsArray = &miAvagoSerdesTxRxTuneParamsArray[_3_125G==baudRate?0:1];
    } else {
        for (i = SPEED_NA; i < LAST_MV_HWS_SERDES_SPEED && hwsAvagoSerdesTxRxTuneParamsArray[i].serdesSpeed != baudRate; i++);
        pTuneParamsArray = &hwsAvagoSerdesTxRxTuneParamsArray[i];
    }

    if ( NULL != pTuneParamsArray ){
        /* debug print */
#if(defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_FW_DEBUG) && defined(MV_HWS_FREE_RTOS))
        hwsOsPrintf("\n sdSpeedIterator=%d \n defaultTxAmp=%d, defaultTxEmph0=%d, defaultTxEmph1=%d \n "
                 " txAmpCalculated=%d, txEmph0Calculated=%d, txEmph1Calculated=%d \n "
                 " [32bit] dbTxAmp=%d, dbTxEmph0=%d, dbTxEmph1=%d \n "
                 " [8bit] dbTxAmp=%d, dbTxEmph0=%d, dbTxEmph1=%d",i,
                 pTuneParamsArray->txParams.txAmp,
                 pTuneParamsArray->txParams.txEmph0,
                 pTuneParamsArray->txParams.txEmph1,
            (pTuneParamsArray->txParams.txAmp+
                (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(serdesNum, baudRate)),
            (pTuneParamsArray->txParams.txEmph0+
                (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(serdesNum, baudRate)),
            (pTuneParamsArray->txParams.txEmph1+
                (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(serdesNum, baudRate)),
                 (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(serdesNum, baudRate),
                 (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(serdesNum, baudRate),
                 (GT_32)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(serdesNum, baudRate),
                 (GT_8)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(serdesNum, baudRate),
                 (GT_8)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(serdesNum, baudRate),
                 (GT_8)/*sign-extend*/MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(serdesNum, baudRate));
#endif /* (defined(MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_FW_DEBUG) && defined(MV_HWS_FREE_RTOS)) */

        rc = mvHwsAvagoSerdesManualTxConfig(devNum,portGroup,serdesNum,
            0xFF&(pTuneParamsArray->txParams.txAmp+
                    MV_HWS_AVAGO_SERDES_TX_AMP_OFFSET_MAC(serdesNum, baudRate)),
            0, /* txAmpAdj */
            0xFF&(pTuneParamsArray->txParams.txEmph0+
                    MV_HWS_AVAGO_SERDES_TX_EMPH0_OFFSET_MAC(serdesNum, baudRate)),
            0xFF&(pTuneParamsArray->txParams.txEmph1+
                    MV_HWS_AVAGO_SERDES_TX_EMPH1_OFFSET_MAC(serdesNum, baudRate)),
            0/* slewRate */, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = mvHwsAvagoSerdesManualCtleConfig(devNum,portGroup,serdesNum,
                        pTuneParamsArray->ctleParams.dcGain,
                        pTuneParamsArray->ctleParams.lowFrequency,
                        pTuneParamsArray->ctleParams.highFrequency,
                        pTuneParamsArray->ctleParams.bandWidth,
                        0 /* loopbackBandwidth */,
                       pTuneParamsArray->ctleParams.squelch);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}
#endif

/************************ Forward-Declarations *******************************************************/
#ifndef MV_HWS_AVAGO_NO_VOS_WA
GT_STATUS mvHwsAvagoSerdesManualVosParamsDBInit
(
    unsigned char  devNum
);
#endif
void mvHwsAvagoAaplInit
(
    unsigned char devNum
)
{
    unsigned int i;
    aaplSerdesDbDef[devNum].buf_cmd = NULL;
    aaplSerdesDbDef[devNum].buf_cmd_end = NULL;
    aaplSerdesDbDef[devNum].aacs_server_buffer = NULL;
    aaplSerdesDbDef[devNum].aacs_server = NULL;
    for(i = 0; i < AAPL_MAX_CHIPS; i++)
    {
        aaplSerdesDbDef[devNum].chip_name[i] = NULL;
        aaplSerdesDbDef[devNum].chip_rev[i] = NULL;
    }
    aaplSerdesDbDef[devNum].data_char_end = NULL;
    aaplSerdesDbDef[devNum].log = NULL;
    aaplSerdesDbDef[devNum].log_end = NULL;
    aaplSerdesDbDef[devNum].data_char = NULL;
    aaplSerdesDbDef[devNum].client_data = NULL;

    if ((HWS_DEV_SILICON_TYPE(devNum) ==  BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        aaplSerdesDb[devNum]->sbus_func_ptr = user_supplied_sbus_function;
    }
/* TBD_BOOKMARK_BOBCAT3 replace flags with BC3 and reduced CM3 flags */
#if !defined(BV_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT)
    else if(HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
       /* In BC3 we have two sbus chains which in avago are specified by rings */
       aaplSerdesDb[devNum]->sbus_func_ptr = user_supplied_sbus_units_function;
       aaplSerdesDb[devNum]->sbus_rings = 2;
    }
#endif /* !defined(BV_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) */
    else
    {
        aaplSerdesDb[devNum]->sbus_func_ptr = NULL;
    }

    /*reset devNumber and portGroup*/
    aaplSerdesDb[devNum]->devNum = devNum;
    aaplSerdesDb[devNum]->portGroup = 0;
    aaplSerdesDbDef[devNum].devNum = devNum;
    aaplSerdesDbDef[devNum].portGroup = 0;


    aapl_user_supplied_log_func_ptr = mvHwsGeneralLogStrMsgWrapper;
}

void mvHwsAvagoAaplAddrGet
(
    unsigned char devNum,
    unsigned int **devAddr,
    unsigned int *devSize
)
{
    *devAddr = (unsigned int *)(&(aaplSerdesDbDef[devNum]));
    *devSize = (unsigned int)sizeof(aaplSerdesDbDef[devNum]);
}

int mvHwsAvagoInitializationCheck
(
    unsigned char devNum,
    unsigned int  serdesNum
)
{
    return (aaplSerdesDb[devNum] == NULL) ? GT_NOT_INITIALIZED : GT_OK;
}

int mvHwsAvagoConvertSerdesToSbusAddr
(
    unsigned char devNum,
    unsigned int  serdesNum,
    unsigned int  *sbusAddr
)
{
    CHECK_STATUS(mvHwsAvagoInitializationCheck(devNum, serdesNum));

    if(serdesNum > hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber)
    {
        return GT_BAD_PARAM;
    }
#ifdef BV_DEV_SUPPORT
    *sbusAddr = serdesNum + 1;

#else
    *sbusAddr = hwsDeviceSpecInfo[devNum].serdesInfo.serdesToAvagoMapPtr[serdesNum];
#endif
    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/*******************************************************************************
* mvHwsAvagoEthDriverInit
*
* DESCRIPTION:
*       Initialize Avago related configurations
*
* INPUTS:
*       devNum    - system device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
unsigned int mvHwsAvagoEthDriverInit(unsigned char devNum)
{
#ifndef ASIC_SIMULATION
    /* Set default values: */

    char ip_address[] = "10.5.32.124";
    int     tcp_port     = 90; /* Default port for Avago HS1/PS1 */

    aapl_connect(aaplSerdesDb[devNum], ip_address, tcp_port);
    if(aaplSerdesDb[devNum]->return_code < 0)
    {
        hwsOsPrintf("aapl_connect failed (return code 0x%x)\n", aaplSerdesDb[devNum]->return_code);
        return GT_INIT_ERROR;
    }

#endif /* ASIC_SIMULATION */
    return GT_OK;
}

#if AAPL_ENABLE_AACS_SERVER

#ifndef MV_HWS_BIN_HEADER
extern unsigned int osTaskCreate(const char *name, unsigned int prio, unsigned int stack,
    unsigned (__TASKCONV *start_addr)(void*), void *arglist, GT_U32 *tid);

/**
* @internal mvHwsAvagoSerdesDebugInit function
* @endinternal
*
* @brief   Create Avago AACS Server Process
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDebugInit
(
    char *name,
    unsigned (__TASKCONV *taskRoutine)(GT_VOID*),
    unsigned char devNum
)
{
    void * params = (void *)(GT_UINTPTR)devNum;

    if((osTaskCreate(name, 250, _8K, taskRoutine, params, &avagoAACS_ServerTid)) != GT_OK)
    {
        return GT_FAIL;
    }

    return GT_OK;
}

/*******************************************************************************
* avagoAACS_ServerRoutine
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
static unsigned __TASKCONV avagoAACS_ServerRoutine
(
    IN GT_VOID * param
)
{
    unsigned int tcpPort = 90;

    avago_aacs_server(aaplSerdesDb[0], tcpPort);

    return GT_OK;
}

/*******************************************************************************
* avagoSerdesAacsServerExec
*
* DESCRIPTION:
*       Initialize the Avago AACS Server
*
* INPUTS:
*       devNum - system device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*******************************************************************************/
#ifndef ASIC_SIMULATION
int avagoSerdesAacsServerExec(unsigned char devNum)
{
    int res;

    res = mvHwsAvagoSerdesDebugInit("mvHwsAvagoAACS_Server", avagoAACS_ServerRoutine, devNum);
    if (res != GT_OK)
    {
        hwsOsPrintf("Failed to init Avago AACS Server\n");
        return res;
    }

    return GT_OK;
}
#endif /* ASIC_SIMULATION */

#endif /* MV_HWS_BIN_HEADER */
#endif /* AAPL_ENABLE_AACS_SERVER */
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

#if 1

/**
* @internal mvHwsAvagoSpicoLoad function
* @endinternal
*
*/
int mvHwsAvagoSpicoLoad(unsigned char devNum, unsigned int sbus_addr)
{
    /*
     * Dynamically select between micro_init Fw vs AP Fw at run time
     * for micro_init swapFwSize is set to binary file size from super-image
     */
    if (serdesFwSize != 0) {
#ifdef FW_DOWNLOAD_FROM_SERVER
        AVAGO_DBG(("Loading file: %s to SBus address %x\n", serdesFileName, sbus_addr));
        avago_spico_upload_file(aaplSerdesDb[devNum], sbus_addr, FALSE, serdesFileName);
        if (serdesSwapFileName != NULL)
        {
            AVAGO_DBG(("Loading swap file: %s to SBus address %x\n", serdesSwapFileName, sbus_addr));
            avago_spico_upload_file(aaplSerdesDb[devNum], sbus_addr, FALSE,serdesSwapFileName);
        }
#else /* Download from embedded firmware file */
        AVAGO_DBG(("Loading to SBus address %x data[0]=%x\n", sbus_addr, serdesFwPtr[0]));
        avago_spico_upload(aaplSerdesDb[devNum], sbus_addr, FALSE, serdesFwSize, serdesFwPtr);
#endif /* FW_DOWNLOAD_FROM_SERVER */
    }
    return GT_OK;
}

/**
* @internal mvHwsAvagoSbusMasterLoad function
* @endinternal
*
*/
int mvHwsAvagoSbusMasterLoad(unsigned char devNum, unsigned int sbus_addr)
{
    /*
     * Dynamically select between micro_init Fw vs AP Fw at run time
     * for micro_init swapFwSize is set to binary file size from super-image
     */
    if (swapFwSize != 0) {
#ifdef FW_DOWNLOAD_FROM_SERVER
        AVAGO_DBG(("Loading file: %s to SBus address %x\n", sbusMasterFileName, sbus_addr));
        avago_spico_upload_file(aaplSerdesDb[devNum], sbus_addr , FALSE, sbusMasterFileName);
#else
        AVAGO_DBG(("Loading to SBus address %x  data[0]=%x\n", sbus_addr , sbusMasterFwPtr[0]));
        avago_spico_upload(aaplSerdesDb[devNum], sbus_addr, FALSE, sbusMasterFwSize, sbusMasterFwPtr);

        if(aaplSerdesDb[devNum]->return_code < 0)
        {
            hwsOsPrintf("Avago FW Load Failed (return code 0x%x)\n", aaplSerdesDb[devNum]->return_code);
            return GT_INIT_ERROR;
        }
#ifdef AVAGO_FW_SWAP_IMAGE_EXIST
        AVAGO_DBG(("Loading SWAP to SBus address %x  data[0]=%x\n", sbus_addr , serdesFwDataSwapPtr[0]));
        avago_spico_upload_swap_image(aaplSerdesDb[devNum], sbus_addr, swapFwSize, serdesFwDataSwapPtr);
#endif /* AVAGO_FW_SWAP_IMAGE_EXIST */
#endif /* FW_DOWNLOAD_FROM_SERVER */
    }
    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoSerdesInit
*
* DESCRIPTION:
*       Initialize Avago related configurations
*
* INPUTS:
*       devNum - system device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0  - on success
*       1  - on error
*******************************************************************************/
#ifndef ASIC_SIMULATION
int mvHwsAvagoSerdesInit(unsigned char devNum)
{
    unsigned int sbus_addr;
    unsigned int curr_adr;
    unsigned int chip=0;    /* we have only one chip per aaplSerdesDb[devNum] */
    unsigned int ring =0;   /* we have only one ring */
    Avago_addr_t addr_struct;
#ifndef AVAGO_AAPL_LGPL
    unsigned int addr = avago_make_addr3(AVAGO_BROADCAST, AVAGO_BROADCAST, AVAGO_BROADCAST);
#endif
    GT_U32 serdes_num;
    GT_U32 regAddr;
    GT_U32 regAddr1;
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }
#endif
    /* Validate AAPL */
    if (aaplSerdesDb[devNum] != NULL)
    {
        /* structures were already initialized */
        return GT_ALREADY_EXIST;
    }

    /* Construct AAPL structure */
    aapl_init(&(aaplSerdesDbDef[devNum]));
    aaplSerdesDb[devNum] = &(aaplSerdesDbDef[devNum]);

    aaplSerdesDb[devNum]->devNum    = devNum;
    aaplSerdesDb[devNum]->portGroup = 0;

    /* In BC3 device we have two different serdes chains. In all
       other devices there is a single chain. Chains are indexed
       by "ring" field name. */
    if (HWS_DEV_SILICON_TYPE(devNum) ==  Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) ==  Aldrin2)
    {
        aaplSerdesDb[devNum]->sbus_rings = 2;
    }
    else
    {
        aaplSerdesDb[devNum]->sbus_rings = 1;
    }

    if ((HWS_DEV_SILICON_TYPE(devNum) ==  BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        aaplSerdesDb[devNum]->sbus_func_ptr =  user_supplied_sbus_function;

        /* Take Avago device out of reset */
        user_supplied_sbus_soft_reset(aaplSerdesDb[devNum]);
    }
/* TBD_BOOKMARK_BOBCAT3 replace flags with BC3 and reduced CM3 flags */
#if !defined(BV_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT)
    else if(HWS_DEV_SILICON_TYPE(devNum) == Bobcat3 || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
       aaplSerdesDb[devNum]->sbus_func_ptr =  user_supplied_sbus_units_function;

        /* Take Avago units device out of reset */
        user_supplied_sbus_units_soft_reset(aaplSerdesDb[devNum]);
    }
#endif /* !defined(BV_DEV_SUPPORT) && !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) */
    else
    {
        return GT_INIT_ERROR;
    }

    /* Change the SBUS master clk divider to 16. 0x6007F828 is indirect address */
    if(HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        regAddr = 0x0607F828;
        regAddr1 = 0x06042404;
    }
    else
    {
        regAddr = 0x6007F828;
        regAddr1 = 0x60042404;
    }

    CHECK_STATUS_EXT(hwsRegisterSetFuncPtr(devNum, aaplSerdesDb[devNum]->portGroup, regAddr, 0x4, 0xFFFF), LOG_ARG_NONE_MAC );

    /* Change the Sensor clk divider to 78 */
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, aaplSerdesDb[devNum]->portGroup, regAddr1, 0x4E, 0xFFFF));

#ifndef MV_HWS_BIN_HEADER
#ifndef MICRO_INIT /* Only communication through SBUS used */
    if (avagoConnection == AVAGO_ETH_CONNECTION)
    {
        aaplSerdesDb[devNum]->communication_method = AAPL_DEFAULT_COMM_METHOD;

        #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        CHECK_STATUS(mvHwsAvagoEthDriverInit(devNum));
        #endif
    }
    else if (avagoConnection == AVAGO_I2C_CONNECTION)
    {
        aaplSerdesDb[devNum]->communication_method = AVAGO_USER_SUPPLIED_I2C;
        #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        CHECK_STATUS(mvHwsAvagoInitI2cDriver(devNum));
        #endif
    }
    else
#endif /*MICRO_INIT*/
    if (avagoConnection == AVAGO_SBUS_CONNECTION)
    {
        aaplSerdesDb[devNum]->communication_method = AVAGO_USER_SUPPLIED_SBUS_DIRECT;
    }
    else
    {
        hwsOsPrintf("mvHwsAvagoSerdesInit : unknown communication method %x\n",
                          aaplSerdesDb[devNum]->communication_method);
        return GT_INIT_ERROR;
    }
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    /* let know to AVAGO that system recovery process is active */
    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        recovery_process = 1;
    }
    else
    {
        recovery_process = 0;
    }
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
    /* Validate access to Avago device */
    mvHwsAvagoAccessValidate(devNum, 0);

    /* Initialize AAPL structure */

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)
    aapl_get_ip_info(aaplSerdesDb[devNum],1);
#endif

#ifndef AVAGO_AAPL_LGPL
    /* Print AAPL structure */
    aapl_print_struct(aaplSerdesDb[devNum],aaplSerdesDb[devNum]->debug > 0, addr, 0);
#endif /* AVAGO_AAPL_LGPL */

#else /* MV_HWS_BIN_HEADER */
    /* Validate access to Avago device */
    mvHwsAvagoAccessValidate(devNum, 0);

    /* Initialize AAPL structure */
    aapl_get_ip_info(aaplSerdesDb[devNum],1);
#endif /*MV_HWS_BIN_HEADER*/
    AVAGO_DBG(("Loading Avago Firmware.......\n"));


    /* Converts the address into an address structure */
    addr_struct.chip = chip;
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
    {
        for (ring=0 ; ring < aaplSerdesDb[devNum]->sbus_rings ; ring++)
        {
            addr_struct.ring = ring;
            /* Download Serdes's Firmware */
            /* ========================== */
            for( curr_adr = 0; curr_adr <= aaplSerdesDb[devNum]->max_sbus_addr[chip][ring]; curr_adr++ )
            {
                if (aaplSerdesDb[devNum]->ip_type[chip][ring][curr_adr] == AVAGO_SERDES)
                {
                    addr_struct.sbus = curr_adr;
                    sbus_addr = avago_struct_to_addr(&addr_struct);

                    CHECK_STATUS(hwsDeviceSpecInfo[devNum].serdesInfo.
                                 sbusAddrToSerdesFuncPtr(devNum, &serdes_num, sbus_addr));
                    if (mvHwsAvagoCheckSerdesAccess(devNum, 0, serdes_num) == GT_NOT_INITIALIZED)
                    {
                        CHECK_STATUS(mvHwsAvagoSpicoLoad(devNum, sbus_addr));
                        if(aaplSerdesDb[devNum]->return_code < 0)
                        {
                            hwsOsPrintf("Avago FW Load Failed (return code 0x%x)\n", aaplSerdesDb[devNum]->return_code);
                            return GT_INIT_ERROR;
                        }
                    }
                }
            }

            /* Download SBus_Master Firmware */
            /* ============================= */
            addr_struct.ring = ring;
            addr_struct.sbus = AVAGO_SBUS_MASTER_ADDRESS;
            sbus_addr = avago_struct_to_addr(&addr_struct);
            CHECK_STATUS(mvHwsAvagoSbusMasterLoad(devNum,sbus_addr) );

            if(aaplSerdesDb[devNum]->return_code < 0)
            {
                hwsOsPrintf("Avago FW Load Failed, ring: %d (return code 0x%x)\n", ring, aaplSerdesDb[devNum]->return_code);
                return GT_INIT_ERROR;
            }
        }
    }
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        if ( (HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin) )
        {
            /* update 5 fields in AAPL */
            aaplSerdesDb[devNum]->ip_rev[0][0][254] = 0xbe;
            aaplSerdesDb[devNum]->ip_type[0][0][253] = 0x3;
            aaplSerdesDb[devNum]->lsb_rev[0][0][253] = 0xff;
            aaplSerdesDb[devNum]->lsb_rev[0][0][255] = 0x4;
            aaplSerdesDb[devNum]->spico_running[0][0][253]=0x1;
        }
    }
#endif /*MICRO_INIT*/
    AVAGO_DBG(("Done\n"));



#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    /* Init Avago Access Protection in multi-process environment */
    /* This protection is implemented at the scope of the Host!! */
    /* ========================================================= */
    hwsOsMutexCreate("avagoAccess", &avagoAccessMutex);
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

#if AAPL_ENABLE_AACS_SERVER
#ifndef MV_HWS_BIN_HEADER
    if (avagoConnection != AVAGO_ETH_CONNECTION)
    {
        if (aacsServerEnable)
        {
            CHECK_STATUS(avagoSerdesAacsServerExec(devNum));
        }
    }
#endif /* MV_HWS_BIN_HEADER */
#endif /* AAPL_ENABLE_AACS_SERVER */
#ifndef MV_HWS_AVAGO_NO_VOS_WA
    if (hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride)
    {
        /* Initializing the VOS override params DB in order to prevent DFX
        register access every time those values are needed. In the CM3 service
        CPU those values will be set manually with the values sent from host.*/
        if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) ||
                        (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
        {
            /* Initializing the VOS override params DB for Bobcat3 in order to prevent DFX
            register access every time those values are needed.*/
            hwsOsMemSetFuncPtr(&serdesVosParams64BitDB[0], 0, sizeof(serdesVosParams64BitDB));
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            if (!serdesVos64BitDbInitialized)
            {
                CHECK_STATUS(mvHwsAvagoSerdesBc3VosConfig(devNum));
            }
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
        }
        else
        {
            hwsOsMemSetFuncPtr(&serdesVosParamsDB[0], 0, sizeof(serdesVosParamsDB));
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            if (!serdesVosParamsDBInitialized)
            {
                CHECK_STATUS(mvHwsAvagoSerdesManualVosParamsDBInit(devNum));
            }
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
        }
    }
#endif /* MV_HWS_AVAGO_NO_VOS_WA */

    aapl_user_supplied_log_func_ptr = mvHwsGeneralLogStrMsgWrapper;

    return GT_OK;
}
#endif /* ASIC_SIMULATION */

#elif 0

/**
* @internal mvHwsAvagoSerdesInit function
* @endinternal
*
* @brief   Initialize Avago related configurations
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesInit(unsigned char devNum)
{
    /* Initialize AAPL structure in CM3 */
    aaplSerdesDb[devNum] = &(aaplSerdesDbDef[devNum]);

    return GT_OK;
}

#endif

#ifndef ASIC_SIMULATION
/**
* @internal mvHwsAvagoSerdesSpicoInterrupt function
* @endinternal
*
* @brief   Issue the interrupt to the Spico processor.
*         The return value is the interrupt number.
*
* @param[out] result                   - spico interrupt return value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesSpicoInterrupt
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    unsigned int    interruptCode,
    unsigned int    interruptData,
    int             *result
)
{
    unsigned int  sbus_addr;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    if (result == NULL)
    {
        mvHwsAvagoAccessLock(devNum);
        avago_spico_int(aaplSerdesDb[devNum], sbus_addr, interruptCode, interruptData);
        mvHwsAvagoAccessUnlock(devNum);
    }
    else
    {
        mvHwsAvagoAccessLock(devNum);
        *result = avago_spico_int(aaplSerdesDb[devNum], sbus_addr, interruptCode, interruptData);
        mvHwsAvagoAccessUnlock(devNum);
    }

    CHECK_AVAGO_RET_CODE();

    return GT_OK;
}

int temperatureGlobal = 0;
unsigned int timeInterval = 5000;
unsigned int lastTimeStamp = 0;

extern GT_U32 mvPortCtrlCurrentTs(void);
/**
* @internal mvHwsAvagoSerdesTemperatureGet function
* @endinternal
*
* @brief   Get the Temperature (in C) from Avago Serdes
*
* @param[out] temperature              - Serdes  degree value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesTemperatureGet
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    int             *temperature
)
{
#ifndef ASIC_SIMULATION
    unsigned int sensor_addr;

#if defined(MV_HWS_FREE_RTOS)
    GT_U32 miliSeconds=0;
    miliSeconds = mvPortCtrlCurrentTs();
    if ((miliSeconds > lastTimeStamp + timeInterval) || (miliSeconds < lastTimeStamp))
    {
#endif /* defined(MV_HWS_FREE_RTOS) */

        /* avoid warning */
        serdesNum = serdesNum;

        if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin))
        {
            sensor_addr = 9;
        }
        else if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3))
        {
            sensor_addr = 26;
        }
        else if ((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
        {
            sensor_addr = 1; /* AVAGO TSEN0 in ring0 */
        }
        else if ((HWS_DEV_SILICON_TYPE(devNum) == Pipe))
        {
            sensor_addr = 18;
        }
        else
        {
            return GT_NOT_SUPPORTED;
        }

        /* trying to get the Temperature in degrees maximum 10 times and set 1ms delay between each time */
        mvHwsAvagoAccessLock(devNum);
        temperatureGlobal = avago_sbm_get_temperature(aaplSerdesDb[devNum], sensor_addr, 0);
        mvHwsAvagoAccessUnlock(devNum);
#if defined(MV_HWS_FREE_RTOS)
        lastTimeStamp = miliSeconds;
    }
#endif /* defined(MV_HWS_FREE_RTOS) */
    *temperature = temperatureGlobal;

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoSerdesCalCodeSet
*
* DESCRIPTION:
*       Set the calibration code(value) for Rx or Tx
*
* INPUTS:
*       devNum    - device number
*       portGroup - port group (core) number
*       serdesNum - SERDES number
*       mode      - True for Tx mode, False for Rx mode
*       therm     - Thermometer of VCO (0...0xFFFF)
*       bin       - bin of VCO (0...0xFFFF)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
unsigned int mvHwsAvagoSerdesCalCodeSet(int devNum, int portGroup, int serdesNum, BOOL mode, int therm, int bin)
{
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4002 | (mode << 9), NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, therm, NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4003 | (mode << 9), NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, bin, NULL));

    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoSerdesCalCodeGet
*
* DESCRIPTION:
*       Get the calibration code(value) for Rx or Tx
*
* INPUTS:
*       devNum    - device number
*       portGroup - port group (core) number
*       serdesNum - SERDES number
*       mode      - True for Tx mode, False for Rx mode
*
* OUTPUTS:
*       therm - Thermometer of VCO  (0...0xFFFF)
*       bin   - bin of VCO  (0...0xFFFF)
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
unsigned int mvHwsAvagoSerdesCalCodeGet
(
    int     devNum,
    int     portGroup,
    int     serdesNum,
    BOOL    mode,
    int     *therm,
    int     *bin
)
{
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4002 | (mode << 9), NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x1A, 0, therm));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4003 | (mode << 9), NULL));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x1A, 0, bin));

    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoCalCodeShift
*
* DESCRIPTION:
*       Shift the calcode according to the amount value
*
* INPUTS:
*       devNum    - device number
*       portGroup - port group (core) number
*       serdesNum - SERDES number
*       mode      - True for Tx mode, False for Rx mode
*       shift     - amount of cal-code shift
*       therm     - Thermometer of VCO  (0...0xFFFF)
*       bin       - bin of VCO  (0...0xFFFF)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
unsigned int mvHwsAvagoCalCodeShift
(
    int     devNum,
    int     portGroup,
    int     serdesNum,
    BOOL    mode,
    int     shift,
    int     therm,
    int     bin
)
{
    unsigned int    res;
    int             bin_portion;

    bin_portion = (bin & 0x7) + shift;

    therm = (bin_portion > 7) ? ((therm  <<1 )+1): therm;
    therm = (bin_portion < 0) ? (therm >> 1) : therm;

    bin_portion &=0x7;
    bin = (bin & 0xFFF8)| bin_portion;

    res = mvHwsAvagoSerdesCalCodeSet(devNum, portGroup, serdesNum, mode, therm, bin);
    if (res != GT_OK)
    {
        hwsOsPrintf("mvHwsAvagoSerdesCalCodeSet failed (%d)\n", res);
        return GT_FAIL;
    }

    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoSerdesShiftCalc
*
* DESCRIPTION:
*       Calculate the Shift value according to the Temperature
*
* INPUTS:
*       devNum      - device number
*       portGroup   - port group (core) number
*       temperature - Temperature (in C) from Avago Serdes
*
* OUTPUTS:
*       shift - Shift value for calibration code in Rx and Tx
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
unsigned int mvHwsAvagoSerdesShiftCalc
(
    unsigned char   devNum,
    unsigned int    portGroup,
    int             temperature,
    int             *shift
)
{
    *shift = 0;

    if (temperature < -20)
        *shift = 2;
    else if ((temperature >= -20) && (temperature <= 0))
        *shift = 1;
    else if ((temperature > 30) && (temperature <= 75))
        *shift = -1;
    else if (temperature > 75)
        *shift = -2;

    return GT_OK;
}

/*******************************************************************************
* mvHwsAvagoSerdesVcoConfig
*
* DESCRIPTION:
*       Compensate the VCO calibration value according to Temperature in order
*       to enable Itemp operation
*
* INPUTS:
*       devNum      - device number
*       portGroup   - port group (core) number
*       serdesNum   - SERDES number
*       temperature - Temperature (in C) from Avago Serdes
*       txTherm     - Tx Thermometer of VCO  (0...0xFFFF)
*       txBin       - Tx bin of VCO  (0...0xFFFF)
*       rxTherm     - Rx Thermometer of VCO  (0...0xFFFF)
*       rxBin       - Rx bin of VCO  (0...0xFFFF)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
unsigned int mvHwsAvagoSerdesVcoConfig
(
    unsigned char   devNum,
    unsigned int    portGroup,
    unsigned int    serdesNum,
    int     temperature,
    int     txTherm,
    int     txBin,
    int     rxTherm,
    int     rxBin
)
{
    int shift=0;
    unsigned int res;

    /* Calculate the Shift value according to the Temperature */
    res = mvHwsAvagoSerdesShiftCalc(devNum, portGroup, temperature, &shift);
    if (res != GT_OK)
    {
        hwsOsPrintf("mvHwsAvagoSerdesShiftCalc failed (%d)\n", res);
        return GT_FAIL;
    }

    /* Shift the calibration code for Tx */
    res = mvHwsAvagoCalCodeShift(devNum, portGroup, serdesNum, TRUE, shift, txTherm, txBin);
    if (res != GT_OK)
    {
        hwsOsPrintf("mvHwsAvagoCalCodeShift failed (%d)\n", res);
        return GT_FAIL;
    }

    /* shift the calibration code for Rx */
    res = mvHwsAvagoCalCodeShift(devNum, portGroup, serdesNum, FALSE, shift, rxTherm, rxBin);
    if (res != GT_OK)
    {
        hwsOsPrintf("mvHwsAvagoCalCodeShift failed (%d)\n", res);
        return GT_FAIL;
    }

    return GT_OK;
}
#endif /* ASIC_SIMULATION */
#ifndef MV_HWS_AVAGO_NO_VOS_WA
/**
* @internal mvHwsAvagoSerdesManualVosConfigSet function
* @endinternal
*
* @brief   Applying VOS correction parameters from eFuze to improve eye openning.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosConfigSet
(
    unsigned char  devNum,
    unsigned int   serdesNum
)
{
    GT_U32 serdesVosValue;
    GT_U32 serdesVosArrIdx;
    GT_U32 dfxVosValue1;
    GT_U32 dfxVosValue2;
    GT_U32 vosIdx;

    /* In BobK there are 12 sequential serdeses applicable for VOS override. To save space, they are arrange
       in an array by they modulu index, 12 indexes from 0 to 11. */
    /* TBD_BOOKMARK_BOBCAT3 */
    if (HWS_DEV_SILICON_TYPE(devNum) == BobK)
    {
        serdesVosArrIdx = serdesNum%12;
    }
    else
    {
        serdesVosArrIdx = serdesNum;
    }

    /* Checking if the serdes is in the list of serdeses applicable for VOS override.
       If not, no VOS override is needed. */
    /* Non CM3 code */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    if (serdesNum != serdesVosOffsetBobK[serdesVosArrIdx].serdes &&
            serdesNum != serdesVosOffsetAldrin[serdesVosArrIdx].serdes)
#else /* CM3 code */
#if defined(BOBK_DEV_SUPPORT)
    if (serdesNum != serdesVosOffsetBobK[serdesVosArrIdx].serdes)
#else
/* BC3/PIPE TBD update VOS offset */
#if defined(ALDRIN_DEV_SUPPORT) /* || defined (BC3_DEV_SUPPORT) || defined (PIPE_DEV_SUPPORT) || defined (ALDRIN2_DEV_SUPPORT) */
    if (serdesNum != serdesVosOffsetAldrin[serdesVosArrIdx].serdes)
#endif /* defined(ALDRIN_DEV_SUPPORT) */
#endif /* defined(BOBK_DEV_SUPPORT) */
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */
    {
        CPSS_LOG_INFORMATION_MAC("Not performing VOS config on serdes %d ", serdesNum);
        return GT_OK;
    }

    /* Getting VOS0-6 from the DB. For each serdes the values are divided into two group.
       one for VOS0-3 and one for VOS4-6, as they are stored in the DFX. */
    dfxVosValue1 = serdesVosParamsDB[(serdesVosArrIdx*2)];
    dfxVosValue2 = serdesVosParamsDB[(serdesVosArrIdx*2)+1];

    CPSS_LOG_INFORMATION_MAC("Serdes Num %d, vos0-3 32bits: 0x%08x, vos4-6 32bits: 0x%08x ", serdesNum, dfxVosValue1, dfxVosValue2);

    /* Checking for corrupted data in VOS0-3. If such exist, the operation can not be completed.
       corrupted data is considered as a value less then 0x60 */
    if ( ((dfxVosValue1 & 0xff) < 0x60) || (((dfxVosValue1 >> 8) & 0xff) < 0x60) ||
         (((dfxVosValue1 >> 16) & 0xff) < 0x60) || (((dfxVosValue1 >> 24) & 0xff) < 0x60) )
    {
        CPSS_LOG_INFORMATION_MAC("VOS override failed. Corrupted DFX value for VOS0-3: 0x%08x at db index: %d", dfxVosValue1, (serdesVosArrIdx*2));
        /* Returning GT_OK as lack of VOS values should not fail the serdes and the entire system */
        return GT_OK;
    }
    /* Checking for corrupted data in VOS4-6 */
    if ( ((dfxVosValue2 & 0xff) < 0x60) || (((dfxVosValue2 >> 8) & 0xff) < 0x60) ||
         (((dfxVosValue2 >> 16) & 0xff) < 0x60) )
    {
        CPSS_LOG_INFORMATION_MAC("VOS override failed. Corrupted DFX value for VOS4-6: 0x%08x at db index: %d", dfxVosValue1, (serdesVosArrIdx*2)+1);
        /* Returning GT_OK as lack of VOS values should not fail the serdes and the entire system */
        return GT_OK;
    }

    /* Calculating the VOS value for each VOS index */
    for (vosIdx = 0; vosIdx <=3 ; vosIdx++)
    {
        /* Calculating the generic formula */
        serdesVosValue = ( 0x1900 + (vosIdx * 0x100) );
        /* Adding the matching 8bits from the 32bit DFX value */
        serdesVosValue += ( ( dfxVosValue1 >> ( vosIdx * 8 ) ) & 0xff );
        /* Applying final VOS value to serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, 0, serdesNum, 0x26, serdesVosValue, NULL));
        /* Log */
        CPSS_LOG_INFORMATION_MAC("Serdes Num %d, VOS index: %d, Value: 0x%08x ", serdesNum, vosIdx, serdesVosValue);
    }

    /* Calculating the VOS value for each VOS index */
    for (vosIdx = 4; vosIdx <=6 ; vosIdx++)
    {
        /* Calculating the generic formula */
        serdesVosValue = ( 0x1900 + (vosIdx * 0x100) );
        /* Adding the matching 8bits from the 32bit DFX value */
        serdesVosValue += ( (dfxVosValue2 >> ( (vosIdx-4) * 8) ) & 0xff );
        /* Applying final VOS value to serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, 0, serdesNum, 0x26, serdesVosValue, NULL));
        /* Log */
        CPSS_LOG_INFORMATION_MAC("Serdes Num %d, VOS index: %d, Value: 0x%08x ", serdesNum, vosIdx, serdesVosValue);
    }

    return GT_OK;
}
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsAvagoSerdesManualVosParamsSet function
* @endinternal
*
* @brief   Set the VOS Override parameters in to the DB. Used to manually set the
*         VOS override parameters instead of using the eFuze burned values.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsSet
(
    unsigned char  devNum,
    unsigned long  *vosParamsPtr,
    unsigned long  bufferIdx,
    unsigned long  bufferLength
)
{
    GT_U32 offset;
    GT_U32 baseOffset;

    /* Calculating the offset of the VOS param DB in which the given buffer should be written */
    baseOffset = bufferLength * bufferIdx;

    /* Null pointer check */
    if (vosParamsPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
    /* Boundary check */
    if ( (baseOffset+bufferLength) > (sizeof(serdesVosParams64BitDB)/sizeof(GT_U32)) )
    {
        return GT_BAD_PARAM;
    }
#else
    /* Boundary check */
    if ( (baseOffset+bufferLength) > (sizeof(serdesVosParamsDB)/sizeof(GT_U32)) )
    {
        return GT_BAD_PARAM;
    }
#endif

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
    baseOffset = baseOffset >> 1;
#endif /* BC3_DEV_SUPPORT */

    /* Writing the values */
    for (offset = 0; offset < bufferLength; offset++)
    {
#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
        if ((HWS_DEV_SILICON_TYPE(devNum) ==  Bobcat3 ) || (HWS_DEV_SILICON_TYPE(devNum) ==  Aldrin2 ) ||
                (HWS_DEV_SILICON_TYPE(devNum) ==  Pipe ))
        {

            if (offset*2 >= bufferLength)
            {
                break;
            }

            /* 64bit per element */
            serdesVosParams64BitDB[baseOffset + offset] = (GT_UL64)(((GT_UL64)vosParamsPtr[(offset*2)+1]) << 32 | ((GT_UL64)(vosParamsPtr[offset*2]) & 0xFFFFFFFF));
        }
        else
#endif /* defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) */
        {
            /* 32bit per element */
            serdesVosParamsDB[baseOffset + offset] = vosParamsPtr[offset];
        }
    }

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
    serdesVos64BitDbInitialized = GT_TRUE;
#endif
    serdesVosParamsDBInitialized = GT_TRUE;

    return GT_OK;
}

GT_STATUS mvHwsAvagoSerdesManualVosMinAndBaseForBc3ParamsSet
(
    unsigned char  devNum,
    unsigned long  minVosValueToSet,
    unsigned long  baseVosValueToSet
)
{
    minVosValue = minVosValueToSet;
    baseVosValue = baseVosValueToSet;

    return GT_OK;
}

#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

/**
* @internal mvHwsAvagoSerdesManualVosParamsDBInit function
* @endinternal
*
* @brief   Init the VOS override DB from the DFX.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsDBInit
(
    unsigned char  devNum
)
{
    GT_U32 dfxVosAddress1;
    GT_U32 dfxVosAddress2;
    GT_U32 serdesVosArrIdx;
    GT_U32 serdesNum;
    GT_U32 serdesStart;
    GT_U32 serdesEnd;
    GT_U32 paramsDbIndex;
    GT_U32 dfxVosVal=0;
    GT_U32 eFuseVosRegion;
    HWS_AVAGO_SERDES_VOS_OFFSET * serdesVosOffsetArrayPtr;

    /* Setting the starting serdes and end serdes, eFuse region address and temp array pointer
       according to device type */
/* Code for Non CM3 CPU (host) and for Bobk CM3  */
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(BOBK_DEV_SUPPORT))
    if (HWS_DEV_SILICON_TYPE(devNum) == BobK)
    {
        eFuseVosRegion = SERVER_HD_EFUSE_VOS_REGION_BOBK;
        serdesVosOffsetArrayPtr = serdesVosOffsetBobK;
        serdesStart = 24;
        serdesEnd = 35;
    }
    else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MV_HWS_REDUCED_BUILD_EXT_CM3) && defined(BOBK_DEV_SUPPORT)) */
/* Code for Non CM3 CPU (host) and for Aldrin CM3  */
    if (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
    {
        eFuseVosRegion = SERVER_HD_EFUSE_VOS_REGION_ALDRIN;
        serdesVosOffsetArrayPtr = serdesVosOffsetAldrin;
        serdesStart = 0;
        serdesEnd = 31;
    }
    else
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ARG_STRING_MAC("silicon type not supported"));
    }

    /* For each serdes, reading the values from the DFX and writing to the DB */
    for (serdesNum=serdesStart, paramsDbIndex=0;
            serdesNum<=serdesEnd;
                serdesNum++, paramsDbIndex++)
    {
        /* In BobK the serdeses indexes not starting from 0 so in order to get the array index for the serdes
           we use modulo */
        if (HWS_DEV_SILICON_TYPE(devNum) == BobK)
        {
            serdesVosArrIdx = serdesNum%12;
        }
        else
        {
            serdesVosArrIdx = serdesNum;
        }
        /* Total DFX address calculation for VOS0-3 is:
           DFX eFuze addess + VOS region + serdes offset */
        dfxVosAddress1 = (SERVER_HD_EFUSE_SLAVE_REG + eFuseVosRegion + serdesVosOffsetArrayPtr[serdesVosArrIdx].serdesVosOffset);
        /* Reading 32bit VOS0-VOS3 value for the serdes from DFX */
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, dfxVosAddress1, &dfxVosVal));
        serdesVosParamsDB[paramsDbIndex*2] = dfxVosVal;
        /* Total DFX address calculation for VOS4-6 is:
           DFX eFuze addess + VOS region + serdes offset + 4 */
        dfxVosAddress2 = (SERVER_HD_EFUSE_SLAVE_REG + eFuseVosRegion + serdesVosOffsetArrayPtr[serdesVosArrIdx].serdesVosOffset + 4);
        /* Reading 32bit VOS4-VOS6 value for the serdes from DFX. */
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, dfxVosAddress2, &dfxVosVal));
        serdesVosParamsDB[(paramsDbIndex*2)+1] = dfxVosVal;
    }

    serdesVosParamsDBInitialized = GT_TRUE;

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualVosParamsGet function
* @endinternal
*
* @brief   Get the VOS Override parameters from the local DB.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualVosParamsGet
(
    unsigned char  devNum,
    unsigned long  *vosParamsPtr
)
{
    /* Null pointer check */
    if (vosParamsPtr==NULL)
    {
        return GT_BAD_PARAM;
    }
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) ||
        (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        if (!serdesVos64BitDbInitialized)
        {
            CHECK_STATUS(mvHwsAvagoSerdesBc3VosConfig(devNum));
        }
    }
    else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */
    {
        /* Initializing VOS db if not initialized by now */
        if (!serdesVosParamsDBInitialized)
        {
            CHECK_STATUS(mvHwsAvagoSerdesManualVosParamsDBInit(devNum));
        }
    }

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    /* Copying values from our local DB */
    if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        hwsOsMemCopyFuncPtr(vosParamsPtr, serdesVosParams64BitDB, sizeof(serdesVosParams64BitDB));
    }
    else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */
    {
        hwsOsMemCopyFuncPtr(vosParamsPtr, serdesVosParamsDB, sizeof(serdesVosParamsDB));
    }

    return GT_OK;
}
#endif /*#ifndef MV_HWS_AVAGO_NO_VOS_WA*/
/**
* @internal mvHwsAvagoSerdesArrayPowerCtrlImpl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesArrayPowerCtrlImpl
(
    unsigned char       devNum,
    unsigned int        portGroup,
    GT_UOPT             numOfSer,
    GT_UOPT             *serdesArr,
    unsigned char       powerUp,
    unsigned int        divider,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
)
{
    Avago_serdes_init_config_t *config;
    Avago_serdes_init_config_t configDef;

    unsigned int sbus_addr;
    unsigned int errors;
    int data;
    GT_UREG_DATA data_ureg;
    GT_UOPT i;
#if defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    GT_U32  tryCount=0;
#endif
    unsigned int ctleBitMapIndex;
    GT_U32 ctleBias;
    GT_U32 sdOffset = 0;
    GT_U32 macOffset = 0;
    GT_U8 numOfLanes;
    int st;

    if(serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

#ifndef MV_HWS_BIN_HEADER
    /* for Serdes PowerDown */
    if (powerUp == GT_FALSE)
    {
        /* if Training is in progress give it chance to finish and turn serdes off anyway */
        mvHwsAvagoAccessLock(devNum);
        for (i = 0; i < numOfSer; i++)
        {
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
            for (i=0; i<25; i++)
            {
                st = hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_CORE_STATUS, &data_ureg, 0x2);
                if (st != GT_OK)
                {
                    mvHwsAvagoAccessUnlock(devNum);
                    CHECK_STATUS(st);
                }
                if (data_ureg != 0)
                {
                    ms_sleep(20);
                }
                else
                {
                    break;
                }
            }
#else
            do
            {
               st = hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_CORE_STATUS, &data_ureg, 0x2);
               if (st != GT_OK)
               {
                   mvHwsAvagoAccessUnlock(devNum);
                   CHECK_STATUS(st);
               }
            }while ((data_ureg != 0) && (tryCount++ < 5));
#endif
            /* turn serdes off */
            st = mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[i], &sbus_addr);
            if (st != GT_OK)
            {
                mvHwsAvagoAccessUnlock(devNum);
                CHECK_STATUS(st);
            }

            /* 0x7 , 0x0
            disable the spico and return its parameters to default. */
            avago_sbus_wr(aaplSerdesDb[devNum], sbus_addr, 0x7, 0x0);

            /* 0x7 , 0x2
            enable the spico which is now at init state (like after loading the spico FW). */
            avago_sbus_wr(aaplSerdesDb[devNum], sbus_addr, 0x7, 0x2);

            avago_serdes_set_tx_rx_enable(aaplSerdesDb[devNum], sbus_addr, GT_FALSE, GT_FALSE, GT_FALSE);
            if (aaplSerdesDb[devNum]->return_code != 0)
            {
                aaplSerdesDb[devNum]->return_code = 0;
            }
        }
        mvHwsAvagoAccessUnlock(devNum);

        /* TxClk/RxClk Tlat_en=0. Return to default */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_0, 0, (3 << 13)));
        }

        /* Read the saved value from AVAGO_SD_METAL_FIX register to check the Rx/Tx width mode */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_SD_METAL_FIX, &data_ureg, 0xFFFF));
            if ((data_ureg >> 9) & 0x1)
            {
                /* Set back the value of AVAGO_SD_METAL_FIX register bit #9 if it was changed in 10BIT_ON mode for GPCS modes */
                CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_SD_METAL_FIX, (0 << 9), (1 << 9)));
            }
        }

#ifdef BV_DEV_SUPPORT
        /*disable external PCIE clock*/
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x30, 0x0, NULL));
        }
#endif

        /* Serdes Reset */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesArr[i], GT_TRUE, GT_TRUE, GT_TRUE));
        }


        return GT_OK;
    }
#endif /* MV_HWS_BIN_HEADER */

    /* for Serdes PowerUp */
    /* Initialize the SerDes slice */
    avago_serdes_init_config(&configDef);
    config = &configDef;

    if(HWS_DEV_SERDES_TYPE(devNum) >= AVAGO_16NM)
    {
        switch(serdesConfigPtr->rxEncoding)
        {
            case SERDES_ENCODING_AVAGO_NRZ:
                config->rx_encoding = AVAGO_SERDES_NRZ;
                break;
            case SERDES_ENCODING_AVAGO_PAM4:
                config->rx_encoding = AVAGO_SERDES_PAM4;
                break;
            default:
                break;
        }

        switch(serdesConfigPtr->txEncoding)
        {
            case SERDES_ENCODING_AVAGO_NRZ:
                config->tx_encoding = AVAGO_SERDES_NRZ;
                break;
            case SERDES_ENCODING_AVAGO_PAM4:
                config->tx_encoding = AVAGO_SERDES_PAM4;
                break;
            default:
                break;
        }
    }

    /* Update tx_divider and rx_divider */
    config->tx_divider = config->rx_divider = divider;

    /* initializes the Avago_serdes_init_config_t struct */
    config->sbus_reset = FALSE;
    config->signal_ok_threshold = 2;

    /* Select the Rx & Tx data path width */
    if (serdesConfigPtr->busWidth == _10BIT_ON)
    {
        config->rx_width = config->tx_width = 10;

        /* Save the width setting for PRBS test in AVAGO_SD_METAL_FIX register.
           It is needed for GPCS modes: the PRBS test can run in these modes only with Rx/Tx width=20BIT.
           If the Rx/Tx width=10BIT then it will be changed to 20BIT in mvHwsAvagoSerdesTestGen function */
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], AVAGO_SD_METAL_FIX, (1 << 9), (1 << 9)));
        }
    }
    else if (serdesConfigPtr->busWidth == _20BIT_ON) config->rx_width = config->tx_width = 20;
    else if (serdesConfigPtr->busWidth == _40BIT_ON) config->rx_width = config->tx_width = 40;
    else if (serdesConfigPtr->busWidth == _80BIT_ON) config->rx_width = config->tx_width = 80;

    if (mvAvagoDb)
    {
        config->init_mode = AVAGO_PRBS31_ILB;
        hwsOsPrintf("Note: This is Avago DB configuration (with PRBS)\n");
    }
    else
    {
        config->init_mode = AVAGO_INIT_ONLY;
    }

    AVAGO_DBG(("mvHwsAvagoSerdesPowerCtrlImpl init_configuration:\n"));
    AVAGO_DBG(("   sbus_reset = %x \n",config->sbus_reset));
    AVAGO_DBG(("   init_tx = %x \n",config->init_tx));
    AVAGO_DBG(("   init_rx = %x \n",config->init_rx));
    AVAGO_DBG(("   init_mode = %x \n",config->init_mode));
    AVAGO_DBG(("   tx_divider = 0x%x \n",config->tx_divider));
    AVAGO_DBG(("   rx_divider = 0x%x \n",config->rx_divider));
    AVAGO_DBG(("   tx_width = 0x%x \n",config->tx_width));
    AVAGO_DBG(("   rx_width = 0x%x \n",config->rx_width));
    AVAGO_DBG(("   tx_phase_cal = %x \n",config->tx_phase_cal));
    AVAGO_DBG(("   tx_output_en = %x \n",config->tx_output_en));
    AVAGO_DBG(("   signal_ok_en = %x \n",config->signal_ok_en));
    AVAGO_DBG(("   signal_ok_threshold= %x \n",config->signal_ok_threshold));

    /* Serdes Analog Un Reset*/
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesArr[i], GT_FALSE, GT_TRUE, GT_TRUE));
    }
    /* config media */
    data = (serdesConfigPtr->media == RXAUI_MEDIA) ? (1 << 2) : 0;
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_0, data, (1 << 2)));
    }
#ifndef BV_DEV_SUPPORT
    /* Reference clock source */
    data = ((serdesConfigPtr->refClockSource == PRIMARY) ? 0 : 1) << 8;
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_0, data, (1 << 8)));
    }
#endif
    /* TxClk/RxClk Tlat_en=1. The logic of the Tlat need to be 1 in order to enable Clk to MAC/PCS regardless of Serdes readiness */
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_0, (3 << 13), (3 << 13)));
    }

    mvHwsAvagoAccessLock(devNum);
    for (i = 0; i < numOfSer; i++)
    {
        st = mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[i], &sbus_addr);
        if (st != GT_OK)
        {
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_STATUS(st);
        }
        errors = avago_serdes_init(aaplSerdesDb[devNum], sbus_addr, config);
        if ((errors > 0) || (aaplSerdesDb[devNum]->return_code != 0))
        {
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();
#ifndef MV_HWS_REDUCED_BUILD
            hwsOsPrintf("SerDes init complete for SerDes at addr %s; Errors in ILB: %d. \n", aapl_addr_to_str(sbus_addr), errors);
#else
            hwsOsPrintf("SerDes init complete for SerDes at addr 0x%x; Errors in ILB: %d. \n", sbus_addr, errors);
#endif /* MV_HWS_REDUCED_BUILD */
        }
        else
        {
#ifndef MV_HWS_REDUCED_BUILD
            AVAGO_DBG(("The SerDes at address %s is initialized.\n", aapl_addr_to_str(sbus_addr)));
#else
            AVAGO_DBG(("The SerDes at address 0x%x is initialized.\n", sbus_addr));
#endif /* MV_HWS_REDUCED_BUILD */
        }
    }
    mvHwsAvagoAccessUnlock(devNum);

#if !defined(ASIC_SIMULATION)
    /*
       By default the VCO calibration is operated on Avago Serdes.
       To bypass the VCO calibration for AP port, change the value of
       apPortGroup parameter in mvHwsPortCtrlApEng.c:
       apPortGroup = AVAGO_SERDES_INIT_BYPASS_VCO
    */
    if (portGroup != AVAGO_SERDES_INIT_BYPASS_VCO)
    {
        int     res;
        int     temperature;
        BOOL    mode;  /* True for Tx mode, False for Rx mode */
        int     txTherm, txBin, rxTherm, rxBin;

        /* get the Avago Serdes Temperature (in C) */
        CHECK_STATUS(mvHwsAvagoSerdesTemperatureGet(devNum, portGroup, 0, &temperature));

        mode = GT_TRUE;
        /* get the calibration code(value) for Tx */
        CHECK_STATUS(mvHwsAvagoSerdesCalCodeGet(devNum, portGroup, serdesArr[0], mode, &txTherm, &txBin));

        mode = GT_FALSE;
        /* get the calibration code(value) for Rx */
        CHECK_STATUS(mvHwsAvagoSerdesCalCodeGet(devNum, portGroup, serdesArr[0], mode, &rxTherm, &rxBin));

        /* Compensate the VCO calibration value according to Temperature */
        for (i = 0; i < numOfSer; i++)
        {
            res = mvHwsAvagoSerdesVcoConfig(devNum, portGroup, serdesArr[i], temperature, txTherm, txBin, rxTherm, rxBin);
            if (res != GT_OK)
            {
                hwsOsPrintf("mvHwsAvagoSerdesVcoConfig failed (%d)\n", res);
                return GT_FAIL;
            }
        }
    }
#endif /* !defined(ASIC_SIMULATION) */

    /* Disable TAP1 before Training */
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x26, TAP1_AVG_DISABLE, NULL));
    }

        /* calculate the bitMap index */
    if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesArr[0] >= 20))
    {
        /* get the mac number */
        macOffset = hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasMacNum;
        ctleBitMapIndex = macOffset/AVAGO_CTLE_BITMAP;
        /* update SerDes offset */
        sdOffset = macOffset - (ctleBitMapIndex*AVAGO_CTLE_BITMAP);
    }
    else
    {
        /* calculate CTLE Bias bitMap index */
        ctleBitMapIndex = serdesArr[0]/AVAGO_CTLE_BITMAP;
    }

    if ((((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesArr[i] <20))) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
        || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3)|| (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
    {
        numOfLanes = hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum;
        if ((numOfLanes == 4) || (numOfLanes == 2))
        {
            CHECK_STATUS(mvHwsAvagoSerdesMultiLaneCtleBiasConfig(devNum, serdesArr[0], numOfLanes, ctleBitMapIndex, &ctleBias));
        }
        else
        {
            /* get CTLE Bias value */
            ctleBias = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesArr[0]%AVAGO_CTLE_BITMAP)) & 0x1);
        }
    }
    else if (((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesArr[0] >=20)))
    {
        numOfLanes = hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum;
        if ((numOfLanes == 4) || (numOfLanes == 2))
        {
            CHECK_STATUS(mvHwsAvagoSerdesMultiLaneCtleBiasConfig(devNum, sdOffset, numOfLanes, ctleBitMapIndex, &ctleBias));
        }
        else
        {
            /* get CTLE Bias value */
            ctleBias = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (sdOffset%AVAGO_CTLE_BITMAP)) & 0x1);
        }
    }
     else
    {
        /* get CTLE Bias value */
        ctleBias = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesArr[0]%AVAGO_CTLE_BITMAP)) & 0x1);
    }

    /* change CTLE bias from 1 to 0 */
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x4033, NULL));

        /* read the CTLE bias */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x1A, 0x0, &data));

        if (ctleBias)
        {
            /* set Ctle Bias to 1 */
            data |= 0x1000;
        }
        else
        {
            /* set Ctle Bias to 0 */
            data &= 0xCFFF;
        }
        /* apply CTLE Bias parameter */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, data, NULL));
    }

#if !defined(MV_HWS_AVAGO_NO_VOS_WA) || ( defined(MICRO_INIT) && !defined (BOBK_DEV_SUPPORT) )
    /* Configure serdes VOS parameters*/
    if (hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride)
    {
        /* VOS Override parameters should be configured on ports with speed >= 10G, so filtering is needed.
           Here in serdes level, we can detect the speed by examine the 'mode' and 'baudRate' parameters*/
        /* First, all of the 10G and above speeds have 20bit and 40bit mode */
        if (serdesConfigPtr->busWidth == _20BIT_ON || serdesConfigPtr->busWidth == _40BIT_ON)
        {
            /* Second, some of the speeds less then 10G also have 20bit mode, so baudRate filtering needed */
            if (serdesConfigPtr->baudRate != _5_15625G && serdesConfigPtr->baudRate != _5G)
            {
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
                /* for BC3 only 10G baudRate ports */
                if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
                {
                    if (_10_3125G == serdesConfigPtr->baudRate)
                    {
                        for (i = 0; i < numOfSer; i++)
                        {
                            CHECK_STATUS(mvHwsAvagoSerdesVosDfxParamsSet(devNum,serdesArr[i]));
                        }
                    }
                }
                else
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) */
                {
                    for (i = 0; i < numOfSer; i++)
                    {
                        CHECK_STATUS(mvHwsAvagoSerdesManualVosConfigSet(devNum, serdesArr[i]));
                    }
                }
            }
        }
    }
#endif /* MV_HWS_AVAGO_NO_VOS_WA */

    /* implement SW WA for linkDown problem: release the Spico driver of Serdes */
    if ((serdesConfigPtr->baudRate == _1_25G)    ||
        (serdesConfigPtr->baudRate == _5G)       ||
        (serdesConfigPtr->baudRate == _10_3125G))
    {
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x8000, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0x2, NULL));
        }
    }

#ifdef MV_HWS_FREE_RTOS /* for AP port only */
    if (serdesConfigPtr->baudRate == _3_125G)
    {
        for (i = 0; i < numOfSer; i++)
        {   /* Improve the CDR lock process */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x40FB, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0x1200, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x40A9, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0x1100, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x4001, NULL));
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0x1FF,  NULL));
        }
    }
#endif /* MV_HWS_FREE_RTOS */

    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(hwsAvagoSerdesTxRxTuneParamsSet(devNum, portGroup, serdesArr[i],
                                                        serdesConfigPtr->baudRate));
    }

    /* Serdes Digital UnReset */
    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesArr[i], GT_FALSE, GT_FALSE, GT_FALSE));
    }

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesPowerCtrlImpl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesPowerCtrlImpl
(
    unsigned char               devNum,
    unsigned int                portGroup,
    unsigned int                serdesNum,
    unsigned char               powerUp,
    unsigned int                divider,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
)
{
    Avago_serdes_init_config_t *config;
    Avago_serdes_init_config_t configDef;

    unsigned int sbus_addr;
    unsigned int errors;
    int data;
    GT_UREG_DATA widthMode = 0;
    GT_UREG_DATA data_ureg;
    GT_U32 ctleBias;
    GT_U8 numOfLanes;
    GT_U8 macOffset = 0;
    GT_U8 sdOffset = 0;
    unsigned int ctleBitMapIndex;
#if  !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    GT_U32 i;
#else
    GT_U32  tryCount=0;
#endif

    if(serdesConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

#ifndef MV_HWS_BIN_HEADER
    /* for Serdes PowerDown */
    if (powerUp == GT_FALSE)
    {
#if  !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
        /* if Training is in progress give it chance to finish and turn serdes off anyway */
        for (i=0; i< 25; i++)
        {
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_CORE_STATUS, &data_ureg, 0x2));
            if ((data_ureg != 0))
            {
                ms_sleep(20);
            }
            else
            {
                break;
            }
        }
#else
        do
        {
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_CORE_STATUS, &data_ureg, 0x2));
        }while ((data_ureg != 0) && (tryCount++ < 5));
#endif
        /* turn serdes off */
        mvHwsAvagoAccessLock(devNum);

         /* 0x7 , 0x0
         disable the spico and return its parameters to default. */
        avago_sbus_wr(aaplSerdesDb[devNum], sbus_addr, 0x7, 0x0);

        /* 0x7 , 0x2
         enable the spico which is now at init state (like after loading the spico FW). */
        avago_sbus_wr(aaplSerdesDb[devNum], sbus_addr, 0x7, 0x2);

        avago_serdes_set_tx_rx_enable(aaplSerdesDb[devNum], sbus_addr, GT_FALSE, GT_FALSE, GT_FALSE);
        if (aaplSerdesDb[devNum]->return_code != 0)
        {
            aaplSerdesDb[devNum]->return_code = 0;
        }
        mvHwsAvagoAccessUnlock(devNum);

        /* TxClk/RxClk Tlat_en=0. Return to default */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, 0, (3 << 13)));

        /* Read the saved value from AVAGO_SD_METAL_FIX register to check the Rx/Tx width mode */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX, &data_ureg, 0xFFFF));
        widthMode = (data_ureg >> 9) & 0x1;
        if (widthMode == 1)
        {
            /* Set back the value of AVAGO_SD_METAL_FIX register bit #9 if it was changed in 10BIT_ON mode for GPCS modes */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX, (0 << 9), (1 << 9)));
        }

#ifdef BV_DEV_SUPPORT
        /*disable external PCIE clock*/
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x30, 0x0, NULL));
#endif

        /* Serdes Reset */
        CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesNum, GT_TRUE, GT_TRUE, GT_TRUE));

        return GT_OK;
    }
#endif /* MV_HWS_BIN_HEADER */

    /* for Serdes PowerUp */
    /* Initialize the SerDes slice */
    avago_serdes_init_config(&configDef);
    config = &configDef;

    if(HWS_DEV_SERDES_TYPE(devNum) >= AVAGO_16NM)
    {
        switch(serdesConfigPtr->rxEncoding)
        {
            case SERDES_ENCODING_AVAGO_NRZ:
                config->rx_encoding = AVAGO_SERDES_NRZ;
                break;
            case SERDES_ENCODING_AVAGO_PAM4:
                config->rx_encoding = AVAGO_SERDES_PAM4;
                break;
            default:
                break;
        }

        switch(serdesConfigPtr->txEncoding)
        {
            case SERDES_ENCODING_AVAGO_NRZ:
                config->tx_encoding = AVAGO_SERDES_NRZ;
                break;
            case SERDES_ENCODING_AVAGO_PAM4:
                config->tx_encoding = AVAGO_SERDES_PAM4;
                break;
            default:
                break;
        }
    }

    /* Update tx_divider and rx_divider */
    config->tx_divider = config->rx_divider = divider;

    /* initializes the Avago_serdes_init_config_t struct */
    config->sbus_reset = FALSE;
    config->signal_ok_threshold = 2;

    /* Select the Rx & Tx data path width */
    if (serdesConfigPtr->busWidth == _10BIT_ON)
    {
        config->rx_width = config->tx_width = 10;

        /* Save the width setting for PRBS test in AVAGO_SD_METAL_FIX register.
           It is needed for GPCS modes: the PRBS test can run in these modes only with Rx/Tx width=20BIT.
           If the Rx/Tx width=10BIT then it will be changed to 20BIT in mvHwsAvagoSerdesTestGen function */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX, (1 << 9), (1 << 9)));
    }
    else if (serdesConfigPtr->busWidth == _20BIT_ON) config->rx_width = config->tx_width = 20;
    else if (serdesConfigPtr->busWidth == _40BIT_ON)
    {
        config->rx_width = config->tx_width = 40;
    }

    if (mvAvagoDb)
    {
        config->init_mode = AVAGO_PRBS31_ILB;
        hwsOsPrintf("Note: This is Avago DB configuration (with PRBS)\n");
    }
    else
    {
        config->init_mode = AVAGO_INIT_ONLY;
    }

    AVAGO_DBG(("mvHwsAvagoSerdesPowerCtrlImpl init_configuration:\n"));
    AVAGO_DBG(("   sbus_reset = %x \n",config->sbus_reset));
    AVAGO_DBG(("   init_tx = %x \n",config->init_tx));
    AVAGO_DBG(("   init_rx = %x \n",config->init_rx));
    AVAGO_DBG(("   init_mode = %x \n",config->init_mode));
    AVAGO_DBG(("   tx_divider = 0x%x \n",config->tx_divider));
    AVAGO_DBG(("   rx_divider = 0x%x \n",config->rx_divider));
    AVAGO_DBG(("   tx_width = 0x%x \n",config->tx_width));
    AVAGO_DBG(("   rx_width = 0x%x \n",config->rx_width));
    AVAGO_DBG(("   tx_phase_cal = %x \n",config->tx_phase_cal));
    AVAGO_DBG(("   tx_output_en = %x \n",config->tx_output_en));
    AVAGO_DBG(("   signal_ok_en = %x \n",config->signal_ok_en));
    AVAGO_DBG(("   signal_ok_threshold= %x \n",config->signal_ok_threshold));

    /* Serdes Analog Un Reset*/
    CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesNum, GT_FALSE, GT_TRUE, GT_TRUE));

    /* config media */
    data = (serdesConfigPtr->media == RXAUI_MEDIA) ? (1 << 2) : 0;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, data, (1 << 2)));
#ifndef BV_DEV_SUPPORT
    /* Reference clock source */
    data = ((serdesConfigPtr->refClockSource == PRIMARY) ? 0 : 1) << 8;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, data, (1 << 8)));
#endif
    /* TxClk/RxClk Tlat_en=1. The logic of the Tlat need to be 1 in order to enable Clk to MAC/PCS regardless of Serdes readiness */
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, (3 << 13), (3 << 13)));

    mvHwsAvagoAccessLock(devNum);
    errors = avago_serdes_init(aaplSerdesDb[devNum], sbus_addr, config);
    mvHwsAvagoAccessUnlock(devNum);

    CHECK_AVAGO_RET_CODE();
    if (errors > 0)
    {
#ifndef MV_HWS_REDUCED_BUILD
        hwsOsPrintf("SerDes init complete for SerDes at addr %s; Errors in ILB: %d. \n", aapl_addr_to_str(sbus_addr), errors);
#else
        hwsOsPrintf("SerDes init complete for SerDes at addr 0x%x; Errors in ILB: %d. \n", sbus_addr, errors);
#endif /* MV_HWS_REDUCED_BUILD */
    }
    if (errors == 0 && aapl_get_return_code(aaplSerdesDb[devNum]) == 0)
    {
#ifndef MV_HWS_REDUCED_BUILD
        AVAGO_DBG(("The SerDes at address %s is initialized.\n", aapl_addr_to_str(sbus_addr)));
#else
        AVAGO_DBG(("The SerDes at address 0x%x is initialized.\n", sbus_addr));
#endif /* MV_HWS_REDUCED_BUILD */
    }

#if !defined(ASIC_SIMULATION)
    /*
       By default the VCO calibration is operated on Avago Serdes.
       To bypass the VCO calibration for AP port, change the value of
       apPortGroup parameter in mvHwsPortCtrlApEng.c:
       apPortGroup = AVAGO_SERDES_INIT_BYPASS_VCO
    */
    if (portGroup != AVAGO_SERDES_INIT_BYPASS_VCO)
    {
        int     res;
        int     temperature;
        BOOL    mode;  /* True for Tx mode, False for Rx mode */
        int     txTherm, txBin, rxTherm, rxBin;

        /* get the Avago Serdes Temperature (in C) */
        CHECK_STATUS(mvHwsAvagoSerdesTemperatureGet(devNum, portGroup, 0, &temperature));

        mode = GT_TRUE;
        /* get the calibration code(value) for Tx */
        CHECK_STATUS(mvHwsAvagoSerdesCalCodeGet(devNum, portGroup, serdesNum, mode, &txTherm, &txBin));

        mode = GT_FALSE;
        /* get the calibration code(value) for Rx */
        CHECK_STATUS(mvHwsAvagoSerdesCalCodeGet(devNum, portGroup, serdesNum, mode, &rxTherm, &rxBin));

        /* Compensate the VCO calibration value according to Temperature */
        res = mvHwsAvagoSerdesVcoConfig(devNum, portGroup, serdesNum, temperature, txTherm, txBin, rxTherm, rxBin);
        if (res != GT_OK)
        {
            hwsOsPrintf("mvHwsAvagoSerdesVcoConfig failed (%d)\n", res);
            return GT_FAIL;
        }
    }
#endif /* ASIC_SIMULATION */

    /* Disable TAP1 before Training */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));

    /* calculate the bitMap index */

    if ((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesNum >= 20))
    {
        /* get the mac number */
        macOffset = hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasMacNum;
        ctleBitMapIndex = macOffset/AVAGO_CTLE_BITMAP;
        /* update SerDes offset */
        sdOffset = macOffset - (ctleBitMapIndex*AVAGO_CTLE_BITMAP);
    }
    else
    {
            ctleBitMapIndex = serdesNum/AVAGO_CTLE_BITMAP;
    }

    if ((((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesNum <20))) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
        || (HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2))
    {
        numOfLanes = hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum;
        if ((numOfLanes == 4) || (numOfLanes == 2))
        {
            CHECK_STATUS(mvHwsAvagoSerdesMultiLaneCtleBiasConfig(devNum, serdesNum, numOfLanes, ctleBitMapIndex, &ctleBias));
        }
        else
        {
            /* get CTLE Bias value */
            ctleBias = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesNum%AVAGO_CTLE_BITMAP)) & 0x1);
        }
    }
    else if (((HWS_DEV_SILICON_TYPE(devNum) == BobK) && (serdesNum >=20)))
    {
        numOfLanes = hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasLanesNum;
        if ((numOfLanes == 4) || (numOfLanes == 2))
        {
            CHECK_STATUS(mvHwsAvagoSerdesMultiLaneCtleBiasConfig(devNum, sdOffset, numOfLanes, ctleBitMapIndex, &ctleBias));
        }
        else
        {
            /* get CTLE Bias value */
            ctleBias = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> sdOffset) & 0x1);
        }
    }
     else
    {
        /* get CTLE Bias value */
        ctleBias = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesNum%AVAGO_CTLE_BITMAP)) & 0x1);
    }

    /* change CTLE bias from 1 to 0 */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4033, NULL));

    /* read the CTLE bias */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x1A, 0x0, &data));

    if (ctleBias)
    {
        /* set CTLE Bias to 1 */
        data |= 0x1000;
    }
    else
    {
        /* set CTLE Bias to 0 */
        data &= 0xCFFF;
    }
    /* apply CTLE Bias */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, data, NULL));

#if !defined(MV_HWS_AVAGO_NO_VOS_WA) || ( defined(MICRO_INIT) && !defined (BOBK_DEV_SUPPORT) )
    /* Configure serdes VOS parameters*/
    if (hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride)
    {
        /* VOS Override parameters should be configured on ports with speed >= 10G, so filtering is needed.
           Here in serdes level, we can detect the speed by examine the 'mode' and 'baudRate' parameters*/
        /* First, all of the 10G and above speeds have 20bit and 40bit mode */
        if (serdesConfigPtr->busWidth == _20BIT_ON || serdesConfigPtr->busWidth == _40BIT_ON)
        {
            /* Second, some of the speeds less then 10G also have 20bit mode, so baudRate filtering needed */
            if (serdesConfigPtr->baudRate != _5_15625G && serdesConfigPtr->baudRate != _5G)
            {
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
                /* for BC3 only 10G baudRate ports */
                if ((HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
                {
                    if (_10_3125G == serdesConfigPtr->baudRate)
                    {
                        CHECK_STATUS(mvHwsAvagoSerdesVosDfxParamsSet(devNum, serdesNum));
                    }
                }
                else
#endif /* #if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) */
                {
                    CHECK_STATUS(mvHwsAvagoSerdesManualVosConfigSet(devNum, serdesNum));
                }
            }
        }
    }
#endif /* MV_HWS_AVAGO_NO_VOS_WA */

    /* implement SW WA for linkDown problem: release the Spico driver of Serdes */
    if ((serdesConfigPtr->baudRate == _1_25G)    ||
        (serdesConfigPtr->baudRate == _5G)       ||
        (serdesConfigPtr->baudRate == _10_3125G))
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x8000, NULL));

        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x2, NULL));
    }

#ifdef MV_HWS_FREE_RTOS /* for AP port only */
    if (serdesConfigPtr->baudRate == _3_125G)
    {   /* Improve the CDR lock process */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x40FB, NULL));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x1200, NULL));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x40A9, NULL));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x1100, NULL));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4001, NULL));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x1FF,  NULL));
    }
#endif /* MV_HWS_FREE_RTOS */

    CHECK_STATUS(hwsAvagoSerdesTxRxTuneParamsSet(devNum, portGroup, serdesNum, serdesConfigPtr->baudRate));

    /* Serdes Digital UnReset */
    CHECK_STATUS(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesNum, GT_FALSE, GT_FALSE, GT_FALSE));

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesMultiLaneCtleBiasConfig function
* @endinternal
*
* @brief   Per SERDES set the CTLE Bias value, according to data base.
*
* @param[in] devNum                   - system device number
*                                      serdesNum       - physical serdes number
* @param[in] numOfLanes               - number of lanes
*                                      ctleBitMapIndex - index of ctle bias bit map
*
* @param[out] ctleBiasPtr              - pointer to ctle bias value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesMultiLaneCtleBiasConfig
(
    GT_U8 devNum,
    unsigned int serdesNum,
    GT_U8 numOfLanes,
    unsigned int ctleBitMapIndex,
    GT_U32 *ctleBiasPtr
)
{
    unsigned int serdesOffset;
    serdesNum = serdesNum%AVAGO_CTLE_BITMAP;
    switch (numOfLanes)
    {
    case 2:
        if (serdesNum % 2)
        {
            *ctleBiasPtr = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesNum-1)) & 0x1);
        }
        else
        {
            *ctleBiasPtr = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> serdesNum) & 0x1);
        }
        break;
    case 4:
        serdesOffset = serdesNum % 4;
        if (serdesOffset)
        {
            *ctleBiasPtr = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> (serdesNum-serdesOffset)) & 0x1);
        }
        else
        {
            *ctleBiasPtr = (((hwsDeviceSpecInfo[devNum].serdesInfo.ctleBiasValue[ctleBitMapIndex]) >> serdesNum) & 0x1);
        }
        break;
    }
    return GT_OK;
}

#if !defined(CPSS_BLOB)
/*******************************************************************************
* mvHwsAvagoSerdesAdaptiveStatusGet
*
* DESCRIPTION:
*       Per SERDES check is adaptive pcal is running
*
* INPUTS:
*       devNum    - system device number
*       serdesNum - physical serdes number
*
* OUTPUTS:
*       adaptiveIsRuning - true= adaptive running, false=not running
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsAvagoSerdesAdaptiveStatusGet
(
    unsigned char   devNum,
    unsigned int    serdesNum,
    GT_BOOL         *adaptiveIsRuning
)
{
    int data;

    *adaptiveIsRuning = GT_TRUE;

    /* get DFE status */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, 0, serdesNum, 0x126, ((0 << 12) | (0xB << 8)), &data));
    if((data & 0x83)== 0x80) /* bit 7 = 1 and bits 0,1 =0 */
    {
       *adaptiveIsRuning = GT_FALSE;
    }

    return GT_OK;
}

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
/*******************************************************************************
* mvHwsAvagoSerdesEyeGet
*
* DESCRIPTION:
*       Per SERDES return the adapted tuning results
*       Can be run after create port.
*
* INPUTS:
*       devNum    - system device number
*       serdesNum - physical serdes number
*       eye_inputPtr - pointer to input parameters structure
*
* OUTPUTS:
*       eye_resultsPtr - pointer to results structure
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
unsigned int mvHwsAvagoSerdesEyeGet
(
    unsigned char                       devNum,
    unsigned int                        serdesNum,
    MV_HWS_AVAGO_SERDES_EYE_GET_INPUT  *eye_inputPtr,
    MV_HWS_AVAGO_SERDES_EYE_GET_RESULT *eye_resultsPtr

)
{
    unsigned int sbus_addr;
    int errors, counter;
    Avago_serdes_eye_config_t *configp;
    Avago_serdes_eye_data_t *datap;
    GT_BOOL    adaptiveIsRuning, adaptiveIsOn = GT_FALSE;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));
    configp = avago_serdes_eye_config_construct(aaplSerdesDb[devNum]);
    if (configp == NULL) {
        return GT_BAD_PTR;
    }

    configp->ec_min_dwell_bits = (bigint)eye_inputPtr->min_dwell_bits;
    configp->ec_max_dwell_bits = (bigint)eye_inputPtr->max_dwell_bits;

    datap = avago_serdes_eye_data_construct(aaplSerdesDb[devNum]);
    if (datap == NULL) {
        avago_serdes_eye_config_destruct(aaplSerdesDb[devNum],configp);
        return GT_BAD_PTR;
    }

    if(mvHwsAvagoSerdesAdaptiveStatusGet(devNum,serdesNum, &adaptiveIsRuning)!= GT_OK)
    {
        avago_serdes_eye_data_destruct(aaplSerdesDb[devNum],datap);
        avago_serdes_eye_config_destruct(aaplSerdesDb[devNum],configp);
        return GT_FAIL;
    }
    else
    {
        if (adaptiveIsRuning) { /* adaptive pcal is running */
            adaptiveIsOn = GT_TRUE;
            CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, 0, serdesNum, DFE_STOP_ADAPTIVE, NULL));
            counter = 0;
            while(adaptiveIsRuning)
            {
                CHECK_STATUS(mvHwsAvagoSerdesAdaptiveStatusGet(devNum,serdesNum, &adaptiveIsRuning));
                ms_sleep(20);
                counter++;
                if (counter> 50) {
                    avago_serdes_eye_data_destruct(aaplSerdesDb[devNum],datap);
                    avago_serdes_eye_config_destruct(aaplSerdesDb[devNum],configp);
                    return GT_FAIL;
                }
            }
        }
    }
    mvHwsAvagoAccessLock(devNum);
    errors = avago_serdes_eye_get(aaplSerdesDb[devNum], sbus_addr, configp, datap);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_STATUS(errors);
    CHECK_AVAGO_RET_CODE();

    if (adaptiveIsOn)
    {   /* need to start adaptive again after reading the eye*/
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, 0, serdesNum, DFE_START_ADAPTIVE, NULL));
    }

    eye_resultsPtr->matrixPtr = avago_serdes_eye_plot_format(datap);
    eye_resultsPtr->x_points = datap->ed_x_points;
    eye_resultsPtr->y_points = datap->ed_y_points;
    eye_resultsPtr->height_mv = datap->ed_height_mV;
    eye_resultsPtr->width_mui = datap->ed_width_mUI;
    eye_resultsPtr->vbtcPtr = avago_serdes_eye_vbtc_format(&datap->ed_vbtc);
    eye_resultsPtr->hbtcPtr = avago_serdes_eye_hbtc_format(&datap->ed_hbtc);

    avago_serdes_eye_data_destruct(aaplSerdesDb[devNum],datap);
    avago_serdes_eye_config_destruct(aaplSerdesDb[devNum],configp);

    return GT_OK;
}
#endif /*MV_HWS_REDUCED_BUILD_EXT_CM3*/
#endif /*CPSS_BLOB*/
/**
* @internal mvHwsAvagoSerdesResetImpl function
* @endinternal
*
* @brief   Per SERDES Clear the serdes registers (back to defaults.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesResetImpl
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     analogReset,
    GT_BOOL     digitalReset,
    GT_BOOL     syncEReset
)
{
    GT_U32  data;

    /* SERDES SD RESET/UNRESET init */
    data = (analogReset == GT_TRUE) ? 0 : 1;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 2), (1 << 2)));

    /* SERDES RF RESET/UNRESET init */
    data = (digitalReset == GT_TRUE) ? 0 : 1;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 3), (1 << 3)));

    /* SERDES SYNCE RESET init */
    if(syncEReset == GT_TRUE)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, 0, (1 << 6)));
    }
    else /* SERDES SYNCE UNRESET init */
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, (1 << 6), (1 << 6)));
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, 0xDD00, 0xFF00));
    }

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesPolarityConfigImpl function
* @endinternal
*
* @brief   Per Serdes invert the Tx or Rx.
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesPolarityConfigImpl
(
    unsigned char     devNum,
    unsigned int      portGroup,
    unsigned int      serdesNum,
    unsigned int     invertTx,
    unsigned int     invertRx
)
{
#ifndef ASIC_SIMULATION
    unsigned int sbus_addr;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    mvHwsAvagoAccessLock(devNum);
    /* Tx polarity En */
    avago_serdes_set_tx_invert(aaplSerdesDb[devNum], sbus_addr, invertTx);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

    mvHwsAvagoAccessLock(devNum);
    /* Rx Polarity En */
    avago_serdes_set_rx_invert(aaplSerdesDb[devNum], sbus_addr, invertRx);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

#endif /* ASIC_SIMULATION */
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesPolarityConfigGetImpl function
* @endinternal
*
* @brief   Returns the Tx and Rx SERDES invert state.
*         Can be run after create port.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
int mvHwsAvagoSerdesPolarityConfigGetImpl
(
    unsigned char    devNum,
    unsigned int     portGroup,
    unsigned int     serdesNum,
    unsigned int     *invertTx,
    unsigned int     *invertRx
)
{
#ifndef ASIC_SIMULATION
    unsigned int sbus_addr;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    mvHwsAvagoAccessLock(devNum);
    /* Get the TX inverter polarity mode: TRUE - inverter is enabled, FALSE - data is not being inverted */
    *invertTx = avago_serdes_get_tx_invert(aaplSerdesDb[devNum], sbus_addr);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

    mvHwsAvagoAccessLock(devNum);
    /* Get the RX inverter polarity mode: TRUE - inverter is enabled, FALSE - data is not being inverted */
    *invertRx = avago_serdes_get_rx_invert(aaplSerdesDb[devNum], sbus_addr);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

#endif /* ASIC_SIMULATION */
    return GT_OK;
}

/**
* @internal mvHwsAvagoAccessValidate function
* @endinternal
*
* @brief   Validate access to Avago device
*/
void mvHwsAvagoAccessValidate
(
    unsigned char devNum,
    uint sbus_addr
)
{
    AVAGO_DBG(("Validate SBUS access (sbus_addr 0x%x)- ", sbus_addr));
    if (avago_diag_sbus_rw_test(aaplSerdesDb[devNum], avago_make_sbus_controller_addr(sbus_addr), 2) == TRUE)
    {
        AVAGO_DBG(("Access Verified\n"));
    }
    else
    {
        AVAGO_DBG(("Access Failed\n"));
    }
}

/**
* @internal mvHwsAvagoCheckSerdesAccess function
* @endinternal
*
* @brief   Validate access to Avago Serdes
*/
GT_STATUS mvHwsAvagoCheckSerdesAccess
(
    unsigned int  devNum,
    unsigned char portGroup,
    unsigned char serdesNum
)
{
  GT_UREG_DATA data;

  /* check analog reset */
  CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum,
                                      SERDES_EXTERNAL_CONFIGURATION_1, &data, 0));

  if(((data >> 3) & 1) == 0)
      return GT_NOT_INITIALIZED;

  return GT_OK;
}

/**
* @internal mvHwsAvagoAccessLock function
* @endinternal
*
* @brief   Protection Definition
*         =====================
*         Avago Firmware cannot be accessed by more than one client concurrently
*         Concurrent access might result in invalid data read from the firmware
*         There are three scenarios that require protection.
*         1. Multi-Process Application
*         This case is protected by SW Semaphore
*         SW Semaphore should be defined for each supported OS: FreeRTOS, Linux,
*         and any customer OS
*         This protection is relevant for Service CPU and Host
*         - Service CPU includes multi-process application, therefore protection is required
*         - Host customer application might / might not include multi-process, but from CPSS
*         point of view protection is required
*         2. Multi-Processor Environment
*         This case is protected by HW Semaphore
*         HW Semaphore is defined based in MSYS / CM3 resources
*         In case customer does not use MSYS / CM3 resources,
*         the customer will need to implement its own HW Semaphore
*         This protection is relevant ONLY in case Service SPU Firmware is loaded to CM3
*         3. Debug Capability
*         Avago GUI provides for extensive debug capabilities,
*         But it interface Avago Firmware directly via SBUS commands
*         Therefore No Semaphore protection can be used
*         Debug flag configured by the Host will enable / disable the
*         periodic behavior of AP State machine
*         Debug flag is out of the scope of this API
*/
void mvHwsAvagoAccessLock
(
    unsigned char devNum
)
{
#ifdef MV_HWS_BIN_HEADER

    /* clear warning */
    devNum = devNum;

#else

    /*
    ** SW Semaphore Protection Section
    ** ===============================
    */
#if defined(CHX_FAMILY) || defined(PX_FAMILY)

    /* Host SW Protection */
    hwsOsMutexLock(avagoAccessMutex);

#elif defined(MV_HWS_FREE_RTOS)

    /* Service CPU SW Protection */
    mvPortCtrlAvagoLock();

#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
    /* When running on any Service CPU, HW semaphore always used */
    mvSemaLock(devNum, MV_SEMA_AVAGO);
#else
    /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
    if ( mvHwsServiceCpuEnableGet(devNum) ) {
        mvSemaLock(devNum, MV_SEMA_AVAGO);
    }
#endif

#endif /* MV_HWS_BIN_HEADER */
}

/**
* @internal mvHwsAvagoAccessUnlock function
* @endinternal
*
* @brief   See description in mvHwsAvagoAccessLock API
*/
void mvHwsAvagoAccessUnlock
(
    unsigned char devNum
)
{
#ifdef MV_HWS_BIN_HEADER

    /* clear warning */
    devNum = devNum;

#else

    /*
    ** SW Semaphore Protection Section
    ** ===============================
    */
#if defined(CHX_FAMILY) || defined(PX_FAMILY)

    /* Host SW Protection */
    hwsOsMutexUnlock(avagoAccessMutex);

#elif defined(MV_HWS_FREE_RTOS)

    /* Service CPU SW Protection */
    mvPortCtrlAvagoUnlock();

#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
    /* When running on any Service CPU, HW semaphore always used */
    mvSemaUnlock(devNum, MV_SEMA_AVAGO);
#else
    /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
    if ( mvHwsServiceCpuEnableGet(devNum) ) {
        mvSemaUnlock(devNum, MV_SEMA_AVAGO);
    }
#endif

#endif /* MV_HWS_BIN_HEADER */
}

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MICRO_INIT) && defined(PIPE_DEV_SUPPORT)) || (defined(MICRO_INIT) && defined(BC3_DEV_SUPPORT))
#define MAX_HDFUSE_LINES 64
/**
* @internal mvHwsAvagoSerdesBc3VosConfig function
* @endinternal
*
* @brief   Reading the HDFuse and locating the AVG block where the compressed VOS
*         data parameters are.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/

GT_STATUS mvHwsAvagoSerdesBc3VosConfig(unsigned char devNum)
{
    GT_U32 i,j; /* iterrators */
    char* src_str = "0000";
    GT_BOOL isBurn = GT_FALSE; /* identify wheather the HDFuse line is burned */
    GT_U32 numOfFuseLines = 0, lineOffset = 0; /* AVG block data lines number */
    GT_U32 dfxVosWord1 = 0, dfxVosWord2 = 0, dfxVosWord3 = 0; /* each HDFuse line contains 3 words */
    GT_U32 numOfSerdes;
    MV_HWS_AVG_BLOCK avgBlockData; /* a structure that holds nessaccary parameters for decoding */
    GT_U32 avgBlockStart = 0;
    GT_U32 minHdEfuseLines; /* minimum lines of AVG data block */
    GT_U32 secondaryEfuseData = 0;
    GT_CHAR allSdsDataStr[MAX_AVG_SIZE] = {0};

    if (HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        numOfSerdes = 16;
        minHdEfuseLines = 8;
    }
    else /* Bobcat3 and Aldrin2 */
    {
        numOfSerdes = 72;
        minHdEfuseLines = 28;
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, SERVER_SECONDARY_EFUSE, &secondaryEfuseData));
        /* Select Secondary Efuse server */
        secondaryEfuseData |= (0x1 << 17);
        CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, SERVER_SECONDARY_EFUSE, secondaryEfuseData));
    }

    /* reading the HDFuse block and finding the address of first data line (AVG block address) */
    for (i=0; i< MAX_HDFUSE_LINES; i++)
    {
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, SERVER_HD_EFUSE_SLAVE_REG+lineOffset, &dfxVosWord1));
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, SERVER_HD_EFUSE_WORD_2+lineOffset, &dfxVosWord2));
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, SERVER_HD_EFUSE_WORD_3+lineOffset, &dfxVosWord3));
        lineOffset += 0x10;
        if ((dfxVosWord1 !=0 && dfxVosWord3 != 0) || (dfxVosWord2 != 0 && dfxVosWord3 !=0))
        {
            isBurn = GT_TRUE;
            numOfFuseLines++;
            avgBlockStart = ((SERVER_HD_EFUSE_SLAVE_REG+lineOffset)-0x10);
        }
        else
        {
            if ((isBurn) && numOfFuseLines >= minHdEfuseLines)
            {
                break;
            }
            else
            {
                numOfFuseLines = 0;
                continue;
            }
        }
    }

    if (numOfFuseLines < minHdEfuseLines)
    {
        return GT_OK;
    }


    /* extract AVG params (30b of first word)*/
    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, avgBlockStart, &dfxVosWord1));
    avgBlockData.revision = (dfxVosWord1 & 0x3);
    avgBlockData.avgBlockLines = (dfxVosWord1 >> 2) & 0x3F;
    avgBlockData.minVal = (dfxVosWord1 >> 8) & 0xFF;
    avgBlockData.baseVal = (dfxVosWord1 >> 16) & 0xFF;
    avgBlockData.bitsPerSds = (dfxVosWord1 >> 24) & 0x3F;

    if (avgBlockData.avgBlockLines != numOfFuseLines)
    {
        return GT_OK;
    }

    /* defining bits per word according to the revision parameter */

    avgBlockData.dataBitsWord1 = 32;
    avgBlockData.dataBitsWord2 = 24;

    lineOffset = 0;
    i = 0;

    hwsOsMemSetFuncPtr(&allSdsDataStr[0], 0, sizeof(allSdsDataStr));
    /* reads 56 or 55 (according to revision) bits of each AVG line and set them in a  binary string
      which contains the whole data */
    while (i < (2*avgBlockData.avgBlockLines))
    {
        if (i % 2 == 0)
        {
            for (j=0; j<avgBlockData.dataBitsWord1; j=j+4)
            {
               CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, avgBlockStart-lineOffset, &dfxVosWord1));
                dfxVosWord1 = (dfxVosWord1 >> j) & 0xF;
                src_str = hex2bin(dfxVosWord1);
                hwsOsStrCatFuncPtr(allSdsDataStr,src_str);

            }
            i++;
        }
        else
        {
            for (j=0; j<avgBlockData.dataBitsWord2; j=j+4)
            {
                CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, avgBlockStart-lineOffset+0x4, &dfxVosWord2));
                dfxVosWord2 = (dfxVosWord2 >> j) & 0xF;
                src_str = hex2bin(dfxVosWord2);
                hwsOsStrCatFuncPtr(allSdsDataStr,src_str);
            }
            if (avgBlockData.revision)
            {
                    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, avgBlockStart-lineOffset+0x4, &dfxVosWord2));
                    dfxVosWord2 = (dfxVosWord2 >> 24) & (0x1);
                    if (dfxVosWord2)
                    {
                        hwsOsStrCatFuncPtr(allSdsDataStr,"1");
                    }
                    else
                    {
                        hwsOsStrCatFuncPtr(allSdsDataStr,"0");
                    }
            }
            lineOffset += 0x10;
            i++;
        }
    }

    minVosValue = avgBlockData.minVal;
    baseVosValue = avgBlockData.baseVal;

    for (i =0; i<numOfSerdes; i++)
    {

        /* get the 7 vos values word for each serdes */
        serdesVosParams64BitDB[i] = mvHwsAvagoSerdesCalcVosDfxParams(devNum,i, avgBlockData.bitsPerSds, allSdsDataStr);
    }

    serdesVos64BitDbInitialized = GT_TRUE;
    /*hdeFuseBc3WaEnable = GT_TRUE;*/

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesCalcVosDfxParams function
* @endinternal
*
* @brief   Calculates each 7 VOS values per serdes.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @param[in] serdesNum                - serdes number.
* @param[in] bitsPerSds               - number of bits to hold 7 VOS values.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_UL64 mvHwsAvagoSerdesCalcVosDfxParams
(
    unsigned char devNum,
    GT_U32 serdesNum,
    GT_U32 bitsPerSds,
    GT_CHAR dataStr[]
)
{
    GT_UL64 tempVal1 =0;
    char tempVal2,sds_word[63]={0};
    GT_U32 i,j=0; /* itterators */
    GT_U32 numOfSdsIndex;
    static GT_U32 nextWord = 0;

    if (HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        numOfSdsIndex = 15;
    }
    else /* Bobcat3 and Aldrin2 */
    {
        numOfSdsIndex = 71;
    }

    for (i=0; i< bitsPerSds; i++)
    {
        sds_word[i] = dataStr[30+i+nextWord];
        if (i == (bitsPerSds-1))
        {
            for (j=0; j< bitsPerSds/2; j++)
            {
                tempVal2 = sds_word[j];
                sds_word[j] = sds_word[bitsPerSds-j-1];
                sds_word[bitsPerSds-j-1] = tempVal2;
            }
            j=0;
            while (j < bitsPerSds)
            {
                tempVal1 *= 2;
                if (sds_word[j] == '1')
                {
                    tempVal1 += 1;
                }
                j++;
            }
        }
    }
    nextWord += bitsPerSds;

    if (serdesNum == numOfSdsIndex)
    {
        nextWord = 0;
    }

    return tempVal1;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 || defined(MICRO_INIT) */

/**
* @internal mvHwsAvagoSerdesVosDfxParamsSet function
* @endinternal
*
* @brief   Decoding the VOS data to each sampler of the serdes and
*         applying VOS correction parameters from HDFuze to improve eye openning.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon.
* @param[in] devNum                   - system device number.
* @param[in] serdesNum                - serdes number.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
#if defined(PIPE_DEV_SUPPORT) ||  defined(ALDRIN2_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
GT_STATUS mvHwsAvagoSerdesVosDfxParamsSet
(
    GT_U32 devNum,
    GT_U32 serdesNum
)
{
    GT_UL64 sds_values, samplersData;
    GT_U32 i; /* itterator */
    GT_U32 delta;
    GT_U32 serdesVosValue;
    GT_U32 maxVosSerdes;

    if (!serdesVos64BitDbInitialized)
    {
        CPSS_LOG_INFORMATION_MAC("Not performing VOS config on serdes %d ", serdesNum);
        return GT_OK;
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        maxVosSerdes = 15;
    }
    else /* Bobcat3 and Aldrin2 */
    {
        maxVosSerdes = 71;
    }

    if (serdesNum > maxVosSerdes)
    {
        CPSS_LOG_INFORMATION_MAC("eFuse VOS params exceed SerDes index limit");
        return GT_OK;
    }
    else
    {
        samplersData = serdesVosParams64BitDB[serdesNum];
    }

    for (i=0; i< 7; i++)
    {
        delta = samplersData % baseVosValue;
        sds_values = delta+minVosValue;
        /* Calculating the generic formula */
        serdesVosValue = ( 0x1900 + (i * 0x100) );
        /* Applying final VOS value to serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, 0, serdesNum, 0x26, serdesVosValue+(sds_values & 0xFF), NULL));
        samplersData = (samplersData-delta)/ baseVosValue;
    }

    return GT_OK;
}
#endif /* #if defined(PIPE_DEV_SUPPORT) ||  defined(ALDRIN2_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) */

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || (defined(MICRO_INIT) && defined(PIPE_DEV_SUPPORT)) || (defined(MICRO_INIT) && defined(BC3_DEV_SUPPORT))
char* hex2bin(GT_U32 val)
{
    char *binVal = "0000";
    switch (val)
    {
    case 0x0:
        binVal = "0000";
        break;
    case 0x1:
        binVal = "1000";
        break;
    case 0x2:
        binVal = "0100";
        break;
    case 0x3:
        binVal = "1100";
        break;
    case 0x4:
        binVal = "0010";
        break;
    case 0x5:
        binVal = "1010";
        break;
    case 0x6:
        binVal = "0110";
        break;
    case 0x7:
        binVal = "1110";
        break;
    case 0x8:
        binVal = "0001";
        break;
    case 0x9:
        binVal = "1001";
        break;
    case 0xa:
        binVal = "0101";
        break;
    case 0xb:
        binVal = "1101";
        break;
    case 0xc:
        binVal = "0011";
        break;
    case 0xd:
        binVal = "1011";
        break;
    case 0xe:
        binVal = "0111";
        break;
    case 0xf:
        binVal = "1111";
        break;
    default:
        return binVal;
    }
    return binVal;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

/*******************************************************************************/
/******************************* DEBUG functions *******************************/
/*******************************************************************************/

#ifndef MV_HWS_FREE_RTOS
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpc.h>

#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG
#define POSITIVE_NEGATIVE_VALUE_DBG(_val) \
    (((_val & 0x10))?                                  \
     ( (GT_8)255 - (GT_8)(((_val & 0xF) *2) - 1) ) :    \
     (_val)*2)
#endif /* MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG */

/* API in order to read tx offset parameters database from FW memory */
extern GT_UINTPTR hwsIpcChannelHandlers[HWS_MAX_DEVICE_NUM];

/* parameters in order to read tx offset parameters database from FW memory */
GT_U32 txParamesOffsetsFwBaseAddr;
GT_U32 txParamesOffsetsFwSize;

#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG
/**
* @internal mvHwsAvagoSerdesTxOffsetParamsDbgTest function
* @endinternal
*
* @brief   This API tests the txOffsets database, it's set API and the affect on the
*         serdes TX parameters which should match the values under test.
*         Note: This API needs to be compiled with the database and the set API.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; Caelum; Aldrin.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxOffsetParamsDbgTest
(
    GT_U32 v0,
    GT_U32 v1,
    GT_U32 v2,
    GT_U32 v3
)
{
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  configParams;
    GT_STATUS rc;
#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A
    GT_U16 val1, val2, val3;
#else
    GT_U16 val0;
#endif
    GT_U8 speedIndex;
    GT_U32 portNum;
    GT_U32 serdesNum;
    GT_U8 i,j;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    speedIndex = 0;

    for (portNum=0; portNum<72; portNum++)
    {
        if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, portNum, _10GBase_KR, &curPortParams))
        {
            continue;
        }

        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            serdesNum = curPortParams.activeLanesList[i];
#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A
            val1 = v1/*9*/;
            val2 = v2/*9*/;
            val3 = v3/*9*/;
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, val1, 0);
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, val2, 1);
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, val3, 2);
#else
            val0 = v0 /*2457*/;
            /*settt2(serdesNum, speedIndex, 2457);*/
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, ((val0)&0x1F), 0);
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, ((val0>>5)&0x1F), 1);
            mvHwsAvagoSerdesTxOffsetParamsSet(serdesNum, speedIndex, ((val0>>10)&0x1F), 2);
#endif
            rc = hwsAvagoSerdesTxRxTuneParamsSet(0,0,serdesNum,curPortParams.serdesSpeed);
            if (rc!= GT_OK)
            {
                hwsOsPrintf("\n hwsAvagoSerdesTxRxTuneParamsSet failed=%d \n",rc);
            }

            rc = mvHwsAvagoSerdesManualTxConfigGet(0,0,serdesNum,&configParams);
            if (rc!= GT_OK)
            {
                hwsOsPrintf("\n mvHwsAvagoSerdesManualTxConfigGet failed=%d \n",rc);
            }

#ifdef MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_IMPLEMENTATION_A
            for (j = SPEED_NA; j < LAST_MV_HWS_SERDES_SPEED; j++)
            {
                if (hwsAvagoSerdesTxRxTuneParamsArray[j].serdesSpeed == curPortParams.serdesSpeed)
                {
                    hwsOsPrintf("\nperforming...\n");
                    if ( ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txAmp+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG(val1)) !=
                            (GT_32)configParams.txAmp ||
                         ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txEmph0+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG(val2)) !=
                            (GT_32)configParams.txEmph0 ||
                         ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txEmph1+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG(val3)) !=
                            (GT_32)configParams.txEmph1 )
                    {
                        hwsOsPrintf("\n Not match!!  %d, %d, %d, %d, %d, %d \n",
                                 val1, val2, val3,
                                 (GT_32)configParams.txAmp,
                                 (GT_32)configParams.txEmph0,
                                 (GT_32)configParams.txEmph1);
                    }
                    break;
                }
            }
#else
            for (j = SPEED_NA; j < LAST_MV_HWS_SERDES_SPEED; j++)
            {
                if (hwsAvagoSerdesTxRxTuneParamsArray[j].serdesSpeed == curPortParams.serdesSpeed)
                {
                    hwsOsPrintf("\nperforming...\n");
                    if ( ( ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txAmp+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG(val0&0x1F)) !=
                                (GT_32)configParams.txAmp ) ||
                         ( ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txEmph0+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG((val0>>5)&0x1F)) !=
                                (GT_32)configParams.txEmph0 ) ||
                         ( ((GT_32)hwsAvagoSerdesTxRxTuneParamsArray[j].txParams.txEmph1+(GT_32)POSITIVE_NEGATIVE_VALUE_DBG((val0>>10)&0x1F)) !=
                                (GT_32)configParams.txEmph1 ) )
                    {
                        hwsOsPrintf("\n Not match!!  %d, %d, %d, %d, %d, %d \n",
                                 (val0&0x1F), ((val0>>5)&0x1F), ((val0>>10)&0x1F),
                                 (GT_32)configParams.txAmp,
                                 (GT_32)configParams.txEmph0,
                                 (GT_32)configParams.txEmph1);
                    }
                    break;
                }
            }
#endif
        }
    }
    return GT_OK;

}
#endif /* MV_HWS_AVAGO_TX_OFFSET_PARAMETERS_HOST_CPU_DEBUG */

/**
* @internal mvHwsAvagoSerdesTxOffsetParamsDbgDumpArray function
* @endinternal
*
* @brief   This API dumps the content of the database from the FW memory.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; Caelum; Aldrin.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxOffsetParamsDbgDumpArray(GT_U32 addr)
{
    GT_U32 i;
    GT_U8 *dataArrPtr;

    if (txParamesOffsetsFwBaseAddr==0 || txParamesOffsetsFwSize==0)
    {
        hwsOsPrintf("\n null pointer or zero size %d, %d", txParamesOffsetsFwBaseAddr, txParamesOffsetsFwSize);
        return GT_FAIL;
    }

    dataArrPtr = (GT_U8 *)hwsOsMallocFuncPtr(txParamesOffsetsFwSize);
    hwsOsPrintf("\n reading offset 0x%08x, size=%d", ((addr == 0) ? txParamesOffsetsFwBaseAddr : addr), txParamesOffsetsFwSize);
    CHECK_STATUS_EXT(prvCpssGenericSrvCpuRead(hwsIpcChannelHandlers[0], ((addr == 0) ? txParamesOffsetsFwBaseAddr : addr), dataArrPtr, txParamesOffsetsFwSize),
                     LOG_ARG_STRING_MAC("reading tx offsets db from FW memory"));

    hwsOsPrintf("\n\n");
    for (i=0; i<txParamesOffsetsFwSize; i++)
    {
        if (dataArrPtr[i]!=0)
        {
            hwsOsPrintf("arr[%d]={** %d **}", i, dataArrPtr[i]);
        }
        else
        {
            hwsOsPrintf("arr[%d]={%d}", i, dataArrPtr[i]);
        }
        if (i%20==0)
        {
            hwsOsPrintf("\n");
        }
    }
    hwsOsPrintf("\n\n");

    hwsOsFreeFuncPtr(dataArrPtr);

    return GT_OK;
}

#endif /* MV_HWS_FREE_RTOS */

GT_STATUS dbgSpicoInterrupt(GT_U32 serdesNum, unsigned int interNum, unsigned int interData)
{
    int data;

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(0, 0, serdesNum, interNum, interData,  &data));
    printf("Data 0x%08x ", data);
    return GT_OK;
}

GT_STATUS dbgSbusReset(GT_U32 serdesNum)
{
        unsigned int  sbus_addr;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(0, serdesNum, &sbus_addr));
        mvHwsAvagoAccessLock(0);
        avago_sbus_wr(aaplSerdesDb[0], sbus_addr, 0x7, 0x0);
        avago_sbus_wr(aaplSerdesDb[0], sbus_addr, 0x07, 0x2);
        mvHwsAvagoAccessUnlock(0);

        return GT_OK;
}
