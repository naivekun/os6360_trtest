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
*/
/**
********************************************************************************
* @file mvSiliconIf.h
*
* @brief Application IF definition.
*
* @version   46
********************************************************************************
*/

#ifndef __siliconIf_H
#define __siliconIf_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GM_USED
    #define FALCON_ASIC_SIMULATION
#endif

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortMiscIf.h>

#if defined(BOBK_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#endif

#ifndef MV_HWS_FREE_RTOS
#include <cpss/generic/cpssHwInfo.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#endif

#ifdef CO_CPU_RUN
typedef GT_U8       GT_UOPT;
typedef GT_U16      GT_UREG_DATA;
typedef GT_16       GT_REG_DATA;
#else
typedef GT_U32      GT_UOPT;
typedef GT_U32      GT_UREG_DATA;
typedef GT_32       GT_REG_DATA;
#endif

#if defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#define hwsOsPrintf osPrintf
#else
#define hwsOsPrintf cpssOsPrintf
#endif

/* printing includes and definitions */
#if defined(CHX_FAMILY) || defined(PX_FAMILY)
#define osPrintf        hwsPrintf
#define osMemSet        hwsMemSet
#define osMemCpy        hwsMemCpy
#elif defined(MV_HWS_REDUCED_BUILD) && !defined(VIPS_BUILD) && !defined(WIN32)
#include <printf.h>
#define osPrintf        printf
#endif

/* Returns the info located at the specified offset & length in data.   */
#define U32_GET_FIELD(data,offset,length)           \
        (((data) >> (offset)) & ((1 << (length)) - 1))

#ifdef HWS_DEBUG
#include <gtOs/gtOsIo.h>
extern GT_FILEP fdHws;
#endif

#ifndef  MV_HWS_FREE_RTOS
extern GT_VOID gtBreakOnFail
(
    GT_VOID
);
#endif

#define HWS_SERVER_WRITE_REG(driverPtr,devNum,regAddr,data) \
    if (driverPtr == NULL){ \
        CHECK_STATUS(hwsServerRegSetFuncPtr(devNum, regAddr, data)); \
    }else {\
        GT_U32 dPtr = data;\
        if (driverPtr->writeMask != NULL){\
            CHECK_STATUS(driverPtr->writeMask(driverPtr, SSMI_FALCON_ADDRESS_SPACE, regAddr, &dPtr, 1, 0));\
        }else{\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);}\
    }

#define HWS_SERVER_READ_REG(driverPtr,devNum,regAddr,data) \
    if (driverPtr == NULL){ \
        CHECK_STATUS(hwsServerRegGetFuncPtr(devNum, regAddr, data)); \
    }else {\
        if (driverPtr->read != NULL){\
            CHECK_STATUS(driverPtr->read(driverPtr, SSMI_FALCON_ADDRESS_SPACE, regAddr, data, 1));\
        }else{\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);}\
    }

#define HWS_REGISTER_WRITE_REG(driverPtr,devNum,regAddr,data, mask) \
    if (driverPtr == NULL){ \
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, regData, mask)); \
    }else {\
        GT_U32 dPtr = data;\
        if (driverPtr->writeMask != NULL){\
            CHECK_STATUS(driverPtr->writeMask(driverPtr, SSMI_FALCON_ADDRESS_SPACE, regAddr, &dPtr, 1, mask));\
        }else{\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);}\
    }

#define HWS_REGISTER_READ_REG(driverPtr,devNum,regAddr,data, mask) \
    if (driverPtr == NULL){ \
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, data, mask)); \
    }else {\
        if (driverPtr->read != NULL){\
            CHECK_STATUS(driverPtr->read(driverPtr, SSMI_FALCON_ADDRESS_SPACE, regAddr, data, mask));\
        }else{\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);}\
    }

/* Global return code status for hws check status usage */
extern GT_STATUS gtStatus;

/*****************************************************/
/*************** Hws Log MACRO APIs ******************/
/*****************************************************/
/*
    Legacy MACRO
*/
#ifdef CHECK_STATUS
#undef CHECK_STATUS
#endif

#if !defined(CM3)
#define CHECK_STATUS(origFunc) \
    CHECK_STATUS_EXT(origFunc, LOG_ARG_NONE_MAC)
#else
#ifdef CPSS_LOG_ERROR_AND_RETURN_MAC
 #undef CPSS_LOG_ERROR_AND_RETURN_MAC
#endif
#define CPSS_LOG_ERROR_AND_RETURN_MAC(_rc, ...) \
    return _rc;
#ifdef LOG_ERROR_NO_MSG
 #undef LOG_ERROR_NO_MSG
#endif
#define LOG_ERROR_NO_MSG ""
#define CHECK_STATUS(origFunc) do {if(GT_OK!=origFunc)CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);} while(0);
#endif /*CM3 && PIPE_DEV_SUPPORT*/

/*
   MACROs for logging Hws APIs. Due to the fact that hws runs in both host CPU and
   in Service CPU, logging should be performed mainly by both. CM3 memory constraint
   force the log handing to be performed in a reduced manner then in host CPU,
   the log extra arguments can be used by the next argument macros ( they cannot
   be called with unlimited format). Use 'CHECK_STATUS_EXT' for function calls
   and 'HWS_LOG_ERROR_AND_RETURN_MAC' for cpss style log of return code and a message.
   Note: library parsing functions like strcpy with max size, and vsnprints that
   parse format and argument list are too heavy for reduced memory architectures.

   Usage examples:

       CHECK_STATUS_EXT(func(devNum, portGroup, serdesNum, regAddr), LOG_ARG_SERDES_MAC(serdesNum), LOG_ARG_REG_ADDR_MAC(regAddr) )
       CHECK_STATUS_EXT(func(), LOG_ARG_NONE_MAC )
       HWS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ARG_STRING_MAC("operation not supported"))

*/

/* Argument macro flags */
#define LOG_ARG_SERDES_FLAG            0
#define LOG_ARG_MAC_FLAG               1
#define LOG_ARG_PCS_FLAG               2
#define LOG_ARG_PORT_FLAG              3
#define LOG_ARG_REG_ADDR_FLAG          4
#define LOG_ARG_STRING_FLAG            5
#define LOG_ARG_GEN_PARAM_FLAG         6
#define LOG_ARG_NONE_FLAG              0xFF

/* Argument macros to be used by user */
#define LOG_ARG_SERDES_IDX_MAC(sdNum)  LOG_ARG_SERDES_FLAG, sdNum
#define LOG_ARG_MAC_IDX_MAC(macNum)    LOG_ARG_MAC_FLAG, macNum
#define LOG_ARG_PCS_IDX_MAC(pcsNum)    LOG_ARG_PCS_FLAG, pcsNum
#define LOG_ARG_PORT_MAC(portNum)      LOG_ARG_PORT_FLAG, portNum
#define LOG_ARG_REG_ADDR_MAC(regAddr)  LOG_ARG_REG_ADDR_FLAG, regAddr
/* This macro require string literal as it uses string literal concatination,
   as we must contain null terminited string because the builder function cannot
   use string copy with size limitition due to memory constraint */
#define LOG_ARG_STRING_MAC(str)        LOG_ARG_STRING_FLAG, str "\0"
/* General parameter macro */
#define LOG_ARG_GEN_PARAM_MAC(param)   LOG_ARG_GEN_PARAM_FLAG, param
/* No args macro */
#define LOG_ARG_NONE_MAC               LOG_ARG_NONE_FLAG

/* Log and return */
#define HWS_LOG_ERROR_AND_RETURN_MAC(rc, ...)   \
        CHECK_STATUS_EXT(rc, __VA_ARGS__)

/* Null pointer check */
#define HWS_NULL_PTR_CHECK_MAC(ptr)         \
do {                                        \
    if (ptr == NULL)                        \
    {                                       \
        CHECK_STATUS_EXT(GT_BAD_PTR,        \
       LOG_ARG_STRING_MAC("param is NULL"));\
    }                                       \
} while (0)


/** TODO - complete information logger. Currently on
 *  information logging is done only in host side */
#ifndef  MV_HWS_FREE_RTOS
#define HWS_INFORMATION_MAC(GT_OK, ...)   \
        CHECK_STATUS_EXT(GT_OK, __VA_ARGS__)
#else
#define HWS_INFORMATION_MAC(GT_OK, ...)
#endif

#ifdef CHECK_STATUS_EXT
#undef CHECK_STATUS_EXT
#endif

/* Host CPU (cpss) without logging enabled */
#ifdef HWS_NO_LOG
#define CHECK_STATUS_EXT(origFunc, ...)                  \
do {                                                     \
    gtStatus = origFunc;                                 \
    if (GT_OK != gtStatus)                               \
    {                                                    \
        gtBreakOnFail();                                 \
        return gtStatus;                                 \
    }                                                    \
} while (0);
#else
/* Host CPU with logging enabled, or Service CPU.
   in either case we need to generate hws message and log it */
#ifdef  MV_HWS_FREE_RTOS
/* Service CPU */
/* This macro should be as minimal as possible in order to
   support the memory constraint of the CM3 architecture */
#define CHECK_STATUS_EXT(origFunc, ...)                         \
do {                                                            \
        if (origFunc!=GT_OK)                                    \
        {                                                       \
            /* Service CPU and Host CPU log  */                 \
            return (mvHwsGeneralLog(__FUNCTION__, __VA_ARGS__,  \
                                           LOG_ARG_NONE_MAC));  \
        }                                                       \
} while(0);
#else
/*  CPSS_LOG_ERROR_AND_RETURN_MAC(rc,...)  */
/* Host CPU with logging enabled */
#define CHECK_STATUS_EXT(origFunc, ...)                       \
do {                                                          \
    gtStatus = origFunc;                                      \
    if (GT_OK != gtStatus)                                    \
    {                                                         \
        gtBreakOnFail();                                      \
        /* Service CPU and Host CPU log  */                   \
        return (mvHwsGeneralLog(__FUNCNAME__, __FILENAME__,   \
         (unsigned int)__LINE__, (int)gtStatus, __VA_ARGS__, LOG_ARG_NONE_MAC)); \
    }                                                         \
} while (0);
#endif /* MV_HWS_FREE_RTOS */
#endif /* HWS_NO_LOG */
/*****************************************************/
/*****************************************************/
/*****************************************************/

            /* ABS definition */
#define ABS(val) ((val) < 0) ? -(val) : (val)

/* max number of devices supported by driver */
#ifdef MV_HWS_REDUCED_BUILD
#define HWS_MAX_DEVICE_NUM (1)
#else
#define HWS_MAX_DEVICE_NUM (128)
#endif

#ifndef CM3

#define MV_HWS_MAX_LANES_NUM_PER_PORT   10

#else

#define MV_HWS_MAX_LANES_NUM_PER_PORT   4

#endif

#define MV_HWS_MAX_PORT_NUM 72
#ifdef ALDRIN_DEV_SUPPORT
#define ALDRIN_PORTS_NUM 32
#endif

#ifdef BV_DEV_SUPPORT
#define MV_PORT_CTRL_BV_MAX_PORT_NUM    8
#endif

#define MV_HWS_MAX_CTLE_BIT_MAPS_NUM 3
#if defined(BC3_DEV_SUPPORT) || defined(BOBK_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)|| defined(ALDRIN2_DEV_SUPPORT)
#define HWS_MAX_CTLE_DB_SIZE 32
#endif

enum{ EXTERNAL_REG, INTERNAL_REG};
enum{ REG_READ_OP, REG_WRITE_OP};

#if defined (CM3) && (defined (BOBK_DEV_SUPPORT) || defined (ALDRIN_DEV_SUPPORT))
typedef enum
{
    BobK,
    Aldrin,
    LAST_SIL_TYPE,
    Lion2A0,
    Lion2B0,
    Puma3A0,
    HooperA0,
    Puma3B0,
    NP5A0,
    BobcatA0,
    Alleycat3A0,
    NP5B0,
    Bobcat3,
    BearValley,
    Pipe,
    Aldrin2,
    Raven,
    Falcon

}MV_HWS_DEV_TYPE;
#else
typedef enum
{
    Lion2A0,
    Lion2B0,
    Puma3A0,
    HooperA0,
    Puma3B0,
    NP5A0,
    BobcatA0,
    Alleycat3A0,
    NP5B0,
    BobK,
    Aldrin,
    Bobcat3,
    BearValley,
    Pipe,
    Aldrin2,
    Raven,
    Falcon,
    LAST_SIL_TYPE

}MV_HWS_DEV_TYPE;
#endif


extern char *devTypeNamesArray[LAST_SIL_TYPE];

typedef enum
{
    GEMAC_UNIT,
    XLGMAC_UNIT,
    HGLMAC_UNIT,
    XPCS_UNIT,
    MMPCS_UNIT,

    CG_UNIT,
    INTLKN_UNIT,
    INTLKN_RF_UNIT,
    SERDES_UNIT,
    SERDES_PHY_UNIT,
    ETI_UNIT,
    ETI_ILKN_RF_UNIT,

    D_UNIT, /* DDR3 */

    CGPCS_UNIT,
    CG_RS_FEC_UNIT,

    PTP_UNIT,

    D2D_EAGLE_UNIT,
    D2D_RAVEN_UNIT,
    RAVEN_UNIT_BASE,
    RAVEN_UNIT_MTI_MAC400,
    RAVEN_UNIT_MTI_MAC100,
    RAVEN_UNIT_MTI_PCS50,
    RAVEN_UNIT_MTI_PCS100,
    RAVEN_UNIT_MTI_PCS200,
    RAVEN_UNIT_MTI_PCS400,
    RAVEN_UNIT_MTI_LOW_SP_PCS, /* low speed 8channel PCS */
    RAVEN_UNIT_MTI_EXT,
    RAVEN_UNIT_MTI_RSFEC,
    RAVEN_UNIT_MSDB,
    RAVEN_UNIT_MPFS,
    RAVEN_UNIT_TSU,
    RAVEN_UNIT_STATISTICS,

    LAST_UNIT

}MV_HWS_UNITS_ID;

typedef enum
{
    HOST2HWS_LEGACY_MODE,
    HOST2SERVICE_CPU_IPC_CONNECTION,
    LAST_HOST2HWS_CONNECTION_TYPE

}MV_HWS_CONNECTION_TYPE;

typedef struct
{
    GT_U32 baseAddr;
    GT_U32 regOffset;
}HWS_UNIT_INFO;

typedef int (*MV_HWS_SBUS_ADDR_TO_SERDES_NUM_FUNC_PTR)
(
    unsigned char devNum,
    GT_U32 *serdesNum,
    unsigned int  sbusAddr
);

typedef struct
{
    GT_U32                          serdesNumber;
    GT_U32                          *serdesToAvagoMapPtr;
    GT_U32                          spicoNumber;
    GT_U32                          *avagoToSerdesMapPtr;
    GT_BOOL                         cpllInitDoneStatusArr[MV_HWS_MAX_CPLL_NUMBER];
    MV_HWS_CPLL_OUTPUT_FREQUENCY    cpllCurrentOutFreqArr[MV_HWS_MAX_CPLL_NUMBER];
    GT_BOOL                         serdesVosOverride;
    MV_HWS_SBUS_ADDR_TO_SERDES_NUM_FUNC_PTR sbusAddrToSerdesFuncPtr;
    GT_U32                          ctleBiasOverride[(MV_HWS_MAX_PORT_NUM+31)/32];
    GT_U32                          ctleBiasValue[(MV_HWS_MAX_PORT_NUM+31)/32];
    GT_U8                           ctleBiasLanesNum;
    GT_U16                          ctleBiasMacNum;

}MV_HWS_AVAGO_SERDES_INFO_STC;

#ifndef MV_HWS_FREE_RTOS
typedef struct{
    GT_U32       phase_tvalBefore_tvSec[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32       phase_tvalBefore_tvUsec[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8        phase_contextSwitchedBack;
    GT_U32       phase_edgeDetectin_Sec;
    GT_U32       phase_edgeDetectin_Usec;
    GT_U8        phase_edgeDetectin_Flag;
    GT_32        phase_tmpTimeout;
    GT_32        phase_firstEO[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32       phase_pCalTimeNano[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32       phase_pCalTimeSec[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL      phase_enable[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL      phase_breakLoop[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL      phase_breakLoopTotal;
    GT_BOOL      phase_breakLoopInd;
    MV_HWS_AUTO_TUNE_RESULTS phase_tuneRes[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL      initFlag;

} MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC;

typedef struct{
    GT_U32  phase_CurrentDelaySize;
    GT_U32  *phase_CurrentDelayPtr;
    GT_U32  phase_InOutI;
    GT_U8   phase_InOutKk;
    GT_BOOL phase_continueExecute;
    GT_U32  phase_Best_LF[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32  phase_Best_eye[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U32  phase_Best_dly[MV_HWS_MAX_LANES_NUM_PER_PORT];
    int     phase_LF1_Arr[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8   subPhase;
    GT_BOOL parametersInitialized;
    GT_BOOL waitForCoreReady;

} MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC;


typedef struct
{
    GT_BOOL coreStatusReady[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_BOOL parametersInitialized;
}MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC;


typedef struct{
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC   *edgeDetectStcPtr;
    MV_HWS_AVAGO_PORT_MANAGER_ENHANCE_TUNE_STC  *enhanceTuneStcPtr;
    MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *oneShotTuneStcPtr;

} MV_HWS_AVAGO_PORT_MANAGER_STC;

typedef MV_HWS_AVAGO_PORT_MANAGER_STC* MV_HWS_AVAGO_PORT_MANAGER_STC_PTR;

/*******************************************************************************
* CORE_CLOCK_GET
*
*
*******************************************************************************/
typedef GT_STATUS (*MV_CORE_CLOCK_GET)
(
    IN  GT_U8   devNum,
    OUT GT_U32  *coreClkDbPtr,
    OUT GT_U32  *coreClkHwPtr
);
#endif /* MV_HWS_FREE_RTOS */


typedef struct{
    GT_U8   phase;
    GT_U8   phaseFinished;

} MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS;

typedef struct{
    GT_U8       phase_CurrentDelaySize;
    GT_U8       *phase_CurrentDelayPtr;
    GT_U8       phase_InOutI;
    GT_U16      phase_Best_eye[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8       phase_Best_dly[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8       subPhase;
    GT_U8       parametersInitialized;
    MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS   phaseEnhanceTuneLiteParams;

} MV_HWS_PORT_ENHANCE_TUNE_LITE_STC;

typedef struct MV_HWS_PORT_INIT_PARAMS_STC
{
    MV_HWS_PORT_STANDARD        portStandard;
    MV_HWS_PORT_MAC_TYPE        portMacType;
    GT_U16                      portMacNumber;
    MV_HWS_PORT_PCS_TYPE        portPcsType;
    GT_U16                      portPcsNumber;
    MV_HWS_PORT_FEC_MODE        portFecMode;
    MV_HWS_SERDES_SPEED         serdesSpeed;
    GT_U16                      firstLaneNum;
    GT_U16                      activeLanesList[MV_HWS_MAX_LANES_NUM_PER_PORT];
    GT_U8                       numOfActLanes;
    MV_HWS_SERDES_MEDIA         serdesMediaType;
    MV_HWS_SERDES_BUS_WIDTH_ENT serdes10BitStatus;
    GT_U8                       fecSupportedModesBmp;

}MV_HWS_PORT_INIT_PARAMS;

typedef struct
{
    GT_U8                           devNum;
    GT_U32                          portsNum;
    MV_HWS_DEV_TYPE                 devType;
    GT_U32                          serdesType;
    GT_U32                          lastSupPortMode;
    GT_U32                          gopRev;
    MV_HWS_CONNECTION_TYPE          ipcConnType;
    MV_HWS_AVAGO_SERDES_INFO_STC    serdesInfo;
    MV_HWS_PORT_INIT_PARAMS         **portsParamsSupModesMap;
#if defined(ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
    MV_HWS_PORT_ENHANCE_TUNE_LITE_STC   portEnhanceTuneLiteDbPtr[MV_PORT_CTRL_MAX_AP_PORT_NUM];
#elif defined(BV_DEV_SUPPORT)
    MV_HWS_PORT_ENHANCE_TUNE_LITE_STC   portEnhanceTuneLiteDbPtr[MV_PORT_CTRL_BV_MAX_PORT_NUM];
#endif
#ifndef MV_HWS_FREE_RTOS
    MV_HWS_AVAGO_PORT_MANAGER_STC_PTR   *portManagerPerPortDbPtrPtr;
    MV_CORE_CLOCK_GET                   coreClockGetPtr;
#endif
}HWS_DEVICE_INFO;

typedef struct intlknIpRegsDef
{
    GT_U32 regOffset;
    GT_U32 defValue;
}MV_INTLK_REG_DEF;

typedef struct
{
    GT_U8  devNum;
    GT_U32 portGroup;
    GT_U32 elementNum;

}MV_HWS_ELEMENTS_ARRAY;

#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
typedef struct
{
    GT_U32 ilkn0MacCfg0;
    GT_U32 ilkn0ChFcCfg0;
    GT_U32 ilkn0MacCfg2;
    GT_U32 ilkn0MacCfg3;
    GT_U32 ilkn0MacCfg4;
    GT_U32 ilkn0MacCfg6;
    GT_U32 ilkn0ChFcCfg1;
    GT_U32 ilkn0PcsCfg0;
    GT_U32 ilkn0PcsCfg1;
    GT_U32 ilkn0En;
    GT_U32 ilkn0StatEn;

}MV_INTLKN_REG;
#endif

typedef enum
{
    GOP_40NM_REV1, /* Lion2-B0/Hooper */
    GOP_40NM_REV2,
    GOP_28NM_REV1, /* AC3, BC2-A0 */
    GOP_28NM_REV2, /* BC2-B0 */
    GOP_28NM_REV3, /* BobK, Aldrin, AC3X, Bobcat3 */
    GOP_16NM_REV1  /* Raven */

}HWS_DEV_GOP_REV;

extern HWS_DEVICE_INFO hwsDeviceSpecInfo[HWS_MAX_DEVICE_NUM];

#define HWS_CORE_PORTS_NUM(devNum)      hwsDeviceSpecInfo[devNum].portsNum
#define HWS_DEV_SERDES_TYPE(devNum)     hwsDeviceSpecInfo[devNum].serdesType
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
 #define HWS_DEV_SILICON_TYPE(devNum)    hwsDeviceSpecInfo[devNum].devType
#else
 #if defined(ALDRIN_DEV_SUPPORT)
  #define HWS_DEV_SILICON_TYPE(devNum) Aldrin
 #elif defined(BOBK_DEV_SUPPORT)
   #define HWS_DEV_SILICON_TYPE(devNum) BobK
 #else
  #define HWS_DEV_SILICON_TYPE(devNum)    hwsDeviceSpecInfo[devNum].devType
 #endif
#endif
#ifdef  MV_HWS_FREE_RTOS
#define HWS_DEV_SILICON_INDEX(devNum)   0
#else
#define HWS_DEV_SILICON_INDEX(devNum)   HWS_DEV_SILICON_TYPE(devNum)
#endif
#define HWS_DEV_PORT_MODES(devNum)      hwsDeviceSpecInfo[devNum].lastSupPortMode
#define HWS_DEV_GOP_REV(devNum)         hwsDeviceSpecInfo[devNum].gopRev

#define HWS_DEV_PORTS_ARRAY(devNum)     (hwsDevicesPortsElementsArray[devNum])

#define HWS_DEV_IS_BOBCAT2_A0(devNum)   (HWS_DEV_GOP_REV(devNum) == GOP_28NM_REV1) && (HWS_DEV_SILICON_TYPE(devNum) == BobcatA0)
#define HWS_DEV_IS_BOBCAT2_B0(devNum)   (HWS_DEV_GOP_REV(devNum) == GOP_28NM_REV2) && (HWS_DEV_SILICON_TYPE(devNum) == BobcatA0)

typedef GT_STATUS (*MV_HWS_REDUNDANCY_VECTOR_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  *sdVector
);

typedef GT_STATUS (*MV_HWS_CLOCK_SELECT_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroupNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);


typedef GT_STATUS (*MV_HWS_PORT_INIT_FUNC_PTR)
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_INIT_INPUT_PARAMS   *portParamsPtr
);

typedef GT_STATUS (*MV_HWS_PORT_RESET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ACTION      action
);

typedef GT_STATUS (*MV_HWS_PORT_PSYNC_BYPASS_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

typedef GT_STATUS (*MV_HWS_PORT_FIX_ALIGN90_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

typedef GT_STATUS (*MV_HWS_PORT_AUTO_TUNE_SET_EXT_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    GT_U32                  optAlgoMask,
    void *                  results
);

typedef GT_STATUS (*MV_HWS_PORT_AUTO_TUNE_SET_WA_FUNC_PTR)
(
       GT_U8                devNum,
       GT_U32               portGroup,
       GT_U32               phyPortNum,
       MV_HWS_PORT_STANDARD portMode,
       GT_U32               *curLanesList
);

typedef GT_STATUS (*MV_HWS_PORT_AUTO_TUNE_STOP_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 stopRx,
    GT_BOOL                 stopTx
);

typedef GT_STATUS (*MV_HWS_PORT_GEAR_BOX_FUNC_PTR)
(
    void
);

typedef GT_STATUS (*MV_HWS_PORT_EXT_MODE_CFG_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 extendedMode
);

typedef GT_STATUS  (*MV_HWS_PORT_EXT_MODE_CFG_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *extendedMode
);

#if 0
typedef GT_STATUS(*MV_HWS_PORT_BETTER_ADAPTATION_SET_FUNC_PTR)
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *betterAlign90
);
#endif

typedef GT_STATUS  (*MV_HWS_PORT_FINE_TUNE_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 configPpm
);

typedef GT_STATUS (*MV_HWS_PORT_LB_SET_WA_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  lbType,
    GT_U32                  *curLanesList
);

typedef GT_U32 (*MV_HWS_PORT_PARAMS_INDEX_GET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

typedef GT_STATUS  (*MV_HWS_PCS_MARK_MODE_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
);

typedef GT_STATUS (*MV_HWS_XPCS_CONNECT_WA_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum
);

typedef GT_U32 (*MV_HWS_PORT_PARAMS_SET_FUNC_PTR)
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

typedef GT_U32 (*MV_HWS_PORT_PARAMS_GET_LANES_FUNC_PTR)
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      portNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
);

typedef GT_U32 (*MV_HWS_CORE_CLOCK_GET_FUNC_PTR)
(
    GT_U8                       devNum,
    GT_U32                      *coreClkDbPtr,
    GT_U32                      *coreClkHwPtr
);

/*******************************************************************************
* MV_SYS_DEVICE_INFO
*
* DESCRIPTION:
*       Return silicon ID and revision ID for current device number.
*
* INPUTS:
*       devNum - system device number
*
* OUTPUTS:
*       devId  - silicon ID
*       revNum - revision number
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_SYS_DEVICE_INFO)
(
    IN  GT_U8  devNum,
    OUT GT_U32 *devId,
    OUT GT_U32 *revNum
);

typedef struct
{
    MV_HWS_REDUNDANCY_VECTOR_GET_FUNC_PTR   redundVectorGetFunc;
    MV_HWS_CLOCK_SELECT_CFG_FUNC_PTR        clkSelCfgGetFunc;
    MV_HWS_PORT_INIT_FUNC_PTR               devPortinitFunc;
    MV_HWS_PORT_RESET_FUNC_PTR              devPortResetFunc;
    MV_HWS_PORT_AUTO_TUNE_SET_EXT_FUNC_PTR  portAutoTuneSetExtFunc;
    MV_HWS_PORT_AUTO_TUNE_SET_WA_FUNC_PTR   portAutoTuneSetWaFunc;
    MV_HWS_PORT_AUTO_TUNE_STOP_FUNC_PTR     portAutoTuneStopFunc;
    MV_HWS_PORT_EXT_MODE_CFG_FUNC_PTR       portExtModeCfgFunc;
    MV_HWS_PORT_EXT_MODE_CFG_GET_FUNC_PTR   portExtModeCfgGetFunc;
    MV_HWS_PORT_FIX_ALIGN90_FUNC_PTR        portFixAlign90Func;
    MV_HWS_PORT_LB_SET_WA_FUNC_PTR          portLbSetWaFunc;
    MV_HWS_PORT_PARAMS_INDEX_GET_FUNC_PTR   portParamsIndexGetFunc;
    MV_HWS_PCS_MARK_MODE_FUNC_PTR           pcsMarkModeFunc;
    MV_SYS_DEVICE_INFO                      deviceInfoFunc;
    MV_HWS_PORT_INIT_FUNC_PTR               devApPortinitFunc;
    MV_HWS_PORT_PARAMS_SET_FUNC_PTR         portParamsSetFunc;
    MV_HWS_PORT_PARAMS_GET_LANES_FUNC_PTR   portParamsGetLanesFunc;
    MV_HWS_CORE_CLOCK_GET_FUNC_PTR          coreClockGetFunc;
}MV_HWS_DEV_FUNC_PTRS;

/*******************************************************************************
* MV_SIL_ILKN_REG_DB_GET
*
* DESCRIPTION:
*       return a ILKN register shadow Db.
*
* INPUTS:
*       devNum      - system device number
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to ILKN DB
*
* COMMENTS:
*       None
*
*******************************************************************************/
#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
typedef MV_INTLKN_REG* (*MV_SIL_ILKN_REG_DB_GET)
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 address
);

extern MV_SIL_ILKN_REG_DB_GET hwsIlknRegDbGetFuncPtr;
#endif
/* os wrapper function prototypes */
/*******************************************************************************
* MV_OS_MALLOC_FUNC
*
* DESCRIPTION:
*       Allocates memory block of specified size.
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*******************************************************************************/
typedef void * (*MV_OS_MALLOC_FUNC)
(
    GT_U32 size
);
/*******************************************************************************
* MV_OS_FREE_FUNC
*
* DESCRIPTION:
*       De-allocates or frees a specified memory block.
*
* INPUTS:
*       memblock - previously allocated memory block to be freed
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*******************************************************************************/
typedef void (*MV_OS_FREE_FUNC)
(
    IN void* const memblock
);

/*******************************************************************************
* MV_OS_MEM_SET_FUNC
*
* DESCRIPTION:
*       Stores 'symbol' converted to an unsigned char in each of the elements
*       of the array of unsigned char beginning at 'start', with size 'size'.
*
* INPUTS:
*       start  - start address of memory block for setting
*       symbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
*******************************************************************************/
typedef void * (*MV_OS_MEM_SET_FUNC)
(
    IN void * start,
    IN int    symbol,
    IN GT_U32 size
);

/*******************************************************************************
* MV_OS_TIME_WK_AFTER_FUNC
*
* DESCRIPTION:
*       Puts current task to sleep for specified number of millisecond.
*
* INPUTS:
*       mils - time to sleep in milliseconds
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_OS_TIME_WK_AFTER_FUNC)
(
    IN GT_U32 mils
);

/*******************************************************************************
* MV_OS_EXACT_DELAY_FUNC
*
* DESCRIPTION:
*       Implement exact time delay for specified number of millisecond.
*
* INPUTS:
*       mils - time to delay in milliseconds
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_OS_EXACT_DELAY_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 mils
);

/*******************************************************************************
* MV_OS_MICRO_DELAY_FUNC
*
* DESCRIPTION:
*       Implement delay for specified number of micro seconds.
*
* INPUTS:
*       microSec - time to delay in micro seconds
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_OS_MICRO_DELAY_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 microSec
);

/*******************************************************************************
* MV_SERDES_REG_ACCESS_SET
*
* DESCRIPTION:
*       Implement write access to SERDES external/internal registers.
*
* INPUTS:
*       devNum - system device number
*       portGroup - port group number
*       serdesNum   - seredes number to access
*       regAddr     - serdes register address (offset) to access
*       data        - data to write
*       mask        - mask write
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_SERDES_REG_ACCESS_SET)
(
    IN  GT_U8        devNum,
    IN  GT_UOPT      portGroup,
    IN  GT_U8        regType,
    IN  GT_UOPT      serdesNum,
    IN  GT_UREG_DATA regAddr,
    IN  GT_UREG_DATA data,
    IN  GT_UREG_DATA mask
);

/*******************************************************************************
* MV_REG_ACCESS_SET
*
* DESCRIPTION:
*       Implement write access to external/internal registers.
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group number
*       address     - register address (offset) to access
*       data        - data to write
*       mask        - mask write
*
* OUTPUTS:
*       none
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_REG_ACCESS_SET)
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 address,
    IN GT_U32 data,
    IN GT_U32 mask
);

/*******************************************************************************
* MV_SERDES_REG_ACCESS_GET
*
* DESCRIPTION:
*       Implement read access from SERDES external/internal registers.
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group number
*       serdesNum   - seredes number to access
*       regAddr     - serdes register address (offset) to access
*       mask        - read mask
*
* OUTPUTS:
*       data        - read data
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_SERDES_REG_ACCESS_GET)
(
    IN  GT_U8        devNum,
    IN  GT_UOPT      portGroup,
    IN  GT_U8        regType,
    IN  GT_UOPT      serdesNum,
    IN  GT_UREG_DATA regAddr,
    IN  GT_UREG_DATA *data,
    IN  GT_UREG_DATA mask
);

/*******************************************************************************
* MV_REG_ACCESS_GET
*
* DESCRIPTION:
*       Implement read access from external/internal registers.
*
* INPUTS:
*       devNum      - system device number
*       portGroup   - port group number
*       address     - register address (offset) to access
*       mask        - read mask
*
* OUTPUTS:
*       data        - read data
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
typedef GT_STATUS (*MV_REG_ACCESS_GET)
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroup,
    IN  GT_U32  address,
    OUT GT_U32  *data,
    IN  GT_U32  mask
);


/******************************************************************************
* Name:     MV_SERVER_REG_ACCESS_SET.
* Desc:     definition of server write prototype
* Args:     TBD
* Notes:
* Returns:  OK if success, other error code if fail.
*/
typedef GT_STATUS (*MV_SERVER_REG_ACCESS_SET)
(
    GT_U8 devNum,
    GT_U32 addr,
    GT_U32 data
);

/******************************************************************************
* Name:     MV_SERVER_REG_ACCESS_GET.
* Desc:     definition of server read prototype
* Args:     TBD
* Notes:
* Returns:  OK if success, other error code if fail.
*/
typedef GT_STATUS (*MV_SERVER_REG_ACCESS_GET)
(
    GT_U8 devNum,
    GT_U32 addr,
    GT_U32 *data
);

/******************************************************************************
* Name:     MV_SERVER_REG_FIELD_ACCESS_SET.
* Desc:     definition of server read prototype
* Args:     TBD
* Notes:
* Returns:  OK if success, other error code if fail.
*/
typedef GT_STATUS (*MV_SERVER_REG_FIELD_ACCESS_SET)
(
    GT_U8   devNum,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  fieldData
);

/******************************************************************************
* Name:     MV_SERVER_REG_FIELD_ACCESS_GET.
* Desc:     definition of server read prototype
* Args:     TBD
* Notes:
* Returns:  OK if success, other error code if fail.
*/
typedef GT_STATUS (*MV_SERVER_REG_FIELD_ACCESS_GET)
(
    GT_U8   devNum,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  *fieldData
);

/******************************************************************************
* Name:     MV_TIMER_GET.
* Desc:     definition of get timer prototype
* Args:     TBD
* Notes:
* Returns:  OK if success, other error code if fail.
*/
typedef GT_STATUS (*MV_TIMER_GET)
(
    GT_U32  *seconds,
    GT_U32  *nanoSeconds
);

/*******************************************************************************
* MV_OS_MEM_COPY_FUNC
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef void (*MV_OS_MEM_COPY_FUNC)
(
    IN void *       destination,
    IN const void * source,
    IN GT_U32       size
);

/*******************************************************************************
* MV_OS_STR_CAT_FUNC
*
* DESCRIPTION:
*       Appends a copy of string 'str2' to the end of string 'str1'.
*
* INPUTS:
*       str1   - destination string
*       str2   - string to add the destination string
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to the destination string (str1)
*
* COMMENTS:
*       The resulting string is null-terminated.
*
*******************************************************************************/
typedef GT_CHAR* (*MV_OS_STR_CAT_FUNC)
(
    IN GT_CHAR         *str1,
    IN const GT_CHAR   *str2
);

/* structure that hold the "os" functions needed be bound to HWS */
typedef struct {
  MV_OS_EXACT_DELAY_FUNC   osExactDelayPtr;
  MV_OS_MICRO_DELAY_FUNC   osMicroDelayPtr;
  MV_OS_TIME_WK_AFTER_FUNC osTimerWkPtr;
  MV_OS_MEM_SET_FUNC       osMemSetPtr;
  MV_OS_FREE_FUNC          osFreePtr;
  MV_OS_MALLOC_FUNC        osMallocPtr;
  MV_SYS_DEVICE_INFO       sysDeviceInfo;
  MV_SERDES_REG_ACCESS_GET serdesRegGetAccess;
  MV_SERDES_REG_ACCESS_SET serdesRegSetAccess;
  MV_SERVER_REG_ACCESS_GET serverRegGetAccess;
  MV_SERVER_REG_ACCESS_SET serverRegSetAccess;
  MV_OS_MEM_COPY_FUNC      osMemCopyPtr;
  MV_OS_STR_CAT_FUNC       osStrCatPtr;
  MV_SERVER_REG_FIELD_ACCESS_SET serverRegFieldSetAccess;
  MV_SERVER_REG_FIELD_ACCESS_GET serverRegFieldGetAccess;
  MV_TIMER_GET             timerGet;
  MV_REG_ACCESS_GET        registerGetAccess;
  MV_REG_ACCESS_SET        registerSetAccess;
  void                     *extFunctionStcPtr;
#ifndef  MV_HWS_FREE_RTOS
  MV_CORE_CLOCK_GET        coreClockGetPtr;
#endif

}HWS_OS_FUNC_PTR;

#ifndef  MV_HWS_FREE_RTOS
typedef GT_STATUS (*MV_OS_GET_DEVICE_DRIVER_FUNC)
(
 IN  GT_U8                       devNum,
 IN  GT_U32                      portGroupId,
 IN  GT_U32                      smiIntr,
 IN  GT_U32                      slaveSmiPhyId,
 /*OUT CPSS_HW_INFO_STC          *hwInfoPtr*/
 OUT CPSS_HW_DRIVER_STC          **ssmiPtr
);
typedef GT_STATUS (*MV_OS_HW_TRACE_ENABLE_FUNC)
(
 IN  GT_U8                       devNum,
 IN  GT_U32                      traceType,/*0=read, 1=write, 2=both, 3=write_delay, 4=all*/
 IN  GT_BOOL                     enable
);

typedef struct {
    MV_OS_GET_DEVICE_DRIVER_FUNC getDeviceDriver;
    MV_OS_HW_TRACE_ENABLE_FUNC   ppHwTraceEnable;
}HWS_EXT_FUNC_STC_PTR;
extern MV_OS_HW_TRACE_ENABLE_FUNC hwsPpHwTraceEnablePtr;

#endif

extern MV_OS_EXACT_DELAY_FUNC   hwsOsExactDelayPtr;
extern MV_OS_MICRO_DELAY_FUNC   hwsOsMicroDelayPtr;
extern MV_OS_TIME_WK_AFTER_FUNC hwsOsTimerWkFuncPtr;
extern MV_OS_MEM_SET_FUNC       hwsOsMemSetFuncPtr;
extern MV_OS_FREE_FUNC          hwsOsFreeFuncPtr;
extern MV_OS_MALLOC_FUNC        hwsOsMallocFuncPtr;
extern MV_SERDES_REG_ACCESS_SET hwsSerdesRegSetFuncPtr;
extern MV_SERDES_REG_ACCESS_GET hwsSerdesRegGetFuncPtr;
extern MV_OS_MEM_COPY_FUNC      hwsOsMemCopyFuncPtr;
extern MV_OS_STR_CAT_FUNC       hwsOsStrCatFuncPtr;
extern MV_SERVER_REG_ACCESS_SET hwsServerRegSetFuncPtr;
extern MV_SERVER_REG_ACCESS_GET hwsServerRegGetFuncPtr;
extern MV_SERVER_REG_FIELD_ACCESS_SET hwsServerRegFieldSetFuncPtr;
extern MV_SERVER_REG_FIELD_ACCESS_GET hwsServerRegFieldGetFuncPtr;
extern MV_TIMER_GET             hwsTimerGetFuncPtr;
extern MV_REG_ACCESS_SET        hwsRegisterSetFuncPtr;
extern MV_REG_ACCESS_GET        hwsRegisterGetFuncPtr;
extern GT_STATUS hwsRegisterSetFieldFunc
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  fieldData
);
extern GT_STATUS hwsRegisterGetFieldFunc
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  *fieldDataPtr
);

/**
* @internal hwsOsLocalMicroDelay function
* @endinternal
*
* @brief   This API is used in case micro sec counter is not supported
*         It will convert micro to msec and round up in case needed
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] microSec                 - Delay in micro sec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsOsLocalMicroDelay
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 microSec
);

/**
* @internal mvUnitExtInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit based on
*         unit index.
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
* @param[in] unitIndex                - unit index
*
* @param[out] baseAddr                 - unit base address in device
* @param[out] unitIndex                - unit index in device
* @param[out] localUnitNumPtr          - (pointer to) local unit number (support for Bobcat3 multi-pipe)
*                                       None
*/
GT_STATUS  mvUnitExtInfoGet
(
    GT_U8           devNum,
    MV_HWS_UNITS_ID unitId,
    GT_U32           unitNum,
    GT_U32          *baseAddr,
    GT_U32          *unitIndex,
    GT_U32          *localUnitNumPtr
);

/**
* @internal mvUnitInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit
*
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @param[out] baseAddr                 - unit base address in device
* @param[out] unitIndex                - unit index in device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoGet
(
    GT_U8           devNum,
    MV_HWS_UNITS_ID unitId,
    GT_U32          *baseAddr,
    GT_U32          *unitIndex
);

/**
* @internal mvUnitInfoSet function
* @endinternal
*
* @brief   Init silicon specific base address and index for specified unit
*
* @param[in] devType                  - Device type
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
* @param[in] baseAddr                 - unit base address in device
* @param[in] unitIndex                - unit index in device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoSet
(
    MV_HWS_DEV_TYPE devType,
    MV_HWS_UNITS_ID unitId,
    GT_U32          baseAddr,
    GT_U32          unitIndex
);

/**
* @internal mvUnitInfoGetByAddr function
* @endinternal
*
* @brief   Return unit ID by unit address in device
*
* @param[in] devNum                   - Device Number
* @param[in] baseAddr                 - unit base address in device
*
* @param[out] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoGetByAddr
(
    GT_U8           devNum,
    GT_U32          baseAddr,
    MV_HWS_UNITS_ID *unitId
);

/**
* @internal genUnitRegisterSet function
* @endinternal
*
* @brief   Implement write access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
*                                      address   - address to access
* @param[in] data                     -  to write
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterSet
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    MV_HWS_UNITS_ID unitId,
    GT_UOPT         unitNum,
    GT_UREG_DATA    regOffset,
    GT_UREG_DATA    data,
    GT_UREG_DATA    mask
);

/**
* @internal genUnitRegisterGet function
* @endinternal
*
* @brief   Read access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
*                                      address   - address to access
*
* @param[out] data                     - read data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterGet
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    MV_HWS_UNITS_ID unitId,
    GT_UOPT         unitNum,
    GT_UREG_DATA    regOffset,
    GT_UREG_DATA    *data,
    GT_UREG_DATA    mask
);

/**
* @internal genInterlakenRegSet function
* @endinternal
*
* @brief   Implement write access to INERLAKEN IP registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] address                  -  to access
* @param[in] data                     -  to write
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genInterlakenRegSet
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 address,
    GT_U32 data,
    GT_U32 mask
);

/**
* @internal genInterlakenRegGet function
* @endinternal
*
* @brief   Implement read access to INERLAKEN IP registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] address                  -  to access
*
* @param[out] data                     - read data
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genInterlakenRegGet
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 address,
    GT_U32 *data,
    GT_U32 mask
);

#if defined(CHX_FAMILY) || !defined(PX_FAMILY)
/**
* @internal hwsLion2IfInit function
* @endinternal
*
* @brief   Init all supported units needed for ports initialization
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsLion2IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsLion2IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsLion2IfClose(GT_U8 devNum);

/**
* @internal hwsHooperIfInit function
* @endinternal
*
* @brief   Init all supported units needed for ports initialization
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsHooperIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsHooperIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsHooperIfClose(GT_U8 devNum);

/**
* @internal hwsNp5IfInit function
* @endinternal
*
* @brief   Init all supported units needed for ports initialization
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsNp5IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsNp5IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsNp5IfClose(GT_U8 devNum);

/**
* @internal hwsBobcat2IfInit function
* @endinternal
*
* @brief   Init all supported units needed for ports initialization
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat2IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsBobcat2IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsBobcat2IfClose(GT_U8 devNum);

/**
* @internal mvHwsAlleycat3IfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat3IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsAlleycat3IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAlleycat3IfClose(GT_U8 devNum);

/**
* @internal hwsBobKIfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobKIfPreInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsBobKIfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobKIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsAldrinIfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrinIfPreInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsBobcat3IfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat3IfPreInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsAldrin2IfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrin2IfPreInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsFalconIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsFalconIfPreInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsAldrinIfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrinIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsBobcat3IfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobcat3IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsAldrin2IfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsAldrin2IfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsRavenIfPreInit function
* @endinternal
 *
*/

/**
* @internal hwsFalconIfInit function
* @endinternal
 *
*/
GT_STATUS hwsFalconIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

GT_STATUS hwsFalconStep0
(
    GT_U8 devNum,
    IN GT_U32 numOfTiles
);

GT_STATUS hwsRavenIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
);

/**
* @internal hwsRavenIfInit function
* @endinternal
 *
*/
GT_STATUS hwsRavenIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsBobKSerdesIfInit function
* @endinternal
 *
*/
GT_STATUS hwsRavenCpllConfig(GT_U8 devNum);

/**
* @internal hwsBobKSerdesIfInit function
* @endinternal
*
* @brief   Init all supported Serdes types.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsBobKSerdesIfInit(GT_U8 devNum);

/**
* @internal hwsBobKIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsBobKIfClose(GT_U8 devNum);

/**
* @internal hwsAldrinIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAldrinIfClose(GT_U8 devNum);

/**
* @internal hwsBobcat3IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsBobcat3IfClose(GT_U8 devNum);

/**
* @internal hwsAldrin2IfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsAldrin2IfClose(GT_U8 devNum);

/**
* @internal hwsFalconIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsFalconIfClose(GT_U8 devNum);


#endif /* #if defined(CHX_FAMILY) || !defined(PX_FAMILY) */

#if defined(PX_FAMILY) || !defined(CHX_FAMILY)
/**
* @internal hwsPipeIfPreInit function
* @endinternal
*
* @brief   Init all supported units needed for port initialization.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPipeIfPreInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsPipeIfInit function
* @endinternal
*
* @brief   Init all software related DB: DevInfo, Port (Port modes, MAC, PCS and SERDES)
*         and address mapping.
*         Must be called per device.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsPipeIfInit(GT_U8 devNum, HWS_OS_FUNC_PTR *funcPtr);

/**
* @internal hwsPipeIfClose function
* @endinternal
*
* @brief   Free all resource allocated for ports initialization.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsPipeIfClose(GT_U8 devNum);

#endif
/**
* @internal mvHwsRedundancyVectorGet function
* @endinternal
*
* @brief   Get SD vector.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRedundancyVectorGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  *sdVector
);

/**
* @internal mvHwsClockSelectorConfig function
* @endinternal
*
* @brief   Configures the DP/Core Clock Selector on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsClockSelectorConfig
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
);

/**
* @internal hwsDeviceSpecGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsDeviceSpecGetFuncPtr(MV_HWS_DEV_FUNC_PTRS **hwsFuncsPtr);

/**
* @internal mvHwsReNumberDevNum function
* @endinternal
*
* @brief   Replace the ID of a device from old device number
*         to a new device number
* @param[in] oldDevNum                - old device num
* @param[in] newDevNum                - new device num
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsReNumberDevNum
(
    GT_U8   oldDevNum,
    GT_U8   newDevNum
);

/**
* @internal mvHwsDeviceInit function
* @endinternal
*
* @brief   Init device,
*         according to device type call function to init all software related DB:
*         DevInfo, Port (Port modes, MAC, PCS and SERDES) and address mapping.
* @param[in] devType                  - enum of the device type
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsDeviceInit
(
    GT_U8             devNum,
    MV_HWS_DEV_TYPE   devType,
    HWS_OS_FUNC_PTR   *funcPtr
);

/**
* @internal mvHwsDeviceClose function
* @endinternal
*
* @brief   Close device,
*         according to device type call function to free all resource allocated for ports initialization.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsDeviceClose
(
    GT_U8   devNum
);

/**
* @internal mvHwsGeneralLog function
* @endinternal
*
* @brief   Hws log message builder and logger functionn for HOST/FW
*
* @param[in] funcName                 - function name
* @param[in] fileName                 - file name
* @param[in] lineNum                  - line number
* @param[in] returnCode               - function return code
*                                      ...         - argument list
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsGeneralLog
(
    const char      *funcName,
#ifndef  MV_HWS_FREE_RTOS
    char            *fileName,
    GT_U32          lineNum,
    GT_STATUS       returnCode,
#endif
    ...
);

/*******************************************************************************
* mvHwsGeneralLogStrMsgWrapper
*
* DESCRIPTION:
*       Hws log message builder wrapper function for a log message that is alreay
*       evaluated by the caller.
*
* INPUTS:
*       funcNamePtr    - function name pointer
*       fileNamePtr    - file name pointer
*       lineNum        - line number
*       returnCode     - function return code
*       strMsgPtr      - string message to log
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
unsigned int mvHwsGeneralLogStrMsgWrapper
(
    const char      *funcNamePtr,
#ifndef  MV_HWS_FREE_RTOS
    char            *fileNamePtr,
    unsigned int    lineNum,
    int             returnCode,
#endif
    char            *strMsgPtr
);

#ifdef __cplusplus
}
#endif

#if 0
/**
* @internal hwsAldrin2PortCGCheck function
* @endinternal
*
* @brief   Check if the port is CG port in Aldrin2 device
*
* @param[in] portNum                  - global MAC port number
*
* @retval 0                        - not CG port
* @retval 1                        - CG port
*/
GT_U32 hwsAldrin2PortCGCheck
(
    GT_UOPT         portNum
);
#endif

/**
* @internal hwsAldrin2SerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for Aldrin2
*
* @param[in] serdesNum                - global serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit (0x13000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsAldrin2SerdesAddrCalc
(
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);
/**
* @internal hwsAldrin2GopAddrCalc function
* @endinternal
*
* @brief   Calculate GOP register address for Aldrin2
*
* @param[in] portNum                  - global port number
* @param[in] regAddr                  - offset of required register in GOP Unit (0x10000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsAldrin2GopAddrCalc
(
    GT_UOPT         portNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);
/**
* @internal hwsAldrin2MibAddrCalc function
* @endinternal
*
* @brief   Calculate MIB register address for Aldrin2
*
* @param[in] portNum                  - global port number
* @param[in] regAddr                  - offset of required register in MIB Unit (0x12000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsAldrin2MibAddrCalc
(
    GT_UOPT         portNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

/**
* @internal hwsBobcat3SerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for BC3
*
* @param[in] serdesNum                - serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsBobcat3SerdesAddrCalc
(
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

/* get indication if the Bobcat3 device is actually Falcon */
/* function actually return the number of Tiles (1,2,4)*/
extern GT_U32  hwsFalconAsBobcat3Check(void);

/**
* @internal hwsFalconSerdesAddrCalc function
* @endinternal
*
* @brief   Calculate serdes register address for Falcon
*
* @param[in] serdesNum                - global serdes lane number
* @param[in] regAddr                  - offset of required register in SD Unit (0x13000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsFalconSerdesAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);
/**
* @internal hwsFalconGopAddrCalc function
* @endinternal
*
* @brief   Calculate GOP register address for Falcon
*
* @param[in] portNum                  - global port number
* @param[in] regAddr                  - offset of required register in GOP Unit (0x10000000)
*
* @param[out] addressPtr               - (ptr to) register address
*                                       None
*/
GT_VOID hwsFalconGopAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         portNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

/**
* @internal HWS_UNIT_BASE_ADDR_TYPE_ENT function
* @endinternal
*
*/
typedef enum {
    HWS_UNIT_BASE_ADDR_TYPE_GOP_E,
    HWS_UNIT_BASE_ADDR_TYPE_MIB_E,
    HWS_UNIT_BASE_ADDR_TYPE_SERDES_E,
    HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E,
    HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E,
    HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E,

    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI64_MAC_E,
    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI400_MAC_E,
    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MAC_STATISTICS_E,
    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E,
    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E,  /* AKA : PTP */
    HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_EXT_E,

    HWS_UNIT_BASE_ADDR_TYPE__MUST_BE_LAST__E
}HWS_UNIT_BASE_ADDR_TYPE_ENT;


GT_VOID hwsFalconAddrCalc
(
    GT_U8           devNum,
    HWS_UNIT_BASE_ADDR_TYPE_ENT addressType,
    GT_UOPT         portNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

/*******************************************************************************
* HWS_UNIT_BASE_ADDR_CALC_BIND
*
* DESCRIPTION:
*       function type for Bind function that Calculate the base address
*                    of a global port, for next units :
*                    MIB/SERDES/GOP
*
* INPUTS:
*       unitId     - the unit : MIB/SERDES/GOP
*       portNum    - the global port num
*
* OUTPUTS:
*       None
*
* RETURNS:
*       The address of the port in the unit
*
*******************************************************************************/
typedef GT_UREG_DATA  (*HWS_UNIT_BASE_ADDR_CALC_BIND)(
    GT_U8                           devNum,
    HWS_UNIT_BASE_ADDR_TYPE_ENT     unitId,
    GT_UOPT                         portNum
);

/**
* @internal hwsFalconUnitBaseAddrCalcBind function
* @endinternal
*
* @brief   for FALCON : Bind the HWS with function that Calculate the base address
*         of a global port, for next units :
*         MIB/SERDES/GOP
* @param[in] cbFunc                   - the callback function
*                                       None
*/
GT_VOID hwsFalconUnitBaseAddrCalcBind
(
    HWS_UNIT_BASE_ADDR_CALC_BIND    cbFunc
);


#endif /* __siliconIf_H */



