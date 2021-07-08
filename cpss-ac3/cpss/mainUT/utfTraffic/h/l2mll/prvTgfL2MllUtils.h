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
* @file prvTgfL2MllUtils.h
*
* @brief Utilities for L2 MLL tests definitions
*
* @version   3
********************************************************************************
*/
#ifndef __prvTgfL2MllUtilsh
#define __prvTgfL2MllUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfL2MllGen.h>

#define NOT_VALID_ENTRY_CNS     0xFFFFFFFF
#define AUTO_CALC_INDEX_CNS     0xFFFFFFF0
/* 12 LSB - mllIndex,
   next 16 bits - lttIndex
   next 4 bits - 0xE */
#define TEST_INDEX_BASE_MAC(lttIndex,mllIndex)     (((lttIndex & 0xFFFF) << 16) |((mllIndex) & 0xFFFF))
#define TEST_MLL_INDEX_GET(value)   (value & 0xFFFF)/* strip the INDEX_BASE_CNS */
#define TEST_LTT_INDEX_GET(value)   (value >> 16)/* strip the INDEX_BASE_CNS */

typedef struct{
    GT_U32          mllIndexAndLttIndex;   /* value that compose the index to the LTT table and the MLL index (see TEST_INDEX_BASE_MAC) */
    GT_U32          nextMllPointer; /* 'mll pointer' in PRV_TGF_L2_MLL_ENTRY_STC */
    GT_BOOL         last;           /* 'last' in PRV_TGF_L2_MLL_ENTRY_STC */
    CPSS_INTERFACE_TYPE_ENT egressInterfaceType;   /*port/trunk/vidx/vid*/
    GT_U32          egressInterfaceValue; /* used in PRV_TGF_L2_MLL_ENTRY_STC::egressInterface
                                            for type == 'port' :
                                                this is index in prvTgfPortsArray[] ,
                                                 use for 'port interface'
                                            for type == trunk : this is trinkId
                                            for type == vidx : this is vidx
                                            for type == vid : 'ignored'
                                             */
}L2_MLL_ENTRY_SHORT_INFO_STC;

/*  START of : example for the DB that should be built: */
#if 0

/*
    we see below 2 link lists that starts at the LTT entries :
    1. LTT index 0    point to MLL index 0   .....
            MLL index 0   point to MLL index 1   (last)
    2. LTT index 1      point to MLL index 111 .....
            MLL index 111 point to MLL index 555 (last)
    3. LTT index 0x1000 point to MLL index 333 .....
            MLL index 333 point to MLL index 556 (last)

    meaning that each list hold 2 MLLs{0,1}{111,555},{333,556}.
*/

static  L2_MLL_ENTRY_SHORT_INFO_STC     testMllDb[] =
{
    /* first list (2 in the same line)*/
    /* pointer from LTT index 0 (start mll 0) */
    {TEST_INDEX_BASE_MAC(0,0) ,      /*ignored*/  100,    GT_FALSE,   CPSS_INTERFACE_PORT_E,2},
    {TEST_INDEX_BASE_MAC(0,0),                      0,    GT_TRUE,    CPSS_INTERFACE_PORT_E,2},

    /* second list (the second entry in 'odd' index) */
    /* pointer from LTT index 1 (start mll 111)*/
    {TEST_INDEX_BASE_MAC(1,111) , TEST_INDEX_BASE_MAC(0,555)  ,    GT_FALSE,   CPSS_INTERFACE_PORT_E,2},
    {TEST_INDEX_BASE_MAC(1,111),                             0,    GT_TRUE,   CPSS_INTERFACE_PORT_E,2},

    /* third list (the second entry in 'even' index) */
    /* pointer from LTT index 0x1000 (start mll 333)*/
    {TEST_INDEX_BASE_MAC(0x1000,333) , TEST_INDEX_BASE_MAC(0,556)  ,    GT_FALSE,   CPSS_INTERFACE_PORT_E,3},
    {TEST_INDEX_BASE_MAC(0x1000,333),                             0,    GT_TRUE,   CPSS_INTERFACE_PORT_E,3},

    {NOT_VALID_ENTRY_CNS,                           0,          0,   0 , 0}/*must be last*/
};
#define  TEST_MLL_DB_NUM_ENTRIES_CNS    ((sizeof(testMllDb))/(sizeof(testMllDb[0])))

static  GT_U32     testMllDb_restoreInfo[TEST_MLL_DB_NUM_ENTRIES_CNS] =
{
    0
    /* filled in runtime according to actual MLL indexes used */
};

#endif /*0*/
/*  End of : example  */

typedef enum{
    MLL_INDEX_TEST_MODE_STANDARD_E,
    MLL_INDEX_TEST_MODE_REFERENCE_FROM_END_OF_TALBE_E,/* index in nextMllPointer are used as index from end of table
                                                        and not from start of table */

    MLL_INDEX_TEST_MODE_LAST_E      /* must be last value */
}MLL_INDEX_TEST_MODE_ENT;

/**
* @internal prvTgfL2MllLConfigReset function
* @endinternal
*
* @brief   Restore configuration for the MLL and the LTT table according to the info.
*
* @param[in] testMllDbArr[]           - array of 'orig' DB MLL and LTT info
* @param[in] testMllDb_restoreInfoArr[] - array of actual indexes of MLL and LTT entries
*                                       None
*/
GT_VOID prvTgfL2MllLConfigReset
(
    IN L2_MLL_ENTRY_SHORT_INFO_STC testMllDbArr[],
    IN GT_U32                      testMllDb_restoreInfoArr[]
);

/**
* @internal prvTgfL2MllLConfigSet function
* @endinternal
*
* @brief   Set configuration for the MLL and the LTT table according to the info.
*
* @param[in] testMllDbArr[]           - array of 'orig' DB MLL and LTT info
* @param[in] setMode                  - specific test mode how to use the MLL indexes
*
* @param[out] testMllDb_restoreInfoArr[] - array of actual indexes of MLL and LTT entries
*                                       None
*/
GT_VOID prvTgfL2MllLConfigSet
(
    IN L2_MLL_ENTRY_SHORT_INFO_STC testMllDbArr[],
    OUT GT_U32                      testMllDb_restoreInfoArr[],
    IN MLL_INDEX_TEST_MODE_ENT      setMode
);

/**
* @internal prvTgfL2MllUtilsGlobalConfigurationGetAndSave function
* @endinternal
*
* @brief   Get related L2 MLL global configuration and save it for later restore.
*/
GT_VOID prvTgfL2MllUtilsGlobalConfigurationGetAndSave
(
    GT_VOID
);

/**
* @internal prvTgfL2MllUtilsGlobalConfigurationRestore function
* @endinternal
*
* @brief   Restore related L2 MLL global configuration previously saved.
*/
GT_VOID prvTgfL2MllUtilsGlobalConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfL2MllUtilsMllAndLttEntriesClear function
* @endinternal
*
* @brief   Clear L2 MLL & LTT entries.
*
* @param[in] lttIndexArr              - Array of LTT indexes to clear.
* @param[in] numOfLttIndexes          - Number of valid entries in <lttIndexArr>.
* @param[in] mllIndexArr              - Array of MLL indexes to clear.
* @param[in] numOfMllIndexes          - Number of valid entries in <mllIndexArr>.
*                                       None
*/
GT_VOID prvTgfL2MllUtilsMllAndLttEntriesClear
(
    IN GT_U32 *lttIndexArr,
    IN GT_U32  numOfLttIndexes,
    IN GT_U32 *mllIndexArr,
    IN GT_U32  numOfMllIndexes
);

/**
* @internal prvTgfL2MllUtilsResetAllEthernetCounters function
* @endinternal
*
* @brief   Clear all test ports Ethernet counters
*/
GT_VOID prvTgfL2MllUtilsResetAllEthernetCounters
(
    GT_VOID
);

/**
* @internal prvTgfL2MllUtilsPacketSend function
* @endinternal
*
* @brief   Function sends packet.
*/
GT_VOID prvTgfL2MllUtilsPacketSend
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr,
    IN GT_U32          prvTgfBurstCount,
    IN GT_U32          prvTgfSendPortIndex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfL2MllUtilsh */


