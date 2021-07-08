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
* @file mvAvagoSerdesIf.c
*
* @brief Avago interface
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>

#ifdef _VISUALC
#pragma warning(disable : 4100)
/*relate to :FORCE_FUNC_CAST */
#pragma warning(disable : 4054) /* warning C4054: from function pointer 'GT_STATUS (__cdecl *)(void)' to data pointer 'void *' */
/*relate to :FORCE_FUNC_CAST */
#pragma warning(disable : 4152) /* warning C4152: nonstandard extension, function/data pointer conversion in expression */
#endif

#ifndef ASIC_SIMULATION
/* Avago include */
#include <aapl/marvell/mv_hws_avago_if.h>
#include <aapl.h>
/* too much work to add header files for these functions */
extern BOOL avago_serdes_error_reset(Aapl_t *aapl, uint addr);
extern BOOL avago_serdes_get_signal_ok_live(Aapl_t *aapl, uint addr);
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#ifdef FW_DOWNLOAD_FROM_SERVER
#include <aapl/marvell/sd28firmware/avago_fw_load.h>
#endif
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
#endif /* ASIC_SIMULATION */

#ifndef MARVELL_AVAGO_NO_DEBUG_FLAG
#define HWS_AVAGO_DBG(s) hwsOsPrintf s
#else
#define HWS_AVAGO_DBG(s)
#endif

#ifdef MARVELL_AVAGO_DEBUG_FLAG
#define AVAGO_DBG(s) osPrintf s
#else
#define AVAGO_DBG(s)
#endif /* MARVELL_AVAGO_DEBUG_FLAG */

/* define a macro to calculate the execution time */
#if !defined(CPSS_BLOB) && !defined(MV_HWS_FREE_RTOS)
    #define EXEC_TIME_START() \
        GT_U32 start_Seconds, start_NanoSeconds; \
        hwsTimeRT(&start_Seconds, &start_NanoSeconds);
    #define EXEC_TIME_END_PRINT(_msg) \
        { \
            GT_U32 end_Seconds, end_NanoSeconds; \
            hwsTimeRT(&end_Seconds, &end_NanoSeconds); \
            if (end_NanoSeconds < start_NanoSeconds) \
            { \
                end_NanoSeconds += 1000000000; \
                end_Seconds--; \
            } \
            HWS_AVAGO_DBG((" Time is: %d.%09d seconds\n", (end_Seconds-start_Seconds), (end_NanoSeconds-start_NanoSeconds))); \
        }
#else
    #define EXEC_TIME_START()
    #define EXEC_TIME_END_PRINT(_msg)
#endif

/* #define TUNE_BY_PHASE_DBG */
#ifdef TUNE_BY_PHASE_DBG
#define AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(...)  \
    osPrintf(__VA_ARGS__)
#else
#define AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(...)
#endif

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <cpssCommon/private/prvCpssEmulatorMode.h>
/* return 0 when Avago     supported
   return 1 when Avago NOT supported
*/
GT_U32  hwsAvagoNotSupportedCheck(void)
{
    /* the emulator not supports the memories of <SERDES And SBC Controller> */
    return cpssDeviceRunCheck_onEmulator();
}
/* function that do nothing and return GT_OK */
/* it should allow to avoid any access to AVAGO driver code */
static  GT_STATUS hwsAvagoNotSupported_dummySet(void)
{
    return GT_OK;
}
/* macro to force casting between 2 functions prototypes */
#define FORCE_FUNC_CAST (void*)
/* macro to bind CB function (for 'Set' functions ... without 'OUT' parameters)*/
#define BIND_SET_FUNC(func)                                 \
    hwsAvagoNotSupportedCheck() ?                           \
        FORCE_FUNC_CAST hwsAvagoNotSupported_dummySet :     \
        func

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesLoopbackGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_SERDES_LB  *lbModePtr
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    *lbModePtr = SERDES_LP_DISABLE;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesTestGenGet
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    MV_HWS_SERDES_TX_PATTERN    *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    txPatternPtr = txPatternPtr;
    modePtr = modePtr;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesTestGenStatus
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    MV_HWS_SERDES_TX_PATTERN        txPattern,
    GT_BOOL                         counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS   *status
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    txPattern = txPattern;
    counterAccumulateMode = counterAccumulateMode;
    hwsOsMemSetFuncPtr(status,0,sizeof(*status));
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesPolarityConfigGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *invertTx,
    GT_BOOL           *invertRx
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    invertTx = invertTx;
    invertRx = invertRx;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesTxEnableGet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     *enablePtr
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    enablePtr = enablePtr;

    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesTxIfSelectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_U32      *serdesTxIfNum
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    *serdesTxIfNum = 0;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdeSpeedGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                serdesNum,
    MV_HWS_SERDES_SPEED   *rate
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    rate = rate;
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesManualTxConfigGet
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         serdesNum,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  *configParams
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    hwsOsMemSetFuncPtr(configParams,0,sizeof(*configParams));
    return GT_OK;
}

static GT_STATUS hwsAvagoNotSupported_mvHwsAvagoSerdesSignalDetectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalDet
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    signalDet = signalDet;
    return GT_OK;
}

/* macro to bind CB function (for 'Get' functions ... with 'OUT' parameters)*/
/* the macro requires function with same name as 'orig' but with prefix 'hwsAvagoNotSupported_' */
#define BIND_GET_FUNC(func)               \
    hwsAvagoNotSupportedCheck() ?         \
        hwsAvagoNotSupported_##func :     \
        func

#define RETURN_GT_OK_ON_EMULATOR       \
    if(hwsAvagoNotSupportedCheck())    \
    {                                  \
        /* no SERDESes on emulator */  \
        return GT_OK;                  \
    }


#else /*defined MV_HWS_REDUCED_BUILD_EXT_CM3 */
#define BIND_SET_FUNC(func)               \
        func

#define BIND_GET_FUNC(func)               \
        func

#define RETURN_GT_OK_ON_EMULATOR       /* empty */

#endif /*defined MV_HWS_REDUCED_BUILD_EXT_CM3 */

/************************* definition *****************************************************/
#define AVAGO_AAPL_LGPL
/*#define MARVELL_AVAGO_DEBUG_FLAG*/

#define MV_AVAGO_MIN_EYE_SIZE           10
#define MV_AVAGO_EYE_TAP                4

#define ICAL_TRAINING_MODE              0
#define PCAL_TRAINING_MODE              1

#define SKIP_SERDES                     0xFFFF

#define MV_AVAGO_TX_EQUALIZE_PASS       (0x54)

/************************* Globals *******************************************************/
#ifndef ASIC_SIMULATION
#ifndef BV_DEV_SUPPORT

/* For non-AP port: delay values for EnhanceTune and EnhanceTuneLite configuration */
GT_U32  static_delay_10G[] = {25, 26, 28, 30}; /* best delay values for 10G speed */
GT_U32  static_delay_25G[] = {15, 17, 19, 21}; /* best delay values for 25G speed */

/* Dividers DB for ref clock 156.25Mhz */
GT_U8 avagoBaudRate2DividerValue_156M[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    8,          /*  _1_25G    */
    20,         /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    24,         /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    32,         /*  _5G       */
    40,         /*  _6_25G    */
    48,         /*  _7_5G     */
    66,         /*  _10_3125G */
    72,         /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    80,         /*  _12_5G    */
    70,         /*  _10_9375G */
    78,         /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    NA_VALUE,   /*  _5_15625G */
    NA_VALUE,   /*  _12_8906G */
    132,        /*  _20_625G  */
    165,        /*  _25_78125G*/
    176,        /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    170,        /*  _26_5625G */
    181,        /*  _28_28125G*/
    170,        /*  _26_5625G_PAM4 */
    181,        /*  _28_28125G_PAM4*/
    168,        /* _26_25G         */
    NA_VALUE,   /* _27_1G          */
};

/* Dividers DB for ref clock 78Mhz */
GT_U8 avagoBaudRate2DividerValue_78M[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    16,         /*  _1_25G    */
    40,         /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    48,         /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    64,         /*  _5G       */
    80,         /*  _6_25G    */
    96,         /*  _7_5G     */
    132,        /*  _10_3125G */
    NA_VALUE,   /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    160,        /*  _12_5G    */
    NA_VALUE,   /*  _10_9375G */
    156,        /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    66,         /*  _5_15625G */
    165,        /*  _12_8906G */
    NA_VALUE,   /*  _20_625G  */
    NA_VALUE,   /*  _25_78125G*/
    NA_VALUE,   /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    NA_VALUE,   /*  _26_5625G */
    NA_VALUE,   /*  _28_28125G*/
    NA_VALUE,   /* _26_5625G_PAM4  */
    NA_VALUE,   /* _28_28125G_PAM4 */
    NA_VALUE,   /* _26_25G         */
    NA_VALUE,   /* _27_1G          */
};

/* Dividers DB for ref clock 312.5Mhz */
GT_U8 avagoBaudRate2DividerValue_312_5M[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    NA_VALUE,   /*  _1_25G    */
    NA_VALUE,   /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    NA_VALUE,   /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    NA_VALUE,   /*  _5G       */
    NA_VALUE,   /*  _6_25G    */
    NA_VALUE,   /*  _7_5G     */
    NA_VALUE,   /*  _10_3125G */
    NA_VALUE,   /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    NA_VALUE,   /*  _12_5G    */
    NA_VALUE,   /*  _10_9375G */
    NA_VALUE,   /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    NA_VALUE,   /*  _5_15625G */
    NA_VALUE,   /*  _12_8906G */
    NA_VALUE,   /*  _20_625G  */
    NA_VALUE,   /*  _25_78125G*/
    88,         /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    NA_VALUE,   /*  _26_5625G */
    NA_VALUE,   /*  _28_28125G*/
    NA_VALUE,   /* _26_5625G_PAM4  */
    NA_VALUE,   /* _28_28125G_PAM4 */
    NA_VALUE,   /* _26_25G         */
    NA_VALUE,   /* _27_1G          */
};

/* Dividers DB for ref clock 312.5Mhz */
GT_U8 avagoBaudRate2DividerValue_164_24M[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    NA_VALUE,   /*  _1_25G    */
    NA_VALUE,   /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    NA_VALUE,   /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    NA_VALUE,   /*  _5G       */
    NA_VALUE,   /*  _6_25G    */
    NA_VALUE,   /*  _7_5G     */
    NA_VALUE,   /*  _10_3125G */
    NA_VALUE,   /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    NA_VALUE,   /*  _12_5G    */
    NA_VALUE,   /*  _10_9375G */
    NA_VALUE,   /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    NA_VALUE,   /*  _5_15625G */
    NA_VALUE,   /*  _12_8906G */
    NA_VALUE,   /*  _20_625G  */
    NA_VALUE,   /*  _25_78125G*/
    NA_VALUE,   /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    NA_VALUE,   /*  _26_5625G */
    NA_VALUE,   /*  _28_28125G*/
    NA_VALUE,   /* _26_5625G_PAM4  */
    NA_VALUE,   /* _28_28125G_PAM4 */
    NA_VALUE,   /* _26_25G         */
    165,        /* _27_1G          */
};

GT_U8 avagoBaudRate2DwellTimeValue[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    8,          /*  _1_25G    */
    8,          /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    8,          /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    8,          /*  _5G       */
    9,          /*  _6_25G    */
    10,         /*  _7_5G     */
    11,         /*  _10_3125G */
    11,         /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    12,         /*  _12_5G    */
    12,         /*  _10_9375G */
    12,         /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    12,         /*  _5_15625G */
    12,         /*  _12_8906G */
    NA_VALUE,   /*  _20_625G  */
    13,         /*  _25_78125G*/
    13,         /*  _27_5G    */
    NA_VALUE,   /*  _28_05G   */
    13,         /*  _26_5625G */
    13,         /*  _28_28125G*/
    13,         /*  _26_5625G_PAM4 */
    13,         /*  _28_28125G_PAM4*/
    13,         /*  _26_25G   */
    13          /* _27_1G     */
};
#else
static GT_U8 mcdBaudRate2DividerValue[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    8,          /*  _1_25G    */
    20,         /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    NA_VALUE,   /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    32,         /*  _5G       */
    40,         /*  _6_25G    */
    NA_VALUE,   /*  _7_5G     */
    66,         /*  _10_3125G */
    NA_VALUE,   /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    80,         /*  _12_5G    */
    70,         /*  _10_9375G */
    78,         /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    NA_VALUE,   /*  _5_15625G */
    NA_VALUE,   /*  _12_8906G */
    132,        /*  _20_625G  */
    165,        /*  _25_78125G*/
    NA_VALUE,   /*  _27_5G    */
    180,        /*  _28_05G   */
    NA_VALUE,   /*  _26_5625G */
    NA_VALUE,   /*  _28_28125G*/
    NA_VALUE,   /*  _26_5625G_PAM4 */
    NA_VALUE,   /*  _28_28125G_PAM4*/
    NA_VALUE,   /*  _26_25G   */
    NA_VALUE,   /* _27_1G     */
};

static GT_U8 mcdBaudRate2DwellTimeValue[LAST_MV_HWS_SERDES_SPEED]=
{
    NA_VALUE,   /*  SPEED_NA  */
    8,          /*  _1_25G    */
    8,          /*  _3_125G   */
    NA_VALUE,   /*  _3_33G    */
    NA_VALUE,   /*  _3_75G    */
    NA_VALUE,   /*  _4_25G    */
    8,          /*  _5G       */
    9,          /*  _6_25G    */
    NA_VALUE,   /*  _7_5G     */
    11,         /*  _10_3125G */
    NA_VALUE,   /*  _11_25G   */
    NA_VALUE,   /*  _11_5625G */
    11,         /*  _12_5G    */
    11,         /*  _10_9375G */
    11,         /*  _12_1875G */
    NA_VALUE,   /*  _5_625G   */
    NA_VALUE,   /*  _5_15625G */
    NA_VALUE,   /*  _12_8906G */
    11,         /*  _20_625G  */
    11,         /*  _25_78125G*/
    NA_VALUE,   /*  _27_5G    */
    11,         /*  _28_05G   */
    NA_VALUE,   /*  _26_5625G */
    NA_VALUE,   /*  _28_28125G*/
    NA_VALUE,   /*  _26_5625G_PAM4 */
    NA_VALUE,   /*  _28_28125G_PAM4*/
    NA_VALUE,   /*  _26_25G   */
};

#endif
extern Aapl_t* aaplSerdesDb[HWS_MAX_DEVICE_NUM];

#if defined(CHX_FAMILY) || defined(PX_FAMILY)
extern HWS_MUTEX avagoAccessMutex;
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

extern GT_STATUS mvCfgSeqRunPolOp
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           index,
    MV_CFG_ELEMENT  *element
);
#endif /* ASIC_SIMULATION */

/*
Covering SerDes IP's:
Avago 15G   LPD6
Avago 28G   D6
SerDes FW Rev   2464
*/
MV_HWS_SERDES_TXRX_TUNE_PARAMS hwsAvagoSerdesTxRxTuneParamsArray[] =
{   /* SD speed  txAmp  emph0 emph1  DC     LF  HF  BW  LB  SQ */
    {_1_25G,    {19,    0,      0}, {100,   15, 15, 13, 0,  150}}
   ,{_3_125G,   {16,    0,      0}, {100,   15, 15, 13, 0,  150}}
   ,{_5G,       {12,    0,      0}, {80 ,   6 , 15, 13, 0,  150}}
   ,{_6_25G,    {12,    0,      0}, {110,   4 , 15, 13, 0,  150}}
   ,{_7_5G,     {12,    0,      0}, {110,   4 , 15, 13, 0,  150}}
#ifndef MV_HWS_FREE_RTOS /* non-AP/Host CPU*/
   ,{_10_3125G, {4,     0,      0}, {100,   3 , 15, 13, 0,  68 }} /* use copper/non-AP values (10G-KR/CR), for 10G-SR/10G-LR mode values use Tx/Rx manual API's */
#else
   ,{_10_3125G, {2,    18,      4}, {100,   3 , 15, 13, 0,  68 }} /* for AP TX values must be same as init */
#endif
   ,{_12_1875G, {4,     0,      0}, {100,   3 , 15, 13, 0,  68 }}
   ,{_5_15625G, {12,    0,      0}, {80 ,   6 , 15, 13, 0,  150}}
   ,{_12_8906G, {4,     0,      0}, {100,   3 , 15, 13, 0,  68 }}
   ,{_20_625G,  {4,     0,      0}, {110,   6 , 9 , 9 , 0,  68 }}
   ,{_25_78125G,{1,     0,      6}, {110,   6 , 9 , 9 , 0,  84 }} /* for AP TX values must be same as init */
   ,{_27_1G,    {1,     0,      6}, {110,   6 , 9 , 9 , 0,  84 }}
   ,{_27_5G,    {1,     0,      6}, {110,   6 , 9 , 9 , 0,  84 }}
   ,{_26_25G,   {1,     0,      6}, {110,   6 , 9 , 9 , 0,  84 }}
};

/* To configure regular port in Micro-init on Service CPU */
#ifdef MICRO_INIT
MV_HWS_SERDES_TXRX_TUNE_PARAMS miAvagoSerdesTxRxTuneParamsArray[] =
{
   {_3_125G,   {16,    0,      0}, {100,   15, 15, 13, 0,  150}}
  ,{_10_3125G, {4,     0,      0}, {100,   3 , 15, 13, 0,  68 }} /* use copper/non-AP values (10G-KR/CR), for 10G-SR/10G-LR mode values use Tx/Rx manual API's */
};
#endif

/*
    For PMD Training in AP: TX values must be same as init values that written
    in hwsAvagoSerdesTxRxTuneParamsArray (for Int 0x15)
*/
                                                                       /* txAmp  emph0  emph1 */
MV_HWS_TX_TUNE_PARAMS hwsAvagoSerdesTxTuneInitParams[2] = {/* _10_3125G  */{2,    18,     4},
                                                           /* _25_78125G */{1,     0,     6}};

/* ------------ end of generic Avago serdes init values ------------- */

/************************* Declaration ***************************************************/
#ifndef CO_CPU_RUN
/*******************************************************************************
* mvHwsAvagoSerdesTypeGetFunc
*
* DESCRIPTION:
*       Return "AVAGO" string
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Return "AVAGO" string
*
*******************************************************************************/
static char* mvHwsAvagoSerdesTypeGetFunc(void)
{
  return "AVAGO";
}

#endif /* CO_CPU_RUN */

/**
* @internal mvHwsAvagoIfInit function
* @endinternal
*
* @brief   Init Avago Serdes IF functions.
*
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoIfInit
(
    GT_U8  devNum,
    MV_HWS_SERDES_FUNC_PTRS *funcPtrArray
)
{
#ifdef ASIC_SIMULATION
    /* avoid warning */
    devNum = devNum;
#else
    GT_STATUS res;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        /* the Avago driver MUST not be accessed */
        res = GT_OK;
    }
    else
#endif /*! MV_HWS_REDUCED_BUILD_EXT_CM3*/
    {
        res = mvHwsAvagoSerdesInit(devNum);
    }

    if(res == GT_ALREADY_EXIST)
    {
        /* Avago Serdes for this device was already initialized */
        return GT_OK;
    }
    else if(res != GT_OK)
    {
        return res;
    }
#endif

    if(hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber > MAX_AVAGO_SERDES_NUMBER)
    {
        HWS_AVAGO_DBG(("Size of serdesArr_copy(%d) is not enough for serdes number = %d\n", MAX_AVAGO_SERDES_NUMBER, hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber));
        return GT_BAD_STATE;
    }

    /* Avago implementation */
    funcPtrArray[AVAGO].serdesArrayPowerCntrlFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesArrayPowerCtrl      );
    funcPtrArray[AVAGO].serdesPowerCntrlFunc         = BIND_SET_FUNC(mvHwsAvagoSerdesPowerCtrl           );
    funcPtrArray[AVAGO].serdesManualTxCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesManualTxConfig      );
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
    funcPtrArray[AVAGO].serdesManualRxCfgFunc        = NULL;
    funcPtrArray[AVAGO].serdesAutoTuneCfgFunc        = NULL;
#endif
    funcPtrArray[AVAGO].serdesRxAutoTuneStartFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesRxAutoTuneStart     );
    funcPtrArray[AVAGO].serdesTxAutoTuneStartFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesTxAutoTuneStart     );
    funcPtrArray[AVAGO].serdesAutoTuneStartFunc      = BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneStart       );
    funcPtrArray[AVAGO].serdesAutoTuneResultFunc     = BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneResult      );
    funcPtrArray[AVAGO].serdesAutoTuneStatusFunc     = BIND_SET_FUNC(mvHwsAvagoSerdesAutoTuneStatus      );
    funcPtrArray[AVAGO].serdesAutoTuneStatusShortFunc =BIND_SET_FUNC( mvHwsAvagoSerdesAutoTuneStatusShort);
    funcPtrArray[AVAGO].serdesResetFunc              = BIND_SET_FUNC(mvHwsAvagoSerdesReset               );
    funcPtrArray[AVAGO].serdesDigitalReset           = BIND_SET_FUNC(mvHwsAvagoSerdesDigitalReset        );
    funcPtrArray[AVAGO].serdesCoreReset              = NULL;
#ifndef CO_CPU_RUN
    funcPtrArray[AVAGO].serdesSeqGetFunc             = NULL;
    funcPtrArray[AVAGO].serdesSeqSetFunc             = NULL;
    funcPtrArray[AVAGO].serdesDfeCfgFunc             = BIND_SET_FUNC(mvHwsAvagoSerdesDfeConfig           );
    funcPtrArray[AVAGO].serdesLoopbackCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesLoopback            );
    funcPtrArray[AVAGO].serdesLoopbackGetFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesLoopbackGet         );
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
    funcPtrArray[AVAGO].serdesPpmCfgFunc             = NULL;
    funcPtrArray[AVAGO].serdesPpmGetFunc             = NULL;
#endif
    funcPtrArray[AVAGO].serdesTestGenFunc            = BIND_SET_FUNC(mvHwsAvagoSerdesTestGen             );
    funcPtrArray[AVAGO].serdesTestGenGetFunc         = BIND_GET_FUNC(mvHwsAvagoSerdesTestGenGet          );
    funcPtrArray[AVAGO].serdesTestGenStatusFunc      = BIND_GET_FUNC(mvHwsAvagoSerdesTestGenStatus       );
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
    funcPtrArray[AVAGO].serdesEomGetFunc             = NULL;
    funcPtrArray[AVAGO].serdesDfeStatusFunc          = NULL;
    funcPtrArray[AVAGO].serdesDfeStatusExtFunc       = NULL;
#endif
    funcPtrArray[AVAGO].serdesPolarityCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesPolarityConfig      );
    funcPtrArray[AVAGO].serdesPolarityCfgGetFunc     = BIND_GET_FUNC(mvHwsAvagoSerdesPolarityConfigGet   );
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
    funcPtrArray[AVAGO].serdesDfeOptiFunc            = NULL;
    funcPtrArray[AVAGO].serdesFfeCfgFunc             = NULL;
    funcPtrArray[AVAGO].serdesCalibrationStatusFunc  = NULL;
#endif
    funcPtrArray[AVAGO].serdesTxEnableFunc           = BIND_SET_FUNC(mvHwsAvagoSerdesTxEnable            );
    funcPtrArray[AVAGO].serdesTxEnableGetFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesTxEnableGet         );
    funcPtrArray[AVAGO].serdesTxIfSelectFunc         = BIND_SET_FUNC(mvHwsAvagoSerdesTxIfSelect          );
    funcPtrArray[AVAGO].serdesTxIfSelectGetFunc      = BIND_GET_FUNC(mvHwsAvagoSerdesTxIfSelectGet       );
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
    funcPtrArray[AVAGO].serdesSqlchCfgFunc           = NULL;
    funcPtrArray[AVAGO].serdesAcTermCfgFunc          = NULL;
#endif
    funcPtrArray[AVAGO].serdesAutoTuneStopFunc       = BIND_SET_FUNC(mvHwsAvagoSerdesTxAutoTuneStop      );
    funcPtrArray[AVAGO].serdesDfeCheckFunc           = NULL;
    funcPtrArray[AVAGO].serdesSpeedGetFunc           = BIND_GET_FUNC(mvHwsAvagoSerdeSpeedGet             );
    funcPtrArray[AVAGO].serdesManualRxCfgGetFunc     = NULL;
    funcPtrArray[AVAGO].serdesManualTxCfgGetFunc     = BIND_GET_FUNC(mvHwsAvagoSerdesManualTxConfigGet   );
    funcPtrArray[AVAGO].serdesSignalDetectGetFunc    = BIND_GET_FUNC(mvHwsAvagoSerdesSignalDetectGet     );
#if ( !defined(MV_HWS_FREE_RTOS) || (defined (BC2_DEV_SUPPORT) || defined(AC3_DEV_SUPPORT)))
    funcPtrArray[AVAGO].serdesCdrLockStatusGetFunc   = NULL;
    funcPtrArray[AVAGO].serdesScanSamplerFunc        = NULL;
    funcPtrArray[AVAGO].serdesFixAlign90Start        = NULL;
    funcPtrArray[AVAGO].serdesFixAlign90Stop         = NULL;
    funcPtrArray[AVAGO].serdesFixAlign90Status       = NULL;
#endif
    funcPtrArray[AVAGO].serdesTypeGetFunc            = mvHwsAvagoSerdesTypeGetFunc;
#endif /* CO_CPU_RUN */

    return GT_OK;
}

/**
* @internal mvHwsAvago16nmIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - array for function registration
*/
GT_STATUS mvHwsAvago16nmIfInit
(
    GT_U8  devNum,
    MV_HWS_SERDES_FUNC_PTRS *funcPtrArray
)
{
#ifdef ASIC_SIMULATION
    /* avoid warning */
    devNum = devNum;
#else
    GT_STATUS res;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        /* the Avago driver MUST not be accessed */
        res = GT_OK;
    }
    else
#endif /*! MV_HWS_REDUCED_BUILD_EXT_CM3*/
    {
        res = mvHwsAvagoSerdesInit(devNum);
    }

    if(res == GT_ALREADY_EXIST)
    {
        /* Avago Serdes for this device was already initialized */
        return GT_OK;
    }
    else if(res != GT_OK)
    {
        return res;
    }
#endif

    if(hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber > MAX_AVAGO_SERDES_NUMBER)
    {
        HWS_AVAGO_DBG(("Size of serdesArr_copy(%d) is not enough for serdes number = %d\n", MAX_AVAGO_SERDES_NUMBER, hwsDeviceSpecInfo[devNum].serdesInfo.serdesNumber));
        return GT_BAD_STATE;
    }

    /* Avago implementation */
    funcPtrArray[AVAGO_16NM].serdesArrayPowerCntrlFunc    = BIND_SET_FUNC(mvHwsAvagoSerdesArrayPowerCtrl      );
    funcPtrArray[AVAGO_16NM].serdesPowerCntrlFunc         = BIND_SET_FUNC(mvHwsAvagoSerdesPowerCtrl           );
    funcPtrArray[AVAGO_16NM].serdesManualTxCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesManualTxConfig      );
    funcPtrArray[AVAGO_16NM].serdesManualRxCfgFunc        = NULL;
    funcPtrArray[AVAGO_16NM].serdesAutoTuneCfgFunc        = NULL;
    funcPtrArray[AVAGO_16NM].serdesRxAutoTuneStartFunc    = NULL;
    funcPtrArray[AVAGO_16NM].serdesTxAutoTuneStartFunc    = NULL;
    funcPtrArray[AVAGO_16NM].serdesAutoTuneStartFunc      = NULL;
    funcPtrArray[AVAGO_16NM].serdesAutoTuneResultFunc     = NULL;
    funcPtrArray[AVAGO_16NM].serdesAutoTuneStatusFunc     = NULL;
    funcPtrArray[AVAGO_16NM].serdesAutoTuneStatusShortFunc =NULL;
    funcPtrArray[AVAGO_16NM].serdesResetFunc              = BIND_SET_FUNC(mvHwsAvagoSerdesReset               );
    funcPtrArray[AVAGO_16NM].serdesDigitalReset           = NULL;
    funcPtrArray[AVAGO_16NM].serdesCoreReset              = NULL;
#ifndef CO_CPU_RUN
    funcPtrArray[AVAGO_16NM].serdesSeqGetFunc             = NULL;
    funcPtrArray[AVAGO_16NM].serdesSeqSetFunc             = NULL;
    funcPtrArray[AVAGO_16NM].serdesDfeCfgFunc             = NULL;
    funcPtrArray[AVAGO_16NM].serdesLoopbackCfgFunc        = BIND_SET_FUNC(mvHwsAvagoSerdesLoopback);
    funcPtrArray[AVAGO_16NM].serdesLoopbackGetFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesLoopbackGet);
    funcPtrArray[AVAGO_16NM].serdesPpmCfgFunc             = NULL;
    funcPtrArray[AVAGO_16NM].serdesPpmGetFunc             = NULL;
    funcPtrArray[AVAGO_16NM].serdesTestGenFunc            = NULL;
    funcPtrArray[AVAGO_16NM].serdesTestGenGetFunc         = NULL;
    funcPtrArray[AVAGO_16NM].serdesTestGenStatusFunc      = NULL;
    funcPtrArray[AVAGO_16NM].serdesEomGetFunc             = NULL;
    funcPtrArray[AVAGO_16NM].serdesDfeStatusFunc          = NULL;
    funcPtrArray[AVAGO_16NM].serdesDfeStatusExtFunc       = NULL;
    funcPtrArray[AVAGO_16NM].serdesPolarityCfgFunc        =  BIND_SET_FUNC(mvHwsAvagoSerdesPolarityConfig      );
    funcPtrArray[AVAGO_16NM].serdesPolarityCfgGetFunc     = NULL;
    funcPtrArray[AVAGO_16NM].serdesDfeOptiFunc            = NULL;
    funcPtrArray[AVAGO_16NM].serdesFfeCfgFunc             = NULL;
    funcPtrArray[AVAGO_16NM].serdesCalibrationStatusFunc  = NULL;
    funcPtrArray[AVAGO_16NM].serdesTxEnableFunc           = BIND_SET_FUNC(mvHwsAvagoSerdesTxEnable            );
    funcPtrArray[AVAGO_16NM].serdesTxEnableGetFunc        = BIND_GET_FUNC(mvHwsAvagoSerdesTxEnableGet         );
    funcPtrArray[AVAGO_16NM].serdesTxIfSelectFunc         = NULL;
    funcPtrArray[AVAGO_16NM].serdesTxIfSelectGetFunc      = NULL;
    funcPtrArray[AVAGO_16NM].serdesSqlchCfgFunc           = NULL;
    funcPtrArray[AVAGO_16NM].serdesAcTermCfgFunc          = NULL;
    funcPtrArray[AVAGO_16NM].serdesAutoTuneStopFunc       = NULL;
    funcPtrArray[AVAGO_16NM].serdesDfeCheckFunc           = NULL;
    funcPtrArray[AVAGO_16NM].serdesSpeedGetFunc           = NULL;
    funcPtrArray[AVAGO_16NM].serdesManualRxCfgGetFunc     = NULL;
    funcPtrArray[AVAGO_16NM].serdesManualTxCfgGetFunc     = NULL;
    funcPtrArray[AVAGO_16NM].serdesSignalDetectGetFunc    = NULL;
    funcPtrArray[AVAGO_16NM].serdesCdrLockStatusGetFunc   = NULL;
    funcPtrArray[AVAGO_16NM].serdesScanSamplerFunc        = NULL;
    funcPtrArray[AVAGO_16NM].serdesFixAlign90Start        = NULL;
    funcPtrArray[AVAGO_16NM].serdesFixAlign90Stop         = NULL;
    funcPtrArray[AVAGO_16NM].serdesFixAlign90Status       = NULL;
    funcPtrArray[AVAGO_16NM].serdesTypeGetFunc            = NULL;
#endif /* CO_CPU_RUN */

    return GT_OK;
}


#ifdef FW_DOWNLOAD_FROM_SERVER
/**
* @internal mvHwsAvagoSetFwDownloadFileNames function
* @endinternal
*
* @brief   Set Avago FW Download files names
*         hwsOsPrintf("Chip %d (%s %s), ring %d, SBus devices:
*         %d\n", chip, aapl->chip_name[chip],
*         aapl->chip_rev[chip], ring,
*         aapl->max_sbus_addr[chip][ring]);
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoSetFwDownloadFileNames
(
    char *serdesFwName,
    char *sbusMasterFwName
)
{
    serdesFileName = serdesFwName;
    sbusMasterFileName = sbusMasterFwName;
}
#endif /* FW_DOWNLOAD_FROM_SERVER */

#ifndef ASIC_SIMULATION
/**
* @internal mvHwsAvagoSerdesRxAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check the Rx training status
*         This function is necessary for 802.3ap functionality
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxStatus                 - status of Rx-Training
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAvagoSerdesRxAutoTuneStatusShort
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus
)
{
    int data;

    *rxStatus = TUNE_NOT_COMPLITED;

    /* get DFE status */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, ((0 << 12) | (0xB << 8)), &data));

    /* 0xC0: Traning PASS with VOS, 0x80: Traning PASS without VOS */
    if(data == 0x80 || data == 0xC0)
    {
        *rxStatus = TUNE_PASS;

        if (HWS_DEV_SERDES_TYPE(devNum) != AVAGO_16NM)
        {
            /* Enable TAP1 after Rx training */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_ENABLE, NULL));
        }
    }
    /* TUNE_FAIL is relevant only for DFE_ICAL mode */
    else if(data == 0x2A0)
        *rxStatus = TUNE_FAIL;
    else if((data & 3) != 0)
        *rxStatus = TUNE_NOT_COMPLITED;

    return GT_OK;
}
#endif /* ASIC_SIMULATION */

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStatusShort function
* @endinternal
*
* @brief   Per SERDES check the Tx training status
*         This function is necessary for 802.3ap functionality
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStatusShort
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    txStatus = txStatus;
#else

    GT_U32 data;
    unsigned int sbus_addr;

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    *txStatus = TUNE_NOT_COMPLITED;

    /* check PMD training status */
    mvHwsAvagoAccessLock(devNum);
    data = avago_serdes_pmd_status(aaplSerdesDb[devNum], sbus_addr);
    mvHwsAvagoAccessUnlock(devNum);
    if (data == 0)
        *txStatus = TUNE_FAIL;
    else if (data == 0x1)
    {
        *txStatus = TUNE_PASS;

        if (HWS_DEV_SERDES_TYPE(devNum) != AVAGO_16NM)
        {
            /* Enable TAP1 after pmd_train is completed */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_ENABLE, NULL));
        }
    }
    else if (data == 0x2)
        *txStatus = TUNE_NOT_COMPLITED;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneStatusShort function
* @endinternal
*
* @brief   Check the Serdes Rx or Tx training status
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxStatus                 - Rx-Training status
* @param[out] txStatus                 - Tx-Training status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStatusShort
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    if(rxStatus)
    {
        *rxStatus = TUNE_PASS;
    }
    if(txStatus)
    {
        *txStatus = TUNE_PASS;
    }
#else
    if ((NULL == rxStatus) && (NULL == txStatus))
    {
        return GT_BAD_PARAM;
    }

    if (rxStatus != NULL)
    {
        CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, rxStatus));
    }

    if (txStatus != NULL)
    {
        CHECK_STATUS(mvHwsAvagoSerdesTxAutoTuneStatusShort(devNum, portGroup, serdesNum, txStatus));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSer                 - quantity of SERDESes to configure
* @param[in] serdesArr                - list of SERDESes to configure
* @param[in] powerUp                  - True for PowerUP, False for PowerDown
*                                      baudRate  - Serdes speed
*                                      refClock  - ref clock value
*                                      refClockSource - ref clock source (primary line or secondary)
*                                      media     - RXAUI or XAUI
*                                      mode      - Serdes bus modes: 10Bits/20Bits/40Bits
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesArrayPowerCtrl
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     numOfSer,
    GT_UOPT                     *serdesArr,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    numOfSer = numOfSer;
    (GT_VOID)serdesArr;
    powerUp = powerUp;
    (GT_VOID)serdesConfigPtr;
#else
    GT_UREG_DATA    data;
    GT_UOPT serdesNum;
    GT_U32 divider, dwell;
#ifndef BV_DEV_SUPPORT

    if(serdesConfigPtr == NULL)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("NULL pointer error\n"));
    }

    /* define deivider value */
    if(powerUp == GT_TRUE)
    {
        switch(serdesConfigPtr->refClock)
        {
            case _78Mhz:
                divider = avagoBaudRate2DividerValue_78M[serdesConfigPtr->baudRate];
                break;

            case _156dot25Mhz:
                divider = avagoBaudRate2DividerValue_156M[serdesConfigPtr->baudRate];
                break;

            case _312_5Mhz:
                divider = avagoBaudRate2DividerValue_312_5M[serdesConfigPtr->baudRate];
                break;

            case _164_24Mhz:
                divider = avagoBaudRate2DividerValue_164_24M[serdesConfigPtr->baudRate];
                break;

            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal reference clock\n"));
        }

        if(divider == NA_VALUE)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal baudrate divider\n"));
        }
    }
    else
    {
        divider = NA_VALUE;
    }

    /* define dwell value */
    if (avagoBaudRate2DwellTimeValue[serdesConfigPtr->baudRate] != NA_VALUE)
    {
        dwell = avagoBaudRate2DwellTimeValue[serdesConfigPtr->baudRate];
    }
    else
    {
        dwell = 11;
    }

#else
    divider = mcdBaudRate2DividerValue[serdesConfigPtr->baudRate];
    if(divider == NA_VALUE)
        return GT_BAD_PARAM;
    dwell = mcdBaudRate2DwellTimeValue[serdesConfigPtr->baudRate];
#endif
    CHECK_STATUS(mvHwsAvagoSerdesArrayPowerCtrlImpl(devNum, portGroup, numOfSer, serdesArr, powerUp,
                                                    divider,
                                                    serdesConfigPtr));
    if(powerUp == GT_TRUE)
    {
        for (serdesNum = 0; serdesNum < numOfSer; serdesNum++)
        {
            /* select DFE tuning dwell time */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x18, 0x7, NULL));
            /* big register write LSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, 1<<dwell, NULL));
            /* big register write MSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, 0, NULL));
        }
    }

    /*  RM-6080676
     *      The SERDES Signal Detect polarity in 1 / 2.5Gbps modes
     *      should be inverted. In 1G and 2.5G mode, Bit[0] in the
     *      SERDES Miscellaneous register should = 1.
    */
    if ( (Bobcat3 == HWS_DEV_SILICON_TYPE(devNum)) || (Pipe == HWS_DEV_SILICON_TYPE(devNum))
         || (Aldrin2 == HWS_DEV_SILICON_TYPE(devNum)) )
    {
        if(((serdesConfigPtr->busWidth == _10BIT_ON) && (serdesConfigPtr->baudRate == _1_25G))  ||
           ((serdesConfigPtr->busWidth == _10BIT_ON) && (serdesConfigPtr->baudRate == _3_125G)) ||
           ((serdesConfigPtr->busWidth == _10BIT_ON) && (serdesConfigPtr->baudRate == _5G)))
        {
            data = (powerUp == GT_FALSE) ? 0 : 1;
        }
        else
        {
            data = 0;
        }
        for (serdesNum = 0; serdesNum < numOfSer; serdesNum++)
        {
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesNum],
                                                AVAGO_SD_METAL_FIX, data, 1));
        }
    }

    return GT_OK;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

#if !defined (ASIC_SIMULATION) && !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
/**
* @internal mvHwsAvagoSerdesPMDdebugPrint function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPMDdebugPrint
(
    GT_U8    devNum,
    GT_U32   serdesNum
)
{
    Avago_serdes_pmd_debug_t *pmd_debug;
    unsigned int    sbus_addr;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Allocates and initializes memory for a PMD debug structure */
    mvHwsAvagoAccessLock(devNum);
    pmd_debug = avago_serdes_pmd_debug_construct(aaplSerdesDb[devNum]);

    /* Gathers up the PMD training debug information from Serdes */
    avago_serdes_pmd_debug(aaplSerdesDb[devNum], sbus_addr, pmd_debug);

    /* print PMD debug info */
    avago_serdes_pmd_debug_print(aaplSerdesDb[devNum], pmd_debug);

    /* Frees the memory pointed */
    avago_serdes_pmd_debug_destruct(aaplSerdesDb[devNum], pmd_debug);
    mvHwsAvagoAccessUnlock(devNum);

    return GT_OK;
}
#endif /* !defined (ASIC_SIMULATION) && !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) */

#if defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
/*******************************************************************************
* mvHwsAvagoSerdesPowerCtrl
*
* DESCRIPTION:
*       Power up single SERDES.
*
* INPUTS:
*       devNum    - system device number
*       portGroup - port group (core) number
*       serdesNum - number of SERDESes to configure
*       powerUp   - True for PowerUP, False for PowerDown
*       baudRate  - Serdes speed
*       refClock  - ref clock value
*       refClockSource - ref clock source (primary line or secondary)
*       media     - RXAUI or XAUI
*       mode      - Serdes bus modes: 10Bits/20Bits/40Bits
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS mvHwsAvagoSerdesPowerCtrl
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
)
{
    GT_UOPT serdesArr[1];

    serdesArr[0] = serdesNum;
    return mvHwsAvagoSerdesArrayPowerCtrl(devNum, portGroup, 1, serdesArr, powerUp, serdesConfigPtr);
}
#else

/**
* @internal mvHwsAvagoSerdesPowerCtrl function
* @endinternal
*
* @brief   Power up Serdes number
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - Serdes number
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfig - pointer to Serdes paramters:
*      refClock  - ref clock value
*      refClockSource - ref clock source (primary line or
*      secondary)
*      baudRate - Serdes speed
*      busWidth - Serdes bus modes: 10Bits/20Bits/40Bits
*      media - RXAUI or XAUI
*      encoding - Rx & Tx data encoding NRZ/PAM4
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsAvagoSerdesPowerCtrl
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    GT_BOOL                     powerUp,
    MV_HWS_SERDES_CONFIG_STR    *serdesConfigPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    powerUp = powerUp;
    (GT_VOID)serdesConfigPtr;
#else
    GT_U32 divider, dwell;
#ifndef BV_DEV_SUPPORT

    if(serdesConfigPtr == NULL)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("NULL pointer error\n"));
    }

    /* define deivider value */
    if(powerUp == GT_TRUE)
    {
        switch(serdesConfigPtr->refClock)
        {
            case _78Mhz:
                divider = avagoBaudRate2DividerValue_78M[serdesConfigPtr->baudRate];
                break;

            case _156dot25Mhz:
                divider = avagoBaudRate2DividerValue_156M[serdesConfigPtr->baudRate];
                break;

            case _312_5Mhz:
                divider = avagoBaudRate2DividerValue_312_5M[serdesConfigPtr->baudRate];
                break;

            case _164_24Mhz:
                divider = avagoBaudRate2DividerValue_164_24M[serdesConfigPtr->baudRate];
                break;

            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal reference clock\n"));
        }

        if(divider == NA_VALUE)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal baudrate divider\n"));
        }
    }
    else
    {
        divider = NA_VALUE;
    }

    /* define dwell value */
    if (avagoBaudRate2DwellTimeValue[serdesConfigPtr->baudRate] != NA_VALUE)
    {
        dwell = avagoBaudRate2DwellTimeValue[serdesConfigPtr->baudRate];
    }
    else
    {
        dwell = 11;
    }

#else
    divider = mcdBaudRate2DividerValue[serdesConfigPtr->baudRate];
    if(divider == NA_VALUE)
        return GT_BAD_PARAM;
    dwell = mcdBaudRate2DwellTimeValue[serdesConfigPtr->baudRate];
#endif
    CHECK_STATUS(mvHwsAvagoSerdesPowerCtrlImpl(devNum, portGroup, serdesNum, powerUp,divider,serdesConfigPtr));

    if(powerUp == GT_TRUE)
    {
        if (HWS_DEV_SERDES_TYPE(devNum) != AVAGO_16NM)
        {
            /* select DFE tuning dwell time */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x7, NULL));

            /* big register write LSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 1 << dwell, NULL));

            /* big register write MSB */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0, NULL));
        }
    }

    /*  RM-6080676 for BC3 only */

    return GT_OK;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif

/**
* @internal mvHwsAvagoSerdesManualTxConfig function
* @endinternal
*
* @brief   Per SERDES configure the TX parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txAmp                    - Tx Attenuator [0...31]
* @param[in] txAmpAdj                 - not used in Avago serdes
* @param[in] emph0                    - Post-Cursor: for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
* @param[in] emph1                    - Pre-Cursor:  for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
* @param[in] slewRate                 - Slew rate  [0...3], 0 is fastest
*                                      (the SlewRate parameter is not supported in Avago Serdes 28nm)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxConfig
(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum,
    GT_UREG_DATA    txAmp,
    GT_BOOL         txAmpAdj,
    GT_REG_DATA     emph0,
    GT_REG_DATA     emph1,
    GT_UREG_DATA    slewRate,
    GT_BOOL         txEmphEn
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    txAmp = txAmp;
    txAmpAdj = txAmpAdj;
    emph0 = emph0;
    emph1 = emph1;
    slewRate = slewRate;
    txEmphEn = txEmphEn;
#else

    Avago_serdes_tx_eq_t tx_eq;
    unsigned int    sbus_addr;
    unsigned int    ip_rev;
    int             rc1, rc2, rc3,rc4;

    /* txAmpAdj is not used in Avago serdes */
    txAmpAdj = GT_FALSE;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    ip_rev = aapl_get_ip_rev(aaplSerdesDb[devNum], sbus_addr);

    if ((txAmp & 0xFFFFFFE0) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if ((emph0 > 31) || (emph1 > 31))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* for BobK/Aldrin devices */
    if(ip_rev == 0xde)
    {
        if ((emph0 < -31) || (emph1 < -31))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Post-Cursor or Pre-Cursor is out of range: < -31\n"));
        }
    }/* for Bobcat3 device */
    else if (ip_rev == 0xcd)
    {
        if ((emph0 < 0) || (emph1 < 0))
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ARG_STRING_MAC("Post-Cursor or Pre-Cursor is out of range: < 0\n"));
        }
    }

    /* Set the TX equalization values */
    tx_eq.atten = (short)txAmp;     /* Attenuator setting */
    tx_eq.post  = (short)emph0;     /* Post-cursor setting */
    tx_eq.pre   = (short)emph1;     /* Pre-cursor setting */
    tx_eq.slew  = (short)slewRate;  /* Slew rate setting */

    if((tx_eq.atten == tx_eq.pre) && (tx_eq.atten == tx_eq.post))
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x15, (3 << 14) | (tx_eq.atten & 0xFF), &rc3));
    }
    else
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x15, (3 << 14) | 0, &rc1));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x15, (2 << 14) | (tx_eq.post & 0xFF), &rc2));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x15, (1 << 14) | (tx_eq.atten & 0xFF), &rc3));
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x15, (0 << 14) | (tx_eq.pre & 0xFF), &rc4));

        /* If SpicoInterrupt 0x15 succeeds to run - the return code is the Interrupt number=0x15
           The TX equalization configuration succeeds only if all the four SpicoInterrupt settings are passed: (0x15)x4=0x54 */
        if ((rc1+rc2+rc3+rc4) != MV_AVAGO_TX_EQUALIZE_PASS)
        {
            HWS_AVAGO_DBG(("SerDes failed to apply since new EQ settings exceed the limit %d %d %d %d\n", rc1, rc2, rc3, rc4));
            return GT_FAIL;
        }
    }

#if 0
    /* the SlewRate parameter is not supported in Avago Serdes */
    /* these settings are needed due to problem with SlewRate configuration in avago_serdes_set_tx_eq */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x4240, NULL));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, (0x184 + slewRate), NULL));
#endif

#endif /* ASIC_SIMULATION */
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters: amplitude, 3 TAP Tx FIR.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] configParams             - Manual Tx params structures:
*                                      txAmp     - Tx Attenuator [0...31]
*                                      txAmpAdj  - not used in Avago serdes
*                                      emph0     - Post-Cursor: for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
*                                      emph1     - Pre-Cursor:  for BobK/Aldrin (IP_Rev=0xde) [-31...31],
*                                      for Bobcat3 (IP_Rev=0xcd) [0...31]
*                                      slewRate  - Slew rate  [0...3], 0 is fastest
*                                      (the SlewRate parameter is not supported in Avago Serdes 28nm)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualTxConfigGet
(
    GT_U8                           devNum,
    GT_UOPT                         portGroup,
    GT_UOPT                         serdesNum,
    MV_HWS_MAN_TUNE_TX_CONFIG_DATA  *configParams
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    hwsOsMemSetFuncPtr(configParams,0,sizeof(*configParams));
#else

    MV_HWS_AUTO_TUNE_RESULTS    results;

    /* txAmpAdj and txAmpShft are not used in Avago serdes */
    configParams->txAmpAdj  = GT_FALSE;
    configParams->txAmpShft = GT_FALSE;

    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesNum, &results));

    /* Get the TX configuration values */
    configParams->txAmp   = results.txAmp;
    configParams->txEmph0 = results.txEmph0;
    configParams->txEmph1 = results.txEmph1;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesSignalDetectGet function
* @endinternal
*
* @brief   Per SERDES get indication check CDR lock and Signal Detect.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalDetectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalDet
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    signalDet = signalDet;
#else

    unsigned int sbus_addr;
    GT_U32 i, data;

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* check CDR lock and Signal Detect on Serdes */
    for(i=0; i < 10; i++)
    {
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &data, (1 << 2)));
        if(data)
        {
            *signalDet = GT_FALSE;
            return GT_OK;
        }
    }

    *signalDet = GT_TRUE;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesSignalOkChange function
* @endinternal
*
* @brief   Per SERDES get indication check CDR lock and Signal
*          Detect change.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkChange
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalOkChange
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    signalOkChange = signalOkChange;
#else

    GT_U32 data;

    RETURN_GT_OK_ON_EMULATOR;


    /* check CDR lock change and Signal Detect change on Serdes */

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_INTERRUPT_CAUSE_REG0, &data, (1 << 5)));
    if(data)
    {
        *signalOkChange = GT_TRUE;
    }
    else
    {
        *signalOkChange = GT_FALSE;
    }

#endif /* ASIC_SIMULATION */

    return GT_OK;
}



/**
* @internal mvHwsAvagoSerdesRxSignalCheck function
* @endinternal
*
* @brief   Per SERDES check there is Rx Signal and indicate if Serdes is ready for Tuning or not
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] rxSignal                 - Serdes is ready for Tuning or not
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxSignalCheck
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_AUTO_TUNE_STATUS     *rxSignal
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    rxSignal = rxSignal;
#else

    unsigned int sbus_addr;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* check if there is Rx Signal on Serdes */
    mvHwsAvagoAccessLock(devNum);
    *rxSignal = ((avago_serdes_mem_rd(aaplSerdesDb[devNum], sbus_addr, AVAGO_LSB, 0x027) & 0x0010) != 0) ? TUNE_READY : TUNE_NOT_READY;
    mvHwsAvagoAccessUnlock(devNum);
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesReset function
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
GT_STATUS mvHwsAvagoSerdesReset
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     analogReset,
    GT_BOOL     digitalReset,
    GT_BOOL     syncEReset
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    analogReset = analogReset;
    digitalReset = digitalReset;
    syncEReset = syncEReset;
#else

  return(mvHwsAvagoSerdesResetImpl(devNum, portGroup, serdesNum, analogReset, digitalReset, syncEReset));
#endif /* ASIC_SIMULATION */

  return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesDigitalReset function
* @endinternal
*
* @brief   Run digital RESET/UNRESET (RF) on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] digitalReset             - digital (RF) Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDigitalReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    digitalReset
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    digitalReset = digitalReset;
#else

    GT_U32  data;

    /* SERDES digital RESET/UNRESET (RF) */
    data = (digitalReset == UNRESET) ? 1 : 0;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_1, (data << 3), (1 << 3)));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
/**
* @internal mvHwsAvagoSerdesPolarityConfig function
* @endinternal
*
* @brief   Per Serdes invert the Tx or Rx.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical Serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    invertTx = invertTx;
    invertRx = invertRx;
#else

    return (mvHwsAvagoSerdesPolarityConfigImpl(devNum, portGroup, serdesNum, invertTx, invertRx));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesPolarityConfigGet function
* @endinternal
*
* @brief   Returns the Tx and Rx SERDES invert state.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] invertTx                 - invert TX polarity (GT_TRUE - invert, GT_FALSE - don't)
* @param[out] invertRx                 - invert RX polarity (GT_TRUE - invert, GT_FALSE - don't)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPolarityConfigGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    GT_BOOL           *invertTx,
    GT_BOOL           *invertRx
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    invertTx = invertTx;
    invertRx = invertRx;
#else

    unsigned int avagoInvertTx;
    unsigned int avagoInvertRx;
    int res;

    res = mvHwsAvagoSerdesPolarityConfigGetImpl(devNum, portGroup, serdesNum,
                                                &avagoInvertTx, &avagoInvertRx);
    if(res != GT_OK)
    {
        HWS_AVAGO_DBG(("mvHwsAvagoSerdesPolarityConfigGetImpl failed (%d)\n", res));
        return GT_FAIL;
    }

    *invertTx = avagoInvertTx;
    *invertRx = avagoInvertRx;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesDfeConfig function
* @endinternal
*
* @brief   Per SERDES configure the DFE parameters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] dfeMode                  - Enable/disable/freeze/Force
* @param[in] dfeCfg                   - array of dfe configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesDfeConfig
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_DFE_MODE     dfeMode,
    GT_REG_DATA         *dfeCfg
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    dfeMode = dfeMode;
    dfeCfg = dfeCfg;
#else

    Avago_serdes_dfe_tune_t  dfe_state;
    unsigned int sbus_addr;

    RETURN_GT_OK_ON_EMULATOR;

    /* dfeCfg is not used in Avago serdes */
    dfeCfg = NULL;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Initialize the dfe_state struct with default values */
    avago_serdes_tune_init(aaplSerdesDb[devNum], &dfe_state);
    CHECK_AVAGO_RET_CODE();

    switch (dfeMode)
    {
        case DFE_STOP_ADAPTIVE:
            dfe_state.tune_mode = AVAGO_DFE_STOP_ADAPTIVE;
            break;
        case DFE_START_ADAPTIVE:
            dfe_state.tune_mode = AVAGO_DFE_START_ADAPTIVE;
            break;
        case DFE_ICAL:
            if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO_16NM)
            {
                /* Disable TAP1 before iCal */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));
            }
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            break;
        case DFE_PCAL:
            dfe_state.tune_mode = AVAGO_DFE_PCAL;
            break;

        case DFE_ICAL_VSR:
            if(HWS_DEV_SERDES_TYPE(devNum) != AVAGO_16NM)
            {
                /* Disable TAP1 before iCal */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));
            }
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            dfe_state.dfe_disable = GT_TRUE;
            break;

        default:
            return GT_BAD_PARAM;
    }


    mvHwsAvagoAccessLock(devNum);
    /* Run/Halt DFE tuning on a serdes based on dfe_tune_mode */
    avago_serdes_tune(aaplSerdesDb[devNum], sbus_addr, &dfe_state);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical Serdes number
* @param[in] txPattern                - pattern to transmit ("Other" means HW default - K28.5
*                                      [alternate running disparity])
* @param[in] mode                     - test  or normal
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTestGen
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    MV_HWS_SERDES_TX_PATTERN    txPattern,
    MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    txPattern = txPattern;
    mode = mode;
#else

    unsigned int sbus_addr;
    Avago_serdes_rx_cmp_data_t  rxPatternData = AVAGO_SERDES_RX_CMP_DATA_OFF;
    Avago_serdes_tx_data_sel_t  txPatternData = AVAGO_SERDES_TX_DATA_SEL_CORE;
    long tx_user[4] = {0};
    GT_UREG_DATA widthMode10Bit;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Read the saved value from AVAGO_SD_METAL_FIX register to check the Rx/Tx width (saved in mvHwsAvagoSerdesPowerCtrlImpl).
       PRBS test in GPCS modes can run only with Rx/Tx width=20BIT. */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_SD_METAL_FIX, &widthMode10Bit, (1 << 9)));
    if ((widthMode10Bit >> 9) == 1)
    {
        /* Change Rx/Tx width mode to 20BIT for GPCS modes */
        mvHwsAvagoAccessLock(devNum);
        avago_serdes_set_tx_rx_width(aaplSerdesDb[devNum], sbus_addr, 20, 20);
        mvHwsAvagoAccessUnlock(devNum);
        CHECK_AVAGO_RET_CODE();
    }

    if (mode == SERDES_TEST)
    {
        switch (txPattern)
        {
            case _1T:
                tx_user[0] = 0xAAAAA;
                tx_user[1] = 0xAAAAA;
                tx_user[2] = 0xAAAAA;
                tx_user[3] = 0xAAAAA;
                break;
            case _2T:
                tx_user[0] = 0xCCCCC;
                tx_user[1] = 0xCCCCC;
                tx_user[2] = 0xCCCCC;
                tx_user[3] = 0xCCCCC;
                break;
            case _5T:
                tx_user[0] = 0x7C1F;
                tx_user[1] = 0x7C1F;
                tx_user[2] = 0x7C1F;
                tx_user[3] = 0x7C1F;
                break;
            case _10T:
                tx_user[0] = 0x3FF;
                tx_user[1] = 0x3FF;
                tx_user[2] = 0x3FF;
                tx_user[3] = 0x3FF;
                break;
            case PRBS7:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS7;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS7;
                break;
            case PRBS9:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS9;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS9;
                break;
            case PRBS11:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS11;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS11;
                break;
            case PRBS15:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS15;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS15;
                break;
            case PRBS23:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS23;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS23;
                break;
            case PRBS31:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS31;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS31;
                break;
            case PRBS13:
                txPatternData = AVAGO_SERDES_TX_DATA_SEL_PRBS13;
                rxPatternData = AVAGO_SERDES_RX_CMP_DATA_PRBS13;
                break;
            default:
                return GT_NOT_SUPPORTED;
        }

        /* USER modes txPattern: 1T, 2T, 5T, 10T */
        if (txPattern <= _10T)
        {
            mvHwsAvagoAccessLock(devNum);
            /* Sets the USER TX data source */
            avago_serdes_set_tx_data_sel(aaplSerdesDb[devNum], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_USER);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();

            mvHwsAvagoAccessLock(devNum);
            /* Sets the USER pattern to compare incoming data against, Auto-seed to received 40 bit repeating pattern */
            avago_serdes_set_rx_cmp_data(aaplSerdesDb[devNum], sbus_addr, AVAGO_SERDES_RX_CMP_DATA_SELF_SEED);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();

            mvHwsAvagoAccessLock(devNum);
            /* Loads the 80-bit value into the TX user data register */
            avago_serdes_set_tx_user_data(aaplSerdesDb[devNum], sbus_addr, tx_user);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();

            mvHwsAvagoAccessLock(devNum);
            /* reset counter */
            avago_serdes_error_reset(aaplSerdesDb[devNum], sbus_addr);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();
        }
        else /* for PRBS modes */
        {
            mvHwsAvagoAccessLock(devNum);
            /* Sets the PRBS TX data source */
            avago_serdes_set_tx_data_sel(aaplSerdesDb[devNum], sbus_addr, txPatternData);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();

            mvHwsAvagoAccessLock(devNum);
            /* Sets the PRBS pattern to compare incoming data against */
            avago_serdes_set_rx_cmp_data(aaplSerdesDb[devNum], sbus_addr, rxPatternData);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();

            /* Sets the data comparisons (0x203): Turn on data comparison with Compare Sum and PRBS-generated data */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x3, 0x203, NULL));

            mvHwsAvagoAccessLock(devNum);
            /* reset counter */
            avago_serdes_error_reset(aaplSerdesDb[devNum], sbus_addr);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();
        }

        /*if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
        {
            mvHwsAvagoAccessLock(devNum);
            / *avago_serdes_aux_counter_start(aaplSerdesDb[devNum], sbus_addr);* /
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();
        }*/

    }
    else /* for SERDES_NORMAL mode */
    {
        if ((widthMode10Bit >> 9) == 1)
        {
            /* Set back the Rx/Tx width mode to 10BIT if it was changed to 20BIT for GPCS modes */
            mvHwsAvagoAccessLock(devNum);
            avago_serdes_set_tx_rx_width(aaplSerdesDb[devNum], sbus_addr, 10, 10);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();

            /* perform Serdes Digital Reset/Unreset */
            CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, portGroup, serdesNum, HWS_DEV_SERDES_TYPE(devNum), RESET));

            /* delay 1ms */
            hwsOsExactDelayPtr(devNum, portGroup, 1);

            CHECK_STATUS(mvHwsSerdesDigitalReset(devNum, portGroup, serdesNum, HWS_DEV_SERDES_TYPE(devNum), UNRESET));
        }

        mvHwsAvagoAccessLock(devNum);
        /* Sets the TX data source */
        avago_serdes_set_tx_data_sel(aaplSerdesDb[devNum], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_CORE);
        mvHwsAvagoAccessUnlock(devNum);
        CHECK_AVAGO_RET_CODE();

        mvHwsAvagoAccessLock(devNum);
        /* Sets the PRBS pattern to compare incoming data against */
        avago_serdes_set_rx_cmp_data(aaplSerdesDb[devNum], sbus_addr, AVAGO_SERDES_RX_CMP_DATA_OFF);
        mvHwsAvagoAccessUnlock(devNum);
        CHECK_AVAGO_RET_CODE();

        /*if (hwsDeviceSpecInfo[devNum].serdesType == AVAGO_16NM)
        {
            mvHwsAvagoAccessLock(devNum);
            / *avago_serdes_aux_counter_disable(aaplSerdesDb[devNum], sbus_addr);* /
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();
        }*/
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTestGenGet function
* @endinternal
*
* @brief   Get configuration of the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] txPatternPtr             - pattern to transmit ("Other" means any mode not
*                                      included explicitly in MV_HWS_SERDES_TX_PATTERN type)
* @param[out] modePtr                  - test mode or normal
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - unexpected pattern
* @retval GT_FAIL                  - HW error
*/
GT_STATUS mvHwsAvagoSerdesTestGenGet
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    MV_HWS_SERDES_TX_PATTERN    *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE *modePtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    txPatternPtr = txPatternPtr;
    modePtr = modePtr;
#else

    unsigned int sbus_addr;
    Avago_serdes_tx_data_sel_t data_sel;
    long tx_user[2] = {0};

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    mvHwsAvagoAccessLock(devNum);
    data_sel = avago_serdes_get_tx_data_sel(aaplSerdesDb[devNum], sbus_addr);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

    switch(data_sel)
    {
        case AVAGO_SERDES_TX_DATA_SEL_PRBS7:
            *txPatternPtr = PRBS7;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS9:
            *txPatternPtr = PRBS9;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS11:
            *txPatternPtr = PRBS11;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS15:
            *txPatternPtr = PRBS15;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS23:
            *txPatternPtr = PRBS23;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS31:
            *txPatternPtr = PRBS31;
            break;
        case AVAGO_SERDES_TX_DATA_SEL_PRBS13:
            *txPatternPtr = PRBS13;
            break;

        case AVAGO_SERDES_TX_DATA_SEL_USER: /* User pattern generator */
            mvHwsAvagoAccessLock(devNum);
            avago_serdes_get_tx_user_data(aaplSerdesDb[devNum], sbus_addr, tx_user);
            mvHwsAvagoAccessUnlock(devNum);
            tx_user[0] = tx_user[0] & 0x3FF;
            tx_user[1] = (tx_user[1] >> 4) & 0x3FFF;

            if (tx_user[0] == 0x2AA)
            {
                *txPatternPtr = _1T;
            }
            else if (tx_user[0] == 0x333)
            {
                *txPatternPtr = _2T;
            }
            else if (tx_user[0] == 0x1F)
            {
                *txPatternPtr = _5T;
            }
            else if ((tx_user[0] == 0x3FF) && (tx_user[1] == 0x3FF0))
            {
                *txPatternPtr = _10T;
            }
            break;
        default:
            *txPatternPtr = Other;
            break;
    }

    *modePtr = (data_sel == AVAGO_SERDES_TX_DATA_SEL_CORE) ? SERDES_NORMAL : SERDES_TEST;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTestGenStatus function
* @endinternal
*
* @brief   Read the tested pattern receive error counters.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - Enable/Disable reset the accumulation of error counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTestGenStatus
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          serdesNum,
    MV_HWS_SERDES_TX_PATTERN        txPattern,
    GT_BOOL                         counterAccumulateMode,
    MV_HWS_SERDES_TEST_GEN_STATUS   *status
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    txPattern = txPattern;
    counterAccumulateMode = counterAccumulateMode;
    hwsOsMemSetFuncPtr(status,0,sizeof(*status));
#else

    unsigned int sbus_addr;
    GT_U32 data;

    /* avoid warning */
    txPattern = txPattern;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &data, (1 <<2)));

    mvHwsAvagoAccessLock(devNum);
    /* Retrieves the value of the error counter */
    status->errorsCntr = avago_serdes_get_errors(aaplSerdesDb[devNum], sbus_addr, AVAGO_LSB, !counterAccumulateMode);
    mvHwsAvagoAccessUnlock(devNum);
    status->lockStatus = (data) ? 0 : 1; /* if bit[2] is set then there is not signal (and vice versa) */
    status->txFramesCntr.l[0] = 0;
    status->txFramesCntr.l[1] = 0;

    CHECK_AVAGO_RET_CODE();

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesLoopback function
* @endinternal
*
* @brief   Perform an Internal/External SERDES loopback mode for Debug use
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - physical Serdes number
* @param[in] lbMode    - loopback mode
*
* @retval 0 - on success
* @retval 1 - on error
*
* @note  - In 'AN_TX_RX' mode, the Rx & Tx Serdes Polarity
*          configurations should be disabled, for getting LinkUp
*          and Traffic on port.
*        - After return to 'Normal' mode, the Rx & Tx Serdes
*          Polarity configuration should be restored, for
*          re-synch again the PCS and MAC units and getting
*          LinkUp and Traffic on port.
*        - Switching between 'AN_TX_RX'/'DIG_RX_TX' modes
*          requires returning to the 'Normal' mode before.
*
*/
GT_STATUS mvHwsAvagoSerdesLoopback
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_SERDES_LB  lbMode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    lbMode = lbMode;
#else

    unsigned int        sbus_addr;
    MV_HWS_SERDES_SPEED rate;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    switch(lbMode)
    {
        case SERDES_LP_DISABLE:
            /* set loopback_clock=ref_clock*/
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x30, 0, NULL));

            /* Disable Loopback mode*/
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x8, 0x100, NULL));

            if (hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM)
            {
                /* CDR Rx power down */
                mvHwsAvagoAccessLock(devNum);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[devNum], sbus_addr, GT_TRUE, GT_FALSE, GT_TRUE);
                mvHwsAvagoAccessUnlock(devNum);

                /* CDR Rx power up for recalibration */
                mvHwsAvagoAccessLock(devNum);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[devNum], sbus_addr, GT_TRUE, GT_TRUE, GT_TRUE);
                mvHwsAvagoAccessUnlock(devNum);
            }

            mvHwsAvagoAccessLock(devNum);
            avago_serdes_set_tx_data_sel(aaplSerdesDb[devNum], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_CORE);
            mvHwsAvagoAccessUnlock(devNum);
            break;
        case SERDES_LP_AN_TX_RX:
            /* set loopback_clock=ref_clock */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x30, 0, NULL));

            /* Internal Loopback mode */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x8, 0x101, NULL));
            if (hwsDeviceSpecInfo[devNum].serdesType != AVAGO_16NM)
            {
                /* CDR Rx power down */
                mvHwsAvagoAccessLock(devNum);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[devNum], sbus_addr, GT_TRUE, GT_FALSE, GT_TRUE);
                mvHwsAvagoAccessUnlock(devNum);

                /* CDR Rx power up for recalibration */
                mvHwsAvagoAccessLock(devNum);
                avago_serdes_set_tx_rx_enable(aaplSerdesDb[devNum], sbus_addr, GT_TRUE, GT_TRUE, GT_TRUE);
                mvHwsAvagoAccessUnlock(devNum);
            }
            break;
        case SERDES_LP_DIG_RX_TX:
            /* set loopback and loopback_clock=CDR_clock */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x30, 0x10, NULL));

            CHECK_STATUS(mvHwsAvagoSerdeSpeedGet(devNum, portGroup, serdesNum, &rate));
            if ((rate == _25_78125G) || (rate == _27_5G) || (rate == _26_25G))
            {
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x8024, NULL));
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0x184c, NULL));
            }

            /* CDR Rx power down */
            mvHwsAvagoAccessLock(devNum);
            avago_serdes_set_tx_rx_enable(aaplSerdesDb[devNum], sbus_addr, GT_TRUE, GT_FALSE, GT_TRUE);
            mvHwsAvagoAccessUnlock(devNum);

            /* CDR Rx power up for recalibration */
            mvHwsAvagoAccessLock(devNum);
            avago_serdes_set_tx_rx_enable(aaplSerdesDb[devNum], sbus_addr, GT_TRUE, GT_TRUE, GT_TRUE);
            mvHwsAvagoAccessUnlock(devNum);

            mvHwsAvagoAccessLock(devNum);
            avago_serdes_set_tx_data_sel(aaplSerdesDb[devNum], sbus_addr, AVAGO_SERDES_TX_DATA_SEL_LOOPBACK);
            mvHwsAvagoAccessUnlock(devNum);
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesLoopbackGet function
* @endinternal
*
* @brief   Gets the status of Internal/External SERDES loopback mode.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical Serdes number
*
* @param[out] lbModePtr                - current loopback mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesLoopbackGet
(
    GT_U8             devNum,
    GT_UOPT           portGroup,
    GT_UOPT           serdesNum,
    MV_HWS_SERDES_LB  *lbModePtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    *lbModePtr = SERDES_LP_DISABLE;
#else

    unsigned int sbus_addr;
    GT_BOOL internalLB;
    Avago_serdes_tx_data_sel_t  externalLB;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    mvHwsAvagoAccessLock(devNum);
    /* Get TRUE if internal loopback is enabled, FALSE if external signal */
    internalLB = avago_serdes_get_rx_input_loopback(aaplSerdesDb[devNum], sbus_addr);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

    if (internalLB == GT_TRUE)
    {
        /* Internal Loopback mode */
        *lbModePtr = SERDES_LP_AN_TX_RX;
        return GT_OK;
    }

    mvHwsAvagoAccessLock(devNum);
    /* Get the selected TX data source */
    externalLB = avago_serdes_get_tx_data_sel(aaplSerdesDb[devNum], sbus_addr);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

    /* Check the External Loopback or Disable mode */
    *lbModePtr = (externalLB == AVAGO_SERDES_TX_DATA_SEL_LOOPBACK) ? SERDES_LP_DIG_RX_TX : SERDES_LP_DISABLE;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxEnable function
* @endinternal
*
* @brief   Enable/Disable Tx.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] enable                   - whether to  or disable Tx.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_READY             - when training is in process.
*/
GT_STATUS mvHwsAvagoSerdesTxEnable
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     enable
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    enable = enable;
#else

    unsigned int sbus_addr;
    unsigned int data;

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Semaphore protection should start before training-check function is performed and take place until
       tx-enable function is finised, to avoid situations where training-check function returned one status
       and by the time we will perform tx-enable, the training status will be changed by another CPU.*/
    mvHwsAvagoAccessLock(devNum);

    /* If training is in process, not performing tx enable */
    data = avago_serdes_pmd_status(aaplSerdesDb[devNum], sbus_addr);
    CHECK_AVAGO_RET_CODE_WITH_ACTION(mvHwsAvagoAccessUnlock(devNum));
    if (data == 0x2)
    {
        HWS_AVAGO_DBG(("mvHwsAvagoSerdesTxEnable failed because training is in process\n"));
        mvHwsAvagoAccessUnlock(devNum);
        return GT_NOT_READY;
    }

    /* Set the TX output Enabled/Disabled */
    avago_serdes_set_tx_output_enable(aaplSerdesDb[devNum], sbus_addr, enable);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxEnableGet function
* @endinternal
*
* @brief   Gets the status of SERDES Tx mode - Enable/Disable transmission of packets.
*         Use this API to disable Tx for loopback ports.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] enablePtr
*                                      - GT_TRUE - Enable transmission of packets from SERDES
*                                      - GT_FALSE - Disable transmission of packets from SERDES
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* @note Disabling transmission of packets from SERDES causes to link down
*       of devices that are connected to the port.
*
*/
GT_STATUS mvHwsAvagoSerdesTxEnableGet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     *enablePtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    enablePtr = enablePtr;
#else

    unsigned int sbus_addr;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    mvHwsAvagoAccessLock(devNum);
    /* Get the SERDES TX output mode: TRUE in enabled, FALSE otherwise */
    *enablePtr = avago_serdes_get_tx_output_enable(aaplSerdesDb[devNum], sbus_addr);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxIfSelect function
* @endinternal
*
* @brief   Tx interface select.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesTxIfNum            - number of serdes Tx interface
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxIfSelect
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    GT_UREG_DATA    serdesTxIfNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    serdesTxIfNum = serdesTxIfNum;
#else

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, serdesTxIfNum, (7 << 3)));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxIfSelectGet function
* @endinternal
*
* @brief   Return the SERDES Tx interface select number.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] serdesTxIfNum            - number of serdes Tx interface
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxIfSelectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_U32      *serdesTxIfNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    *serdesTxIfNum = 0;
#else

    GT_UREG_DATA data;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (7 << 3)));
    data = data >> 3;
    if (data > 7)
    {
        return GT_BAD_PARAM;
    }

    *serdesTxIfNum = data;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneStartExt function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training modes
* @param[in] txTraining               - Tx Training modes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStartExt
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_RX_TRAINING_MODES    rxTraining,
    MV_HWS_TX_TRAINING_MODES    txTraining
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    rxTraining = rxTraining;
    txTraining = txTraining;
#else

    unsigned int sbus_addr;
    Avago_serdes_dfe_tune_t   dfe_state;
    Avago_serdes_pmd_config_t pmd_mode;
    GT_BOOL signalDet = GT_FALSE;

    RETURN_GT_OK_ON_EMULATOR;

    if ((txTraining != IGNORE_TX_TRAINING) && (rxTraining != IGNORE_RX_TRAINING))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Initialize the dfe_state struct with default values */
    avago_serdes_tune_init(aaplSerdesDb[devNum], &dfe_state);
    CHECK_AVAGO_RET_CODE();

    /* Initialize the pmd_mode */
    avago_serdes_pmd_init(aaplSerdesDb[devNum], &pmd_mode);

    switch(rxTraining)
    {
        case IGNORE_RX_TRAINING:
            break;
        case START_CONTINUE_ADAPTIVE_TUNING:
            /* start continues adaptive tuning */
            dfe_state.tune_mode = AVAGO_DFE_START_ADAPTIVE;
            break;
        case STOP_CONTINUE_ADAPTIVE_TUNING:
            /* stop continues adaptive tuning */
            dfe_state.tune_mode = AVAGO_DFE_STOP_ADAPTIVE;
            break;
        case ONE_SHOT_DFE_TUNING:
            if (HWS_DEV_SERDES_TYPE(devNum) != AVAGO_16NM)
            {
                /* Disable TAP1 before iCal */
                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));
            }
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            break;
        case ONE_SHOT_DFE_VSR_TUNING:
            dfe_state.tune_mode = AVAGO_DFE_ICAL;
            dfe_state.dfe_disable = GT_TRUE;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    if ((rxTraining != IGNORE_RX_TRAINING))
    {
        /* signal check - if currently no signal could be detected on the serdes, skippin the rx training */
        CHECK_STATUS(mvHwsAvagoSerdesSignalDetectGet(devNum, portGroup, serdesNum, &signalDet));
        if (signalDet == GT_TRUE)
        {
            mvHwsAvagoAccessLock(devNum);
            /* Launches and halts DFE tuning procedures */
            avago_serdes_tune(aaplSerdesDb[devNum], sbus_addr, &dfe_state);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Training can not be made when no signal is detected\n");
        }
    }

    if (txTraining != IGNORE_TX_TRAINING)
    {
        pmd_mode.sbus_addr = sbus_addr;
        pmd_mode.disable_timeout = TRUE;
        pmd_mode.train_mode = (txTraining == START_TRAINING) ? AVAGO_PMD_TRAIN : AVAGO_PMD_BYPASS;

        if (HWS_DEV_SERDES_TYPE(devNum) != AVAGO_16NM)
        {
            /* Disable TAP1 before PMD training */
            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, TAP1_AVG_DISABLE, NULL));
        }

        /* change IF_Dwell_Shift to 0x1 */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, 0x5C00, NULL));

        mvHwsAvagoAccessLock(devNum);
        /* Launches and halts PMD link training procedure */
        avago_serdes_pmd_train(aaplSerdesDb[devNum], &pmd_mode);
        mvHwsAvagoAccessUnlock(devNum);
        CHECK_AVAGO_RET_CODE();
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training & Rx Training starting.
*         This function calls to the mvHwsAvagoSerdesAutoTuneStartExt, which includes
*         all the functional options.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true (AVAGO_DFE_ICAL) /false)
* @param[in] txTraining               - Tx Training (true (AVAGO_PMD_TRAIN) /false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStart
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     rxTraining,
    GT_BOOL     txTraining
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    rxTraining = rxTraining;
    txTraining = txTraining;
#else

    MV_HWS_RX_TRAINING_MODES    rxMode;
    MV_HWS_TX_TRAINING_MODES    txMode;

    if (txTraining && rxTraining)
    {
        return GT_BAD_PARAM;
    }

    /* for TRUE: set Single time iCal */
    rxMode = (rxTraining == GT_TRUE) ? ONE_SHOT_DFE_TUNING : IGNORE_RX_TRAINING;

    txMode = (txTraining == GT_TRUE) ? START_TRAINING : IGNORE_TX_TRAINING;

    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneStartExt(devNum, portGroup, serdesNum, rxMode, txMode));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesRxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the Rx Training starting.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] rxTraining               - Rx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesRxAutoTuneStart
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     rxTraining
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    rxTraining = rxTraining;
#else

    MV_HWS_RX_TRAINING_MODES    rxMode;

    /* for TRUE: set Single time iCal */
    rxMode = (rxTraining == GT_TRUE) ? ONE_SHOT_DFE_TUNING : IGNORE_RX_TRAINING;

    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneStartExt(devNum, portGroup, serdesNum, rxMode, IGNORE_TX_TRAINING));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStart function
* @endinternal
*
* @brief   Per SERDES control the TX training starting.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txTraining               - Tx Training (true/false)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStart
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_BOOL     txTraining
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    txTraining = txTraining;
#else

    MV_HWS_TX_TRAINING_MODES    txMode;

    /* for TRUE: set the AVAGO_PMD_TRAIN mode, for FALSE: set the AVAGO_PMD_BYPASS mode */
    txMode = (txTraining == GT_TRUE) ? START_TRAINING : STOP_TRAINING;

    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneStartExt(devNum, portGroup, serdesNum, IGNORE_RX_TRAINING, txMode));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneStatus function
* @endinternal
*
* @brief   Per SERDES check the Rx & Tx training status
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneStatus
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    rxStatus = rxStatus;
    txStatus = txStatus;
#else
    GT_U32 i;

    if ((NULL == rxStatus) && (NULL == txStatus))
    {
        return GT_BAD_PARAM;
    }

    if (rxStatus != NULL)
    {
        *rxStatus = TUNE_NOT_COMPLITED;

        for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
        {
            /* Delay in 1ms */
            hwsOsExactDelayPtr(devNum, portGroup, 1);

            CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, rxStatus));
            if (*rxStatus == TUNE_PASS)
                break;
        }

        /* in case training failed or took too long/short */
        if ((*rxStatus != TUNE_PASS) || (i == MV_AVAGO_TRAINING_TIMEOUT))
        {
            *rxStatus = TUNE_FAIL; /* else *rxStatus = TUNE_PASS */
        }
    }

    if (txStatus != NULL)
    {
        *txStatus = TUNE_NOT_COMPLITED;

        for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
        {
            /* Delay in 1ms */
            hwsOsExactDelayPtr(devNum, portGroup, 1);

            CHECK_STATUS(mvHwsAvagoSerdesTxAutoTuneStatusShort(devNum, portGroup, serdesNum, txStatus));
            if (*txStatus == TUNE_PASS)
                break;
        }

        /* in case training failed or took too long/short */
        if ((*txStatus != TUNE_PASS) || (i == MV_AVAGO_TRAINING_TIMEOUT))
        {
            *txStatus = TUNE_FAIL; /* else *txStatus = TUNE_PASS */
        }
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Per SERDES stop the TX training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTxAutoTuneStop
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  serdesNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
#else

    unsigned int sbus_addr;
    Avago_serdes_pmd_config_t pmd_mode;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* Initialize the pmd_mode */
    avago_serdes_pmd_init(aaplSerdesDb[devNum], &pmd_mode);

    pmd_mode.sbus_addr = sbus_addr;
    pmd_mode.disable_timeout = TRUE;
    pmd_mode.train_mode = AVAGO_PMD_RESTART;

    mvHwsAvagoAccessLock(devNum);
    /* stop the PMD link training procedure */
    avago_serdes_pmd_train(aaplSerdesDb[devNum], &pmd_mode);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualCtleConfig function
* @endinternal
*
* @brief   Set the Serdes Manual CTLE config for DFE
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] dcGain                   - DC-Gain value        (rang: 0-255)
* @param[in] lowFrequency             - CTLE Low-Frequency   (rang: 0-15)
* @param[in] highFrequency            - CTLE High-Frequency  (rang: 0-15)
* @param[in] bandWidth                - CTLE Band-width      (rang: 0-15)
* @param[in] loopBandwidth            - CTLE Loop Band-width (rang: 0-15)
* @param[in] squelch                  - Signal OK threshold  (rang: 0-310)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesManualCtleConfig
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     dcGain,
    GT_UOPT     lowFrequency,
    GT_UOPT     highFrequency,
    GT_UOPT     bandWidth,
    GT_UOPT     loopBandwidth,
    GT_UOPT     squelch
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    dcGain = dcGain;
    lowFrequency = lowFrequency;
    highFrequency = highFrequency;
    bandWidth = bandWidth;
    loopBandwidth = loopBandwidth;
    squelch = squelch;
#else

    unsigned int sbus_addr;
    GT_U32  ctleParam=0;
    GT_U32  row;
    GT_U32  column = 2;
    GT_U32  data;

    if (squelch > 308)
    {
        return GT_BAD_PARAM;
    }

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    for (row = 0; row <= 4; row++)
    {
        switch (row)
        {
            case 0:
                ctleParam = highFrequency;
                break;
            case 1:
                ctleParam = lowFrequency;
                break;
            case 2:
                ctleParam = dcGain;
                break;
            case 3:
                ctleParam = bandWidth;
                break;
            case 4:
                ctleParam = loopBandwidth;
                break;
        }

        data = ((column << 12) | (row << 8)) | ctleParam;

        /* Update the param values of DFE */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x26, data, NULL));
    }

    /* formula to convert the milli-volt to fix value */
    squelch = (squelch < 68) ? 68 : squelch;
    squelch = (squelch - 68) / 16;

    /* Set the signal OK threshold on Serdes */
    CHECK_STATUS(mvHwsAvagoSerdesSignalOkCfg(devNum, portGroup, serdesNum, squelch));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesManualCtleConfigGet function
* @endinternal
*
* @brief   Get the Serdes CTLE (RX) configurations
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical lane number
*
* @param[out] configParams             - Ctle params structures:
*                                      dcGain         DC-Gain value        (rang: 0-255)
*                                      lowFrequency   CTLE Low-Frequency   (rang: 0-15)
*                                      highFrequency  CTLE High-Frequency  (rang: 0-15)
*                                      bandWidth      CTLE Band-width      (rang: 0-15)
*                                      loopBandwidth  CTLE Loop Band-width (rang: 0-15)
*                                      squelch        Signal OK threshold  (rang: 0-310)
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* @note The squelch transforms from units 0 - 15 to
*       units 0 - 308 by formula (x 16) + 68. So it's value can be not the same as it was set.
*       For example, was set the squelch = 110. It's transforms and safes inside as
*       integer part of (squelch - 68) / 16 = (integer part)(110 - 68) / 16 = 2
*       And we obtain after getting (2 16) + 68 = 100
*
*/
GT_STATUS mvHwsAvagoSerdesManualCtleConfigGet
(
    GT_U8                               devNum,
    GT_UOPT                             portGroup,
    GT_UOPT                             serdesNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA    *configParams
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    hwsOsMemSetFuncPtr(configParams,0,sizeof(*configParams));
#else

    MV_HWS_AUTO_TUNE_RESULTS    results;
    GT_UOPT                     signalThreshold;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        hwsOsMemSetFuncPtr(configParams,0,sizeof(*configParams));
        /* no SERDESes on emulator */
        return GT_OK;
    }
#endif /* not MV_HWS_REDUCED_BUILD_EXT_CM3*/

    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesNum, &results));

    configParams->highFrequency = results.HF;
    configParams->lowFrequency = results.LF;
    configParams->dcGain = results.DC;
    configParams->bandWidth = results.BW;
    configParams->loopBandwidth = results.LB;

    /* Get the signal OK threshold on Serdes */
    CHECK_STATUS(mvHwsAvagoSerdesSignalOkThresholdGet(devNum, portGroup, serdesNum, &signalThreshold));
    configParams->squelch = (signalThreshold * 16) + 68;

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesSignalOkCfg function
* @endinternal
*
* @brief   Set the signal OK threshold on Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] signalThreshold          - Signal OK threshold (0-15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkCfg
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     signalThreshold
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    signalThreshold = signalThreshold;
#else

    unsigned int sbus_addr;

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    mvHwsAvagoAccessLock(devNum);
    avago_serdes_initialize_signal_ok(aaplSerdesDb[devNum], sbus_addr, signalThreshold);
    mvHwsAvagoAccessUnlock(devNum);

    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesSignalLiveDetectGet function
* @endinternal
*
* @brief   Per SERDES get live indication check CDR lock and Signal Detect.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalDet                - TRUE if signal detected and FALSE otherwise.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalLiveDetectGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalDet
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    signalDet = signalDet;
#else

    unsigned int sbus_addr;
    GT_U32 i;

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    /* check CDR lock and Signal Detect on Serdes */
    mvHwsAvagoAccessLock(devNum);
    for(i=0; i < 10; i++)
    {
        if(FALSE == avago_serdes_get_signal_ok_live(aaplSerdesDb[devNum], sbus_addr))
        {
            mvHwsAvagoAccessUnlock(devNum);
            *signalDet = GT_FALSE;
            return GT_OK;
        }
    }
    mvHwsAvagoAccessUnlock(devNum);

    *signalDet = GT_TRUE;
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesSignalOkThresholdGet function
* @endinternal
*
* @brief   Get the signal OK threshold on Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] signalThreshold          - Signal OK threshold (0-15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesSignalOkThresholdGet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     *signalThreshold
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    *signalThreshold = 0;
#else

    unsigned int sbus_addr;

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    mvHwsAvagoAccessLock(devNum);
    *signalThreshold = avago_serdes_get_signal_ok_threshold(aaplSerdesDb[devNum], sbus_addr);
    mvHwsAvagoAccessUnlock(devNum);

    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX or TX data
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfBits                - Number of error bits to inject (max=65535)
* @param[in] serdesDirection          - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesErrorInject
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     numOfBits,
    MV_HWS_SERDES_DIRECTION    serdesDirection
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    numOfBits = numOfBits;
    serdesDirection = serdesDirection;
#else

    unsigned int sbus_addr;

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    mvHwsAvagoAccessLock(devNum);
    (serdesDirection == RX_DIRECTION) ? avago_serdes_rx_inject_error(aaplSerdesDb[devNum], sbus_addr, numOfBits)
                                      : avago_serdes_tx_inject_error(aaplSerdesDb[devNum], sbus_addr, numOfBits);
    mvHwsAvagoAccessUnlock(devNum);

    CHECK_AVAGO_RET_CODE();
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesShiftSamplePoint function
* @endinternal
*
* @brief   Shift Serdes sampling point earlier in time
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] delay                    - set the  (0...31)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesShiftSamplePoint
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_U32      delay
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    delay = delay;
#else
    int y;
    int intValue;

    RETURN_GT_OK_ON_EMULATOR;

    y = (delay - 15);
    intValue = (y |(y << 4));

    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x402e, NULL));
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, intValue, NULL));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesArrayAutoTuneSet function
* @endinternal
*
* @brief   Set iCAL(CTLE and DFE) or pCAL(DFE) Auto Tuning on multiple Serdeses
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] trainingMode             - for value 0: set iCAL mode,
*                                      for value 1: set pCAL mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesArrayAutoTuneSet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_BOOL     trainingMode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum       = devNum;
    portGroup    = portGroup;
    serdesArr    = serdesArr;
    numOfSerdes  = numOfSerdes;
    trainingMode = trainingMode;
#else

    GT_U32  i, k, j;
    GT_U32  serdesArr_copy[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_U32  passCounter = 0;
    GT_U32  numOfActiveSerdes = 0;
    GT_BOOL signalDet = GT_FALSE;
    MV_HWS_AUTO_TUNE_STATUS     rxStatus;
    MV_HWS_DFE_MODE             dfeMode;

    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArr_copy, serdesArr, sizeof(GT_U32) * numOfSerdes);

    dfeMode = (trainingMode) ? DFE_PCAL : DFE_ICAL;

    for (j=0; j < numOfSerdes; j++)
    {
        if (serdesArr_copy[j] == SKIP_SERDES) /* skip Serdes[j] if No Signal-Detect */
            continue;

        /* signal check - if currently no signal could be detected on the serdes, skippin the training */
        CHECK_STATUS(mvHwsAvagoSerdesSignalDetectGet(devNum, portGroup, serdesArr_copy[j], &signalDet));
        if (signalDet == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Training can not be made when no signal is detected\n");
        }

        numOfActiveSerdes++;

        /* run iCAL(CTLE and DFE) or pCAL(DFE) */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArr_copy[j], dfeMode, NULL));
    }

    if (numOfActiveSerdes == 0)
    {
#ifndef MICRO_INIT
        HWS_AVAGO_DBG(("No need to run Training on Serdes\n"));
#endif
        return GT_OK;
    }

    for (k = 0; k < MV_AVAGO_TRAINING_TIMEOUT; k++)
    {
        for (i=0; i < numOfSerdes; i++)
        {
            if (serdesArr_copy[i] == SKIP_SERDES) /* if Serdes pass then no need to check status on it */
                continue;

            /* check that training completed */
            CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesArr_copy[i], &rxStatus));
            if (rxStatus == TUNE_PASS)
            {
                passCounter++;

                /* Stop traning if Timeout reached or if traning finished too fast */
                if ((k >= MV_AVAGO_TRAINING_TIMEOUT-1) || (k < 10))
                {
                    /* stop the training */
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr_copy[i], 0xa, 0x0, NULL));
                    HWS_AVAGO_DBG(("Error: Training failed. Stopped training k=%d\n", k));
                    return GT_TIMEOUT;
                }

                serdesArr_copy[i] = SKIP_SERDES;
            }
            else if ((dfeMode == DFE_ICAL) && (rxStatus == TUNE_FAIL))
            {
                serdesArr_copy[i] = SKIP_SERDES;
                return GT_FAIL;
            }
        }

        /* when all Serdeses finish, then stop the Timeout loop */
        if (passCounter == numOfActiveSerdes)
            break;
        else
            hwsOsExactDelayPtr(devNum, portGroup, 1); /* Delay of 1ms */
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}


#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

GT_STATUS mvHwsAvagoSerdesArrayAutoTuneSetByPhase
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_BOOL     trainingMode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum       = devNum;
    portGroup    = portGroup;
    serdesArr    = serdesArr;
    numOfSerdes  = numOfSerdes;
    trainingMode = trainingMode;
#else

    GT_U32  j;
    GT_U32  serdesArr_copy[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_U32  numOfActiveSerdes = 0;
    GT_BOOL signalDet = GT_FALSE;


    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArr_copy, serdesArr, sizeof(GT_U32) * numOfSerdes);

    trainingMode = (trainingMode) ? DFE_PCAL : DFE_ICAL;

    for (j=0; j < numOfSerdes; j++)
    {
        if (serdesArr_copy[j] == SKIP_SERDES) /* skip Serdes[j] if No Signal-Detect */
            continue;

        /* signal check - if currently no signal could be detected on the serdes, skippin the training */
        CHECK_STATUS(mvHwsAvagoSerdesSignalDetectGet(devNum, portGroup, serdesArr_copy[j], &signalDet));
        if (signalDet == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Training can not be made when no signal is detected\n");
        }

        numOfActiveSerdes++;

        /* run iCAL(CTLE and DFE) or pCAL(DFE) */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArr_copy[j], trainingMode, NULL));
    }

    if (numOfActiveSerdes == 0)
    {
#ifndef MICRO_INIT
        HWS_AVAGO_DBG(("No need to run Training on Serdes\n"));
#endif
        return GT_OK;
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of enhance-tune-by-phase algorithm used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhaceTuneByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    phyPortNum = phyPortNum;
#else
    GT_U32 i;

    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_CurrentDelaySize = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_CurrentDelayPtr = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_InOutI = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_InOutKk = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->subPhase = 0;
    hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->waitForCoreReady = GT_FALSE;

    for (i=0; i<MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_Best_LF[i] = 0;
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_Best_eye[i] = 0;
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_Best_dly[i] = 0;
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->enhanceTuneStcPtr->phase_LF1_Arr[i]  = 0;
    }
#endif

    return GT_OK;
}
/**
* @internal mvHwsAvagoSerdesOneShotTuneByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of one shot -tune-by-phase algorithm for
* KR2/CR2 mode, used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesOneShotTuneByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  phyPortNum
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    phyPortNum = phyPortNum;
#else
    GT_U32 i;
    for (i=0; i<MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->oneShotTuneStcPtr->coreStatusReady[i] = GT_FALSE;
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesPrePostTuningByPhase function
* @endinternal
*
* @brief   reset/unreset CG_UNIT Rx GearBox Register, and clock
*          from SerDes to GearBox before running iCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] resetEnable              - reset True/False.
* @param[in] fecCorrect               - FEC type.
* @param[in] port                     - Port Mode.
* @param[out] coreStatusReady                 -  array that
*       indicates if the core status of all lanes is ready
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesPrePostTuningByPhase
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      phyPortNum,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_BOOL     resetEnable,
    GT_BOOL     *coreStatusReady,
    MV_HWS_PORT_FEC_MODE  fecCorrect,
    MV_HWS_PORT_STANDARD  portMode
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    serdesArr = serdesArr;
    numOfSerdes = numOfSerdes;
    resetEnable = resetEnable;
    coreStatusReady = coreStatusReady;
    fecCorrect = fecCorrect;
    portMode = portMode;
#else
    GT_U32                      i, regData;
    GT_BOOL  gearBoxRegAccess = GT_TRUE;

    /* 50/40G/52_5G KR2 NO FEC workaround */
    if((HWS_DEV_SILICON_TYPE(devNum) == Aldrin2) || (HWS_DEV_SILICON_TYPE(devNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(devNum) == Pipe))
    {
        if((fecCorrect == FEC_OFF) &&
          ((portMode == _50GBase_KR2) || (portMode == _40GBase_KR2) || (portMode == _50GBase_SR2) || (portMode == _50GBase_CR2) || (portMode == _52_5GBase_KR2)))
        {
            if(resetEnable == GT_FALSE)
            {
                for (i = 0; i < numOfSerdes; i++)
                {
                    if (coreStatusReady[i])
                    {
                        continue;
                    }
                    else
                    {
                        /* Poll for o_core_status[4] == 1 */
                        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, SERDES_UNIT, (serdesArr[i] & 0xFFFF), AVAGO_CORE_STATUS, &regData, 0));
                        if(((regData >> 4) & 1) != 1)
                        {
                            {
                                gearBoxRegAccess = GT_FALSE;
                                coreStatusReady[i] = GT_FALSE;
                                continue;
                            }
                        }
                        else
                        {
                            coreStatusReady[i] = GT_TRUE;
                        }
                    }
                }
            }

            if (gearBoxRegAccess == GT_TRUE)
            {
                regData = (resetEnable == GT_TRUE) ? 0 : 1;
                /* Disable rx_tlat - stop clock from SERDES to GearBox */
                for (i = 0; i < numOfSerdes; i++)
                {
                    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, (serdesArr[i] & 0xFFFF), SERDES_EXTERNAL_CONFIGURATION_0, (regData << 13), (1 << 13)));
                }

                /* Reset sd_rx_reset_ - GearBox RX reset*/
                if(phyPortNum % 4 == 0)
                {
                    regData = (resetEnable == GT_TRUE) ? 0x0 : 0x3;
                    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_RESETS, regData, 0x3));
                }
                else if(phyPortNum % 2 == 0)
                {
                    regData = (resetEnable == GT_TRUE) ? 0x0 : 0xC;
                    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, CG_UNIT, (phyPortNum & HWS_2_LSB_MASK_CNS), CG_RESETS, regData, 0xC));
                }
                else
                {
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("illegal port num for 50G/40G WA"));
                }
            }
        }
    }
#endif
    return GT_OK;
}


/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase1 function
* @endinternal
*
* @brief   Move sample point and launch iCal. this is phase1 of the phased enhance-tune
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] currentDelayPtr          - delays attay to set initial values with
* @param[in] currentDelaySize         - delay array size
* @param[in] best_eye                 -  array to set initial values with
*
* @param[out] currentDelayPtr          - delays attay to set initial values with
* @param[out] currentDelaySize         - delay array size
* @param[out] best_eye                 -  array to set initial values with
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase1
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U32      **currentDelayPtr,
    GT_U32      *currentDelaySize,
    GT_U32      *best_eye
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesArr = serdesArr;
    numOfSerdes = numOfSerdes;
    currentDelayPtr = currentDelayPtr;
    currentDelaySize = currentDelaySize;
    best_eye = best_eye;
#else

    unsigned int sbus_addr;
    GT_U32  sdDetect;
    GT_U32  i;
    GT_U32  serdesArrValid[MAX_AVAGO_SERDES_NUMBER] = {0};
    MV_HWS_SERDES_SPEED rate;


    for (i=0; i < MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        best_eye[i] = 4;
    }

    if (numOfSerdes > MAX_AVAGO_SERDES_NUMBER)
    {
        HWS_AVAGO_DBG(("numOfSerdes %d is greater than MAX_AVAGO_SERDES_NUMBER %d\n", numOfSerdes, MAX_AVAGO_SERDES_NUMBER));
        return GT_BAD_PARAM;
    }

    if(HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3)
    {
        CHECK_STATUS(mvHwsAvagoSerdeSpeedGet(devNum, portGroup, serdesArr[0], &rate));
        if(rate >= _20_625G)
        {
            *currentDelayPtr = static_delay_25G;
            *currentDelaySize = sizeof(static_delay_25G)/sizeof(GT_U32);
        }
        else
        {
            *currentDelayPtr = static_delay_10G;
            *currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
        }
    }
    else
    {
        *currentDelayPtr = static_delay_10G;
        *currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 1 ## \n");
    /* check CDR lock and Signal Detect on all Serdeses */
    for (i=0; i < numOfSerdes; i++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 2 ## serdesNum=%d\n", MV_HWS_SERDES_NUM(serdesArr[i]));
        CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, MV_HWS_SERDES_NUM(serdesArr[i]), &sbus_addr));

        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, MV_HWS_SERDES_NUM(serdesArr[i]),
                                            AVAGO_EXTERNAL_STATUS, &sdDetect, (1 << 2)));
        if (sdDetect != 0)
        {
            serdesArr[i] = SKIP_SERDES; /* change the Serdes number to mark this array's element as not relevant */
            continue;
        }

        /* set SamplePoint to default value (delay=0) */
        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[i], (*currentDelayPtr)[0]) );
    }

    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 3 ## \n");
    /* run iCAL(CTLE and DFE) */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSetByPhase(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase2 function
* @endinternal
*
* @brief   Getting LF values from serdeses. this is phase2 of the phased enhance-tune
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_LF                  - array to be filled with LF values from serdes
*
* @param[out] best_LF                  - array to be filled with LF values from serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase2
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U32      *best_LF
)
{

#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    serdesArr = serdesArr;
    numOfSerdes = numOfSerdes;
    best_LF = best_LF;
#else
    int LF;
    GT_U8 k;
    /* save the LF value of all Serdeses */
    for (k=0; k < numOfSerdes; k++)
    {
        if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
            continue;

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 4 ## serdesNum=%d\n", serdesArr[k]);
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x126, ((2 << 12) | (1 << 8)), &LF));
        best_LF[k]=LF;
    }
#endif
    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 5 ## \n");
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase3 function
* @endinternal
*
* @brief   Move sample point, then launch pCal than calculate eye. this is phase3
*         of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] currentDelaySize         - delays array size
* @param[in] currentDelayPtr          - delays array
* @param[in] inOutI                   - iteration index between different phases
* @param[in] best_LF                  - best LF array to update
* @param[in] best_eye                 - best eyes array to update
* @param[in] best_dly                 - best delays array to update
* @param[in] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @param[out] inOutI                   - iteration index between different phases
* @param[out] best_LF                  - best LF array to update
* @param[out] best_eye                 - best eyes array to update
* @param[out] best_dly                 - best delays array to update
* @param[out] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase3
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U32      currentDelaySize,
    GT_U32      *currentDelayPtr,
    GT_U32      *inOutI,
    GT_U32      *best_LF,
    GT_U32      *best_eye,
    GT_U32      *best_dly,
    GT_U8       *subPhase
)
{

#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    serdesArr = serdesArr;
    numOfSerdes = numOfSerdes;
    currentDelaySize = currentDelaySize;
    currentDelayPtr = currentDelayPtr;
    inOutI = inOutI;
    best_LF = best_LF;
    best_eye = best_eye;
    best_dly = best_dly;
    subPhase = subPhase;
#else
    GT_U32  eye;
    int LF;
    GT_U32 i,k, j;
    unsigned int sbus_addr;
    GT_U32      dly;

    /* find the peak of the eye accoding to delay */
    for (i=(*inOutI); i < currentDelaySize; i++)
    {
        if (*subPhase == 0)
        {
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 6 ## \n");
            dly = currentDelayPtr[i];

            for (k=0; k < numOfSerdes; k++)
            {
                AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 7 ## serdesNum=%d\n", serdesArr[k]);
                if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                    continue;

                CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], dly));
            }

            /* trigger pCAL(DFE) on all relevant Serdeses */
            CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSetByPhase(devNum, portGroup, serdesArr, numOfSerdes, PCAL_TRAINING_MODE));
            *subPhase = 1;
            break;
        }
        else
        {
            /**********
              ************
              ***********
              ****************/

            for (j=0; j < numOfSerdes; j++)
            {
                dly = currentDelayPtr[i];

                if (serdesArr[j] == SKIP_SERDES)
                    continue;

                AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 8 ## serdesNum=%d\n", serdesArr[j]);
                CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[j], &sbus_addr));

                mvHwsAvagoAccessLock(devNum);

                eye = avago_serdes_eye_get_simple_metric(aaplSerdesDb[devNum], sbus_addr);
                mvHwsAvagoAccessUnlock(devNum);
                CHECK_AVAGO_RET_CODE();

                CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[j], 0x126, ((2 << 12) | (1 << 8)), &LF));

                if (eye > (best_eye[j] + MV_AVAGO_EYE_TAP))
                {
                    best_eye[j] = eye;
                    best_dly[j] = dly;

                    if (LF >= best_LF[j])
                        best_LF[j] = LF;
                }
    #ifndef MICRO_INIT
                AVAGO_DBG(("current_eye[%d]=%d dly[%d]=%d\n", j, eye, j, dly));
    #endif
                if (best_eye[j] < MV_AVAGO_MIN_EYE_SIZE)
                {
                    HWS_AVAGO_DBG(("Error on Serdes %d: Eye is too small (size is %d). Training Failed\n", serdesArr[j], best_eye[j]));
                    return GT_FAIL;
                }
            }
            /**********
            ************
            ***********
            ****************/
            *subPhase = 0;
            /*
            i++;
            break;*/
        }
    }

    if (i<currentDelaySize)
    {
        (*inOutI) = i;
    }
    else
    {
        (*inOutI) = 0xFFFFFFFF;
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase4 function
* @endinternal
*
* @brief   Move sample point to the best delay value that was found in previous phase
*         then launch iCal. this is phase4 of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_dly                 - best delay to set the serdes with
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase4
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U32      *best_dly
)
{
#ifdef ASIC_SIMULATION

    devNum      =devNum     ;
    portGroup   =portGroup  ;
    serdesArr   =serdesArr  ;
    numOfSerdes =numOfSerdes;
    best_dly    =best_dly   ;
#else
    GT_U32 k;

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 9 ## \n");
    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 10 ## serdesNum=%d\n", serdesArr[k]);
        if (serdesArr[k] == SKIP_SERDES) /* skip Serdes if the dly is the same as the best_dly */
            continue;

        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], best_dly[k]));
    }

    /* run iCAL(CTLE and DFE) with best delay */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSetByPhase(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));
#endif
    return GT_OK;

}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase5 function
* @endinternal
*
* @brief   Get LF values from serdes then launch iCal. this is phase5 of the phased
*         enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] LF1_Arr                  - LF array to fill with values
*
* @param[out] LF1_Arr                  - LF array to fill with values
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase5
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    int         *LF1_Arr
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    serdesArr = serdesArr;
    numOfSerdes = numOfSerdes;
    LF1_Arr = LF1_Arr;
#else
    GT_U32 k;
    GT_U32  serdesArrValid[MAX_AVAGO_SERDES_NUMBER] = {0};

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 11 ## \n");
    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 12 ## serdesNum=%d\n", serdesArrValid[k]);
        /* read the LF value */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((2 << 12) | (1 << 8)), &LF1_Arr[k]));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 13 ## \n");
    /* run iCAL(CTLE and DFE) with best delay */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSetByPhase(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTunePhase6 function
* @endinternal
*
* @brief   Read LF and Gain from serdes after iCal was launched in previous phase,
*         than calculate best LF and Gain, write to serdes and launch pCal.
*         this is phase6 of the phased enhance-tune algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] LF1_Arr                  - LF array to fill with values
* @param[in] min_LF                   - minimum LF for reference
* @param[in] max_LF                   - maimum LF for reference
* @param[in] LF1_Arr                  - previous LF values for reference
* @param[in] best_eye                 - besy eye for printing purpose
* @param[in] best_dly                 - besy delay for printing purpose
* @param[in] inOutKk                  - iterator index
* @param[in] continueExecute          - whether or not to continue algorithm execution
*
* @param[out] inOutKk                  - iterator index
* @param[out] continueExecute          - whether or not to continue algorithm execution
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTunePhase6
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U8       min_LF,
    GT_U8       max_LF,
    int         *LF1_Arr,
    GT_U32      *best_eye,
    GT_U32      *best_dly,
    GT_U8       *inOutKk,
    GT_BOOL     *continueExecute
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    portGroup = portGroup;
    serdesArr = serdesArr;
    numOfSerdes = numOfSerdes;
    min_LF = min_LF;
    max_LF = max_LF;
    LF1_Arr = LF1_Arr;
    best_eye = best_eye;
    best_dly = best_dly;
    inOutKk = inOutKk;
    continueExecute = continueExecute;
#else
    GT_U32  serdesArrValid[MAX_AVAGO_SERDES_NUMBER] = {0};
    int LF2_Arr[MAX_AVAGO_SERDES_NUMBER]  = {0};
    int DFEgain_Arr[MAX_AVAGO_SERDES_NUMBER]    = {0};
    unsigned int LF, GAIN;
    GT_U32 k;

    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    for (k=*inOutKk; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 14 ## serdesNum=%d\n", serdesArrValid[k]);
        /* read again the LF value after run the iCAL */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((2 << 12) | (1 << 8)), &LF2_Arr[k]));

        /* read the DFE gain */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((3 << 12) | (0 << 8)), &DFEgain_Arr[k]));

        LF = (LF1_Arr[k] + LF2_Arr[k])/2; /* take the avarge from two tunning values */
        LF = (LF < min_LF) ? min_LF : LF;
        LF = (LF > max_LF) ? max_LF : LF;

        /* write the selected LF value back to Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x26, ((2 << 12) | (1 << 8) | LF), NULL));

        GAIN = (DFEgain_Arr[k] > 1) ? (DFEgain_Arr[k]-1) : DFEgain_Arr[k];
        /* write the selected GAIN value back to Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x26, ((3 << 12) | (0 << 8) | GAIN), NULL));
#ifndef MICRO_INIT
        AVAGO_DBG(("Serdes %d: setting best_dly=%d best_eye=%d LF=%d GAIN=%d\n", serdesArrValid[k], best_dly[k], best_eye[k], LF, GAIN));
#endif

        /* run  pCAL(DFE) */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup,  serdesArrValid[k], 0xa, 2, NULL));

        /*k++;
        break;*/
    }

    if (k<numOfSerdes)
    {
        *continueExecute = GT_TRUE;
    }
    else
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 15 ## \n");
        *continueExecute = GT_FALSE;
    }
#endif
    return GT_OK;
}

#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT))
/**
* @internal mvHwsAvagoSerdesEnhanceTune function
* @endinternal
*
* @brief   Set the ICAL with shifted sampling point to find best sampling point
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] min_LF                   - Minimum LF value that can be set on Serdes (0...15)
* @param[in] max_LF                   - Maximum LF value that can be set on Serdes (0...15)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTune
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U8       min_LF,
    GT_U8       max_LF
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesArr = serdesArr;
    numOfSerdes = numOfSerdes;
    min_LF = min_LF;
    max_LF = max_LF;
#else

    unsigned int sbus_addr;
    GT_U32  dly, eye;
    GT_U32  sdDetect;
    GT_U32  i, j, k;
    int     LF, GAIN;
    GT_U32  best_dly[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_U32  best_eye[MAX_AVAGO_SERDES_NUMBER] = {0};
    GT_32   best_LF[MAX_AVAGO_SERDES_NUMBER]  = {0};
    int     LF1_Arr[MAX_AVAGO_SERDES_NUMBER]  = {0};
    int     LF2_Arr[MAX_AVAGO_SERDES_NUMBER]  = {0};
    int     DFEgain_Arr[MAX_AVAGO_SERDES_NUMBER]    = {0};
    GT_U32  serdesArrValid[MAX_AVAGO_SERDES_NUMBER] = {0};
    MV_HWS_SERDES_SPEED rate;
    GT_U32 *currentDelayPtr;
    GT_U32 currentDelaySize;

    EXEC_TIME_START();

    RETURN_GT_OK_ON_EMULATOR;

    for (i=0; i < MAX_AVAGO_SERDES_NUMBER; i++)
    {
        best_eye[i] = 4;
    }

    if (numOfSerdes > MAX_AVAGO_SERDES_NUMBER)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT("numOfSerdes %d is greater than MAX_AVAGO_SERDES_NUMBER %d\n", numOfSerdes, MAX_AVAGO_SERDES_NUMBER);
        return GT_BAD_PARAM;
    }

    if(HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3)
    {
        CHECK_STATUS(mvHwsAvagoSerdeSpeedGet(devNum, portGroup, serdesArr[0], &rate));
        if(rate >= _20_625G)
        {
            currentDelayPtr = static_delay_25G;
            currentDelaySize = sizeof(static_delay_25G)/sizeof(GT_U32);
        }
        else
        {
            currentDelayPtr = static_delay_10G;
            currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
        }
    }
    else
    {
        currentDelayPtr = static_delay_10G;
        currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 1 ## \n");

    /* check CDR lock and Signal Detect on all Serdeses */
    for (i=0; i < numOfSerdes; i++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 2 ## serdesNum=%d\n", MV_HWS_SERDES_NUM(serdesArr[i]));

        CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, MV_HWS_SERDES_NUM(serdesArr[i]), &sbus_addr));

        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, MV_HWS_SERDES_NUM(serdesArr[i]),
                                            AVAGO_EXTERNAL_STATUS, &sdDetect, (1 << 2)));
        if (sdDetect != 0)
        {
            serdesArr[i] = SKIP_SERDES; /* change the Serdes number to mark this array's element as not relevant */
            continue;
        }

        /* set SamplePoint to default value (delay=0) */
        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[i], currentDelayPtr[0]));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 3 ## \n");
    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    /* run iCAL(CTLE and DFE) */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));

    /* save the LF value of all Serdeses */
    for (k=0; k < numOfSerdes; k++)
    {
        if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
            continue;

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 4 ## serdesNum=%d\n", serdesArr[k]);
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x126, ((2 << 12) | (1 << 8)), &LF));
        best_LF[k]=LF;
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 5 ## \n");
    /* find the peak of the eye accoding to delay */
    for (i=0; i < currentDelaySize; i++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 6 ## \n");
        dly = currentDelayPtr[i];

        for (k=0; k < numOfSerdes; k++)
        {
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 7 ## serdesNum=%d\n", serdesArr[k]);
            if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                continue;

            CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], dly));
        }

        /* trigger pCAL(DFE) on all relevant Serdeses */
        CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, PCAL_TRAINING_MODE));

        for (j=0; j < numOfSerdes; j++)
        {
            if (serdesArr[j] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                continue;

            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 8 ## serdesNum=%d\n", serdesArr[j]);
            CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[j], &sbus_addr));

            mvHwsAvagoAccessLock(devNum);
            /* get a simple eye metric in range [0..1000] */
            eye = avago_serdes_eye_get_simple_metric(aaplSerdesDb[devNum], sbus_addr);
            mvHwsAvagoAccessUnlock(devNum);
            CHECK_AVAGO_RET_CODE();

            CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[j], 0x126, ((2 << 12) | (1 << 8)), &LF));

            if (eye > (best_eye[j] + MV_AVAGO_EYE_TAP))
            {
                best_eye[j] = eye;
                best_dly[j] = dly;

                if (LF >= best_LF[j])
                    best_LF[j] = LF;
            }
#ifndef MICRO_INIT
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT("current_eye[%d]=%d dly[%d]=%d\n", j, eye, j, dly);
#endif
            if (best_eye[j] < MV_AVAGO_MIN_EYE_SIZE)
            {
                AVAGO_TUNE_BY_PHASE_DEBUG_PRINT("Error on Serdes %d: Eye is too small (size is %d). Training Failed\n", serdesArr[j], best_eye[j]);
                return GT_FAIL;
            }
        }
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 9 ## \n");
    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 10 ## serdesNum=%d\n", serdesArr[k]);
        if (serdesArr[k] == SKIP_SERDES) /* skip Serdes if the dly is the same as the best_dly */
            continue;

        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], best_dly[k]));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 11 ## \n");
    /* run iCAL(CTLE and DFE) with best delay */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));

    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 12 ## serdesNum=%d\n", serdesArrValid[k]);
        /* read the LF value */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((2 << 12) | (1 << 8)), &LF1_Arr[k]));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 13 ## \n");
    /* run iCAL(CTLE and DFE) with best delay */
    CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, ICAL_TRAINING_MODE));

    for (k=0; k < numOfSerdes; k++)
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 14 ## serdesNum=%d\n", serdesArrValid[k]);
        /* read again the LF value after run the iCAL */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((2 << 12) | (1 << 8)), &LF2_Arr[k]));

        /* read the DFE gain */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x126, ((3 << 12) | (0 << 8)), &DFEgain_Arr[k]));

        LF = (LF1_Arr[k] + LF2_Arr[k])/2; /* take the avarge from two tunning values */
        LF = (LF < min_LF) ? min_LF : LF;
        LF = (LF > max_LF) ? max_LF : LF;

        /* write the selected LF value back to Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x26, ((2 << 12) | (1 << 8) | LF), NULL));

        GAIN = (DFEgain_Arr[k] > 1) ? (DFEgain_Arr[k]-1) : DFEgain_Arr[k];
        /* write the selected GAIN value back to Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArrValid[k], 0x26, ((3 << 12) | (0 << 8) | GAIN), NULL));

#ifndef MICRO_INIT
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT("Serdes %d: setting best_dly=%d best_eye=%d LF=%d GAIN=%d\n", serdesArrValid[k], best_dly[k], best_eye[k], LF, GAIN);
#endif
        /* run pCAL(DFE) */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArrValid[k], DFE_PCAL, NULL));
    }

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" enhanceTune ## 15 ## \n");

    EXEC_TIME_END_PRINT("mvHwsAvagoSerdesEnhanceTune");

#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif /* !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(MICRO_INIT)) */

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsAvagoSerdesEnhanceTuneLite function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] serdesSpeed              - speed of serdes
* @param[in] min_dly                  - Minimum delay_cal value:
*                                      - for Serdes speed 10G (25...28)
*                                      - for Serdes speed 25G (15...19)
* @param[in] max_dly                  - Maximum delay_cal value:
*                                      - for Serdes speed 10G (30)
*                                      - for Serdes speed 25G (21)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLite
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    MV_HWS_SERDES_SPEED     serdesSpeed,
    GT_U8                   min_dly,
    GT_U8                   max_dly
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesArr = serdesArr;
    numOfSerdes = numOfSerdes;
    serdesSpeed = serdesSpeed;
    min_dly = min_dly;
    max_dly = max_dly;
    serdesSpeed = serdesSpeed;

#else

    unsigned int sbus_addr;
    GT_U32  dly, eye;
    MV_HWS_AUTO_TUNE_STATUS  sdDetect;
    GT_U32  i, j, k;
    GT_U32  best_dly[HWS_MAX_PORT_AVAGO_SERDES_NUM] = {0};
    GT_U32  best_eye[HWS_MAX_PORT_AVAGO_SERDES_NUM] = {0};
    GT_U32  serdesArrValid[HWS_MAX_PORT_AVAGO_SERDES_NUM] = {0};
    GT_U32 *currentDelayPtr;
    GT_U32 currentDelaySize;
    GT_U32 dwell;

    RETURN_GT_OK_ON_EMULATOR;

    if (numOfSerdes > HWS_MAX_PORT_AVAGO_SERDES_NUM)
    {
        HWS_AVAGO_DBG(("numOfSerdes %d is greater than MAX_AVAGO_SERDES_NUMBER %d\n", numOfSerdes, 4));
        return GT_BAD_PARAM;
    }

    for (i=0; i < numOfSerdes; i++)
    {
        best_eye[i] = 4;
    }

    /* define dwell value */
    if (avagoBaudRate2DwellTimeValue[serdesSpeed] != NA_VALUE)
    {
        dwell = avagoBaudRate2DwellTimeValue[serdesSpeed];
    }
    else
    {
        dwell = 11;
    }

    if(HWS_DEV_SILICON_TYPE(devNum) >= Bobcat3)
    {
        if(serdesSpeed >= _20_625G)
        {
            currentDelayPtr = static_delay_25G;
            currentDelaySize = sizeof(static_delay_25G)/sizeof(GT_U32);
        }
        else
        {
            currentDelayPtr = static_delay_10G;
            currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
        }
    }
    else
    {
        currentDelayPtr = static_delay_10G;
        currentDelaySize = sizeof(static_delay_10G)/sizeof(GT_U32);
    }

    for (i=0; i < numOfSerdes; i++)
    {
        CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, MV_HWS_SERDES_NUM(serdesArr[i]), &sbus_addr));

        /* check CDR lock and Signal Detect on Serdes */
        CHECK_STATUS(mvHwsAvagoSerdesRxSignalCheck(devNum, portGroup, MV_HWS_SERDES_NUM(serdesArr[i]), &sdDetect));
        if (sdDetect == TUNE_NOT_READY)
        {
            serdesArr[i] = SKIP_SERDES; /* change the Serdes number to mark this array's element as not relevant */
            continue;
        }

        /* select DFE tuning dwell time */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x18, 0x7, NULL));
        /* big register write LSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 1 << dwell, NULL));
        /* big register write MSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[i], 0x19, 0, NULL));
    }

    /* copy the original Serdes array for saving the data */
    hwsOsMemCopyFuncPtr(serdesArrValid, serdesArr, sizeof(GT_U32) * numOfSerdes);

    /* find the peak of the eye according to delay */
    for (i=0; i < currentDelaySize; i++)
    {
        dly = currentDelayPtr[i];

        if(dly >= min_dly && dly <= max_dly)
        {

            for (k=0; k < numOfSerdes; k++)
            {
                if (serdesArr[k] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                    continue;

                CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], dly));
            }

            /* trigger pCAL(DFE) on all relevant Serdeses */
            CHECK_STATUS(mvHwsAvagoSerdesArrayAutoTuneSet(devNum, portGroup, serdesArr, numOfSerdes, PCAL_TRAINING_MODE));

            for (j=0; j < numOfSerdes; j++)
            {
                if (serdesArr[j] == SKIP_SERDES) /* No Signal-Detect on this Serdes */
                    continue;

                CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[j], &sbus_addr));

                mvHwsAvagoAccessLock(devNum);
                /* get a simple eye metric in range [0..1000] */
                eye = avago_serdes_eye_get_simple_metric(aaplSerdesDb[devNum], sbus_addr);
                mvHwsAvagoAccessUnlock(devNum);
                CHECK_AVAGO_RET_CODE();

                if (eye > (best_eye[j] + MV_AVAGO_EYE_TAP))
                {
                    best_eye[j] = eye;
                    best_dly[j] = dly;
                }
                if (best_eye[j] < MV_AVAGO_MIN_EYE_SIZE)
                {
                    HWS_AVAGO_DBG(("Error on Serdes %d: Eye is too small (size is %d). Training Failed\n", serdesArr[j], best_eye[j]));
                    return GT_FAIL;
                }
            }
        }
    }

    for (k=0; k < numOfSerdes; k++)
    {
        if (serdesArr[k] == SKIP_SERDES) /* skip Serdes if the dly is the same as the best_dly */
            continue;

        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], best_dly[k]));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

/**
* @internal mvHwsAvagoSerdesEnhanceTuneLitePhase1 function
* @endinternal
*
* @brief   Set the PCAL with shifted sampling point to find best sampling point.
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] serdesSpeed              - speed of serdes
* @param[in] currentDelaySize         - delays array size
* @param[in] currentDelayPtr          - delays array
* @param[in] inOutI                   - iteration index between different phases
* @param[in] best_eye                 - best eyes array to update
* @param[in] best_dly                 - best delays array to update
* @param[in] subPhase                 - assist flag to know which code to execute in this sub-phase
* @param[in] min_dly                  - Minimum delay_cal value: (rang: 0-31)
* @param[in] max_dly                  - Maximum delay_cal value: (rang: 0-31)
*
* @param[out] inOutI                   - iteration index between different phases
* @param[out] best_eye                 - best eyes array to update
* @param[out] best_dly                 - best delays array to update
* @param[out] subPhase                 - assist flag to know which code to execute in this sub-phase
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLitePhase1
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    MV_HWS_SERDES_SPEED     serdesSpeed,
    GT_U8       currentDelaySize,
    GT_U8       *currentDelayPtr,
    GT_U8       *inOutI,
    GT_U16      *best_eye,
    GT_U8       *best_dly,
    GT_U8       *subPhase,
    GT_U8       min_dly,
    GT_U8       max_dly
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum    = devNum;
    portGroup = portGroup;
    serdesArr   = serdesArr;
    numOfSerdes = numOfSerdes;
    serdesSpeed = serdesSpeed;
    currentDelaySize = currentDelaySize;
    currentDelayPtr  = currentDelayPtr;
    inOutI   = inOutI;
    best_eye = best_eye;
    best_dly = best_dly;
    subPhase = subPhase;
    serdesSpeed = serdesSpeed;
    min_dly = min_dly;
    max_dly = max_dly;
#else

    unsigned int sbus_addr;
    GT_U8   dly;
    GT_U16  eye;
    GT_U8   i, k, j;
    GT_U8   dwell;

    if (numOfSerdes > HWS_MAX_PORT_AVAGO_SERDES_NUM)
    {
        HWS_AVAGO_DBG(("numOfSerdes %d is greater than MAX_AVAGO_SERDES_NUMBER %d\n", numOfSerdes, 4));
        return GT_BAD_PARAM;
    }

    /* define dwell value */
    if (avagoBaudRate2DwellTimeValue[serdesSpeed] != NA_VALUE)
    {
        dwell = avagoBaudRate2DwellTimeValue[serdesSpeed];
    }
    else
    {
        dwell = 11;
    }

    /* find the peak of the eye according to delay */
    for (i=(*inOutI); i < currentDelaySize; i++)
    {
        if (*subPhase == 0)
        {
            dly = currentDelayPtr[i];

            /* filter undesired delays */
            if ( dly >= min_dly && dly <= max_dly )
            {
                for (k = 0; k < numOfSerdes; k++)
                {
                    /* select DFE tuning dwell time */
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x18, 0x7, NULL));
                    /* big register write LSB */
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x19, 1 << dwell, NULL));
                    /* big register write MSB */
                    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[k], 0x19, 0, NULL));

                    CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], dly));

                    CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArr[k], DFE_PCAL, NULL));
                }
            }

            *subPhase = 1;
            break;
        }
        else
        {
            for (j=0; j < numOfSerdes; j++)
            {
                dly = currentDelayPtr[i];

                CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesArr[j], &sbus_addr));

                mvHwsAvagoAccessLock(devNum);
                /* get a simple eye metric in range [0..1000] */
                eye = avago_serdes_eye_get_simple_metric(aaplSerdesDb[devNum], sbus_addr);
                mvHwsAvagoAccessUnlock(devNum);
                CHECK_AVAGO_RET_CODE();
                if (eye > (best_eye[j] + MV_AVAGO_EYE_TAP))
                {
                    best_eye[j] = eye;
                    best_dly[j] = dly;
                }
                if (best_eye[j] < MV_AVAGO_MIN_EYE_SIZE)
                {
                    HWS_AVAGO_DBG(("Error on Serdes %d: Eye is too small (size is %d). Training Failed\n", serdesArr[j], best_eye[j]));
                    return GT_FAIL;
                }
            }

            *subPhase = 0;
        }
    }

    if (i < currentDelaySize)
    {
        (*inOutI) = i;
    }
    else
    {
        if ((*subPhase)==0)
        {
            (*inOutI) = 0xFF;
        }
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEnhanceTuneLitePhase2 function
* @endinternal
*
* @brief   Set shift sample point to with the best delay_cal value.
*         This API runs only for AP port after linkUp indication and before running
*         the Rx-Training Adative pCal
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesArr                - collection of SERDESes to configure
* @param[in] numOfSerdes              - number of SERDESes to configure
* @param[in] best_dly                 - best delay to set on serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEnhanceTuneLitePhase2
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      *serdesArr,
    GT_UOPT     numOfSerdes,
    GT_U8       *best_dly
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum      = devNum;
    portGroup   = portGroup;
    serdesArr   = serdesArr;
    numOfSerdes = numOfSerdes;
    best_dly    = best_dly;
#else
    GT_U8 k;

    for (k=0; k < numOfSerdes; k++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesShiftSamplePoint(devNum, portGroup, serdesArr[k], best_dly[k]));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/********************************************************************************
   runMultipleEnhanceTune is a debug function for Testing only for running
   the mvHwsAvagoSerdesEnhanceTune from Terminal
********************************************************************************/
void runMultipleEnhanceTune
(
    GT_U32  serdesNumbers0_31,
    GT_U32  serdesNumbers32_63,
    GT_U8   min_LF,
    GT_U8   max_LF
)
{
    static GT_U32  serdesArr[64]= {0};
    GT_U32  i, numOfSerdes;

    numOfSerdes=0; /* num of Serdes to run the function */
    for (i=0; i<32; i++)
    {
        if ((((serdesNumbers0_31 >> i) & 1) == 1))
        {
            serdesArr[numOfSerdes] = i;
            numOfSerdes++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    for (i=32; i<64; i++)
    {
        if ((((serdesNumbers32_63 >> (i-32)) & 1) == 1))
        {
            serdesArr[numOfSerdes] = i;
            numOfSerdes++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    mvHwsAvagoSerdesEnhanceTune(0, 0, serdesArr, numOfSerdes, min_LF, max_LF);
}

void bobcat3RunMultipleEnhanceTune
(
    GT_U32  serdesNumbers0_31,
    GT_U32  serdesNumbers32_63,
    GT_U32  serdesNumbers64_95,
    GT_U8   min_LF,
    GT_U8   max_LF
)
{
    static GT_U32  serdesArr[96]= {0};
    GT_U32  i, numOfSerdeses = 0;

    for (i=0; i<32; i++)
    {
        if(((serdesNumbers0_31 >> i) & 1) == 1)
        {
            serdesArr[numOfSerdeses] = i;
            numOfSerdeses++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    for (i=32; i<64; i++)
    {
        if(((serdesNumbers32_63 >> (i-32)) & 1) == 1)
        {
            serdesArr[numOfSerdeses] = i;
            numOfSerdeses++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    for (i=64; i<96; i++)
    {
        if(((serdesNumbers64_95 >> (i-64)) & 1) == 1)
        {
            serdesArr[numOfSerdeses] = i;
            numOfSerdeses++;
            HWS_AVAGO_DBG(("Serdes %d\n", i));
        }
    }

    mvHwsAvagoSerdesEnhanceTune(0, 0, serdesArr, numOfSerdeses, min_LF, max_LF);
}


/**
* @internal mvHwsAvagoSerdesVoltageGet function
* @endinternal
*
* @brief   Get the voltage (in mV) from Avago Serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number  (currently not used)
*
* @param[out] voltage                  - Serdes  value (in mV)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVoltageGet
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_UOPT     *voltage
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    *voltage = 0;
#else

    GT_U32          data;
    unsigned int    i;
    unsigned int    retry=10;
    unsigned int    sensor_addr;

    /* avoid warnings */
    serdesNum = serdesNum;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        *voltage = 0;
        /* no SERDESes on emulator */
        return GT_OK;
    }
#endif /* not MV_HWS_REDUCED_BUILD_EXT_CM3*/

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
        sensor_addr = 1; /* sbus_address 1 of TSEN0 in ring0 */
    }
    else if ((HWS_DEV_SILICON_TYPE(devNum) == Pipe) || (HWS_DEV_SILICON_TYPE(devNum) == Raven))
    {
        sensor_addr = 18;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    /* trying to get the Voltage value maximum 10 times and set 1ms delay between each time */
    for(i=0; i < retry; i++)
    {
        mvHwsAvagoAccessLock(devNum);
        data = avago_spico_int(aaplSerdesDb[devNum], AVAGO_SBUS_MASTER_ADDRESS, 0x18, (0 << 12) | (sensor_addr & 0xff));
        mvHwsAvagoAccessUnlock(devNum);
        if (data & 0x8000)
        {
            data &= 0x0FFF;         /* Mask down to 12b voltage value */
            *voltage = (data / 2);  /* Scale to milli-volt */

            return GT_OK;
        }

        /* Delay in 1ms */
        hwsOsExactDelayPtr(devNum, portGroup, 1);
    }

    return GT_BAD_VALUE;  /* voltage not valid */

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesAutoTuneResult function
* @endinternal
*
* @brief   Per SERDES return the adapted tuning results
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] results                  - the adapted tuning results.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesAutoTuneResult
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    MV_HWS_AUTO_TUNE_RESULTS    *results
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    hwsOsMemSetFuncPtr(results,0,sizeof(*results));

#else

    unsigned int sbus_addr;
    GT_U32  data, column, i;
    int     ctle[5];
    int     dfe[13];
    Avago_serdes_tx_eq_t    tx_eq;
    GT_UOPT                 signalThreshold;

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        hwsOsMemSetFuncPtr(results,0,sizeof(*results));
        /* no SERDESes on emulator */
        return GT_OK;
    }
#endif /* not MV_HWS_REDUCED_BUILD_EXT_CM3*/

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    mvHwsAvagoAccessLock(devNum);
    /* Gets the TX equalization values */
    avago_serdes_get_tx_eq(aaplSerdesDb[devNum], sbus_addr, &tx_eq);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

    results->txAmp   = tx_eq.atten;
    results->txEmph0 = tx_eq.post;
    results->txEmph1 = tx_eq.pre;

    for (i=0; i <= 12; i++)
    {
        if (i==1)
        {
            column=1;
            data = (column << 12) | (8 << 8);
        }
        else
        {
            column=3;
            data = (column << 12) | (i << 8);
        }
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, data, &dfe[i]));
        (dfe[i] > 0x8000) ? (dfe[i] = dfe[i] - 0x10000) : dfe[i];
        results->DFE[i] = dfe[i];
        AVAGO_DBG(("   DFE[%d] = %d \n", i, results->DFE[i]));
    }

    column=2;
    for (i = 0; i <= 4; i++)
    {
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126, ((column << 12) | (i << 8)), &ctle[i]));
    }

    results->HF = ctle[0];
    results->LF = ctle[1];
    results->DC = ctle[2];
    results->BW = ctle[3];
    results->LB = ctle[4];
    mvHwsAvagoAccessLock(devNum);
    results->EO = avago_serdes_eye_get_simple_metric(aaplSerdesDb[devNum], sbus_addr);
    mvHwsAvagoAccessUnlock(devNum);
    CHECK_AVAGO_RET_CODE();

    /* Get the signal OK threshold on Serdes */
    CHECK_STATUS(mvHwsAvagoSerdesSignalOkThresholdGet(devNum, portGroup, serdesNum, &signalThreshold));
    results->sqleuch = (signalThreshold * 16) + 68;

    AVAGO_DBG(("   HF = %d \n", results->HF));
    AVAGO_DBG(("   LF = %d \n", results->LF));
    AVAGO_DBG(("   DC = %d \n", results->DC));
    AVAGO_DBG(("   BW = %d \n", results->BW));
    AVAGO_DBG(("   LB = %d \n", results->LB));
    AVAGO_DBG(("   EO = %d \n", results->EO));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdeCpllOutputRefClkGet function
* @endinternal
*
* @brief   Return the output frequency of CPLL reference clock source of SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] refClk                   - CPLL reference clock frequency
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdeCpllOutputRefClkGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    MV_HWS_CPLL_OUTPUT_FREQUENCY    *refClk
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    refClk = refClk;
#else
    GT_U32  readDataAddressReg, readDataOutReg;
    GT_U32  data1, data2, data3, shift = 0 , addrShift = 16;

    if(HWS_DEV_SILICON_TYPE(devNum) == Bobcat3)
    {
        if((serdesNum < 36) || (serdesNum == 72))
        {
            readDataOutReg = DEVICE_GENERAL_STATUS_3;
            readDataAddressReg = DEVICE_GENERAL_CONTROL_17; /* CPLL_0 */
        }
        else
        {
            readDataOutReg = DEVICE_GENERAL_STATUS_4;
            readDataAddressReg = DEVICE_GENERAL_CONTROL_18; /* CPLL_1 */
        }
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        if(serdesNum < 24)
        {
            readDataOutReg = DEVICE_GENERAL_STATUS_3;
            readDataAddressReg = DEVICE_GENERAL_CONTROL_17; /* CPLL_0 */
        }
        else
        {
            readDataOutReg = DEVICE_GENERAL_STATUS_4;
            readDataAddressReg = DEVICE_GENERAL_CONTROL_18; /* CPLL_1 */
        }
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == Aldrin)
    {
        readDataAddressReg = DEVICE_GENERAL_CONTROL_13;
        readDataOutReg = DEVICE_GENERAL_STATUS_4;
        shift = 16;
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == BobK)
    {
        readDataAddressReg = DEVICE_GENERAL_CONTROL_21;
        readDataOutReg = DEVICE_GENERAL_STATUS_4;
    }
    else if(HWS_DEV_SILICON_TYPE(devNum) == Pipe)
    {
        readDataAddressReg = DEVICE_GENERAL_CONTROL_16;
        addrShift = 22;
        readDataOutReg = DEVICE_GENERAL_STATUS_1;
        shift = 14;
    }
    else
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal device type\n"));
    }

    CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, readDataAddressReg, addrShift, 8, 0xA));
    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, readDataOutReg, &data1));
    data1 = (data1 >> shift) & 0xFFFF; /* Get cpll_ndiv_Msb: Bits #0_15 */

    /* set 1ms delay */
    hwsOsExactDelayPtr(devNum, devNum, 1);

    CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, readDataAddressReg, addrShift, 8, 0xB));
    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, readDataOutReg, &data2));
    data2 = (data2 >> shift) & 0xFFFF; /* Get cpll_ndiv_Lsb: Bits #0_15 */

    /* set 1ms delay */
    hwsOsExactDelayPtr(devNum, devNum, 1);

    CHECK_STATUS(hwsServerRegFieldSetFuncPtr(devNum, readDataAddressReg, addrShift, 8, 0xD));
    CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, readDataOutReg, &data3));
    data3 = (data3 >> shift) & 0xFFFF; /* Get cpll_NHF_NLF: Bits #0_15 */

    /*
        For divider values per output frequency check CPLL init sequences:
            cpllDataArray
        or
            bc3CpllDataArray
    */

    if (( ((data1==0x8555) && ((data2==0x5555)||(data2==0xA938/*+9.6PPM*/))) ||
          ((data1==0x8556) && (data2==0x2FC9) /*+25 PPM*/)||
          ((data1==0x8000) && (data2==0xD1B7) /*+25PPM*/)) && (data3==0x4400))
    {
        *refClk = MV_HWS_156MHz_OUT; /* 156.25Mhz */
    }
    else if (( ((data1==0x8555) && ((data2==0x5555)||(data2==0xA938/*+9.6PPM*/))) ||
          ((data1==0x8556) && (data2==0x2FC9) /*+25PPM*/) ||
          ((data1==0x8000) && (data2==0xD1B7) /*+25PPM*/)) && (data3==0x4800))
    {
        *refClk = MV_HWS_78MHz_OUT; /* 78.125Mhz */
    }
    else if (( ((data1==0x8555) && ((data2==0x5555)||(data2==0xA938/*+9.6PPM*/))) ||
          ((data1==0x8556) && (data2==0x2FC9) /*+25PPM*/)||
          ((data1==0x8000) && (data2==0xD1B7) /*+25PPM*/)) && (data3==0x4000))
    {
        *refClk = MV_HWS_312MHz_OUT; /* 312.5Mhz */
    }
    else if ((data1==0x8C28) && (data2==0x325D) /*+25PPM*/ )
    {
        *refClk = MV_HWS_164MHz_OUT; /* 164.24Mhz */
    }
    else
    {
        *refClk = 0;
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("unknown CPLL divider value\n"));
    }

#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdeSpeedGet function
* @endinternal
*
* @brief   Return SERDES baud rate.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdeSpeedGet
(
    GT_U8                 devNum,
    GT_U32                portGroup,
    GT_U32                serdesNum,
    MV_HWS_SERDES_SPEED   *rate
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    rate = rate;
#else

    unsigned int  sbus_addr;
    unsigned int  divider, serdesSpeed;
    MV_HWS_CPLL_OUTPUT_FREQUENCY  outRefClk;
    GT_U32  refClk;
    GT_U32  data;
    GT_BOOL isCpllUsed;
    MV_HWS_DEV_TYPE devType;

    if (rate == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsAvagoConvertSerdesToSbusAddr(devNum, serdesNum, &sbus_addr));

    devType = HWS_DEV_SILICON_TYPE(devNum);

    if (devType == Raven)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ARG_STRING_MAC("mvHwsAvagoSerdeCpllOutputRefClkGet is NOT_IMPLEMENTED for Raven\n"));
    }
    else
    {
        mvHwsAvagoAccessLock(devNum);
        divider = divider_lookup28nm(avago_serdes_mem_rd(aaplSerdesDb[devNum], sbus_addr, AVAGO_ESB, 0x220));
        mvHwsAvagoAccessUnlock(devNum);
        CHECK_AVAGO_RET_CODE();
    }

    /* Get the reference clock source */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, 0));

    if((devType == Bobcat3) || (devType == Pipe) || (devType == Raven) || (devType == Aldrin2))
    {
        isCpllUsed = (((data >> 8) & 1) == 0) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        /* BobK, Aldrin */
        isCpllUsed = (((data >> 8) & 1) == 1) ? GT_TRUE : GT_FALSE;
    }

    if (isCpllUsed == GT_TRUE) /* CPLL */
    {
        CHECK_STATUS(mvHwsAvagoSerdeCpllOutputRefClkGet(devNum, portGroup, serdesNum, &outRefClk));

        switch (outRefClk)
        {
            case MV_HWS_156MHz_OUT:
                refClk = 156250;  /* 156.25Mhz */
                if (devType == Pipe)
                {
                    /* need to read reference clock ratio divider */
                    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (3 << 11)));
                    /* get bits 12:11 */
                    data = (data >> 11) & 3;
                    switch(data)
                    {
                        case 1:
                            refClk = 156250;  /* 156.25Mhz */
                            break;
                        case 2:
                            refClk = 78125;   /* 78.125Mhz */
                            break;
                        case 3:
                        default:
                            HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal clock ratio value\n"));
                    }
                }
                break;
            case MV_HWS_78MHz_OUT:
                refClk = 78125;   /* 78.125Mhz */
                break;
            case MV_HWS_312MHz_OUT:
                refClk = 312500;  /* 312.5Mhz */
                if ((devType == Bobcat3) || (devType == Pipe) || (devType == Raven) || (devType == Aldrin2))
                {
                    /* need to read reference clock ratio divider */
                    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (3 << 11)));
                    /* get bits 12:11 */
                    data = (data >> 11) & 3;
                    switch(data)
                    {
                        case 1:
                            refClk = 312500;  /* 312.5Mhz */
                            break;
                        case 2:
                            refClk = 156250;  /* 156.25Mhz */
                            break;
                        case 3:
                            refClk = 78125;   /* 78.125Mhz */
                            break;
                        default:
                            HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal clock ratio value\n"));
                    }
                }
                break;

            case MV_HWS_164MHz_OUT:
                refClk = 164242;  /* 164.24Mhz */
                if ((devType == Bobcat3) || (devType == Aldrin2))
                {
                    /* need to read reference clock ratio divider */
                    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (3 << 11)));
                    /* get bits 12:11 */
                    data = (data >> 11) & 3;
                    switch(data)
                    {
                        case 1:
                            refClk = 164242;  /* 164.24Mhz */
                            break;
                        default:
                            HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal clock ratio value\n"));
                    }
                }
                break;

            default:
                refClk = 0;
                return GT_NOT_INITIALIZED; /* this return value used in mvHwsPortInterfaceGet if port was not initialized */
        }
    }
    else /* External Ref Clock */
    {
        if ((devType == Bobcat3) || (devType == Pipe) || (devType == Raven) || (devType == Aldrin2))
        {
            /* need to read reference clock ratio divider */
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SERDES_EXTERNAL_CONFIGURATION_0, &data, (3 << 11)));
            /* get bits 12:11 */
            data = (data >> 11) & 3;
            switch (data)
            {
                case 1:
                    refClk = 156250; /* 156.25Mhz */
                    break;
                case 2:
                    refClk = 78125;  /* 78.125Mhz */
                    break;
                default:
                    HWS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ARG_STRING_MAC("illegal clock ratio value\n"));
            }
        }
        else
        { /* BobK and Aldrin */
            refClk = 156250; /* 156.25Mhz */
        }
    }

    serdesSpeed = divider * refClk;
    switch (serdesSpeed)
    {
        case 1250000:
            *rate = _1_25G;
            break;
        case 3125000:
            *rate = _3_125G;
            break;
        case 5000000:
            *rate = _5G;
            break;
        case 5156250:
            *rate = _5_15625G;
            break;
        case 6250000:
            *rate = _6_25G;
            break;
        case 7500000:
            *rate = _7_5G;
            break;
        case 10312500:
            *rate = _10_3125G;
            break;
        case 12500000:
            *rate = _12_5G;
            break;
        case 12187500:
            *rate = _12_1875G;
            break;
        case 12890625:
            *rate = _12_8906G;
            break;
        case 25781250:
            *rate = _25_78125G;
            break;
        case 27500000:
            *rate = _27_5G;
            break;
        case 20625000:
            *rate = _20_625G;
            break;
        case 26562500:
            *rate = _26_5625G;
            break;
        case 28281250:
            *rate = _28_28125G; /*TODO need to read NRZ*/
            break;
        case 26250000:
            *rate = _26_25G;
            break;
        case 27099930:
            *rate = _27_1G;
            break;
        default:
            *rate = SPEED_NA;
            HWS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ARG_STRING_MAC("illegal serdesSpeed\n"), LOG_ARG_GEN_PARAM_MAC(serdesSpeed));
    }
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesEomUiGet function
* @endinternal
*
* @brief   Return SERDES baud rate in Ui.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] baudRatePtr              - current system baud rate in pico-seconds.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesEomUiGet
(
    GT_U8             devNum,
    GT_U32            portGroup,
    GT_U32            serdesNum,
    GT_U32            *baudRatePtr
)
{

#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    baudRatePtr = baudRatePtr;
#else
    MV_HWS_SERDES_SPEED   rate;

    RETURN_GT_OK_ON_EMULATOR;

    CHECK_STATUS(mvHwsAvagoSerdeSpeedGet(devNum,portGroup,serdesNum,&rate));
    switch (rate)
    {
        case _1_25G:
            *baudRatePtr = 800;
            break;
        case _3_125G:
            *baudRatePtr = 320;
            break;
        case _5G:
            *baudRatePtr = 200;
            break;
        case _5_15625G:
            *baudRatePtr = 194;
            break;
        case _6_25G:
            *baudRatePtr = 160;
            break;
        case _10_3125G:
            *baudRatePtr = 97;
            break;
        case _12_5G:
            *baudRatePtr = 80;
            break;
        case _12_1875G:
            *baudRatePtr = 82;
            break;
        case _12_8906G:
            *baudRatePtr = 77;
            break;
        case _25_78125G:
            *baudRatePtr = 39;
            break;
        case _27_5G:
            *baudRatePtr = 36;
            break;
        case _26_25G:
            *baudRatePtr = 38;
            break;
        default:
            *baudRatePtr = 0;
            return GT_NOT_INITIALIZED;
    }
#endif /* ASIC_SIMULATION */
    return GT_OK;
}

#ifndef MV_HWS_AVAGO_NO_VOS_WA
/**
* @internal mvHwsAvagoSerdesVosOverrideModeSet function
* @endinternal
*
* @brief   Set the override mode of the serdeses VOS parameters. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number
* @param[in] vosOverride              - GT_TRUE means override, GT_FALSE means not.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosOverrideModeSet
(
    GT_U8   devNum,
    GT_BOOL vosOverride
)
{
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride = vosOverride;

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesVosOverrideModeGet function
* @endinternal
*
* @brief   Get the override mode of the serdeses VOS parameters. If the override mode
*         is set to true, the VOS parameters will be overriden, if set to false, they will
*         not be overriden.
* @param[in] devNum                   - device number
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosOverrideModeGet
(
    GT_U8   devNum,
    GT_BOOL *vosOverridePtr
)
{
    if (vosOverridePtr == NULL)
    {
        return GT_BAD_PTR;
    }
    *vosOverridePtr = hwsDeviceSpecInfo[devNum].serdesInfo.serdesVosOverride;

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesVosParamsGet function
* @endinternal
*
* @brief   Get the VOS Override parameters from the local DB.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesVosParamsGet
(
    unsigned char  devNum,
    unsigned long  *vosParamsPtr
)
{
#ifndef ASIC_SIMULATION
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    if(hwsAvagoNotSupportedCheck())
    {
        *vosParamsPtr = 0;
        /* no SERDESes on emulator */
        return GT_OK;
    }
#endif /* not MV_HWS_REDUCED_BUILD_EXT_CM3*/

    CHECK_STATUS(mvHwsAvagoSerdesManualVosParamsGet(devNum, vosParamsPtr));
#else
    /* avoid warnings */
    *vosParamsPtr = 0;
    devNum = devNum;
#endif

    return GT_OK;
}
#endif /*#ifndef MV_HWS_AVAGO_NO_VOS_WA*/
#ifndef CO_CPU_RUN
/**
* @internal mvHwsAvagoIfClose function
* @endinternal
*
* @brief   Release all system resources allocated by Serdes IF functions.

* @param[in] devNum                   - system device number
*/
void mvHwsAvagoIfClose(GT_U8 devNum)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
#else
#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    GT_U32  i;
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

    CPSS_LOG_INFORMATION_MAC("Close AAPL for devNum %d\n", devNum);

    if(aaplSerdesDb[devNum] == NULL)
    {
        CPSS_LOG_INFORMATION_MAC("AAPL is already closed for devNum %d\n", devNum);
        return;
    }

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    /* close AAPL */
    aapl_destruct(aaplSerdesDb[devNum]);
#endif /* MV_HWS_REDUCED_BUILD_EXT_CM3 */

    aaplSerdesDb[devNum] = NULL;


#if defined(CHX_FAMILY) || defined(PX_FAMILY)
    /* if there is no active device delete Mutex */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++)
    {
        if (aaplSerdesDb[i] != NULL)
        {
            return;
        }
    }

    /* Init Avago Access Protection in multi-process environment */
    /* This protection is implemented at the scope of the Host!! */
    /* ========================================================= */
    hwsOsMutexDelete(avagoAccessMutex);
#endif /* defined(CHX_FAMILY) || defined(PX_FAMILY) */

#endif /* ASIC_SIMULATION */
}

#endif

#if 0
/* TBD - function to measure TRX training time over Avago serdes for bring-up/debug */
extern GT_STATUS osTimeRT
(
    OUT GT_U32  *seconds,
    OUT GT_U32  *nanoSeconds
);

GT_U32 trainTest
(
    GT_VOID
)
{
    GT_U32  startSec, endSec;
    GT_U32  startNano, endNano;
    GT_U32  result;
    GT_32   rc;

    for (i = 0; i < 4; i++)
    {
        rc = mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, i, 0x4, 0, NULL);
        if (rc != GT_OK)
        {
            HWS_AVAGO_DBG(("serdesNum=%d,0x4,0-fail\n", i));
            return rc;
        }
    }

    osTimeRT(&startSec, &startNano);
    if (rc != GT_OK)
    {
        HWS_AVAGO_DBG(("osTimeRT start-fail\n"));
        return rc;
    }

    for (i = 0; i < 4; i++)
    {
        rc = mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, i, 0x4, 0x12, NULL);
        if (rc != GT_OK)
        {
            HWS_AVAGO_DBG(("serdesNum=%d,0x4,0x12-fail\n", i));
            return rc;
        }
    }

    for (i = 0; i < 1000; i++)
    {
        rc = mvHwsAvagoSerdesSpicoInterrupt(0, destination_serdes, 0x4027, 0, &result);
        if((result&0x4) == 0x4)
        {
            osTimeRT(&endSec, &endNano);
            if (rc != GT_OK)
            {
                HWS_AVAGO_DBG(("osTimeRT end-fail\n"));
                return rc;
            }
            break;
        }
        else
        {
            HWS_AVAGO_DBG(("."));
            hwsOsTimerWkFuncPtr(10);
        }
    }

    HWS_AVAGO_DBG(("startSec=%d,endSec=%d,startNano=%d,endNano=%d\n", startSec, endSec, startNano, endNano));
    HWS_AVAGO_DBG(("KR training took %d\n",startNano-endNano));

}
#endif

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb function
* @endinternal
*
* @brief   Init Hws DB of edge-detect-by-phase algorithm used by port manager.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectByPhaseInitDb
(
    GT_U8   devNum,
    GT_U32  phyPortNum,
    GT_U32  timeOut
)
{
#ifdef ASIC_SIMULATION
    devNum = devNum;
    phyPortNum = phyPortNum;
    timeOut = timeOut;
#else
    GT_U32 i;
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC   *edgeDetectStcPtr;

    edgeDetectStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[phyPortNum]->edgeDetectStcPtr;

    edgeDetectStcPtr->phase_breakLoopTotal = 0;
    edgeDetectStcPtr->phase_breakLoopInd = 0;

    edgeDetectStcPtr->phase_edgeDetectin_Flag = 0;
    edgeDetectStcPtr->phase_edgeDetectin_Sec = 0;
    edgeDetectStcPtr->phase_edgeDetectin_Usec = 0;

    edgeDetectStcPtr->phase_tmpTimeout = timeOut;

    edgeDetectStcPtr->phase_contextSwitchedBack = 0;

    for (i=0; i<MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
    {
        edgeDetectStcPtr->phase_tvalBefore_tvSec[i] = 0;
        edgeDetectStcPtr->phase_tvalBefore_tvUsec[i] = 0;

        edgeDetectStcPtr->phase_firstEO[i] = 0;
        edgeDetectStcPtr->phase_enable[i] = 0;
        edgeDetectStcPtr->phase_breakLoop[i] = 0;

        edgeDetectStcPtr->phase_pCalTimeSec[i] = 0;
        edgeDetectStcPtr->phase_pCalTimeNano[i] = 0;
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase1 function
* @endinternal
*
* @brief   Set DFE dwell_time parameter. This is phase1 of the by-phase edge detect
*         algorithm.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase1
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      numOfSerdeses,
    GT_U32      *serdesArr,
    GT_32       threshEO,
    GT_U32      dwellTime
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum    = devNum;
    portGroup = portGroup;
    numOfSerdeses = numOfSerdeses;
    serdesArr = serdesArr;
    threshEO  = threshEO;
    dwellTime = dwellTime;
#else
    /*GT_U32   i;*/

    GT_U32 serdesNum;

    for (serdesNum=0; serdesNum<numOfSerdeses; serdesNum++)
    {
        /* select DFE tuning dwell time */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x18, 0x7, NULL));

        /* big register write LSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, 1<<dwellTime, NULL));

        /* big register write MSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, 0, NULL));
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 1 ## serdesNum=%d \n", serdesArr[serdesNum]);
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1 function
* @endinternal
*
* @brief   Launching pCal and take starting time for the pCal process. This is
*         phase2_1 of the by-phase edge detect algorithm.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase2_1
(
    GT_U8       devNum,
    GT_U32      portNum,
    GT_UOPT     portGroup,
    GT_U32      numOfSerdeses,
    GT_U32      *serdesArr,
    GT_32       threshEO,
    GT_U32      dwellTime
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum    = devNum;
    portGroup = portGroup;
    numOfSerdeses = numOfSerdeses;
    serdesArr = serdesArr;
    threshEO  = threshEO;
    dwellTime = dwellTime;
    portNum = portNum;
#else
    GT_STATUS rc;
    GT_U32 serdesNum, tmpData;
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC   *edgeDetectStcPtr;

    edgeDetectStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portNum]->edgeDetectStcPtr;

    for (serdesNum=0; serdesNum<numOfSerdeses &&
          edgeDetectStcPtr->phase_breakLoopTotal == GT_FALSE; serdesNum++)
    {
        if (edgeDetectStcPtr->phase_breakLoop[serdesNum]==GT_TRUE)
        {
            continue;
        }

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 2 ## serdesNum=%d \n", serdesArr[serdesNum]);
        /*gettimeofday(&phase_tvalBefore[devNum][portNum][serdesNum], NULL); */
        edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum] = 0;
        edgeDetectStcPtr->phase_tvalBefore_tvUsec[serdesNum] = 0;
        rc = hwsTimerGetFuncPtr(
            &edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum],
            &edgeDetectStcPtr->phase_tvalBefore_tvUsec[serdesNum]);
        if (rc!=GT_OK)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
        }

        /* check that there is a Signal-detect before running the PCAL */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesNum],
                                            AVAGO_EXTERNAL_STATUS,
            &tmpData, (1 << 2)));
        if (tmpData != 0)
        {
            return GT_NOT_READY;
        }

        /* PCAL also stop the START_ADAPTIVE if it was ran before */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArr[serdesNum], DFE_PCAL, NULL));
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2 function
* @endinternal
*
* @brief   Calculate pCal time duration, check for timeout or for peer TxAmp change
*         (edge-detection). This is phase2_2 of the by-phase edge detect algorithm.
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
* @param[in] portGroup                - port group (core) number
* @param[in] numOfSerdeses            - num of serdeses in the serdeses array
* @param[in] serdesArr                - serdeses array
* @param[in] dwellTime                - define dwell value
* @param[in] continueExecute          - whether or not algorithm execution should be continued
* @param[in] isInNonBlockWaitPtr      - whether or not algorithm is in non-block waiting time
*
* @param[out] continueExecute          - whether or not algorithm execution should be continued
* @param[out] isInNonBlockWaitPtr      - whether or not algorithm is in non-block waiting time
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectPhase2_2
(
    GT_U8       devNum,
    GT_U32      portNum,
    GT_UOPT     portGroup,
    GT_U32      numOfSerdeses,
    GT_U32      *serdesArr,
    GT_32       threshEO,
    GT_U32      dwellTime,
    GT_BOOL     *continueExecute,
    GT_BOOL     *isInNonBlockWaitPtr
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum    = devNum;
    portGroup = portGroup;
    numOfSerdeses = numOfSerdeses;
    serdesArr = serdesArr;
    threshEO  = threshEO;
    dwellTime = dwellTime;
    portNum = portNum;
    continueExecute = continueExecute;
    isInNonBlockWaitPtr = isInNonBlockWaitPtr;
#else
    GT_U32 serdesNum;
    GT_STATUS rc;
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC   *edgeDetectStcPtr;

    edgeDetectStcPtr = hwsDeviceSpecInfo[devNum].portManagerPerPortDbPtrPtr[portNum]->edgeDetectStcPtr;

    /* normal execution */
    if ((edgeDetectStcPtr->phase_edgeDetectin_Flag & 0x1) ==0)
    {
        *isInNonBlockWaitPtr = GT_FALSE;
        for (serdesNum=(edgeDetectStcPtr->phase_edgeDetectin_Flag>>2); serdesNum < numOfSerdeses &&
              edgeDetectStcPtr->phase_breakLoopTotal == GT_FALSE; serdesNum++)
        {
            /* flag for returning to execution from the point where non-blocking-waiting
               was triggered (simlulating "context switch")*/
            if ( ((edgeDetectStcPtr->phase_edgeDetectin_Flag>>1) & 1) == 0)
            {
                if (edgeDetectStcPtr->phase_breakLoop[serdesNum] == GT_FALSE)
                {
                    GT_U32 phase_tvalAfter_tvSec, phase_tvalAfter_tvUsec;
                    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 4 ## serdesNum=%d \n", serdesArr[serdesNum]);
                    /* take the EO value */
                    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesArr[serdesNum],
                            &edgeDetectStcPtr->phase_tuneRes[serdesNum]));

                    /* get time of day */
                    phase_tvalAfter_tvSec = 0;
                    phase_tvalAfter_tvUsec = 0;
                    rc = hwsTimerGetFuncPtr(
                        &phase_tvalAfter_tvSec,
                        &phase_tvalAfter_tvUsec);
                    if (rc!=GT_OK)
                    {
                        HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
                    }

                    /* nanoseconds diff time accumulation. It needs to be accumulated
                       as it will be evaludated to 0 when dividing be a orderly big number
                       like 1000 to microsec or 1000000 to milisec */
                    if (phase_tvalAfter_tvSec >
                        edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum])
                    {
                        edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] +=
                        1000000000U - edgeDetectStcPtr->phase_tvalBefore_tvUsec[serdesNum] +
                            phase_tvalAfter_tvUsec;
                    }
                    else
                    {
                        edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] +=
                            phase_tvalAfter_tvUsec -
                            edgeDetectStcPtr->phase_tvalBefore_tvUsec[serdesNum];
                    }
                    if (edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]>1000000000U)
                    {
                        edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] +=1;
                        if (phase_tvalAfter_tvSec - edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum] > 1)
                        {
                            edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] +=
                                (phase_tvalAfter_tvSec - edgeDetectStcPtr->phase_tvalBefore_tvSec[serdesNum]) - 1;
                        }
                        edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] -= 1000000000U;
                    }

                    /* TIMEOUT CHECK */
                    if (edgeDetectStcPtr->phase_tmpTimeout <=
                        ( ( edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] * 1000) +
                          (edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]/1000000) ) )
                    {
                        edgeDetectStcPtr->phase_breakLoop[serdesNum] = GT_TRUE;
                        edgeDetectStcPtr->phase_breakLoopInd++;
                        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 4.5 ## TIMEOUT. serdesNum=%d \n", serdesArr[serdesNum]);
                    }

                    /* take the first EO value as reference */
                    if (edgeDetectStcPtr->phase_enable[serdesNum] == GT_FALSE
                        && edgeDetectStcPtr->phase_breakLoop[serdesNum] == GT_FALSE)
                    {
                        edgeDetectStcPtr->phase_firstEO[serdesNum] =
                            edgeDetectStcPtr->phase_tuneRes[serdesNum].EO;
                        edgeDetectStcPtr->phase_enable[serdesNum]  = GT_TRUE;
                    }

                    if (1 && edgeDetectStcPtr->phase_breakLoop[serdesNum] == GT_FALSE
                        /*((GT_32)(edgeDetectStcPtr->phase_firstEO[serdesNum] -
                                 edgeDetectStcPtr->phase_tuneRes[serdesNum].EO) > threshEO) ||
                        ((GT_32)(edgeDetectStcPtr->phase_firstEO[serdesNum] -
                                 edgeDetectStcPtr->phase_tuneRes[serdesNum].EO) < (-threshEO))*/)
                    {
                        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 5 ## serdesNum=%d \n", serdesArr[serdesNum]);
                        /* eye openning changed between first tune eye value and last tune eye value - delay in 20ms */
#if 0
                        hwsOsExactDelayPtr(devNum, portGroup, 20);
                        edgeDetectStcPtr->phase_breakLoop[serdesNum] = GT_TRUE;
                        edgeDetectStcPtr->phase_breakLoopInd++;
#endif
                        /* marking that the waiting time should begin, but doing it in non blocking way */
                        edgeDetectStcPtr->phase_edgeDetectin_Flag =
                            ( (GT_U8)serdesNum << 0x2)/*bits[2:7] for serdes number*/ | 0x1/* bit[0] for flag1*/ | 0x2/*bit[1] for flag2*/;
                        /* taking time for reference. this time will be the base time in whic 20 msec time will need to pass from
                           in order to continue execution */
                        /* those timers are not per serdes, because the reference edge detect behavior sleeps on a single
                           lane indication, and this non-blocking behavior should execute same seuqnece, no need per lane timers,
                           one set will do for a whole non-blocking-waiting sequence.
                           If one lane edge-detected, whole algorithm halts */
                        /*
                           NOTE: due the implementation of this algorithm, if one lane edge-detected and is going
                           to wait (non-block) than after the wait is finished and algorithm is continued,
                           other lanes can fall in the timeout as the wait took time. But it is fine as the wait already
                           been done due to the first lane, and other lanes waited anway. If use wants to wait to edge change
                           per lane, independant of other lanes, currently the timeout value should be multipled
                           by (edje-detected-wait-time * lanes amount). If requirements are changed, algorithm should be changed.
                           (implementation similar to legacy edge detect). Also the port manager training timeout
                           should bo considered. */
                        rc = hwsTimerGetFuncPtr(
                        &edgeDetectStcPtr->phase_edgeDetectin_Sec,
                        &edgeDetectStcPtr->phase_edgeDetectin_Usec);
                        if (rc!=GT_OK)
                        {
                            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
                        }
                        *continueExecute = GT_TRUE;
                        /* those counters will be used in non-blocking-wait scope and should not be used again afterwards
                           because of the  edgeDetectStcPtr->phase_breakLoop[serdesNum] = GT_TRUE */
                        edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] = 0;
                        edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] = 0;
                        *isInNonBlockWaitPtr = GT_TRUE;
                        return GT_OK;
                    }
                }
            }
            else
            {
                /* when we reached here, we finish non-blocking-waiting on an edge detected lane.
                   By here "context-switch-back" succesfully made as if the blocking wait just ended */
                /* marking flag2 (bit[1]) to zero and setting serdes valued (bits[2:7]) to zero,
                   flag1 (bit[0]) was already cleared in different scope (time non-blocking-waiting scope) */
                edgeDetectStcPtr->phase_edgeDetectin_Flag = 0;
                edgeDetectStcPtr->phase_contextSwitchedBack = 1;
            }
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 6 ## serdesNum=%d. breakInd=%d. sdNum=%d \n", serdesArr[serdesNum],
                                            edgeDetectStcPtr->phase_breakLoopInd, numOfSerdeses);

            if (edgeDetectStcPtr->phase_contextSwitchedBack && ((serdesNum +1) ==  numOfSerdeses) )
            {
                /* by here we finished this phase on all serdeses, and we are after non-block-wait
                   on a lane */
                *isInNonBlockWaitPtr = GT_FALSE;
                edgeDetectStcPtr->phase_contextSwitchedBack = 0;
            }

            /* if all serdeses finished */
            if (edgeDetectStcPtr->phase_breakLoopInd==numOfSerdeses)
            {
                edgeDetectStcPtr->phase_breakLoopTotal = GT_TRUE;
            }
        }

        if (edgeDetectStcPtr->phase_breakLoopTotal == GT_TRUE)
        {
            *continueExecute = GT_FALSE;

            for (serdesNum=0; serdesNum<numOfSerdeses; serdesNum++)
            {
                HWS_AVAGO_DBG(("tuneRes.EO=%d, SD=%d\n",
                        edgeDetectStcPtr->phase_tuneRes[serdesNum].EO,serdesArr[serdesNum]));
            }
        }
        else
        {
            *continueExecute = GT_TRUE;
        }
    }
    else /* serdes edge detection waiting execution */
    {
        GT_U32 nonBlockingWaitingScopeUsec,nonBlockingWaitingScopeSec;
        GT_U8 serdesNum = (edgeDetectStcPtr->phase_edgeDetectin_Flag>>2);
        *continueExecute = GT_TRUE;
        *isInNonBlockWaitPtr = GT_TRUE;
        rc = hwsTimerGetFuncPtr(
        &nonBlockingWaitingScopeSec,
        &nonBlockingWaitingScopeUsec);
        if (rc!=GT_OK)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
        }

        /* accumulating the nanoseconds */
        if (nonBlockingWaitingScopeSec >
                edgeDetectStcPtr->phase_edgeDetectin_Sec)
        {
            /* we can use here the phase_pCalTimeNano of the serdes parameter as by here the usage of it is done */
            edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] +=
                1000000000U - edgeDetectStcPtr->phase_edgeDetectin_Usec +
                    nonBlockingWaitingScopeUsec;
        }
        else
        {
            /* we can use here the phase_pCalTimeNano of the serdes parameter as by here the usage of it is done */
            edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] +=
                    nonBlockingWaitingScopeUsec - edgeDetectStcPtr->phase_edgeDetectin_Usec;
        }
        if (edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]>1000000000U)
        {
            edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] +=1;
            if ((nonBlockingWaitingScopeSec - edgeDetectStcPtr->phase_edgeDetectin_Sec)>1)
            {
                edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] +=
                    (nonBlockingWaitingScopeSec - edgeDetectStcPtr->phase_edgeDetectin_Sec) - 1;
            }
            edgeDetectStcPtr->phase_pCalTimeNano[serdesNum] -= 1000000000U;
        }

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## wait non-block ## serdesNum=%d START-CONTINUE. timestamp=%d\n",
                                        serdesArr[serdesNum],
                  ((edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] * 1000)
             + edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]/1000000)
                );

        /* time passed from first taking time */
        if ( ((edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] * 1000)
             + edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]/1000000) > (20 /* wait time in milisec */))
        {
            /* setting flag1 (bit[0]) to zero to mark that waiting time ended */
            edgeDetectStcPtr->phase_edgeDetectin_Flag &= 0xFE;
            edgeDetectStcPtr->phase_breakLoop[serdesNum] = GT_TRUE;
            edgeDetectStcPtr->phase_breakLoopInd++;
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## wait non-block ## serdesNum=%d FINISHED. timestamp=%d\n",
                                            serdesArr[serdesNum],
                        ((edgeDetectStcPtr->phase_pCalTimeSec[serdesNum] * 1000)
                        + edgeDetectStcPtr->phase_pCalTimeNano[serdesNum]/1000000)
                            );
            return GT_OK;
        }
        edgeDetectStcPtr->phase_edgeDetectin_Sec = nonBlockingWaitingScopeSec;
        edgeDetectStcPtr->phase_edgeDetectin_Usec = nonBlockingWaitingScopeUsec;
    }

#endif /* ASIC_SIMULATION */

    return GT_OK;
}


#if 0
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetectByPhase
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_U32      numOfSerdeses,
    GT_U32      *serdesArr,
    GT_32       timeout,
    GT_32       threshEO,
    GT_U32      dwellTime
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum    = devNum;
    portGroup = portGroup;
    timeout   = timeout;
    threshEO  = threshEO;
    dwellTime = dwellTime;
    serdesArr=serdesArr;
    numOfSerdeses=numOfSerdeses;
#else

    MV_HWS_AUTO_TUNE_RESULTS    tuneRes[MAX_NUM_OF_SERDESES];
    GT_32    firstEO[MAX_NUM_OF_SERDESES]={0};
    GT_U32   pCalTime[MAX_NUM_OF_SERDESES];
    GT_BOOL  enable[MAX_NUM_OF_SERDESES]={GT_FALSE};
    GT_BOOL  breakLoop[MAX_NUM_OF_SERDESES]={GT_FALSE};
    GT_BOOL  breakLoopTotal=GT_FALSE;
    GT_BOOL  breakLoopInd=0;
    /*GT_U32   i;*/
    GT_U32   sdDetect[MAX_NUM_OF_SERDESES];
    /*MV_HWS_AUTO_TUNE_STATUS  rxStatus[MAX_NUM_OF_SERDESES] = {TUNE_NOT_COMPLITED};*/
    GT_32       tmpTimeout[MAX_NUM_OF_SERDESES];
    GT_U32 serdesNum;

    struct timeval tvalBefore[MAX_NUM_OF_SERDESES];
    struct timeval tvalAfter[MAX_NUM_OF_SERDESES];

    for (serdesNum=0; serdesNum<numOfSerdeses; serdesNum++)
    {
        /* select DFE tuning dwell time */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x18, 0x7, NULL));

        /* big register write LSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, 1<<dwellTime, NULL));

        /* big register write MSB */
        CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesArr[serdesNum], 0x19, 0, NULL));
    }

    for (serdesNum=0; serdesNum<numOfSerdeses && breakLoopTotal == GT_FALSE; serdesNum++)
    {
        if (breakLoop[serdesNum]==GT_TRUE)
        {
            continue;
        }

        gettimeofday(&tvalBefore[serdesNum], NULL);

        /* check that there is a Signal-detect before running the PCAL */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesNum],
                                            AVAGO_EXTERNAL_STATUS, &sdDetect[serdesNum], (1 << 2)));
        if (sdDetect[serdesNum] != 0)
        {
            return GT_NOT_READY;
        }

        /* PCAL also stop the START_ADAPTIVE if it was ran before */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesArr[serdesNum], DFE_PCAL, NULL));

        /*
        rxStatus[serdesNum] = TUNE_NOT_COMPLITED;

        for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
        {
            hwsOsExactDelayPtr(devNum, portGroup, 1);

            CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, &rxStatus));
            if (rxStatus == TUNE_PASS)
            {
                break;
            }
            if (rxStatus == TUNE_FAIL)
            {
                HWS_AVAGO_DBG(("rxStatus is in TUNE_FAIL\n"));
                return GT_FAIL;
            }
        }

        if ((rxStatus != TUNE_PASS) || (i == MV_AVAGO_TRAINING_TIMEOUT))
        {
            rxStatus = TUNE_FAIL;
            return GT_FAIL;
        }
        */
    }


    for (serdesNum=0; serdesNum<numOfSerdeses && breakLoopTotal == GT_FALSE; serdesNum++)
    {
        /* take the EO value */
        CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesArr[serdesNum], &tuneRes[serdesNum]));

        gettimeofday (&tvalAfter[serdesNum], NULL);

        /* diff time in microseconds */
        pCalTime[serdesNum] = (((tvalAfter[serdesNum].tv_sec - tvalBefore[serdesNum].tv_sec)*1000000L +
                                tvalAfter[serdesNum].tv_usec) - tvalBefore[serdesNum].tv_usec);
        pCalTime[serdesNum] = pCalTime[serdesNum] / 1000; /* diff time in ms */

        tmpTimeout[serdesNum] = tmpTimeout[serdesNum] - pCalTime[serdesNum];
        if (tmpTimeout[serdesNum] <= 0)
        {
            breakLoop[serdesNum] = GT_TRUE;
            breakLoopInd++;
        }

        /* take the first EO value as reference */
        if (enable[serdesNum] == GT_FALSE)
        {
            firstEO[serdesNum] = tuneRes[serdesNum].EO;
            enable[serdesNum]  = GT_TRUE;
        }

        if (((GT_32)(firstEO[serdesNum] - tuneRes[serdesNum].EO) > threshEO) ||
            ((GT_32)(firstEO[serdesNum] - tuneRes[serdesNum].EO) < (-threshEO)))
        {
            /* delay in 20ms */
            hwsOsExactDelayPtr(devNum, portGroup, 20);
            breakLoop[serdesNum] = GT_TRUE;
            breakLoopInd++;
        }

        /* if all serdeses finished */
        if (breakLoopInd==numOfSerdeses)
        {
            breakLoopTotal = GT_TRUE;
        }
    }

    HWS_AVAGO_DBG(("Run CTLE training\n"));
    CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesNum, DFE_ICAL, NULL));

    /*
    for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
    {
        hwsOsExactDelayPtr(devNum, portGroup, 1);

        CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, &rxStatus));
        if (rxStatus == TUNE_PASS)
        {
            break;
        }
        if (rxStatus == TUNE_FAIL)
        {
            HWS_AVAGO_DBG(("rxStatus is in TUNE_FAIL\n"));
            return GT_FAIL;
        }
    }
    */

    /*
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &sdDetect, (1 << 2)));
    if (sdDetect != 0)
    {
        return GT_NOT_READY;
    }

    CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesNum, DFE_START_ADAPTIVE, NULL));


    hwsOsExactDelayPtr(devNum, portGroup, 5);


    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesNum, &tuneRes));
    HWS_AVAGO_DBG(("tuneRes.EO=%d\n", tuneRes.EO));
    */
#endif /* ASIC_SIMULATION */

    return GT_OK;
}
#endif


#ifndef ASIC_SIMULATION
static GT_U32 tvalBefore_tvSec=0;
static GT_U32 tvalBefore_tvUsec=0;
static GT_U32 tvalAfter_tvSec=0;
static GT_U32 tvalAfter_tvUsec=0;
#endif

/**
* @internal mvHwsAvagoSerdesWaitForEdgeDetect function
* @endinternal
*
* @brief   Detect TxAmp peak for positive or negative changing, then run iCal
*         and start adaptive pCal.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] timeout                  -  in miliseconds
* @param[in] threshEO                 - Eye monitor threshold
* @param[in] dwellTime                - define dwell value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesWaitForEdgeDetect
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     serdesNum,
    GT_32       timeout,
    GT_32       threshEO,
    GT_U32      dwellTime
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum    = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    timeout   = timeout;
    threshEO  = threshEO;
    dwellTime = dwellTime;
#else

    MV_HWS_AUTO_TUNE_RESULTS    tuneRes;
    GT_32    firstEO=0;
    GT_U32   pCalTime;
    GT_BOOL  enable=GT_FALSE;
    GT_BOOL  breakLoop=GT_FALSE;
    GT_U32   i;
    GT_U32   sdDetect;
    MV_HWS_AUTO_TUNE_STATUS  rxStatus = TUNE_NOT_COMPLITED;
    GT_STATUS rc;

    /*struct timeval tvalBefore, tvalAfter;*/

    tvalBefore_tvSec = 0;
    tvalBefore_tvUsec = 0;
    tvalAfter_tvSec = 0;
    tvalAfter_tvUsec = 0;

    /* select DFE tuning dwell time */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x18, 0x7, NULL));

    /* big register write LSB */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 1<<dwellTime, NULL));

    /* big register write MSB */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x19, 0, NULL));

    AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 1 ## serdesNum=%d \n", serdesNum);
    do
    {
        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 2 ## serdesNum=%d \n", serdesNum);
        /*gettimeofday (&tvalBefore, NULL);*/
        rc = hwsTimerGetFuncPtr(&tvalBefore_tvSec,&tvalBefore_tvUsec);
        if (rc!=GT_OK)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
        }

        /* check that there is a Signal-detect before running the PCAL */
        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &sdDetect, (1 << 2)));
        if (sdDetect != 0)
        {
            return GT_NOT_READY;
        }

        /* PCAL also stop the START_ADAPTIVE if it was ran before */
        CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesNum, DFE_PCAL, NULL));

        rxStatus = TUNE_NOT_COMPLITED;

        for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
        {
            /* Delay in 1ms */
            hwsOsExactDelayPtr(devNum, portGroup, 1);

            CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, &rxStatus));
            if (rxStatus == TUNE_PASS)
            {
                break;
            }
        }

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 4 ## serdesNum=%d \n", serdesNum);
        /* in case training failed or took too long/short */
        if ((rxStatus != TUNE_PASS) || (i == MV_AVAGO_TRAINING_TIMEOUT))
        {
            rxStatus = TUNE_FAIL;
            return GT_FAIL;
        }

        /* take the EO value */
        CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesNum, &tuneRes));

        /*gettimeofday (&tvalAfter, NULL);*/
        rc = hwsTimerGetFuncPtr(&tvalAfter_tvSec,&tvalAfter_tvUsec);
        if (rc!=GT_OK)
        {
            HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("HWS os timer API returned error"), LOG_ARG_GEN_PARAM_MAC(rc));
        }

        /* diff time in microseconds */
        /*pCalTime = (((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L + tvalAfter.tv_usec) - tvalBefore.tv_usec);*/
        pCalTime = (((tvalAfter_tvSec - tvalBefore_tvSec)*1000000L + (tvalAfter_tvUsec*1000)) - (tvalBefore_tvUsec*1000));
        pCalTime = pCalTime / 1000; /* diff time in ms */

        timeout = timeout - pCalTime;
        if (timeout <= 0)
        {
            breakLoop = GT_TRUE;
        }

        /* take the first EO value as reference */
        if (enable == GT_FALSE)
        {
            firstEO = tuneRes.EO;
            enable  = GT_TRUE;
        }

        if (((GT_32)(firstEO - tuneRes.EO) > threshEO) || ((GT_32)(firstEO - tuneRes.EO) < (-threshEO)))
        {
            AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 5 ## serdesNum=%d \n", serdesNum);
            /* delay in 20ms */
            hwsOsExactDelayPtr(devNum, portGroup, 20);
            breakLoop = GT_TRUE;
        }

        /* for testing only
        if (timeout < 550)
        {
            CHECK_STATUS(mvHwsAvagoSerdesManualTxConfig(devNum, portGroup, serdesNum, 0, 0, 0, 0, 0));
        }
        */

        AVAGO_TUNE_BY_PHASE_DEBUG_PRINT(" edgeDetect ## 6 ## serdesNum=%d \n", serdesNum);
    } while (breakLoop == GT_FALSE);

    HWS_AVAGO_DBG(("Run CTLE training\n"));
    CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesNum, DFE_ICAL, NULL));

    for (i = 0; i < MV_AVAGO_TRAINING_TIMEOUT; i++)
    {
        /* Delay in 1ms */
        hwsOsExactDelayPtr(devNum, portGroup, 1);

        CHECK_STATUS(mvHwsAvagoSerdesRxAutoTuneStatusShort(devNum, portGroup, serdesNum, &rxStatus));
        if (rxStatus == TUNE_PASS)
        {
            break;
        }
        if (rxStatus == TUNE_FAIL)
        {
            HWS_AVAGO_DBG(("rxStatus is in TUNE_FAIL\n"));
            return GT_FAIL;
        }
    }

    /* check that there is a Signal-detect before running the PCAL */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_EXTERNAL_STATUS, &sdDetect, (1 << 2)));
    if (sdDetect != 0)
    {
        return GT_NOT_READY;
    }

    CHECK_STATUS(mvHwsAvagoSerdesDfeConfig(devNum, portGroup, serdesNum, DFE_START_ADAPTIVE, NULL));

    /* Delay in 5ms */
    hwsOsExactDelayPtr(devNum, portGroup, 5);

    /* take the EO value */
    CHECK_STATUS(mvHwsAvagoSerdesAutoTuneResult(devNum, portGroup, serdesNum, &tuneRes));
    HWS_AVAGO_DBG(("tuneRes.EO=%d\n", tuneRes.EO));
#endif /* ASIC_SIMULATION */

    return GT_OK;
}

/**
* @internal mvHwsAvagoSerdesRxSignalOkGet function
* @endinternal
*
* @brief   Get the rx_signal_ok indication from o_core_status serdes macro.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*
* @param[out] signalOk                 - (pointer to) whether or not rx_signal_ok is up
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* @note rx_signal_ok is a consilidation of multiple conditions in the serdes.
*       It can be expresses as the following AND operation:
*       _________
*       CDR_LOCK --|     |
*       |  AND  |
*       RX_RDY  --| (logic |--> rx_signal_ok
*       | gate) |
*       RX_IDLE --|_________|
*
*/
GT_STATUS mvHwsAvagoSerdesRxSignalOkGet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *signalOk
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
    *signalOk = *signalOk;
#else
    GT_U32       data;

    RETURN_GT_OK_ON_EMULATOR;

    HWS_NULL_PTR_CHECK_MAC(signalOk);

    *signalOk = GT_FALSE;

    /* read rx_signal_ok indication from o_core_status[4] */
    CHECK_STATUS_EXT(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, AVAGO_CORE_STATUS, &data, (1 << 4)),
                    LOG_ARG_SERDES_IDX_MAC(serdesNum), LOG_ARG_STRING_MAC("reading o_core_status[4]"));
    if (data>0)
    {
        *signalOk = GT_TRUE;
    }
#endif

    return GT_OK;
}
#endif /*MV_HWS_REDUCED_BUILD_EXT_CM3*/

/*******************************************************************************/
/******************************* DEBUG functions *******************************/
/*******************************************************************************/

/**
* @internal mvHwsAvagoSerdesTap1DbgPrint function
* @endinternal
*
* @brief   Get tap1 value from spico of a given serdes.
*
* @note   APPLICABLE DEVICES:      Caelum; Bobcat3; Aldrin2; Falcon; Aldrin.
* @param[in] devNum                   - device number
* @param[in] portGroup                - port group number
* @param[in] serdesNum                - serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAvagoSerdesTap1DbgPrint
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 serdesNum
)
{
#ifdef ASIC_SIMULATION
    /* avoid warnings */
    devNum    = devNum;
    portGroup = portGroup;
    serdesNum = serdesNum;
#else

    int data = 0;

    /* get HW value */
    CHECK_STATUS(mvHwsAvagoSerdesSpicoInterrupt(devNum, portGroup, serdesNum, 0x126,
                            ((0x5 << 12) | (0xB << 8)), &data));

    HWS_AVAGO_DBG((" TAP1=%s, HW value bits[0:3]=0x%02x \n", (data & 8) ? "ENABLED" : "DISABLED", data));

#endif
    return GT_OK;
}


