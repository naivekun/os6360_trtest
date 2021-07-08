/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoBoardConfig.h
*
* @brief Includes board specific initialization definitions and data-structures.
*
* @version   29
********************************************************************************
*/
#ifndef __appDemoBoardConfigh
#define __appDemoBoardConfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef IMPL_FA
#include <cpssFa/generic/cpssFaHwInit/gtCoreFaHwInit.h>
#endif

#ifdef IMPL_XBAR
#include <cpssXbar/generic/xbarHwInit/gtCoreXbarHwInit.h>
#endif

#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

#define APP_DEMO_MAX_HIR_SUPPORTED_PP_CNS 32
#define IMPL_DBG_TRACE

#ifdef IMPL_DBG_TRACE
    #ifdef  _WIN32
        #define DBG_TRACE(x) osPrintf x
    #else /*!_WIN32*/
        #define DBG_TRACE(x) osPrintSync x
    #endif /*!_WIN32*/
#else
    #define DBG_TRACE(x)
#endif

/* flag to state that the trace will print also functions that return GT_OK */
/* usually we not care about 'GT_OK' only about fails */
/* the flag can be changed before running the cpssInitSystem(...) --> from terminal vxWorks */
extern GT_U32  appDemoTraceOn_GT_OK;

#define CPSS_ENABLER_DBG_TRACE_RC_MAC(_title, _rc)     \
   if(_rc != GT_OK || appDemoTraceOn_GT_OK)             \
       DBG_TRACE(("%s returned: 0x%X at file %s, line = %d\r\n", _title, _rc, __FILE__, __LINE__))

extern GT_BOOL appDemoCpssInitSkipHwReset;/* indication that the appDemo initialize devices that skipped HW reset.
                    GT_TRUE  - the device is doing cpssInitSystem(...) after HW did NOT do soft/hard reset.
                    GT_FALSE - the device is doing cpssInitSystem(...) after HW did        soft/hard reset.
 */
extern GT_U32    appDemoCpssCurrentDevIndex;/* the appDemo device index in appDemoPpConfigList[] that currently initialized */

/*******************************************************************************************/

/* Trace enabled by appDemoTraceEnableFilterAdd(line_start, line_end, file_name_substring) */
/**
* @internal appDemoTraceEnableFilterCheck function
* @endinternal
*
* @brief   This function ckecks is trace printing enable for this trace statement
*
* @param[in] filePath                 - path to source file returned by __FILE__ macro.
* @param[in] lineNum                  - number of line returned by __LINE__ macro.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_BOOL appDemoTraceEnableFilterCheck
(
    IN GT_U8  *filePath,
    IN GT_U32 lineNum
);

/* Trace */
/* Trace print line */
#define CPSS_ENABLER_FILTERED_TRACE_MAC(x) \
    if (appDemoTraceEnableFilterCheck((GT_U8*)__FILE__, (GT_U32)__LINE__) != GT_FALSE) {DBG_TRACE(x);}
/* Trace call evaluate expession on trace condition */
#define CPSS_ENABLER_FILTERED_TRACE_EXEC_MAC(x) \
    if (appDemoTraceEnableFilterCheck((GT_U8*)__FILE__, (GT_U32)__LINE__) != GT_FALSE) {x;}
/*******************************************************************************************/

/* when appDemoOnDistributedSimAsicSide == 1
   this image is running on simulation:
   1. our application on distributed "Asic side"
   2. the Other application that run on the "application side" is in
      charge about all Asic configurations
   3. our application responsible to initialize the cpss DB.
   4. the main motivation is to allow "Galtis wrappers" to be called on
      this side !
      (because the Other side application may have difficult running
       "Galtis wrappers" from there)
*/
extern GT_U32  appDemoOnDistributedSimAsicSide;

/*
 * Typedef: FUNCP_GET_BOARD_INFO
 *
 * Description:
 *      Provides board specific memory size configuration.
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      numOfMemBytes   - Memory size in bytes.
 *
 */
typedef GT_STATUS (*FUNCP_GET_BOARD_MEMORY_INFO)
(
    IN  GT_U8   boardRevId,
    OUT GT_U32  *numOfMemBytes
);

/*
 * Typedef: FUNCP_BOARD_CLEAN_DB_DURING_SYSTEM_RESET
 *
 * Description:
 *      clear the DB of the specific board config file , as part of the 'system rest'
 *      to allow the 'cpssInitSystem' to run again as if it is the first time it runs
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_BOARD_CLEAN_DB_DURING_SYSTEM_RESET)
(
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_GET_BOARD_INFO
 *
 * Description:
 *      General board information for initial initialization.
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      numOfPp         - Number of Packet processors in system.
 *      numOfFa         - Number of Fabric Adaptors in system.
 *      numOfXbar       - Number of Crossbar devices in system.
 *      isB2bSystem     - GT_TRUE, the system is a B2B system.
 *
 */
typedef GT_STATUS (*FUNCP_GET_BOARD_INFO)
(
    IN  GT_U8   boardRevId,
    OUT GT_U8   *numOfPp,
    OUT GT_U8   *numOfFa,
    OUT GT_U8   *numOfXbar,
    OUT GT_BOOL *isB2bSystem
);

#ifdef PX_FAMILY
/*
 * Typedef: FUNCP_GET_BOARD_HW_INFO
 *
 * Description:
 *      General board information for initial initialization.
 *
 * Fields:
 *      hwInfoPtr       - HW info structure.
 *      pciInfo         - PCI device information.
 *
 */
typedef GT_STATUS (*FUNCP_GET_BOARD_HW_INFO)
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO      *pciInfo
);
#endif /*PX_FAMILY*/

#ifdef IMPL_XBAR
/*
 * Typedef: FUNCP_GET_CORE_XBAR_CFG_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      coreCfg1Params  - Core xbar config parameters struct.
 */
typedef GT_STATUS (*FUNCP_GET_CORE_XBAR_CFG_PARAMS)
(
    IN  GT_U8                   boardRevId,
    OUT GT_XBAR_CORE_SYS_CFG    *xbarCfgParams
);


/*
 * Typedef: FUNCP_GET_XBAR_PHASE1_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      xbarIdx         - The Xbar device index to get the parameters for.
 *      xbarPhase1Params- Phase1 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_XBAR_PHASE1_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       xbarIdx,
    OUT GT_XBAR_PHASE1_INIT_PARAMS  *phase1Params
);

/*
 * Typedef: FUNCP_GET_XBAR_PHASE2_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      oldXbardevNum   - The old Xbar device number to get the parameters for.
 *      xbarPhase2Params- Phase2 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_XBAR_PHASE2_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldXbardevNum,
    OUT GT_XBAR_PHASE2_INIT_PARAMS  *phase2Params
);

#endif

#ifdef IMPL_FA
/*
 * Typedef: FUNCP_GET_FA_PHASE1_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      faIdx           - The Fa device index to get the parameters for.
 *      faPhase1Params  - Phase1 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_FA_PHASE1_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       faIdx,
    OUT GT_FA_PHASE1_INIT_PARAMS    *phase1Params
);

/*
 * Typedef: FUNCP_GET_FA_PHASE2_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      oldFaDevNum     - The old Fa device number to get the parameters for.
 *      faPhase2Params  - Phase2 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_FA_PHASE2_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldFaDevNum,
    OUT GT_FA_PHASE2_INIT_PARAMS    *phase2Params
);


/*
 * Typedef: FUNCP_GET_FA_REG_CFG_LIST
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devNum          - The device number to get the parameters for.
 *      regCfgList      - A pointer to the register list.
 *      regCfgListSize  - Number of elements in regListPtr.
 *
 */
typedef GT_STATUS (*FUNCP_GET_FA_REG_CFG_LIST)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devNum,
    OUT CPSS_REG_VALUE_INFO_STC     **regCfgList,
    OUT GT_U32                      *regCfgListSize
);
#endif


/*
 * Typedef: FUNCP_BEFORE_PHASE1_CONFIG
 *
 * Description:  prototype for function that initialize "pre-phase 1"
 *               configuration
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_BEFORE_PHASE1_CONFIG)
(
    IN  GT_U8                       boardRevId
);

/*
 * Typedef: FUNCP_GET_PP_PHASE1_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devIdx          - The Pp Index to get the parameters for.
 *      ppPhase1Params  - Phase1 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_PP_PHASE1_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devIdx,
    OUT CPSS_PP_PHASE1_INIT_PARAMS *phase1Params
);


/*
 * Typedef: FUNCP_AFTER_PHASE1_CONFIG
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_AFTER_PHASE1_CONFIG)
(
    IN  GT_U8   boardRevId
);


/*
 * Typedef: FUNCP_GET_PP_PHASE2_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      oldDevNum       - The old Pp device number to get the parameters for.
 *      ppPhase2Params  - Phase2 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_PP_PHASE2_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldDevNum,
    OUT CPSS_PP_PHASE2_INIT_PARAMS  *phase2Params
);


/*
 * Typedef: FUNCP_AFTER_PHASE2_CONFIG
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_AFTER_PHASE2_CONFIG)
(
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_GET_PP_LOGICAL_INIT_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devNum          - The Pp device number to get the parameters for.
 *      ppLogInitParams - Pp logical init parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_PP_LOGICAL_INIT_PARAMS)
(
    IN  GT_U8                      boardRevId,
    IN  GT_U8                      devNum,
    OUT CPSS_PP_CONFIG_INIT_STC   *ppLogInitParams
);


/*
 * Typedef: FUNCP_GET_LIB_INIT_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devNum          - The Pp device number to get the parameters for.
 *      libInitParams   - Library initialization parameters.
 *
 */
typedef GT_STATUS (*FUNCP_GET_LIB_INIT_PARAMS)
(
    IN  GT_U8                        boardRevId,
    IN  GT_U8                        devNum,
    OUT APP_DEMO_LIB_INIT_PARAMS    *libInitParams
);


/*
 * Typedef: FUNCP_AFTER_INIT_CONFIG
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      libInitParams   - Library initialization parameters.
 *
 */
typedef GT_STATUS (*FUNCP_AFTER_INIT_CONFIG)
(
    IN  GT_U8                       boardRevId
);

/* The following 5 function pointer were added to support stack. */
/*
 * Typedef: FUNCP_GET_PP_REG_CFG_LIST
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devNum          - The device number to get the parameters for.
 *      regCfgList      - A pointer to the register list.
 *      regCfgListSize  - Number of elements in regListPtr.
 *
 */
typedef GT_STATUS (*FUNCP_GET_PP_REG_CFG_LIST)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devNum,
    OUT CPSS_REG_VALUE_INFO_STC     **regCfgList,
    OUT GT_U32                      *regCfgListSize
);

/*
 * Typedef: FUNCP_GET_STACK_CONFIG_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      deviceIDsPtr    - List of the device IDs
 *      localUnitNum    - Local unit number
 *      stackParams     - Stack configuration parameters.
 *
 */
typedef GT_STATUS (*FUNCP_GET_STACK_CONFIG_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U32                      *deviceIDsPtr,
    OUT GT_VOID                     *stackParams
);

/*
 * Typedef: FUNCP_GET_MNG_PORT_NAME
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      mngPortName     - Management port name string.
 *
 */
typedef GT_STATUS (*FUNCP_GET_MNG_PORT_NAME)
(
    IN  GT_U8                       boardRevId,
    OUT GT_U8                       *mngPortName
);

/*
 * Typedef: FUNCP_AFTER_STACK_READY_CONFIG
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_AFTER_STACK_READY_CONFIG)
(
    IN  GT_U8                       boardRevId
);

/*
 * Typedef: FUNCP_SIMPLE_INIT
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_SIMPLE_INIT)
(
    IN  GT_U8                       boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_AFTER_INIT_CONFIG
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_DEVICE_AFTER_INIT_CONFIG)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_AFTER_PHASE1
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_DEVICE_AFTER_PHASE1)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_AFTER_PHASE2
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_DEVICE_AFTER_PHASE2)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_CLEANUP
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */

typedef GT_STATUS (*FUNCP_DEVICE_CLEANUP)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_SWCLEANUP
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_DEVICE_SWCLEANUP)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/**
* @struct GT_BOARD_CONFIG_FUNCS
 *
 * @brief Includes board specific control functions.
*/
typedef struct
{
    FUNCP_GET_BOARD_MEMORY_INFO         boardGetMemInfo;
    FUNCP_GET_BOARD_INFO                boardGetInfo;
#ifdef PX_FAMILY
    FUNCP_GET_BOARD_HW_INFO             boardGetHwInfo;
#endif
#ifdef IMPL_XBAR
    FUNCP_GET_CORE_XBAR_CFG_PARAMS      boardGetXbarCfgParams;
#endif

#if defined (IMPL_FA) || defined (IMPL_XBAR)
    FUNCP_GET_XBAR_PHASE1_PARAMS        boardGetXbarPh1Params;
    FUNCP_GET_XBAR_PHASE2_PARAMS        boardGetXbarPh2Params;
#endif
#ifdef IMPL_FA
    FUNCP_GET_FA_PHASE1_PARAMS          boardGetFaPh1Params;
    FUNCP_GET_FA_PHASE2_PARAMS          boardGetFaPh2Params;
    FUNCP_GET_FA_REG_CFG_LIST           boardGetFaRegCfgList;
#endif

    FUNCP_BEFORE_PHASE1_CONFIG          boardBeforePhase1Config;

    FUNCP_GET_PP_PHASE1_PARAMS          boardGetPpPh1Params;
    FUNCP_AFTER_PHASE1_CONFIG           boardAfterPhase1Config;
    FUNCP_GET_PP_PHASE2_PARAMS          boardGetPpPh2Params;
    FUNCP_AFTER_PHASE2_CONFIG           boardAfterPhase2Config;
    FUNCP_GET_PP_LOGICAL_INIT_PARAMS    boardGetPpLogInitParams;
    FUNCP_GET_LIB_INIT_PARAMS           boardGetLibInitParams;
    FUNCP_AFTER_INIT_CONFIG             boardAfterInitConfig;
    FUNCP_GET_PP_REG_CFG_LIST           boardGetPpRegCfgList;
    FUNCP_GET_STACK_CONFIG_PARAMS       boardGetStackConfigParams;
    FUNCP_GET_MNG_PORT_NAME             boardGetMngPortName;
    FUNCP_AFTER_STACK_READY_CONFIG      boardAfterStackReadyConfig;

    FUNCP_BOARD_CLEAN_DB_DURING_SYSTEM_RESET boardCleanDbDuringSystemReset;
    FUNCP_SIMPLE_INIT                   boardSimpleInit;
    /* APIs related to PP insert/remove */
    FUNCP_DEVICE_AFTER_INIT_CONFIG      deviceAfterInitConfig;
    FUNCP_DEVICE_AFTER_PHASE1           deviceAfterPhase1;
    FUNCP_DEVICE_AFTER_PHASE2           deviceAfterPhase2;
    FUNCP_DEVICE_CLEANUP                deviceCleanup;
}GT_BOARD_CONFIG_FUNCS;


/*
 * Typedef: FUNCP_REGISTER_BOARD_FUNCS
 *
 * Description:
 *      Register board initialization functions.
 *
 * Fields:
 *
 */
typedef GT_STATUS (*FUNCP_REGISTER_BOARD_FUNCS)
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardFuncs
);

/* number of different initializations for the specific board */
#define GT_BOARD_LIST_NUM_OF_BORAD_INITS_CNS   40
/* number of characters in name of board / name of revision */
#define GT_BOARD_LIST_STRING_LEN_CNS            100
/* string for revision that not used */
#define GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS      "not used"

/**
* @struct GT_BOARD_LIST_ELEMENT
 *
 * @brief Holds a board's information for the show function and registration.
*/
typedef struct{

    /** Registration function pointer. */
    FUNCP_REGISTER_BOARD_FUNCS registerFunc;

    GT_CHAR boardName[GT_BOARD_LIST_STRING_LEN_CNS];

    GT_U8 numOfRevs;

    GT_CHAR boardRevs[GT_BOARD_LIST_NUM_OF_BORAD_INITS_CNS][GT_BOARD_LIST_STRING_LEN_CNS];

} GT_BOARD_LIST_ELEMENT;

/**
* @enum APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT
 *
 * @brief PP access type enumeration
*/
typedef enum{

    /** PP access type is read. */
    APP_DEMO_TRACE_HW_ACCESS_TYPE_READ_E,

    /** PP access type is write */
    APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_E,

    /** PP access type is read and write */
    APP_DEMO_TRACE_HW_ACCESS_TYPE_BOTH_E,

    /** PP access type is write and delay */
    APP_DEMO_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E,

    /** PP access type is read,write and delay */
    APP_DEMO_TRACE_HW_ACCESS_TYPE_ALL_E

} APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT;

/**
* @enum APP_DEMO_TRACE_OUTPUT_MODE_ENT
 *
 * @brief PP access type enumeration
*/
typedef enum{

    /** use osPrintf. */
    APP_DEMO_TRACE_OUTPUT_MODE_DIRECT_E,

    /** use osPrintSynch need fo ISR debug */
    APP_DEMO_TRACE_OUTPUT_MODE_DIRECT_SYNC_E,

    /** store the data in db */
    APP_DEMO_TRACE_OUTPUT_MODE_DB_E,

    /** store the data in file */
    APP_DEMO_TRACE_OUTPUT_MODE_FILE_E,

    /** the last parameter */
    APP_DEMO_TRACE_OUTPUT_MODE_LAST_E

} APP_DEMO_TRACE_OUTPUT_MODE_ENT;


/**
* @enum APP_DEMO_CPSS_LOG_TASK_MODE_ENT
 *
 * @brief Determines if to log all CPSS API calls or just calls by a specified task(s).
*/
typedef enum{

    /** log all threads API calls */
    APP_DEMO_CPSS_LOG_TASK_ALL_E,

    /** log single task API calls */
    APP_DEMO_CPSS_LOG_TASK_SINGLE_E,

    /** @brief log task(s) whose id is read from
     *  some specified address
     */
    APP_DEMO_CPSS_LOG_TASK_REF_E

} APP_DEMO_CPSS_LOG_TASK_MODE_ENT;


/*******************************************************************************
* APP_DEMO_CALL_BACK_ON_REMOTE_PORTS_FUNC
*
* DESCRIPTION:
*       function called for remote physical ports to set extra config on the port,
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical or CPU port number
*       linkUp     - indication that the port is currently 'up'/'down'
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       the function called from the event handler task that recognized :
*       link_change / auto-negotiation ended
*
*******************************************************************************/
typedef GT_STATUS (*APP_DEMO_CALL_BACK_ON_REMOTE_PORTS_FUNC)
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp
);

/**
* @struct APP_DEMO_MICRO_INIT_STC
 *
 * @brief cpss micto-init parameters
*/
typedef struct{

    /** device Id */
    GT_U32 deviceId;

    /** interface channel to CPU */
    GT_U32 mngInterfaceType;

    /** packet processor core clock */
    GT_U32 coreClock;

    /** @brief cpss features participated in code generation
     *  Comments:
     */
    GT_U32 featuresBitmap;

} APP_DEMO_MICRO_INIT_STC;

extern APP_DEMO_CALL_BACK_ON_REMOTE_PORTS_FUNC appDemoCallBackOnRemotePortsFunc;

/**
* @internal appDemoShowBoardsList function
* @endinternal
*
* @brief   This function displays the boards list which have an automatic
*         initialization support.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note The printed table should look like:
*       +-------------------------------+-----------------------+
*       | Board name          | Revisions       |
*       +-------------------------------+-----------------------+
*       | 01 - RD-EX120-24G       |            |
*       |                | 01 - Rev 1.0     |
*       |                | 02 - Rev 2.0     |
*       +-------------------------------+-----------------------+
*       | 02 - DB-MX610-48F4GS     |            |
*       |                | 01 - Rev 1.0     |
*       |                | 02 - Rev 2.5     |
*       +-------------------------------+-----------------------+
*
*/
GT_STATUS appDemoShowBoardsList
(
    GT_VOID
);


/**
* @internal cpssInitSystem function
* @endinternal
*
* @brief   This is the main board initialization function for CPSS driver.
*
* @param[in] boardIdx                 - The index of the board to be initialized from the board
*                                      list.
* @param[in] boardRevId               - Board revision Id.
* @param[in] reloadEeprom             - Whether the Eeprom should be reloaded when
*                                      corePpHwStartInit() is called.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);

/**
* @internal cpssReInitSystem function
* @endinternal
*
* @brief   Run cpssInitSystem according to previously initialized parameters
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssReInitSystem
(
    GT_VOID
);

/**
* @internal appDemoTraceOn_GT_OK_Set function
* @endinternal
*
* @brief   This function configures 'print returning GT_OK' functions flag
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note - in unix-like systems special function needed to configure appDemoTraceOn_GT_OK
*       specific ifdef's needless, because this function can't harm anything
*       - must be external to ensure it's not ommited by optimization.
*
*/
GT_VOID appDemoTraceOn_GT_OK_Set
(
    GT_U32 enable
);


/**
* @internal appDemoOnDistributedSimAsicSideSet function
* @endinternal
*
* @brief   set value to the appDemoOnDistributedSimAsicSide flag.
*
* @param[in] value                    -  to be set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - if called after initialization of system -- after cpssInitSystem
*
* @note Function is called from the TERMINAL
*
*/
GT_STATUS appDemoOnDistributedSimAsicSideSet
(
    IN GT_U32  value
);

/**
* @internal appDemoTraceHwAccessEnable function
* @endinternal
*
* @brief   Trace HW read access information.
*
* @param[in] devNum                   - PP device number
* @param[in] accessType               - access type: read or write
* @param[in] enable                   - GT_TRUE:  tracing for given access type
*                                      GT_FALSE: disable tracing for given access type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad access type
*/
GT_STATUS appDemoTraceHwAccessEnable
(
    GT_U8                               devNum,
    APP_DEMO_TRACE_HW_ACCESS_TYPE_ENT   accessType,
    GT_BOOL                             enable
);

/**
* @internal appDemoTraceHwAccessOutputModeSet function
* @endinternal
*
* @brief   Set output tracing mode.
*
* @param[in] mode                     - output tracing mode: print, printSync or store.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state
* @retval GT_BAD_PARAM             - on bad mode
*
* @note It is not allowed to change mode, while one of the HW Access DB
*       is enabled.
*
*/
GT_STATUS appDemoTraceHwAccessOutputModeSet
(
    IN APP_DEMO_TRACE_OUTPUT_MODE_ENT   mode
);

/**
* @internal appDemoDbPpDump function
* @endinternal
*
* @brief   Dump App Demo DB
*
* @retval GT_OK                    - GT_OK ant case
*/
GT_STATUS appDemoDbPpDump(void);

/**
* @internal appDemoDebugDeviceIdSet function
* @endinternal
*
* @brief   Set given device ID value for specific device.
*         Should be called before cpssInitSystem().
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on devIndex > 127
*/
GT_STATUS appDemoDebugDeviceIdSet
(
    IN GT_U8    devNum,
    IN GT_U32   deviceId
);

/**
* @internal appDemoDebugDeviceIdReset function
* @endinternal
*
* @brief   Reverts to HW device ID value for all devices.
*         Should be called before cpssInitSystem().
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDebugDeviceIdReset
(
    IN GT_VOID
);

/**
* @internal appDemoDebugDeviceIdGet function
* @endinternal
*
* @brief   Get given device ID value for specific device.
*
* @param[out] deviceIdPtr              - pointer to device type
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - on not set debug device ID
* @retval GT_OUT_OF_RANGE          - on devIndex > 127
*/
GT_STATUS appDemoDebugDeviceIdGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *deviceIdPtr
);

/**
* @internal appDemoTrafficDisableSet function
* @endinternal
*
* @brief   Set traffic enable/disable mode - API that will enable/disable
*         execution of traffic enable algorithm during cpssInitSystem
* @param[in] disable                  - GT_TRUE - do not enable traffic during cpssInitSystem
*                                      - GT_FALSE - enable traffic during cpssInitSystem
*                                       None
*/
GT_VOID appDemoTrafficDisableSet
(
    IN GT_BOOL disable
);

/**
* @internal appDemoTraceHwAccessDbLine function
* @endinternal
*
* @brief   returns a line of the trace database, either the read or write database,
*         delay uses the write database
* @param[in] index                    -  of the line to retrieve
*
* @param[out] dbLine                   - A non-null pointer to a APP_DEMO_CPSS_HW_ACCESS_DB_STC struct
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS appDemoTraceHwAccessDbLine
(
    GT_U32 index,
    OUT APP_DEMO_CPSS_HW_ACCESS_DB_STC * dbLine
);

/**
* @internal appDemoTraceHwAccessClearDb function
* @endinternal
*
* @brief   Clear HW access db
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS appDemoTraceHwAccessClearDb
(
    GT_VOID
);

/**
* @internal appDemoTraceHwAccessDbIsCorrupted function
* @endinternal
*
* @brief   Check if the HW access data base has reached the limit
*
* @retval GT_OK                    - on success else if failed
*/

GT_STATUS appDemoTraceHwAccessDbIsCorrupted
(
    GT_BOOL * corrupted
);



/**
* @internal appDemoSupportAaMessageSet function
* @endinternal
*
* @brief   Configure the appDemo to support/not support the AA messages to CPU
*
* @param[in] supportAaMessage         -
*                                      GT_FALSE - not support processing of AA messages
*                                      GT_TRUE - support processing of AA messages
*                                       nothing
*/
GT_VOID appDemoSupportAaMessageSet
(
    IN GT_BOOL supportAaMessage
);
/**
* @internal cpssResetSystem function
* @endinternal
*
* @brief   API performs CPSS reset that includes device remove, mask interrupts,
*         queues and tasks destroy, all static variables/arrays clean.
* @param[in] doHwReset                - indication to do HW reset
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssResetSystem(
    IN GT_BOOL doHwReset
);

/**
* @internal cpssDevSupportSystemResetSet function
* @endinternal
*
* @brief   API controls if SYSTEM reset is supported, it is used to disable the
*         support on devices that do support. vice versa is not possible
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssDevSupportSystemResetSet(

    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);
/**
* @internal cpssDevMustNotResetPortSet function
* @endinternal
*
* @brief   API controls if SYSTEM reset (without HW reset) should skip reset of
*         specific port.
*         this for example needed for 'OOB' port (CPU out of band port) that serves
*         , for example : 'TELNET' . so need skip reset this port to avoid
*         disconnection of the session.
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssDevMustNotResetPortSet(

    IN  GT_U8       devNum,
    IN  GT_U32      portNum,
    IN  GT_BOOL     enable
);


/**
* @internal appDemoDevIdxGet function
* @endinternal
*
* @brief   Find device index in "appDemoPpConfigList" array based on device SW number.
*
* @param[in] devNum                   - SW device number
*
* @param[out] devIdxPtr                - (pointer to) device index in array "appDemoPpConfigList".
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
GT_STATUS appDemoDevIdxGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *devIdxPtr
);

/************************************************************************/
/* Tracing specific registers access */
/************************************************************************/

/**
* @internal appDemoHwPpDrvRegTracingRegAddressesRangeSet function
* @endinternal
*
* @brief Set register address ranges to be traced.
*
* @param[in] regAddrMask0    - The first register's address mask.
* @param[in] regAddrPattern0 - The first register's address pattern.
* @param[in] ...             - other pairs of register's addresses masks and patterns
*                              terminated by 0 value
*
*/
void appDemoHwPpDrvRegTracingRegAddressesRangeSet
(   IN GT_U32 regAddrMask0,
    IN GT_U32 regAddrPattern0,
    ...
);

/**
* @enum APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_TYPE_ENT
 *
 * @brief Types of register address clasification rule
*/
typedef enum
{
    /** Range parameteres: Base, size */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,
    /** Range parameteres: base, step, amount, size  */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E,
    /** Range parameteres: addr */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E,
    /** Range parameteres: base, step, amount  */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,
    /** marks end of array */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_LAST_E,

} APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_TYPE_ENT;

/**
* @struct APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC
*
* @brief Structure for register address classification rule
*/
typedef struct
{
    /** state Id value for wich the rule is applicable */
    GT_U32                                          applicableId;
    /** enum rule type */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_TYPE_ENT ruleType;
    /** new state Id value if the rule matches the address */
    GT_U32                                          resultApplicableId;
    /** the part of register name-path appended by this rule */
    const char*                                     resultName;
    /** rule type depended parameters */
    GT_U32                                          ruleParam[4];
} APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC;

/**
* @internal appDemoHwPpDrvRegTracingRegNameGet function
* @endinternal
*
* @brief   Get name (with path) of register by address.
*
* @param[in] rulesArr    - array of rules to reconize register by address.
* @param[in] regAddr     - address of register.
*
* @param[out] resultStr  - pointer to buffer for name of register.
*
*/
void appDemoHwPpDrvRegTracingRegNameGet
(
    IN APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC rulesArr[],
    IN GT_U32                                     regAddr,
    OUT char*                                     resultStr
);

/**
* @internal appDemoHwPpDrvRegTracingRegNameBind function
* @endinternal
*
* @brief   Bind register names array.
*
*/
void appDemoHwPpDrvRegTracingRegNameBind
(
    IN APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC *regNameArrPtr
);

/**
* @internal appDemoHwPpDrvRegTracingBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                        GT_FALSE - un bind callback routines.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoHwPpDrvRegTracingBind
(
    GT_BOOL                     bind
);

/**
* @internal cpssSystemBaseInit function
* @endinternal
*
* @brief   General initializations that are not relevant to specific devices. This part will
*          be called during CPSS init regardless to the existence of any PP in the system.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssSystemBaseInit
(
    GT_VOID
);

/**
* @internal cpssPpInsert function
* @endinternal
*
* @brief   Insert packet processor.
* @param[in] pciBusNum      - PCI bus number.
* @param[in] pciDevNum      - PCI Device number.
* @param[in] devNum         - cpss Logical Device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssPpInsert
(
    IN  GT_U8 pciBusNum,
    IN  GT_U8 pciDevNum,
    IN  GT_U8 devNum
);

/**
* @internal cpssPpRemove function
* @endinternal
*
* @brief   Remove packet processor.
* @param[in] devNum         - cpss Logical Device number.
* @param[in] managedRemoval - by default set to TRUE. 
                             Will be false in case of unmanaged removal of Device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssPpRemove
(
    IN  GT_U8 devNum,
    IN  GT_BOOL managedRemoval
);

/**
* @internal cpssPpShowDevices function
* @endinternal
*
* @brief   List all PP devices' PCI info.
*
*/
void cpssPpShowDevices (void);

/**
* @internal cpssLspci function
* @endinternal
*
* @brief   List all detected Marvell PCI devices.
*
*/
void cpssLspci (void);

/**
* @internal cpssPciRemove function
* @endinternal
*
* @brief   Remove specific PCI device from Sysfs.
*
*/
void cpssPciRemove (
    IN  GT_U8 pciBus,
    IN  GT_U8 pciDev
);

/**
* @internal cpssPciRescan function
* @endinternal
*
* @brief   Rescan all PCI devices.
*
*/
void cpssPciRescan (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoBoardConfigh */




