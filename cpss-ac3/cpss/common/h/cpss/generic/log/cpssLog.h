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
* @file cpssLog.h
*
* @brief Includes definitions for CPSS log functions.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssLogh
#define __cpssLogh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>



/**
* @enum CPSS_LOG_POINTER_FORMAT_ENT
 *
 * @brief This enum determines how pointer values will look in the log.
*/
typedef enum{

    /** usual hex value: 0xfff */
    CPSS_LOG_POINTER_FORMAT_SIMPLE_E,

    /** hex with prefix: (addr)0xfff */
    CPSS_LOG_POINTER_FORMAT_PREFIX_E

} CPSS_LOG_POINTER_FORMAT_ENT;

/**
* @enum CPSS_LOG_API_FORMAT_ENT
 *
 * @brief This enum defines the different formats of the log for documenting the
 * parameters values of a function.
*/
typedef enum{

    /** not documenting any paramter */
    CPSS_LOG_API_FORMAT_NO_PARAMS_E,

    /** @brief documenting all the parameters
     *  values
     */
    CPSS_LOG_API_FORMAT_ALL_PARAMS_E,

    /** @brief documenting only the non zero
     *  parameters values
     */
    CPSS_LOG_API_FORMAT_NON_ZERO_PARAMS_E

} CPSS_LOG_API_FORMAT_ENT;

/**
* @enum CPSS_LOG_LIB_ENT
 *
 * @brief This enum defines the different libraries in cpss.
 * used for enable/disable the log activity for each lib.
*/
typedef enum{
/* 0*/ CPSS_LOG_LIB_APP_DRIVER_CALL_E   /** driver call                       */
/* 1*/,CPSS_LOG_LIB_BRIDGE_E            /** bridge                            */
/* 2*/,CPSS_LOG_LIB_CNC_E               /** cnc                               */
/* 3*/,CPSS_LOG_LIB_CONFIG_E            /** confing                           */
/* 4*/,CPSS_LOG_LIB_COS_E               /** cos                               */
/* 5*/,CPSS_LOG_LIB_HW_INIT_E           /** hw init                           */
/* 6*/,CPSS_LOG_LIB_CSCD_E              /** cscd                              */
/* 7*/,CPSS_LOG_LIB_CUT_THROUGH_E       /** cut through                       */
/* 8*/,CPSS_LOG_LIB_DIAG_E              /** diag                              */
/* 9*/,CPSS_LOG_LIB_FABRIC_E            /** fabric                            */
/*10*/,CPSS_LOG_LIB_IP_E                /** ip                                */
/*11*/,CPSS_LOG_LIB_IPFIX_E             /** ipfix                             */
/*12*/,CPSS_LOG_LIB_IP_LPM_E            /** ip lpm                            */
/*13*/,CPSS_LOG_LIB_L2_MLL_E            /** l2 mll                            */
/*14*/,CPSS_LOG_LIB_LOGICAL_TARGET_E    /** logical target                    */
/*15*/,CPSS_LOG_LIB_LPM_E               /** lpm                               */
/*16*/,CPSS_LOG_LIB_MIRROR_E            /** mirror                            */
/*17*/,CPSS_LOG_LIB_MULTI_PORT_GROUP_E  /** multy port group                  */
/*18*/,CPSS_LOG_LIB_NETWORK_IF_E        /** network if                        */
/*19*/,CPSS_LOG_LIB_NST_E               /** nst                               */
/*20*/,CPSS_LOG_LIB_OAM_E               /** oam                               */
/*21*/,CPSS_LOG_LIB_PCL_E               /** pcl                               */
/*22*/,CPSS_LOG_LIB_PHY_E               /** phy                               */
/*23*/,CPSS_LOG_LIB_POLICER_E           /** policer                           */
/*24*/,CPSS_LOG_LIB_PORT_E              /** port                              */
/*25*/,CPSS_LOG_LIB_PROTECTION_E        /** protection                        */
/*26*/,CPSS_LOG_LIB_PTP_E               /** ptp                               */
/*27*/,CPSS_LOG_LIB_SYSTEM_RECOVERY_E   /** system recovery                   */
/*28*/,CPSS_LOG_LIB_TCAM_E              /** tcam                              */
/*29*/,CPSS_LOG_LIB_TM_GLUE_E           /** tm glue                           */
/*30*/,CPSS_LOG_LIB_TRUNK_E             /** trunk                             */
/*31*/,CPSS_LOG_LIB_TTI_E               /** tti                               */
/*32*/,CPSS_LOG_LIB_TUNNEL_E            /** tunnel                            */
/*33*/,CPSS_LOG_LIB_VNT_E               /** vnt                               */
/*34*/,CPSS_LOG_LIB_RESOURCE_MANAGER_E  /** resource manager                  */
/*35*/,CPSS_LOG_LIB_VERSION_E           /** version                           */
/*36*/,CPSS_LOG_LIB_TM_E                /** traffic manager                   */
/*37*/,CPSS_LOG_LIB_SMI_E               /** SMI                               */
/*38*/,CPSS_LOG_LIB_INIT_E              /** init                              */
/*39*/,CPSS_LOG_LIB_DRAGONITE_E         /** dragonite                         */
/*40*/,CPSS_LOG_LIB_VIRTUAL_TCAM_E      /** virtual TCAM                      */
/*41*/,CPSS_LOG_LIB_INGRESS_E           /** ingress processing in PIPE devices*/
/*42*/,CPSS_LOG_LIB_EGRESS_E            /** egress  processing in PIPE devices*/
/*43*/,CPSS_LOG_LIB_LATENCY_MONITORING_E/** latency monitoring                */
/*44*/,CPSS_LOG_LIB_TAM_E               /** TAM (Telemetry Analytics and Monitoring)*/
/*45*/,CPSS_LOG_LIB_EXACT_MATCH_E       /** exact match                       */
/*46*/,CPSS_LOG_LIB_PHA_E               /** PHA(programmable HA- DXCH devices)*/
/*47*/,CPSS_LOG_LIB_ALL_E               /** all the libs                      */

} CPSS_LOG_LIB_ENT;

/**
* @enum CPSS_LOG_TYPE_ENT
 *
 * @brief This enum defines the different types of the log.
*/
typedef enum{

    /** information log */
    CPSS_LOG_TYPE_INFO_E,

    /** log of entry level of APIs only */
    CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E,

    /** log of internal functions and not entry level APIs */
    CPSS_LOG_TYPE_NON_ENTRY_LEVEL_FUNCTION_E,

    /** driver */
    CPSS_LOG_TYPE_DRIVER_E,

    /** error log */
    CPSS_LOG_TYPE_ERROR_E,

    /** all types */
    CPSS_LOG_TYPE_ALL_E

} CPSS_LOG_TYPE_ENT;

/**
* @enum CPSS_LOG_TIME_FORMAT_ENT
 *
 * @brief This enum describes log time format.
*/
typedef enum{

    /** Date and time representation (Thu Aug 23 14:55:02 2001) */
    CPSS_LOG_TIME_FORMAT_DATE_TIME_E,

    /** Short MM/DD/YY and 12hour clock time (08/23/01 02:55:02 pm) */
    CPSS_LOG_TIME_FORMAT_SHORT_DATE_TIME_E,

    /** Short MM/DD/YY and 24hour clock time (08/23/01 14:55:02) */
    CPSS_LOG_TIME_FORMAT_SHORT_DATE_ISO_TIME_E,

    /** Timestamp is not logged */
    CPSS_LOG_TIME_FORMAT_NO_DATE_TIME_E

} CPSS_LOG_TIME_FORMAT_ENT;

/**
* @internal cpssLogEnableSet function
* @endinternal
*
* @brief   The function enables/disables Log feature.
*
* @param[in] enable                   - GT_TRUE for log  and GT_False for log disable
*                                       None.
*/
GT_VOID cpssLogEnableSet
(
    IN GT_BOOL enable
);

/**
* @internal cpssLogEnableGet function
* @endinternal
*
* @brief   The function gets the status of the Log feature (enabled/disabled).
*
* @retval GT_TRUE                  - if the log is enabled
* @retval GT_FALSE                 - if the log is disabled
*/
GT_BOOL cpssLogEnableGet(GT_VOID);

/**
* @internal cpssLogLibEnableSet function
* @endinternal
*
* @brief   The function enables/disables specific type of log information for
*         specific or all libs.
* @param[in] lib                      -  to enable/disable log for.
* @param[in] type                     - log type.
* @param[in] enable                   - GT_TRUE for log  and GT_FALSE for log disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter lib or type
*/
GT_STATUS cpssLogLibEnableSet
(
    IN  CPSS_LOG_LIB_ENT    lib,
    IN  CPSS_LOG_TYPE_ENT   type,
    IN  GT_BOOL             enable
);

/**
* @internal cpssLogLibEnableGet function
* @endinternal
*
* @brief   The function gets the status of specific type of log information for
*         specific lib (enabled/disabled).
* @param[in] lib                      -  to get the log status for.
* @param[in] type                     - log type.
*
* @param[out] enablePtr                - GT_TRUE - log is enabled for the specific lib and log type.
*                                      GT_FALSE - log is enabled for the specific lib and log type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - bad pointer enablePtr
* @retval GT_BAD_PARAM             - bad parameter lib or type
*/
GT_STATUS cpssLogLibEnableGet
(
    IN   CPSS_LOG_LIB_ENT   lib,
    IN   CPSS_LOG_TYPE_ENT  type,
    OUT  GT_BOOL            * enablePtr
);

/**
* @internal cpssLogApiFormatSet function
* @endinternal
*
* @brief   The function set format for API related log.
*
* @param[in] format                   -  of API related logs.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter format
*/
GT_STATUS cpssLogApiFormatSet
(
    IN CPSS_LOG_API_FORMAT_ENT format
);

/**
* @internal cpssLogApiFormatGet function
* @endinternal
*
* @brief   The function gets the format of API related log.
*/
CPSS_LOG_API_FORMAT_ENT cpssLogApiFormatGet(GT_VOID);

/**
* @internal cpssLogStackSizeSet function
* @endinternal
*
* @brief   Defines the size of the stack, how many function will be logged before
*         an error occurred.
* @param[in] size                     - the amount of functions.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssLogStackSizeSet
(
    IN GT_U32 size
);

/**
* @internal cpssLogStackSizeGet function
* @endinternal
*
* @brief   The function gets the size of the log stack
*/
GT_U32 cpssLogStackSizeGet(GT_VOID);

/**
* @internal cpssLogTimeFormatSet function
* @endinternal
*
* @brief   The function sets log time format.
*
* @param[in] format                   - log time format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter format
*/
GT_STATUS cpssLogTimeFormatSet
(
    IN CPSS_LOG_TIME_FORMAT_ENT format
);

/**
* @internal cpssLogHistoryDump function
* @endinternal
*
* @brief   Dump log history database.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if history log disabled
*/
GT_STATUS cpssLogHistoryDump
(
    GT_VOID
);

/**
* @internal cpssLogHistoryFileNameSet function
* @endinternal
*
* @brief   Set log history file name.
*
* @param[in] fileNamePtr              - pointer to history file name string
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - string exceeds maximmal size
*/
GT_STATUS cpssLogHistoryFileNameSet
(
    IN const char * fileNamePtr
);


/**
* @internal cpssLogLineNumberEnableSet function
* @endinternal
*
* @brief   The function enables/disables using of a source code line number by
*         some logging functions
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*                                       None.
*/
GT_VOID cpssLogLineNumberEnableSet
(
     IN GT_BOOL enable
);

/**
* @internal cpssLogLineNumberEnableGet function
* @endinternal
*
* @brief   The function gets the status of the using of a sorce code line number
*         by some logging functions
*
* @retval GT_TRUE                  - if enabled
* @retval GT_FALSE                 - if disabled
*/
GT_BOOL cpssLogLineNumberEnableGet
(
    GT_VOID
);

/**
* @internal cpssLogPointerFormatSet function
* @endinternal
*
* @brief   Set a format of pointer values
*
* @param[in] value                    - a format type
*                                       None.
*/
GT_VOID cpssLogPointerFormatSet
(
     IN CPSS_LOG_POINTER_FORMAT_ENT value
);

/**
* @internal cpssLogPointerFormatGet function
* @endinternal
*
* @brief   get a format used for a pointer-values logging.
*
* @retval a format used for a pointer -values logging.
*/
CPSS_LOG_POINTER_FORMAT_ENT cpssLogPointerFormatGet
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssLogh */


