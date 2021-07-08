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
* @file tgfCommon.h
*
* @brief Common helper API for enhanced UTs
*
* @version   69
********************************************************************************
*/
#ifndef __tgfCommonh
#define __tgfCommonh

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cos/cpssCosTypes.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <extUtils/tgf/tgfCommon.h>

#ifdef CHX_FAMILY
#include <cpss/generic/cpssCommonDefs.h>
#endif /*CHX_FAMILY*/



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/******************************************************************************\
 *                     Default device\port numbers                            *
\******************************************************************************/
/* device number of first device */
#define PRV_TGF_DEV_0_CNS    0

/* max ports in test */
/* NOTE: trunk LBH tests for Puma need 12 members in trunk , so value was set to 20 */
#define PRV_TGF_MAX_PORTS_NUM_CNS               20
/* default number for ports in test */
/* most tests written for 4 ports , but specific test
    can set it to higher value */
#define PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS   4

/* Macros to determine element count in static array */
#define PRV_TGF_ELEMENTS_NUM_MAC(_x) (sizeof(_x)/sizeof((_x)[0]))

/* devices array */
extern GT_U8    prvTgfDevsArray[];/* = {0, 0,  0,  0}; */

/* number of ports/devices in arrays of :prvTgfPortsArray,prvTgfDevsArray */
extern GT_U8    prvTgfPortsNum;/* default value is PRV_TGF_DEFAULT_NUM_PORTS_IN_TEST_CNS
    max value allowed is PRV_TGF_MAX_PORTS_NUM_CNS */

/* check port index */
#define PRV_TGF_PORT_INDEX_CHECK_MAC(_portIndex) \
        if ((_portIndex) >= prvTgfPortsNum) \
            return GT_BAD_PARAM;

/* check port index */
#define PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(_portIndex)          \
        if ((_portIndex) >= prvTgfPortsNum)                             \
        {                                                               \
            PRV_UTF_LOG0_MAC("portIndex out of port array range\n");    \
            return;                                                     \
        }

/* macro that declare that test set vid interface . but it is valid for "multi port-group FDB lookup system" */
/* internal macro -- to be used from other macros */
#define ___MULTI_PORT_GROUP_FDB_LOOKUP_SYSTEM_LIMITATION_REDIRECT_TO_VID_INTERFACE_CHECK_MAC(testName,engineName) \
    if(prvTgfDevPortForceNum)   \
    {                           \
        PRV_UTF_LOG1_MAC("=====%s : ================================== \n",#testName);\
        PRV_UTF_LOG1_MAC("=====the test try to set [%s] entry on VID interface : \n",#engineName);                               \
        PRV_UTF_LOG0_MAC("=====Limitation for multi port group FDB lookup system : set redircect to VID interface is not allowed \n");\
        PRV_UTF_LOG0_MAC("===== otherwise traffic will endlessly loop via the ring ports  \n");\
        PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_ALL_FAMILY_E);                   \
    }

/* macro that declare that test set vid interface from the FDB . but it is valid for "multi port-group FDB lookup system" */
#define PRV_TGF_MULTI_PORT_GROUP_FDB_LOOKUP_SYSTEM_LIMITATION_REDIRECT_TO_VID_INTERFACE_FROM_FDB_CHECK_MAC(testName) \
    ___MULTI_PORT_GROUP_FDB_LOOKUP_SYSTEM_LIMITATION_REDIRECT_TO_VID_INTERFACE_CHECK_MAC(testName,FDB)

/* enum for indexes to access ports through arrays prvTgfDevsArray & prvTgfPortsArray */
enum{
/* uplink ports */
    U1_INDEX_E = 0,
    U2_INDEX_E,
    U3_INDEX_E,
    U4_INDEX_E,
    U5_INDEX_E,
    U6_INDEX_E,
    U7_INDEX_E,
    U8_INDEX_E,

/* network ports */
    N1_INDEX_E,
    N2_INDEX_E,
    N3_INDEX_E,
    N4_INDEX_E,
    N5_INDEX_E,
    N6_INDEX_E,
    N7_INDEX_E,
    N8_INDEX_E,

    /* last index - used as last network port valid index */
    LAST_INDEX_E

};


/* save prvTgfPortsArray into storePtr and  - if this is AC3X - modify
 * prvTgfPortsArray to ensure first "num" items are not remote ports
 * (they can't be configured as cascade)*/
#define PRV_TGF_XCAT3X_DEF_PORTS_SAVE_AND_FILTER_CASCADE_MAC(storePtr,num)                   \
    do {                                                                                     \
        cpssOsMemCpy((storePtr), prvTgfPortsArray, (num) * sizeof(prvTgfPortsArray[0]));     \
        if (prvTgfXcat3xExists() == GT_TRUE)                                                 \
        {                                                                                    \
            GT_U32 actualNum = prvTgfFilterDefPortsArray(GT_FALSE, prvTgfPortsArray, (num)); \
            UTF_VERIFY_EQUAL0_STRING_MAC((num), actualNum,                                   \
                                         "a number of cascade ports is less than expected"); \
        }                                                                                    \
    } while (0)

/* Set hwDevNum in DSA Tag */
#define PRV_TGF_SET_HW_DEV_NUM_DSA_TAG_MAC(pattern, hwDev) \
    pattern &= 0xe0;                                       \
    pattern |= hwDev;                                      \

/* enum for port device type */
typedef enum
{
    PRV_TGF_28_PORT_DEVICE_TYPE_E = 0,
    PRV_TGF_64_PORT_DEVICE_TYPE_E,      /*Lion, Lion2, Bobcat2, Caelum, Bobcat3 */
    PRV_TGF_128_PORT_DEVICE_TYPE_E,     /*Lion2, Bobcat2, Caelum, Bobcat3 */
    PRV_TGF_PORTS_FROM_56_DEVICE_TYPE_E, /* Cetus */

    PRV_TGF_DEVICE_TYPE_COUNT_E
} PRV_TGF_DEVICE_TYPE_ENT;

/* enum for device port mode type */
typedef enum
{
    PRV_TGF_DEVICE_PORT_MODE_DEFAULT_E,  /* Default mode, UT defines ports by itself */
    PRV_TGF_DEVICE_PORT_MODE_XLG_E      /* Lion2 40G mode - local ports 0, 4, 8, 9 */

} PRV_TGF_DEVICE_PORT_MODE_ENT;

extern PRV_TGF_DEVICE_PORT_MODE_ENT prvTgfDevicePortMode;



/* use this constant when need exact size in compilation time */
#define PRV_TGF_PORTS_NUM_CNS   4

/* macro to verify return code */
#define PRV_UTF_VERIFY_GT_OK(rc, msg)                                        \
    if (GT_OK != rc)                                                         \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", msg, rc);        \
        return rc;                                                           \
    }

#define PRV_UTF_VERIFY_RC1(rc, name)                                           \
    if (GT_OK != (rc))                                                         \
    {                                                                          \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);         \
        rc1 = rc;                                                              \
    }

/* this macro used both for configuration and for restore sequences           */
/* for configuration _stop parameter is GT_TRUE and the first fail causes     */
/* exit from the function                                                     */
/* for restore _stop parameter is GT_FALSE and any fail causes rc saving only */
#define PRV_UTF_VERIFY_COND_STOP_MAC(_stop, _saveRc, _rc, _name)              \
    if (GT_OK != _rc)                                                         \
    {                                                                         \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", _name, _rc);      \
        _saveRc = _rc;                                                        \
        if (_stop != GT_FALSE)                                                \
        {                                                                     \
            return _rc;                                                       \
        }                                                                     \
    }

/* as previous, but also increases error counter */
#define PRV_UTF_VERIFY_LOG_COND_STOP_MAC(_stop, _saveRc, _rc, _name)          \
    if (GT_OK != _rc)                                                         \
    {                                                                         \
        utfEqualVerify(GT_OK, _rc, __LINE__, __FILE__);                       \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", _name, _rc);      \
        _saveRc = _rc;                                                        \
        if (_stop != GT_FALSE)                                                \
        {                                                                     \
            return _rc;                                                       \
        }                                                                     \
    }

#define PRV_UTF_VERIFY_LOG_NO_STOP_MAC(_saveRc, _rc, _name)                   \
    if (GT_OK != _rc)                                                         \
    {                                                                         \
        utfEqualVerify(GT_OK, _rc, __LINE__, __FILE__);                       \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", _name, _rc);      \
        _saveRc = _rc;                                                        \
    }

/* macro to verify return code with message log */
#define PRV_UTF_VERIFY_GT_OK_LOG_MAC(_rc, _msg)                              \
    if (GT_OK != rc)                                                         \
    {                                                                        \
        utfEqualVerify(GT_OK, _rc, __LINE__, __FILE__);                      \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", _msg, _rc);      \
        return _rc;                                                          \
    }

/* default boolean parameter value */
#define PRV_TGF_DEFAULT_BOOLEAN_CNS     GT_FALSE
#define PRV_TGF_DEFAULT_U8_CNS          0xFF
#define PRV_TGF_DEFAULT_U16_CNS         0xFFFF
#define PRV_TGF_DEFAULT_U32_CNS         0xFFFFFFFF

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @struct PRV_TGF_QOS_PARAM_MODIFY_STC
 *
 * @brief QoS modify parameters
*/
typedef struct{

    /** enable/disable modifing traffic class */
    GT_BOOL modifyTc;

    /** enable/disable modifing IEEE 802.1p User Priority */
    GT_BOOL modifyUp;

    /** enable/disable modifing Drop Precedence */
    GT_BOOL modifyDp;

    /** enable/disable modifing IP DSCP field */
    GT_BOOL modifyDscp;

    /** enable/disable modifing MPLS label EXP value */
    GT_BOOL modifyExp;

} PRV_TGF_QOS_PARAM_MODIFY_STC;

/**
* @struct PRV_TGF_QOS_PARAM_STC
 *
 * @brief The Quality of Service parameters
*/
typedef struct{

    /** egress TX traffic class queue */
    GT_U32 tc;

    /** Drop Precedence (color) (Green, Yellow or Red) */
    CPSS_DP_LEVEL_ENT dp;

    /** IEEEE 802.1p User Priority */
    GT_U32 up;

    /** IP DCSP field */
    GT_U32 dscp;

    /** MPLS label EXP value */
    GT_U32 exp;

} PRV_TGF_QOS_PARAM_STC;

/**
* @enum PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_ENT
 *
 * @brief enumerator that hold values for the type of how many bits
 * are used in a trunk member selection function.
 * Used for trunk member selection and by L2 ECMP member selection.
*/
typedef enum{

    /** @brief Use the entire 12 bit hash in the trunk member selection function.
     *  ((Hash[11:0] #members)/4096)
     */
    PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_12_BITS_E = 0,

    /** @brief Use only the 6 least significant bits in the trunk member selection.
     *  ((Hash[5:0] #members)/64)
     */
    PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_6_LSB_E,

    /** @brief Use only the 6 most significant bits in the trunk member selection.
     *  ((Hash[11:6] #members)/64)
     */
    PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_6_MSB_E

} PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_ENT;

/******************************************************************************\
 *                              Marco definitions                             *
\******************************************************************************/

/* case to convert from\into generic values */
#define PRV_TGF_SWITCH_CASE_MAC(varName, val1, val2)                           \
    case val1:                                                                 \
    {                                                                          \
        varName = val2;                                                        \
        break;                                                                 \
    }

/* macro to check if equal expected and recieved port's counters */
#define PRV_TGF_VERIFY_COUNTERS_MAC(isEqual, expCntrs, rcvCntrs)               \
    isEqual = rcvCntrs.goodOctetsSent.l[0] == expCntrs.goodOctetsSent.l[0] &&  \
              rcvCntrs.goodPktsSent.l[0]   == expCntrs.goodPktsSent.l[0] &&    \
              rcvCntrs.ucPktsSent.l[0]     == expCntrs.ucPktsSent.l[0] &&      \
              rcvCntrs.brdcPktsSent.l[0]   == expCntrs.brdcPktsSent.l[0] &&    \
              rcvCntrs.mcPktsSent.l[0]     == expCntrs.mcPktsSent.l[0] &&      \
              rcvCntrs.goodOctetsRcv.l[0]  == expCntrs.goodOctetsRcv.l[0] &&   \
              rcvCntrs.goodPktsRcv.l[0]    == expCntrs.goodPktsRcv.l[0] &&     \
              rcvCntrs.ucPktsRcv.l[0]      == expCntrs.ucPktsRcv.l[0] &&       \
              rcvCntrs.brdcPktsRcv.l[0]    == expCntrs.brdcPktsRcv.l[0] &&     \
              rcvCntrs.mcPktsRcv.l[0]      == expCntrs.mcPktsRcv.l[0]

/* macro to print port's counters that not match the expected and received */
#define PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqual, expCntrs, rcvCntrs)           \
    if(isEqual == 0)                                                                                                    \
    {/* at least one counter not match */                                                                               \
        PRV_UTF_LOG0_MAC(" counter name ,   expected , received \n");                                                   \
        PRV_UTF_LOG0_MAC(" ==================================== \n");                                                   \
        if(rcvCntrs.goodOctetsSent.l[0] != expCntrs.goodOctetsSent.l[0])                                                \
            PRV_UTF_LOG2_MAC("  goodOctetsSent , %d , %d \n", expCntrs.goodOctetsSent.l[0],rcvCntrs.goodOctetsSent.l[0]);\
        if(rcvCntrs.goodPktsSent.l[0]   != expCntrs.goodPktsSent.l[0])                                                  \
            PRV_UTF_LOG2_MAC("  goodPktsSent   , %d , %d \n", expCntrs.goodPktsSent.l[0],rcvCntrs.goodPktsSent.l[0]);   \
        if(rcvCntrs.ucPktsSent.l[0]     != expCntrs.ucPktsSent.l[0])                                                    \
            PRV_UTF_LOG2_MAC("  ucPktsSent     , %d , %d \n", expCntrs.ucPktsSent.l[0],rcvCntrs.ucPktsSent.l[0]);       \
        if(rcvCntrs.brdcPktsSent.l[0]   != expCntrs.brdcPktsSent.l[0])                                                  \
            PRV_UTF_LOG2_MAC("  brdcPktsSent   , %d , %d \n", expCntrs.brdcPktsSent.l[0],rcvCntrs.brdcPktsSent.l[0]);   \
        if(rcvCntrs.mcPktsSent.l[0]   != expCntrs.mcPktsSent.l[0])                                                      \
            PRV_UTF_LOG2_MAC("  mcPktsSent     , %d , %d \n", expCntrs.mcPktsSent.l[0],rcvCntrs.mcPktsSent.l[0]);       \
        if(rcvCntrs.goodOctetsRcv.l[0]   != expCntrs.goodOctetsRcv.l[0])                                                \
            PRV_UTF_LOG2_MAC("  goodOctetsRcv  , %d , %d \n", expCntrs.goodOctetsRcv.l[0],rcvCntrs.goodOctetsRcv.l[0]); \
        if(rcvCntrs.goodPktsRcv.l[0]   != expCntrs.goodPktsRcv.l[0])                                                    \
            PRV_UTF_LOG2_MAC("  goodPktsRcv    , %d , %d \n", expCntrs.goodPktsRcv.l[0],rcvCntrs.goodPktsRcv.l[0]);     \
        if(rcvCntrs.ucPktsRcv.l[0]   != expCntrs.ucPktsRcv.l[0])                                                        \
            PRV_UTF_LOG2_MAC("  ucPktsRcv      , %d , %d \n", expCntrs.ucPktsRcv.l[0],rcvCntrs.ucPktsRcv.l[0]);         \
        if(rcvCntrs.brdcPktsRcv.l[0]   != expCntrs.brdcPktsRcv.l[0])                                                    \
            PRV_UTF_LOG2_MAC("  brdcPktsRcv    , %d , %d \n", expCntrs.brdcPktsRcv.l[0],rcvCntrs.brdcPktsRcv.l[0]);     \
        if(rcvCntrs.mcPktsRcv.l[0]   != expCntrs.mcPktsRcv.l[0])                                                        \
            PRV_UTF_LOG2_MAC("  mcPktsRcv      , %d , %d \n", expCntrs.mcPktsRcv.l[0],rcvCntrs.mcPktsRcv.l[0]);         \
        PRV_UTF_LOG0_MAC("\n");                                                                                         \
    }

/**
* @enum PRV_TGF_MULTI_DESTINATION_TYPE_ENT
 *
 * @brief enum for types of multi-destination sending
 * Values:
 * PRV_TGF_MULTI_DESTINATION_TYPE_BC_E - BC with incremental SA
 * PRV_TGF_MULTI_DESTINATION_TYPE_MC_E - MC with incremental DA
 * PRV_TGF_MULTI_DESTINATION_TYPE_UNKNOWN_UC_SA_INCREMENT_E - unknown UC with incremental SA
 * PRV_TGF_MULTI_DESTINATION_TYPE_UNKNOWN_UC_SA_INCREMENT_E - unknown UC with incremental DA
 * PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E - all type
*/
typedef enum{

    PRV_TGF_MULTI_DESTINATION_TYPE_BC_E,

    PRV_TGF_MULTI_DESTINATION_TYPE_MC_E,

    PRV_TGF_MULTI_DESTINATION_TYPE_UNKNOWN_UC_SA_INCREMENT_E,

    PRV_TGF_MULTI_DESTINATION_TYPE_UNKNOWN_UC_DA_INCREMENT_E,

    PRV_TGF_MULTI_DESTINATION_TYPE_ALL_E

} PRV_TGF_MULTI_DESTINATION_TYPE_ENT;



/******************************************************************************\
 *                       Public functions section                             *
\******************************************************************************/

/**
* @internal prvTgfCommonFixHwDevId function
* @endinternal
*
* @brief   Function fixes using SW dev Id instead of HW dev Id
*
* @param[in] swDevId                  - value that should be hwDevId, but probably used swDevId
* @param[in] toKeepNull               - relevant only when swDevId == 0
*                                      GT_TRUE - to return 0, GT_FALSE - convert to HW Id
*                                       HW dev Id
*/
GT_U32 prvTgfCommonFixHwDevId
(
    IN  GT_U32  swDevId,
    IN  GT_BOOL toKeepNull
);

/**
* @internal tgfSimSleep function
* @endinternal
*
* @brief   Private function to sleep given time (simulation only)
*
* @param[in] sleepTime                - sleep time (milliseconds)
*                                       None
*/
GT_VOID tgfSimSleep
(
    IN GT_U32  sleepTime
);

/**
* @internal prvTgfCommonIsExternalTcamUseForced function
* @endinternal
*
* @brief   Checks the preconfigured request to use External TCAM instead of Internal TCAM in tests.
*         For such preconfiguration set forceExtTcamUseInTests=1.
*
* @retval IsExternalTcamUseForced  - GT_TRUE use External TCAM, GT_FALSE - use internal TCAM
*/
GT_BOOL prvTgfCommonIsExternalTcamUseForced
(
    GT_VOID
);

/**
* @internal prvTgfResetCountersEth function
* @endinternal
*
* @brief   Reset port counters and set this port in linkup mode
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfResetCountersEth
(
    IN GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
);

/**
* @internal prvTgfSetTxSetupEth function
* @endinternal
*
* @brief   Setting transmit parameters
*
* @param[in] devNum                   - the CPU device number
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - (pointer to) array of VFDs
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfSetTxSetupEth
(
    IN GT_U8        devNum,
    IN TGF_PACKET_STC      *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[]
);

/**
* @internal prvTgfSetTxSetup2Eth function
* @endinternal
*
* @brief   Setting transmit 2 parameters .
*         NOTE:
*         This function should be called after prvTgfSetTxSetupEth(..) ,
*         when need to set the parameters of this function.
* @param[in] sleepAfterXCount         - do 'sleep' after X packets sent
*                                      when = 0 , meaning NO SLEEP needed during the burst
*                                      of 'burstCount'
* @param[in] sleepTime                - sleep time (in milliseconds) after X packets sent , see
*                                      parameter sleepAfterXCount
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvTgfSetTxSetup2Eth
(
    IN GT_U32               sleepAfterXCount,
    IN GT_U32               sleepTime
);

/**
* @internal prvTgfBurstTraceSet function
* @endinternal
*
* @brief   Set max number of packets in burst count that will be printed
*
* @param[in] packetCount              - disable logging when burst is lager then packetCount
*
* @retval GT_OK                    - on success.
*
* @note To restore default setting call function with packetCount == 0
*
*/
GT_STATUS prvTgfBurstTraceSet
(
    IN GT_U32       packetCount
);

/**
* @internal prvTgfStartTransmitingEth function
* @endinternal
*
* @brief   Transmit packet
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfStartTransmitingEth
(
    IN GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
);

/**
* @internal prvTgfStopTransmitingEth function
* @endinternal
*
* @brief   Stop transmit packets
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfStopTransmitingEth
(
    IN GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
);

/**
* @internal prvTgfTxModeSetupEth function
* @endinternal
*
* @brief   Transmit mode configuration
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - Tx mode
* @param[in] units                    - Tx units type
* @param[in] units                    - Tx  values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
*/
GT_STATUS prvTgfTxModeSetupEth
(
    IN GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN PRV_TGF_TX_MODE_ENT              mode,
    IN PRV_TGF_TX_CONT_MODE_UNIT_ENT    unitsType,
    IN GT_U32                           units
);

/**
* @internal prvTgfReadPortCountersEth function
* @endinternal
*
* @brief   Read port's counters
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enablePrint              - enable\disable counters print
*
* @param[out] portCountersPtr          - (pointer to) received counters values
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_FAIL                  - if test doesn't pass comparing.
*/
GT_STATUS prvTgfReadPortCountersEth
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_BOOL                        enablePrint,
    OUT CPSS_PORT_MAC_COUNTER_SET_STC *portCountersPtr
);

/**
* @internal prvTgfPrintPortCountersEth function
* @endinternal
*
* @brief   Print counters of all TGF ports
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_FAIL                  - if test doesn't pass comparing.
*/
GT_STATUS prvTgfPrintPortCountersEth
(
    GT_VOID
);

/**
* @internal prvTgfPacketSizeGet function
* @endinternal
*
* @brief   Calculate packet size
*
* @param[in] partsArray[]             - packet part's array
* @param[in] partsCount               - number of parts
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_FAIL                  - if test doesn't pass comparing.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfPacketSizeGet
(
    IN  TGF_PACKET_PART_STC  partsArray[],
    IN  GT_U32               partsCount,
    OUT GT_U32              *packetSizePtr
);

/**
* @internal prvTgfPortCapturedPacketPrint function
* @endinternal
*
* @brief   Print captured packet on specific port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfPortCapturedPacketPrint
(
    IN GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
);

/**
* @internal prvTgfEthCountersCheck function
* @endinternal
*
* @brief   Read and check port's counters
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] packetsCountRx           - Expected number of Rx packets
* @param[in] packetsCountTx           - Expected number of Tx packets
* @param[in] packetSize               - Size of packets, 0 means don't check
* @param[in] burstCount               - Number of packets with the same size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthCountersCheck
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32                         packetsCountRx,
    IN  GT_U32                         packetsCountTx,
    IN  GT_U32                         packetSize,
    IN  GT_U32                         burstCount
);

/**
* @internal prvTgfEthPortSetCountersCheck function
* @endinternal
*
* @brief   Read and check counters of set of ports
*
* @param[in] devNum                   - device number
* @param[in] allPortsBmpPtr           - (pointer to)bitmap of all checked ports
* @param[in] rxPortsBmpPtr            - (pointer to)bitmap of ports expected to receive packets
*                                      must be subset of allPortsBmpPtr
* @param[in] txPortsBmpPtr            - (pointer to)bitmap of ports expected to send packets
*                                      must be subset of allPortsBmpPtr
* @param[in] packetSize               - Size of packets, 0 means don't check
* @param[in] burstCount               - Number of packets with the same size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthPortSetCountersCheck
(
    IN  GT_U8                          devNum,
    IN  CPSS_PORTS_BMP_STC             *allPortsBmpPtr,
    IN  CPSS_PORTS_BMP_STC             *rxPortsBmpPtr,
    IN  CPSS_PORTS_BMP_STC             *txPortsBmpPtr,
    IN  GT_U32                         packetSize,
    IN  GT_U32                         burstCount
);

/**
* @internal prvTgfEthCountersReset function
* @endinternal
*
* @brief   Reset all Ethernet port's counters
*         also do tgfTrafficTableRxPcktTblClear()
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthCountersReset
(
    IN GT_U8 devNum
);

/**
* @internal prvTgfEthIngressPacketsCounterCheck function
* @endinternal
*
* @brief   Read and check port's ingress packets counter
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] packetsCountRx           - Expected number of Rx packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthIngressPacketsCounterCheck
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32                         packetsCountRx
);

/**
* @internal prvTgfEthCounterLoopbackEgrPort function
* @endinternal
*
* @brief   Enable/disable loopback on port to count ingress packets
*         instead of egress counters.
*         Workaround on BC2 GM that counts egress packets dropped by EPCL.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] loopback                 - GT_TRUE - enable loopback.
*                                      pVid and pVidForce retrieved, saved,
*                                      and set pVid=0, pVidForce=GT_TRUE
*                                      GT_FALSE - disable loopback.
*                                      pVid and pVidForce restored.
* @param[in,out] pVidPtr                  - (pointer to) Port VID.
* @param[in,out] pVidForcePtr             - (pointer to) Port VID Force.
* @param[in,out] pVidPtr                  - (pointer to) Port VID.
* @param[in,out] pVidForcePtr             - (pointer to) Port VID Force.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong interface
*/
GT_STATUS prvTgfEthCounterLoopbackEgrPort
(
    IN    GT_U8                          devNum,
    IN    GT_PHYSICAL_PORT_NUM           portNum,
    IN    GT_BOOL                        loopback,
    INOUT GT_U16                         *pVidPtr,
    INOUT GT_BOOL                        *pVidForcePtr
);

/**
* @internal prvTgfCommonMultiDestinationTrafficSend function
* @endinternal
*
* @brief   Send multi-destination traffic , meaning from requested port:
*         Send BC (FF:FF:FF:FF:FF:FF) with burstNum incremental SA.
*         Send unregistered MC (01:02:03:04:05:06) with burstNum incremental SA.
*         Send unknown unicast (00:09:99:99:99:99) with burstNum incremental DA and static SA
*         Send unknown unicast (00:09:99:99:99:99) with burstNum static DA and incremental SA
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] burstNum                 - number of packets to send in the burst
* @param[in] trace                    - enable\disable packet tracing
*                                       None
*/
void prvTgfCommonMultiDestinationTrafficSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32  burstNum,
    IN  GT_BOOL trace
);

/**
* @internal prvTgfCommonMultiDestinationTrafficCounterCheck function
* @endinternal
*
* @brief   check that traffic counter match the needed value
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] expectedCounterValue     - expected counter value (counter of goodPktsSent.l[0])
*                                       None
*/
void prvTgfCommonMultiDestinationTrafficCounterCheck
(
    IN GT_U8    devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN GT_U32   expectedCounterValue
);

/**
* @internal prvTgfCommonIncrementalSaMacSend function
* @endinternal
*
* @brief   Send packets with incremental SA mac address (that start from specified mac
*         address)
*         -- this used for 'mac incremental learning'
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] saMacPtr                 - (pointer to) the base mac address (of source mac)
* @param[in] burstNum                 - number of packets to send in the burst
*                                       None
*/
void prvTgfCommonIncrementalSaMacSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  TGF_MAC_ADDR    *saMacPtr,
    IN  GT_U32  burstNum
);

/**
* @internal prvTgfCommonIncrementalSaMacByPortNumSend function
* @endinternal
*
* @brief   Send packets with incremental SA mac address .
*         this start from mac address of:  [00:00:port:port:port:port]
*         For example :
*         for port 0x22   --> [00:00:22:22:22:22]
*         and for port 0x6 --> [00:00:06:06:06:06]
*         -- this used for 'mac incremental learning'
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] burstNum                 - number of packets to send in the burst
*                                       None
*/
void prvTgfCommonIncrementalSaMacByPortNumSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32  burstNum
);


/**
* @internal prvTgfCommonIncrementalDaMacSend function
* @endinternal
*
* @brief   Send packets with incremental DA mac address (that start from specified mac
*         address) and static SA mac address
*         -- this used for checking the already learned mac addresses
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] saMacPtr                 - (pointer to) the SA mac address
* @param[in] daMacPtr                 - (pointer to) the DA base mac address
* @param[in] burstNum                 - number of packets to send in the burst
*                                       None
*/
void prvTgfCommonIncrementalDaMacSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  TGF_MAC_ADDR    *saMacPtr,
    IN  TGF_MAC_ADDR    *daMacPtr,
    IN  GT_U32  burstNum
);

/**
* @internal prvTgfCommonIncrementalDaMacByPortNumSend function
* @endinternal
*
* @brief   Send packets with incremental DA mac address (according to dstPortNum)
*         and static SA mac address (according to portNum)
*         DA start from mac address of:  [00:00:dstPortNum:dstPortNum:dstPortNum:dstPortNum]
*         For example :
*         for port 0x22   --> [00:00:22:22:22:22]
*         and for port 0x6 --> [00:00:06:06:06:06]
*         SA start from mac address of:  [00:00:portNum:portNum:portNum:portNum]
*         For example :
*         for port 0x33   --> [00:00:33:33:33:33]
*         and for port 0x8 --> [00:00:08:08:08:08]
*         -- this used for checking the already learned mac addresses
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
*                                      this port number also used for SA mac address
* @param[in] dstPortNum               - the alleged port number to receive the traffic
*                                      this port number used for base DA mac address
*                                      daMacPtr     - (pointer to) the DA base mac address
* @param[in] burstNum                 - number of packets to send in the burst
*                                       None
*/
void prvTgfCommonIncrementalDaMacByPortNumSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32   dstPortNum,
    IN  GT_U32  burstNum
);

/**
* @internal prvTgfCommonMultiDestinationTrafficTypeSend function
* @endinternal
*
* @brief   like prvTgfCommonMultiDestinationTrafficSend it :
*         Send multi-destination traffic , meaning from requested port:
*         Send BC (FF:FF:FF:FF:FF:FF) with burstNum incremental SA.
*         Send unregistered MC (01:02:03:04:05:06) with burstNum incremental SA.
*         Send unknown unicast (00:09:99:99:99:99) with burstNum incremental DA and static SA
*         Send unknown unicast (00:09:99:99:99:99) with burstNum static DA and incremental SA
*         BUT the packet itself (beside the SA,DA) is given by the caller.
* @param[in] devNum                   - the device to send traffic from.
* @param[in] portNum                  - the port to send traffic from.
* @param[in] burstNum                 - number of packets to send in the burst
* @param[in] trace                    - enable\disable packet tracing
* @param[in] packetPtr                - pointer to the packet
* @param[in] type                     - multi-destination  of sending
*                                       None
*/
void prvTgfCommonMultiDestinationTrafficTypeSend
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  GT_U32  burstNum,
    IN  GT_BOOL trace,
    IN  TGF_PACKET_STC *packetPtr,
    IN  PRV_TGF_MULTI_DESTINATION_TYPE_ENT  type
);

/**
* @internal prvTgfTransmitPacketsWithCapture function
* @endinternal
*
* @brief   None
*
* @param[in] inDevNum                 - ingress device number
* @param[in] inPortNum                - ingress port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - (pointer to) array of VFDs
* @param[in] outDevNum                - egress device number
* @param[in] outPortNum               - egress port number
* @param[in] captureMode              - packet capture mode
* @param[in] captureOnTime            - time the capture will be enabe (in milliseconds)
*                                       None
*/
GT_STATUS prvTgfTransmitPacketsWithCapture
(
    IN GT_U8                inDevNum,
    IN GT_U32                inPortNum,
    IN TGF_PACKET_STC       *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[],
    IN GT_U8                outDevNum,
    IN GT_U32                outPortNum,
    IN TGF_CAPTURE_MODE_ENT captureMode,
    IN GT_U32               captureOnTime
);

/**
* @internal prvTgfTransmitPackets function
* @endinternal
*
* @brief   None
*
* @param[in] inDevNum                 - ingress device number
* @param[in] inPortNum                - ingress port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - (pointer to) array of VFDs
*                                       None
*/
GT_STATUS prvTgfTransmitPackets
(
    IN GT_U8                inDevNum,
    IN GT_U32                inPortNum,
    IN TGF_PACKET_STC       *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[]
);

/**
* @internal prvTgfTransmitPacketsWithDelay function
* @endinternal
*
* @brief   None
*
* @param[in] inDevNum                 - ingress device number
* @param[in] inPortNum                - ingress port number
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] burstCount               - number of frames
* @param[in] numVfd                   - number of VFDs
* @param[in] vfdArray[]               - (pointer to) array of VFDs
*                                       None
* @param[in] sleepTime                - Delay between each packet
*/
GT_STATUS prvTgfTransmitPacketsWithDelay
(
    IN GT_U8                inDevNum,
    IN GT_U32                inPortNum,
    IN TGF_PACKET_STC       *packetInfoPtr,
    IN GT_U32               burstCount,
    IN GT_U32               numVfd,
    IN TGF_VFD_INFO_STC     vfdArray[],
    IN GT_U32               sleepTime
);


/**
* @internal prvTgfDefPortsArrayFromListSet function
* @endinternal
*
* @brief   Set predefined number of ports and port's array in PreInit or PostInit
*         phase
* @param[in] numOfPorts               - number of ports
* @param[in] portNum                  - list of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_SIZE              - wrong array size
* @retval GT_BAD_PTR               - wrong pointer
*/
GT_STATUS prvTgfDefPortsArrayFromListSet
(
    IN GT_U8      numOfPorts,
    IN GT_U32     portNum,
    IN ...
);

/**
* @internal prvTgfDefPortsArrayModeSet function
* @endinternal
*
* @brief   Set default arrays of test ports according to specific mode.
*
* @param[in] portMode                 - port mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong input parameter
*/
GT_STATUS prvTgfDefPortsArrayModeSet
(
    IN PRV_TGF_DEVICE_PORT_MODE_ENT     portMode
);

/**
* @internal prvTgfDefPortsArraySet function
* @endinternal
*
* @brief   Set predefined number of ports and port's array
*
* @param[in] portsArray[]             - array of ports
* @param[in] arraySize                - number of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_SIZE              - wrong array size
* @retval GT_BAD_PTR               - wrong pointer
*
* @note Call this function each time when needs to override default ports array
*       by specific values
*
*/
GT_STATUS prvTgfDefPortsArraySet
(
    IN GT_U32    portsArray[],
    IN GT_U8    arraySize
);

/**
* @internal prvTgfDefPortsArrayPortToPortIndex function
* @endinternal
*
* @brief   Search for port in ports array and return the index of this port in the
*         array.
* @param[in] port                     -  to search
*
* @param[out] portIndexPtr             - points to index of the port in the array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - wrong pointer
* @retval GT_NOT_FOUND             - port is not part of ports array
*/
GT_STATUS prvTgfDefPortsArrayPortToPortIndex
(
    IN  GT_U32   port,
    OUT GT_U8   *portIndexPtr
);

/**
* @internal prvTgfTestsPortRangeSelectPreInit function
* @endinternal
*
* @brief   force pre init the device type for ports array to be used in the UTF.
*         allow to set tests with ports : 0..59 or 0..123 for a device with 124 ports.
*         for example : by default AslanB will test ports 0..59 BUT by call from
*         the terminal we can make it to use ports 0..123
*
* @retval GT_OK                    - in case of success
*/
GT_STATUS prvTgfTestsPortRangeSelectPreInit
(
    IN GT_U32   numOfPorts
);

/**
* @internal prvTgfTestsPortRangeSelect function
* @endinternal
*
* @brief   force device type for ports array to be used in the UTF.
*         allow to set tests with ports : 0..59 or 0..123 for a device with 124 ports.
*         for example : by default AslanB will test ports 0..59 BUT by call from
*         the terminal we can make it to use ports 0..123
*/
GT_VOID prvTgfTestsPortRangeSelect
(
    IN GT_U32   numOfPorts
);

/**
* @internal prvTgfPortsArrayByDevTypeSet function
* @endinternal
*
* @brief   Override default ports from prvTgfPortsArray by new ports from
*         prvTgfDefPortsArray depending on device type
*/
GT_VOID prvTgfPortsArrayByDevTypeSet
(
    GT_VOID
);

/**
* @internal prvTgfPortsArrayByDevTypeRandomSet function
* @endinternal
*
* @brief   Fill ports array with valid random port numbers
*
* @param[in] seed                     -  value for random engine
*                                       None
*/
GT_VOID prvTgfPortsArrayByDevTypeRandomSet
(
    IN  GT_U32      seed
);

/**
* @internal prvTgfPortsArrayPrint function
* @endinternal
*
* @brief   Print number of used ports and port's array prvTgfPortsArray
*/
GT_VOID prvTgfPortsArrayPrint
(
    GT_VOID
);

/**
* @internal prvTgfCommonInit function
* @endinternal
*
* @brief   Initialize TFG configuration
*
* @param[in] firstDevNum              - device number of first device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Put all necessary initializations into this function
*
*/
GT_STATUS prvTgfCommonInit
(
    GT_U8 firstDevNum
);

/**
* @internal prvTgfCommonIsUseLastTpid function
* @endinternal
*
* @brief   check if the test use last TPIDs for ingress
*
* @retval GT_TRUE                  - the device use last TPIDs for ingress
* @retval GT_FALSE                 - the device NOT use last TPIDs for ingress
*/
GT_BOOL prvTgfCommonIsUseLastTpid(
    void
);

/**
* @internal prvTgfCommonUseLastTpid function
* @endinternal
*
* @brief   state that test use last TPIDs for ingress or not
*
* @param[in] enable
*                                      GT_TRUE  - the device  use last TPIDs for ingress
*                                      GT_FALSE - the device  NOT use last TPIDs for ingress
*/
void prvTgfCommonUseLastTpid(
    IN GT_BOOL  enable
);

/**
* @internal prvTgfCommonVntTimeStampsRateGet function
* @endinternal
*
* @brief   Get rate of events by VNT Time Stamps.
*
* @param[in] dev                      - device number
* @param[in] startTimeStamp           - start counting time stamp
* @param[in] endTimeStamp             - end counting time stamp
* @param[in] eventCount               - events count between end and start time stamps
*                                       events rate in events per second
*/
GT_U32 prvTgfCommonVntTimeStampsRateGet
(
    IN  GT_U8          dev,
    IN  GT_U32         startTimeStamp,
    IN  GT_U32         endTimeStamp,
    IN  GT_U32         eventCount
);

/**
* @internal prvTgfCommonPortTxTwoUcTcRateGet function
* @endinternal
*
* @brief   Get rate of two unicast flows by egress counters.
*
* @param[in] portIdx                  - index of egress port
* @param[in] tc1                      - traffic class of first unicast flow
* @param[in] tc2                      - traffic class of second unicast flow
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRate1Ptr              - (pointer to) Tx rate of first flow
* @param[out] outRate1Ptr              - (pointer to) Tx rate of second flow
* @param[out] dropRate1Ptr             - (pointer to) drop rate of first flow
* @param[out] dropRate1Ptr             - (pointer to) drop rate of second flow
*                                       none
*/
GT_VOID prvTgfCommonPortTxTwoUcTcRateGet
(
    IN  GT_U32 portIdx,
    IN  GT_U32 tc1,
    IN  GT_U32 tc2,
    IN  GT_U32 timeOut,
    OUT GT_U32 *outRate1Ptr,
    OUT GT_U32 *outRate2Ptr,
    OUT GT_U32 *dropRate1Ptr,
    OUT GT_U32 *dropRate2Ptr
);

/**
* @internal prvTgfCommonPortCntrRateGet function
* @endinternal
*
* @brief   Get rate of specific port MAC MIB counter.
*
* @param[in] portIdx                  - index of egress port
* @param[in] cntrName                 - name of counter
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRatePtr               - (pointer to) rate of port's counter
*                                       none
*/
GT_VOID prvTgfCommonPortCntrRateGet
(
    IN  GT_U32                      portIdx,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    IN  GT_U32                      timeOut,
    OUT GT_U32                      *outRatePtr
);

/**
* @internal prvTgfCommonPortTxRateGet function
* @endinternal
*
* @brief   Get Tx rate of port by MAC MIB counters.
*
* @param[in] portIdx                  - index of egress port
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRatePtr               - (pointer to) Tx rate of port
*                                       none
*/
GT_VOID prvTgfCommonPortTxRateGet
(
    IN  GT_U32 portIdx,
    IN  GT_U32 timeOut,
    OUT GT_U32 *outRatePtr
);

/**
* @internal prvTgfCommonPortRxRateGet function
* @endinternal
*
* @brief   Get Rx rate of port by MAC MIB counters.
*
* @param[in] portIdx                  - index of ingress port
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRatePtr               - (pointer to) Rx rate of port
*                                       none
*/
GT_VOID prvTgfCommonPortRxRateGet
(
    IN  GT_U32 portIdx,
    IN  GT_U32 timeOut,
    OUT GT_U32 *outRatePtr
);

/**
* @internal prvTgfCommonPortsRxRateGet function
* @endinternal
*
* @brief   Get Rx rate of ports by MAC MIB counters.
*
* @param[in] portIdxArr[]             - array of indexes of ingress ports
* @param[in] numOfPorts               - number of ports
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRateArr[]             - (pointer to) array of Rx rate of ports
*                                       none
*/
GT_VOID prvTgfCommonPortsRxRateGet
(
    IN  GT_U32 portIdxArr[],
    IN  GT_U32 numOfPorts,
    IN  GT_U32 timeOut,
    OUT GT_U32 outRateArr[]
);

/**
* @internal prvTgfCommonPortsTxRateGet function
* @endinternal
*
* @brief   Get Tx rate of ports by MAC MIB counters.
*
* @param[in] portIdxArr[]             - array of indexes of egress ports
* @param[in] numOfPorts               - number of ports
* @param[in] timeOut                  - timeout in milli seconds to calculate rate
*
* @param[out] outRateArr[]             - (pointer to) array of Tx rate of ports
*                                       none
*/
GT_VOID prvTgfCommonPortsTxRateGet
(
    IN  GT_U32 portIdxArr[],
    IN  GT_U32 numOfPorts,
    IN  GT_U32 timeOut,
    OUT GT_U32 outRateArr[]
);


/**
* @internal prvTgfCommonPortWsRateGet function
* @endinternal
*
* @brief   Get wire speed rate of port in packets per second
*
* @param[in] portIdx                  - index of egress port
* @param[in] packetSize               - packet size in bytes including CRC
*                                       wire speed rate of port in packets per second.
*
* @retval 0xFFFFFFFF               - on error
*/
GT_U32 prvTgfCommonPortWsRateGet
(
    IN  GT_U32 portIdx,
    IN  GT_U32 packetSize
);

/**
* @internal prvTgfPrefixPrint function
* @endinternal
*
* @brief   Print IP address, mask and prefix
*
* @param[in] protocol                 -  type
* @param[in] ipAddrPtr                - (pointer to) IP address
* @param[in] ipMaskPtr                - (pointer to) IP mask
* @param[in] prefixLen                - prefix length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvTgfPrefixPrint
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN GT_U8                      *ipAddrPtr,
    IN GT_U8                      *ipMaskPtr,
    IN GT_U8                      prefixLen
);

/**
* @internal appDemoGenEventCounterGet function
* @endinternal
*
* @brief   get the number of times that specific event happened.
*
* @param[in] devNum                   - device number
* @param[in] uniEvent                 - unified event
* @param[in] clearOnRead              - do we 'clear' the counter after 'read' it
*                                      GT_TRUE - set counter to 0 after get it's value
*                                      GT_FALSE - don't update the counter (only read it)
*
* @param[out] counterPtr               - (pointer to)the counter (the number of times that specific event happened)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum or uniEvent.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - the counters DB not initialized for the device.
*
* @note none
*
*/
extern GT_STATUS appDemoGenEventCounterGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_BOOL                  clearOnRead,
    OUT GT_U32                  *counterPtr
);

/**
* @internal prvTgfCommonAllBridgeCntrsReset function
* @endinternal
*
* @brief   Reset hw counters - egress counters, bridge host group counters,
*         bridge ingress counters, bridge host group counters.
*/
GT_STATUS prvTgfCommonAllBridgeCntrsReset
(
    GT_VOID
);

/**
* @internal prvTgfCommonForceLinkUpOnAllTestedPorts function
* @endinternal
*
* @brief   force link up on all tested ports.
*/
void prvTgfCommonForceLinkUpOnAllTestedPorts
(
    void
);

/**
* @internal prvTgfPortEgressCntIsCaelumErratumExist function
* @endinternal
*
* @brief   Returns GT_TRUE if device has Caelum's egress counters erratum.
*
* @param[in] devNum                   - device number
*
* @retval GT_TRUE                  - device has Caelum's egress counters erratum.
* @retval GT_FALSE                 - device doesn't has Caelum's egress counters erratum.
*/
GT_BOOL prvTgfPortEgressCntIsCaelumErratumExist
(
    IN GT_U8        devNum
);

/**
* @internal prvTgfDefPortsArrange function
* @endinternal
*
* @brief   Permute prvTgfPortsArray items so that items with specified
*         indices satisfy condition.
*         Example: prvTgfPortsArray = (11, 22, 33, 44, 55);
*         remote ports: 11, 22, 33
*         After prvTgfDefPortsArrange(GT_FALSE, 0,3,-1) the result can be
*         (44, 22, 33, 55, 11) or
*         (55, 22, 33, 44, 11) or
*         any other where items [0], [3] are not remote ports
* @param[in] isRemote                 - the condition.
*                                      GT_TRUE  - ports should be remote ports
*                                      GT_FALSE - ports must not be remote ports.
* @param[in] index                    - first  that is need to be checked.
*                                      ...        - rest of the indices. The negative values mark the end of
*                                      the list.
*
* @retval GT_BAD_PARAM             - wrong index. Index exceeds either size of prvTgfPortsArray
*                                       or the number of actual (not PRV_TGF_INVALID_PORT_NUM_CNS)
*                                       ports in prvTgfPortsArray.
* @retval GT_FAIL                  - prvTgfPortsArrays items can't be organized to satisfy
*                                       requirements.
*
* @note Don't forget pass value < 0 to mark the list of indices is finished.
*
*/
GT_STATUS prvTgfDefPortsArrange
(
    IN GT_BOOL isRemote,
    IN GT_32   index,
    IN ...
);

/**
* @internal prvTgfFilterDefPortsArray function
* @endinternal
*
* @brief   Fill destination array with only ports from prvTgfPortsArray which
*         appropriate to specified filter
* @param[in] getRemote                - a filter.
*                                      GT_TRUE  - get only remote ports.
*                                      GT_FALSE - get only local ports.
* @param[in] destArrLen               - maximum number of items in the destination array
*
* @param[out] destArr                  - destination array with gathered ports
*                                       actual number of items in destArr. Can't exceed destArrLen
*/
GT_U32 prvTgfFilterDefPortsArray
(
    IN  GT_BOOL                  getRemote,
    OUT GT_U32                   *destArr,
    IN  GT_U32                   destArrLen
);

/**
* @internal prvTgfXcat3xExistsSet function
* @endinternal
*
* @brief   Raise flag indicating existance of AC3X (Aldrin + 88E1690)
*         in the system
* @param[in] isExists                 - GT_TRUE  - exists
*                                      GT_FALSE - not exists
*                                       None.
*/
GT_VOID prvTgfXcat3xExistsSet
(
    GT_BOOL isExists
);

/**
* @internal prvTgfXcat3xExists function
* @endinternal
*
* @brief   Get flag indicating existance of AC3X (Aldrin + 88E1690) in the system
*
* @retval GT_TRUE                  - exists
* @retval GT_FALSE                 - not exists
*/
GT_BOOL prvTgfXcat3xExists
(
    GT_VOID
);

/**
* @internal prvTgfFilterDefPortsArray function
* @endinternal
*
* @brief   Fill destination array with only ports from prvTgfPortsArray which
*         appropriate to specified filter
* @param[in] dsaType                 - a filter according to the DSA type.
*                       check if the port number is special (59-63) when need to
*                       be in the DSA tag as the DSA hold limited number of bits
*                       for the portNum.
*
* @param[in] destArrLen     - maximum number of items in the destination array
*
* @param[out] destArr        - destination array with gathered ports
*                             actual number of items in destArr. Can't exceed destArrLen
*/
GT_U32 prvTgfFilterDsaSpecialPortsArray
(
    IN  TGF_DSA_TYPE_ENT         dsaType,
    OUT GT_U32                   *destArr,
    IN  GT_U32                   destArrLen
);

/**
* @internal prvTgfFilterLimitedBitsSpecialPortsArray function
* @endinternal
*
* @brief   Fill destination array with only ports from prvTgfPortsArray which
*         appropriate to specified filter
* @param[in] numOfBits - a filter according to the number of bits.
*                       check if the port number is special (59-63) when need to
*                       be in limited to <numOfBits> .
* @param[in] offset    - offset to added to ports in the array , only to check
*                       that value not in the filtered range .
*
* @param[in] destArrLen     - maximum number of items in the destination array
*
* @param[out] destArr        - destination array with gathered ports
*                             actual number of items in destArr. Can't exceed destArrLen
*/
GT_U32 prvTgfFilterLimitedBitsSpecialPortsArray
(
    IN  GT_U32                   numOfBits,
    IN  GT_32                    offset,
    OUT GT_U32                   *destArr,
    IN  GT_U32                   destArrLen
);



/**
* @internal prvTgfPortModeGet function
* @endinternal
*
* @brief   return the number of ports use by the Device for 
*          falcon . 64,128,256,512 and 1024
*  
*/
GT_U32 prvTgfPortModeGet
(
  GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __fgfCommonh */

