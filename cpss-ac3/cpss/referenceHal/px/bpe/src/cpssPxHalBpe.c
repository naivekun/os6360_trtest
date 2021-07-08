/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssPxHalBpe.c
*
* DESCRIPTION:
*       CPSS Px bridge port extender implementation.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/px/cpssPxTypes.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/cos/cpssPxCos.h>
#include <cpss/px/cnc/cpssPxCnc.h>
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpssPxHalBpe.h>

/**************************** Constants definitions *****************************/
#define PRV_CPSS_PX_TRUNKS_NUM_CNS                  17
#define PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS 127
#define PRV_CPSS_PX_LAG_DESIGNATED_TABLE_MC_MAX_CNS 255
#define PRV_CPSS_PX_PORTS_NUM_CNS                   17
#define PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS   19  /* number of reserved entries in destination portmap table */
#define PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS  0  /* index for upstream traffic in destination portmap table */
#define PRV_CPSS_HAL_BPE_TRAFFIC_2_CPU_INDEX_CNS     1  /* index for traffic to cpu in destination portmap table */
#define PRV_CPSS_HAL_BPE_DISCARDED_TRAFFIC_INDEX_CNS 2  /* index for discarded traffic in destination portmap table */
#define PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS  3  /* start index for traffic from cpu in destination portmap table; range 3..18 */
#define PRV_CPSS_HAL_BPE_CTRL_DATA_RULE_CONVERT_CNS  9  /* convert for rule index (0..CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS) to internal rule index (9..29)*/
#define PRV_CPSS_HAL_BPE_CNC_BLOCKS_NUM_CNS          2  /* number of CNC blocks allowed to be used by HAL */
/****************************** Enum definitions ********************************/
/**
* @enum PRV_CPSS_PX_HAL_BPE_PKT_TYPE_ENT
 *
 * @brief Enumeration of bridge port extender ingress packet type.
*/
typedef enum{

    /** A packet from upstream in the downstream direction with a unicast E-Tag */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E    =       0,

    /** A packet from upstream in the downstream direction with a multicast E-Tag */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E,

    /** @brief A packet from upstream to CPU with an E-Tag; E-Tag<E-CID>= Source port number
     *  (control traffic to PE's CPU)
     */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_CPU_E,

    /** A packet from cascade to upstream with an E-Tag */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E,

    /** A packet from cascade to upstream without an E-Tag (control traffic to CB's CPU) */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E,

    /** A packet from extended to upstream with an E-Tag; Should be dropped */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E,

    /** A packet from extended to upstream without an E-Tag with C-Tag */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E,

    /** A packet from extended to upstream without an E-Tag or C-Tag */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E,

    /** @brief A packet from CPU to upstream with an E-Tag; E-Tag<E-CID>= Target port number
     *  (control traffic to CB's CPU)
     */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E,

    /** reserved 1 packet type for future use */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_1_E,

    /** reserved 2 packet type for future use */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_2_E,

    /** first rule for data traffic from extended to upstream with user-defined QoS */
    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E,

    PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E = PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E

} PRV_CPSS_PX_HAL_BPE_PKT_TYPE_ENT;

/**
* @enum PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_ENT
 *
 * @brief Enumeration of bridge port extender src port profile.
*/
typedef enum{

    /** src port profile idle; not used */
    PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_IDLE_E = 0,

    /** src port profile upstream */
    PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_UPSTREAM_E,

    /** src port profile cascade */
    PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CASCADE_E,

    /** src port profile extended */
    PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_EXTENDED_E,

    /** src port profile cpu */
    PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CPU_E

} PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_ENT;

/********************************* Structures definitions ***********************/
/**
* @struct CPSS_PX_HAL_BPE_TRUNK_INFO_STC
 *
 * @brief This structure defines trunk info.
*/
typedef struct{

    /** trunk type. */
    CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT trunkType;   

    /* bitmap of ports */
    CPSS_PX_PORTS_BMP portsBmpArr[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /* number of ports */
    GT_U32 numOfPortsArr[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /** if this trunk is valid */
    GT_BOOL valid;

    /** @brief trunk cid
     *  Comments:
     */
    GT_U32 cid;

} CPSS_PX_HAL_BPE_TRUNK_INFO_STC;

/**
* @struct CPSS_PX_HAL_BPE_PORT_INFO_STC
 *
 * @brief This structure defines port info.
*/
typedef struct{

    /** port interface. */
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC interfce;

    /** port BPE mode. */
    CPSS_PX_HAL_BPE_PORT_MODE_ENT mode;

    /** port cid */
    GT_U32 pcid;

    /** port Upstream interface */
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC upstream;

    /** @brief port Qos
     *  Comments:
     */
    CPSS_PX_HAL_BPE_PORT_QOS portQos;

} CPSS_PX_HAL_BPE_PORT_INFO_STC;

/*
 * Typedef: struct CPSS_PX_HAL_BPE_CHANNEL_INFO_STC
 *
 * Description:
 *     This structure defines channel info.
 *
 * Fields:
 *     interfce         - channel interface
 *     cid              - channel cid
 *     nextChannelPtr   - (pointer to) next channel
 *
 *  Comments:
 */
typedef struct CPSS_PX_HAL_BPE_CHANNEL_INFO_STC_
{
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC          interfce;
    GT_U32                                      cid;
    struct CPSS_PX_HAL_BPE_CHANNEL_INFO_STC_    *nextChannelPtr;
} CPSS_PX_HAL_BPE_CHANNEL_INFO_STC;

/**
* @struct CPSS_PX_HAL_BPE_DEV_INFO_STC
 *
 * @brief This structure defines device info.
*/
typedef struct{

    CPSS_PX_HAL_BPE_TRUNK_INFO_STC trunkDbArr[PRV_CPSS_PX_TRUNKS_NUM_CNS];

    CPSS_PX_HAL_BPE_PORT_INFO_STC portDbArr[PRV_CPSS_PX_PORTS_NUM_CNS];

    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC *channelDbPtr;

    /** BPE package was initialized for the device */
    GT_BOOL valid;

    /** current number of upstream port defined */
    GT_U32 numOfUpstreamPorts;

    /** max number of unicast E */
    GT_U32 maxBpeUnicastEChannels;

    /** max number of multicast E */
    GT_U32 maxBpeMulticastEChannels;

    /** current number of unicast E */
    GT_U32 numBpeUnicastEChannel;

    /** current number of multicast E */
    GT_U32 numBpeMulticastEChannel;

    /** bitmap of CNC Blocks that can be used by 802.1BR refHal for debug (APPLICABLE RANGE: 0..3) */
    GT_U32 cncBitmap;

    CPSS_PX_CNC_CLIENT_ENT cncClientConfig[PRV_CPSS_HAL_BPE_CNC_BLOCKS_NUM_CNS];

    /** Offset of Dst_Idx into destination portmap table for UC packets */
    GT_U32 ucDstIndexCns;

    /** Offset of Dst_Idx into destination portmap table for MC packets; can be negative */
    GT_U32 mcDstIndexCns;

    GT_U32 ip2MeIndex[CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS];

} CPSS_PX_HAL_BPE_DEV_INFO_STC;

/* array of packet types; index in array is used to perform a TCAM-like search in Packet Type Table */
GT_U32  CPSS_PX_HAL_BPE_PKT_TYPE_ARR[CPSS_PX_PACKET_TYPE_MAX_CNS] = {0};

/* array of device database */
CPSS_PX_HAL_BPE_DEV_INFO_STC devDbArr[PRV_CPSS_MAX_PP_DEVICES_CNS];

/**************************** Private macro definitions ***********************/

/* Log numeric format */
const GT_CHAR *prvHalBpeFormatNumFormat            = "%s = %d";

/* format parameter value by its name */
#define BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(_param)      prvHalBpeFormatNumFormat, #_param, _param

/* macro to print error message */
GT_BOOL prvHalBpeDebugErrors = GT_TRUE;
#define BPE_LOG_ERROR_AND_RETURN_MAC(_rc, ...)                                                                              \
    do {                                                                                                                    \
        if (prvHalBpeDebugErrors == GT_TRUE) prvCpssPxHalBpeLogError(__FUNCTION__, __LINE__, _rc, __VA_ARGS__);                        \
        return _rc;                                                                                                         \
    } while (0)

/* macro to log unknown ENUM parameter , and return GT_BAD_PARAM */
#define BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(enumParam)                                                                     \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ENUM [%s] not supports value [%d]",                                     \
            #enumParam,                                                                                                     \
            enumParam)

/* check that the device is valid */
#define PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum)                                                                           \
    if (devDbArr[devNum].valid != GT_TRUE)                                                                                  \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "devNum[%d] is not valid",(devNum));                                     \
    }

/* check that the trunk is valid */
#define PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_MAC(devNum, trunkId)                                                                \
    if (devDbArr[devNum].trunkDbArr[trunkId].valid != GT_TRUE)                                                              \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "{devNum[%d],trunkId[%d]} not valid",(devNum),(trunkId));                \
    }

/* check that the port is in mode */
#define PRV_CPSS_PX_HAL_BPE_PORT_MODE_CHECK_MAC(devNum, portNum, mode_)                                                      \
    if (devDbArr[devNum].portDbArr[portNum].mode != mode_)                                                                   \
    {                                                                                                                        \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "{devNum[%d],portNum[%d]} mode is [%d]",(devNum),(portNum),(devDbArr[devNum].portDbArr[portNum].mode));      \
    }

/* check that the port is in mode EXTENDED or CASCADE */
#define PRV_CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum, portNum)                                         \
    if (devDbArr[devNum].portDbArr[portNum].mode != CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E &&                                  \
        devDbArr[devNum].portDbArr[portNum].mode != CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E)                                     \
    {                                                                                                                        \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "{devNum[%d],portNum[%d]} mode is [%d]",(devNum),(portNum),(devDbArr[devNum].portDbArr[portNum].mode));      \
    }

/* check that the trunk is in mode EXTENDED or CASCADE */
#define PRV_CPSS_PX_HAL_BPE_TRUNK_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum, trunkId)                                         \
    if (devDbArr[devNum].trunkDbArr[trunkId].trunkType != CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E  &&                                \
        devDbArr[devNum].trunkDbArr[trunkId].trunkType != CPSS_PX_HAL_BPE_TRUNK_CASCADE_E)                                    \
    {                                                                                                                         \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "{devNum[%d],trunkId[%d]} type is [%d]",(devNum),(trunkId),(devDbArr[devNum].trunkDbArr[trunkId].trunkType));      \
    }

/* check that the trunk is in type */
#define PRV_CPSS_PX_HAL_BPE_TRUNK_TYPE_CHECK_MAC(devNum, trunkId, type_)                                                     \
    if (devDbArr[devNum].trunkDbArr[trunkId].trunkType != type_)                                                             \
    {                                                                                                                        \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "{devNum[%d],trunkId[%d]} type is [%d]",(devNum),(trunkId),(devDbArr[devNum].trunkDbArr[trunkId].trunkType));      \
    }

/* check if cid is in valid range (0<=param<4K-1)*/
#define PRV_CPSS_PX_HAL_BPE_UNICAST_CID_CHECK_RANGE_MAC(param)                                                              \
    if (param >= 0x1000)                                                                                                    \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cid must be in range [0..0x1000]");                                     \
    }


/* check if cid is in valid range (0<=param<8K-1)*/
#define PRV_CPSS_PX_HAL_BPE_MULTICAST_CID_CHECK_RANGE_MAC(param)                                                            \
    if (param >= 0x2000)                                                                                                    \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cid must be in range [0..0x2000]");                                     \
    }

/* check if trunk is in valid range (0<param<max)*/
#define PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_RANGE_MAC(param)                                                                    \
    if ((param == 0) || (param >= PRV_CPSS_PX_TRUNKS_NUM_CNS))                                                              \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "trunkId must be in range [1..0x%x]",                                    \
        (GT_U32)(PRV_CPSS_PX_TRUNKS_NUM_CNS-1));                                                                            \
    }

/* check if maxBpeUpstreamPorts is in valid range (1<=param<=4)*/
#define PRV_CPSS_PX_HAL_BPE_MAX_UPSTREAM_PORTS_CHECK_RANGE_MAC(param)                                                       \
    if ((param == 0) || (param >= 5))                                                                                       \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "maxBpeUpstreamPorts must be in range [1..4]");                          \
    }

/* check if maxBpeCascadePorts is in valid range (0<=param<=12)*/
#define PRV_CPSS_PX_HAL_BPE_MAX_CASCADE_PORTS_CHECK_RANGE_MAC(param)                                                        \
    if (param >= 13)                                                                                                        \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "maxBpeCascadePorts must be in range [0..12]");                          \
    }

/* check that the PHYSICAL port number hold valid number */
#define PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(portNum)                                                                     \
    if (portNum >= PRV_CPSS_PX_PORTS_NUM_CNS)                                                                               \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "portNum must be in range [0..16]");                                     \
    }

/* check that the port bitmap is valid */
#define PRV_CPSS_PX_HAL_PORT_BMP_CHECK_MAC(portsBmp)                                                                        \
    if (portsBmp >= BIT_17)                                                                                                 \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Port bitmap not valid [0..%x]",                                         \
            (BIT_17-1));                                                                                                    \
    }

/* check that the cnc bitmap is valid */
#define PRV_CPSS_PX_HAL_CNC_BMP_CHECK_MAC(cncBmp)                                                                           \
    if (cncBmp >= BIT_2)                                                                                                    \
    {                                                                                                                       \
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "CNC bitmap not valid [0..%x]",                                          \
            (BIT_2-1));                                                                                                     \
    }

/* macro to set a port in the bmp of ports */
#define PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum)    \
    portsBmp |= 1 << ((portNum)& 0x1ffff)

/* macro to clear a port in the bmp of ports */
#define PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,portNum)   \
    portsBmp &= ~(1 << ((portNum)& 0x1ffff))

/* macro to check if port is set in the bmp of ports (is corresponding bit is 1) */
#define PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(portsBmp,portNum)   \
    ((portsBmp & (1 << ((portNum)& 0x1ffff))) ? 1 : 0)

static char* srcPortProfileStr(PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_ENT profile);
static char* udbAnchorTypeStr(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT  udbAnchorType);
static char* packetTypeStr(PRV_CPSS_PX_HAL_BPE_PKT_TYPE_ENT  packetType);
static char* headerAlterationTypeStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  type);
static char* sourcePortTypeStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  type);
static char* targetPortTypeStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  type);
static char* cosModeStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  cosMode);
static char* dpLevelStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  dpLevel);
static char* ingressHashTypeStr(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT  ingressHash);
static char* trunkTypeStr(CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT  trunkType);
static char* portTypeStr(CPSS_PX_HAL_BPE_PORT_MODE_ENT  portType);
static char* packetTypeStrArr[CPSS_PX_PACKET_TYPE_MAX_CNS+1];
static char* reservedDestinationStr(GT_U32);
static GT_STATUS ipv4Mask2PrefixLen(GT_IPADDR  ipv4Mask, GT_U32 *prefixLenPtr);
static GT_STATUS ipv6Mask2PrefixLen(GT_IPV6ADDR  ipv6Mask, GT_U32 *prefixLenPtr);


/**
* @internal prvCpssPxHalBpeLogError function
* @endinternal
 *
* @param[in] functionName             - name of function that generates error.
* @param[in] line                     - number in file, may be excluded from log by global configuration.
* @param[in] rc                       - the return code of the function.
* @param[in] formatPtr                - formatted message. 
* @param[in] ...                      - optional parameters according to formatPtr  
*/
static GT_VOID prvCpssPxHalBpeLogError
(
    IN const char *functionName,
    IN GT_U32 line,
    IN GT_STATUS rc,
    IN const char * formatPtr,
    ...
)
{
    va_list args;

    /* Need separate buffer for info message to prevent override data by recursive call using global buffer */
    static char buffer[2048];

    va_start(args, formatPtr);
    cpssOsVsprintf(buffer, formatPtr, args);
    va_end(args);

    /* Call common error log function */
    cpssOsPrintf("[%s:%d] rc = %d : %s\n", functionName, line, rc, buffer);
}

/**
* @internal prvCpssPxHalBpePortDbSet function
* @endinternal
*
* @brief   Set BPE port attributes into DB.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] portNum                  -       port number
* @param[in] mode                     -       port mode
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssPxHalBpePortDbSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PORT_NUM                     portNum,
    IN  CPSS_PX_HAL_BPE_PORT_MODE_ENT   mode
)
{
    cpssOsMemSet(&devDbArr[devNum].portDbArr[portNum],0,sizeof(CPSS_PX_HAL_BPE_PORT_INFO_STC));

    devDbArr[devNum].portDbArr[portNum].interfce.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
    devDbArr[devNum].portDbArr[portNum].interfce.devPort.portNum = portNum;
    devDbArr[devNum].portDbArr[portNum].interfce.devPort.devNum = devNum;
    devDbArr[devNum].portDbArr[portNum].mode = mode;
    devDbArr[devNum].portDbArr[portNum].upstream.type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;

    return GT_OK;
}

/**
* @internal prvCpssPxHalBpePortDbIdleModeSet function
* @endinternal
*
* @brief   Set BPE port mode IDLE.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] portNum                  -       port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxHalBpePortDbIdleModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PORT_NUM                     portNum
)
{
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC portKey;

    /* Set in DB port mode is IDLE */
    prvCpssPxHalBpePortDbSet(devNum,portNum,CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E);
    devDbArr[devNum].portDbArr[portNum].upstream.type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;
    devDbArr[devNum].portDbArr[portNum].pcid = 0;

    /* Set src port profile as IDLE */
    cpssOsMemSet(&portKey,0,sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    portKey.srcPortProfile = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_IDLE_E;
    return cpssPxIngressPortPacketTypeKeySet(devNum,portNum,&portKey);
}


/**
* @internal prvCpssPxHalBpeLagDesignatedPortEntrySet function
* @endinternal
*
* @brief   Enable/Disable port in LAG Designated port map table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] portNum                  -       port number
* @param[in] enable                   -       GT_TRUE:   port
*                                      GT_FALSE: disable port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxHalBpeLagDesignatedPortEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PORT_NUM                     portNum,
    IN  GT_BOOL                         enable
)
{
#if 0
    GT_STATUS   rc;
    CPSS_PX_PORTS_BMP portsBmp;
    GT_U32  i;


    /* Set for all entries in LAG Designated port map table for portNum */
    for (i=0; i<=PRV_CPSS_PX_LAG_DESIGNATED_TABLE_MAX_CNS; i++) {

        rc = cpssPxIngressHashDesignatedPortsEntryGet(devNum,i,&portsBmp);
        if(GT_OK != rc)
        {
            return rc;
        }

        if (enable)
        {
            /* Set appropriate bit for portNum */
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum);
        }
        else
        {
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,portNum);
        }

        /* Configure LAG Designated port map table entries for the appropriate bit */
        rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,i,portsBmp);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
#endif
    return GT_OK;
}


/**
* @internal prvCpssPxHalBpePortModeSet function
* @endinternal
*
* @brief   Set BPE port from IDLE to mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] portNum                  -       port number
* @param[in] mode                     -       port mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxHalBpePortModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PORT_NUM                     portNum,
    IN  CPSS_PX_HAL_BPE_PORT_MODE_ENT   mode
)
{
    GT_STATUS   rc;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC portKey;
    CPSS_PX_PORTS_BMP portsBmp;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT sourcePortInfo;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT sourceInfoType;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT destPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT destInfoType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT operationInfo;
    CPSS_PX_PACKET_TYPE packetType;
    GT_BOOL found = GT_FALSE;
    GT_U32 i;
    GT_BOOL bypassLagDesignatedBitmap;

    cpssOsMemSet(&operationInfo,0,sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));

    switch (mode) {
        /* IDLE -> UPSTREAM: all packets without E-tag are trapped to CPU ; all packets with E-tag are forward.
           Traffic without E-tag will have a match as packet type Upstream-to-CPU and dst format table will forward to CPU.
           Traffic with E-tag will have a match as packet type Upstream-to-Downstream-UC or Upstream-to-Downstream-MC; packet will be forward
        */
        case CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E:

            devDbArr[devNum].numOfUpstreamPorts++;

            /* Set in DB port mode is UPSTREAM */
            prvCpssPxHalBpePortDbSet(devNum,portNum,mode);

            /* Set src port profile as UPSTREAM */
            cpssOsMemSet(&portKey,0,sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

            portKey.srcPortProfile = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_UPSTREAM_E;
            portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
            portKey.portUdbPairArr[0].udbByteOffset = 16; /* take 8 bits; the mask is done in Packet Type Key table for bits 4,5 */
            rc = cpssPxIngressPortPacketTypeKeySet(devNum,portNum,&portKey);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* if exist TRUNK type Upstream --> don't add the new Upstream port to Dst port map table;
               all the traffic to Upstream ports should be through the designated port from the TRUNK */
            for (i=0; i<PRV_CPSS_PX_TRUNKS_NUM_CNS; i++)
            {
                if (devDbArr[devNum].trunkDbArr[i].trunkType == CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E && devDbArr[devNum].trunkDbArr[i].valid == GT_TRUE)
                {
                    found = GT_TRUE;
                    break;
                }
            }
            if (found == GT_FALSE)
            {
                rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,&portsBmp,
                                                  &bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Set appropriate bit for portNum */
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum);

                /* Configure Dst port map table entry (UPSTREAM TRAFFIC) for the appropriate bit */
                rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,portsBmp,bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Set for all entries in LAG Designated port map table '1' for portNum */
                rc = prvCpssPxHalBpeLagDesignatedPortEntrySet(devNum,portNum,GT_TRUE);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }

            sourcePortInfo.info_802_1br.srcPortInfo.srcFilteringVector =1 << portNum;
            /* Update Target Port Table Metadata */
            rc = cpssPxEgressTargetPortEntryGet(devNum, portNum, &destInfoType, &destPortInfo);
            if(GT_OK != rc)
            {
                return rc;
            }

            destPortInfo.info_common.pcid = portNum;
            destPortInfo.info_common.tpid = 0x893F;
            rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, destInfoType, &destPortInfo);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
            for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

                switch (packetType)
                {
                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E:

                        rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                        break;

                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E:

                        rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                        break;

                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E:
                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:
                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:

                        rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                        break;

                    default:
                        /* do nothing */
                        break;
                }
            }

            break;

        /* IDLE -> TRUSTED: all packets without E-tag are trapped to CPU; all packets with E-tag are dropped.
           Traffic without E-tag will have a match as packet type Upstream-to-CPU and dst format table will forward to CPU.
           Traffic with E-tag will have a match as packet type Upstream-to-Downstream-UC or Upstream-to-Downstream-MC;
           No bit is set for entry in dst format table and the packet will be dropped
        */
        case CPSS_PX_HAL_BPE_PORT_MODE_TRUSTED_E:

            /* Set in DB port mode is TRUSTED */
            prvCpssPxHalBpePortDbSet(devNum,portNum,mode);

            /* Set src port profile as UPSTREAM */
            cpssOsMemSet(&portKey,0,sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

            portKey.srcPortProfile = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_UPSTREAM_E;
            portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
            portKey.portUdbPairArr[0].udbByteOffset = 16; /* take 8 bits; the mask is done in Packet Type Ket table for bits 4,5 */
            rc = cpssPxIngressPortPacketTypeKeySet(devNum,portNum,&portKey);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Update Source Port Table Metadata */
            rc = cpssPxEgressSourcePortEntryGet(devNum, portNum, &sourceInfoType, &sourcePortInfo);
            if(GT_OK != rc)
            {
                return rc;
            }

            sourcePortInfo.info_802_1br.pcid = portNum;
            sourcePortInfo.info_802_1br.srcPortInfo.srcFilteringVector = 1 << portNum;

            rc = cpssPxEgressSourcePortEntrySet(devNum, portNum, sourceInfoType, &sourcePortInfo);
            if(GT_OK != rc)
            {
                return rc;
            }

           /* Update Target Port Table Metadata */
            rc = cpssPxEgressTargetPortEntryGet(devNum, portNum, &destInfoType, &destPortInfo);
            if(GT_OK != rc)
            {
                return rc;
            }

            destPortInfo.info_common.pcid = portNum;
            destPortInfo.info_common.tpid = 0x893F;
            rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, destInfoType, &destPortInfo);
            if(GT_OK != rc)
            {
                return rc;
            }

            rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;

        /* IDLE -> CASCADE: all packets with E-tag are forward; all packets without E-tag, E-tag is added then packet is forward.
           Traffic with E-tag will have a match as packet type Cascade-to-Upstream-E-Tagged; packet will be forward
           Traffic without E-tag will have a match as packet type Cascade-to-Upstream-Untagged; packet will be forward
        */
        case CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E:

            /* Set in DB port mode is CASCADE */
            prvCpssPxHalBpePortDbSet(devNum,portNum,mode);

            /* Set src port profile as CASCADE */
            cpssOsMemSet(&portKey,0,sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

            portKey.srcPortProfile = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CASCADE_E;
            rc = cpssPxIngressPortPacketTypeKeySet(devNum,portNum,&portKey);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Set for all entries in LAG Designated port map table '1' for portNum */
            rc = prvCpssPxHalBpeLagDesignatedPortEntrySet(devNum,portNum,GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
            for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

                switch (packetType)
                {
                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:

                        rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E,&operationInfo);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                        break;

                    default:
                        /* do nothing */
                        break;
                }
            }

            break;

        /* IDLE -> EXTENDED: all packets with E-tag are dropped; all packets without E-tag are forward.
           Traffic with E-tag will have a match as packet type Extended-to-Upstream-E-Tagged and dst format table will drop the packet
           Traffic with C-tag will have a match as packet type Extended-to-Upstream-C-Tagged; packet will be forward
           Traffic untagged will have a match as packet type Extended-to-Upstream-Untagged; packet will be forward
        */
        case CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E:

            /* Set in DB port mode is EXTENDED */
            prvCpssPxHalBpePortDbSet(devNum,portNum,mode);

            /* Set src port profile as EXTENDED */
            cpssOsMemSet(&portKey,0,sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

            portKey.srcPortProfile = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_EXTENDED_E;
            portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
            portKey.portUdbPairArr[0].udbByteOffset = 2; /* IP version and DSCP fields. the mask is done in Packet Type Key table */
            portKey.portUdbPairArr[1].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
            portKey.portUdbPairArr[1].udbByteOffset = 8; /* IPv6 next header (protocol). the mask is done in Packet Type Key table */
            portKey.portUdbPairArr[2].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
            portKey.portUdbPairArr[2].udbByteOffset = 10; /* IPv4 protocol. the mask is done in Packet Type Key table */
            portKey.portUdbPairArr[3].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
            portKey.portUdbPairArr[3].udbByteOffset = 0; /* L3 EtherType (for packet EthV2)/ LLC-SNAP PID (for packet LLC-SNAP) /
                                                            SSAP-DSAP (for packet LLC-NON-SNAP).
                                                            the mask is done in Packet Type Key table */            

            rc = cpssPxIngressPortPacketTypeKeySet(devNum,portNum,&portKey);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Set for all entries in LAG Designated port map table '1' for portNum */
            rc = prvCpssPxHalBpeLagDesignatedPortEntrySet(devNum,portNum,GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Update Target Port Table Metadata */
            rc = cpssPxEgressTargetPortEntryGet(devNum, portNum, &destInfoType, &destPortInfo);
            if(GT_OK != rc)
            {
                return rc;
            }

            destPortInfo.info_common.tpid = 0x8100;
            rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, destInfoType, &destPortInfo);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
            for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

                switch (packetType)
                {

                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
                    case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:

                        rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,&operationInfo);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                        break;

                    default:
                        /* do nothing */
                        break;
                }
            }
            break;

        case CPSS_PX_HAL_BPE_PORT_MODE_INTERNAL_E:

            /* TBD - will be implemented in phase 2 */
            BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(mode));
            break;

        case CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E:

            /* do nothing */
            break;

        default:
            BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(mode));
            break;

    }

    return GT_OK;
}

/*******************************************************************************
* prvCpssPxHalBpeGetChannel
*
* DESCRIPTION:
*       Set BPE port from IDLE to mode.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       devPtr   - (pointer to) device number
*       cid      - E-channel Id (APPLICABLE RANGE: 0..0..maxBpeUnicastEChannels + maxBpeUnicastEChannels+maxBpeMulticastEChannels-1)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       A pointer to channel if succeeded, NULL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static CPSS_PX_HAL_BPE_CHANNEL_INFO_STC* prvCpssPxHalBpeGetChannel
(
    IN CPSS_PX_HAL_BPE_DEV_INFO_STC *devPtr,
    IN GT_U32 cid
)
{
        CPSS_PX_HAL_BPE_CHANNEL_INFO_STC        *chPtr;
        for (chPtr = devPtr->channelDbPtr; chPtr; chPtr = chPtr->nextChannelPtr)
        {
            if (chPtr->cid == cid)
            {
                    return chPtr;
            }
        }
        return NULL;
}

/**
* @internal prvCpssPxHalBpePortExtendedToUpstreamIsValid function
* @endinternal
*
* @brief   Check Extended port Unicast channel configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devPtr                   - (pointer to) device number
* @param[in] portPtr                  - (pointer to) port number
* @param[in] cid                      - E-channel Id (APPLICABLE RANGE: 0..maxBpeUnicastEChannels-1)
*
* @retval GT_TRUE                  - Extended port E-channel configuration is valid
* @retval GT_FALSE                 - Extended port E-channel configuration is not valid
*/
static GT_BOOL prvCpssPxHalBpePortExtendedToUpstreamIsValid
(
    IN  CPSS_PX_HAL_BPE_DEV_INFO_STC    *devPtr,
    IN  CPSS_PX_HAL_BPE_PORT_INFO_STC   *portPtr,
    IN  GT_U32                          cid
)
{
    /* Extended port Unicast channel configuration depends on all the following conditions:
        - Extended port have PCID configured
        - Channel created with CID equal to PCID and channel contains the extended port
        - Extended port have a defined upstream.
    */
    if ( portPtr->interfce.type != CPSS_PX_HAL_BPE_INTERFACE_PORT_E)
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(portPtr->interfce.type));
        return GT_FALSE;
    }

    if (portPtr->mode == CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E &&
        portPtr->pcid == cid)
    {
        return GT_TRUE;
    }
    return GT_FALSE;
}


/**
* @internal prvCpssPxHalBpePortCascadeToUpstreamIsValid function
* @endinternal
*
* @brief   Check Cascade port Unicast channel configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devPtr                   - (pointer to) device number
* @param[in] portPtr                  - (pointer to) port number
*                                      cid      - E-channel Id (APPLICABLE RANGE: 0..maxBpeUnicastEChannels-1)
*
* @retval GT_TRUE                  - Cascade port E-channel configuration is valid
* @retval GT_FALSE                 - Cascade port E-channel configuration is not valid
*/
static GT_BOOL prvCpssPxHalBpePortCascadeToUpstreamIsValid
(
    IN  CPSS_PX_HAL_BPE_DEV_INFO_STC    *devPtr,
    IN  CPSS_PX_HAL_BPE_PORT_INFO_STC   *portPtr
)
{
    /* Cascade port Unicast channel configuration depends on all the following conditions:
        - Cascade port have PCID configured
        - Channel contains the cascade port
        - Cascade port have a defined upstream.
    */
    if ( portPtr->interfce.type != CPSS_PX_HAL_BPE_INTERFACE_PORT_E)
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(portPtr->interfce.type));
        return GT_FALSE;
    }

    if (portPtr->mode == CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E)
    {
        return GT_TRUE;
    }
    return GT_FALSE;
}

/**
* @internal prvCpssPxHalBpePortCheckChannel function
* @endinternal
*
* @brief   Check no E-Channels are attached to this port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] portNum                  -       port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxHalBpePortCheckChannel
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PORT_NUM                     portNum
)
{
    GT_STATUS   rc;
    CPSS_PX_PORTS_BMP portsBmp;
    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC *chPtr;
    GT_U32 i;
    GT_BOOL bypassLagDesignatedBitmap;

    /* Check no E-Channels are attached to this port */
    chPtr = prvCpssPxHalBpeGetChannel(&devDbArr[devNum],devDbArr[devNum].portDbArr[portNum].pcid);
    if (chPtr) {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "{Exist Unicast E-Channel connected to this devNum[%d],portNum[%d]}",(devNum),(portNum));
    }
    for (i=devDbArr[devNum].maxBpeUnicastEChannels; i<devDbArr[devNum].maxBpeUnicastEChannels+devDbArr[devNum].maxBpeMulticastEChannels; i++)
    {
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,i+PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS,&portsBmp,
                                          &bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }

        if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(portsBmp, portNum))
        {
            BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "{Exist Multicast E-Channel connected to this devNum[%d],portNum[%d]}",(devNum),(portNum));
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxHalBpeCncConfig function
* @endinternal
*
* @brief   Configure CNC for all clients.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
*/
GT_STATUS prvCpssPxHalBpeCncConfig
(
    IN  GT_SW_DEV_NUM   devNum
)
{

    GT_STATUS   rc;

    /* Configure CNC block 0 */
    if (PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].cncBitmap, 0) == GT_TRUE) {
        rc = cpssPxHalBpeCncConfigClient(devNum, devDbArr[devNum].cncClientConfig[0], 0);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Configure CNC block 1 */
    if (PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].cncBitmap, 1) == GT_TRUE) {
        rc = cpssPxHalBpeCncConfigClient(devNum, devDbArr[devNum].cncClientConfig[1], 1);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxHalBpePortExtendedUntaggedVlanAdd function
* @endinternal
*
* @brief   Configure the Extended interface to egress packets untagged
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] portNum                  -   port number
* @param[in] vid                      -   vlanId to egress packet untagged
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHalBpePortExtendedUntaggedVlanAdd
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_U32                              vid
)
{

    CPSS_PX_PORTS_BMP   portsBmp;    
    GT_STATUS           rc;
    
    /* Get VLAN tag state entry for target ports */ 
    rc = cpssPxEgressVlanTagStateEntryGet(devNum,vid,&portsBmp);  
    if(GT_OK != rc)
    {
        return rc;
    }     

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,portNum);  
     
    /* Set VLAN tag state entry for target ports */ 
    rc = cpssPxEgressVlanTagStateEntrySet(devNum,vid,portsBmp);        

    return rc;
}


/**
* @internal prvCpssPxHalBpePortExtendedUntaggedVlanDel function
* @endinternal
*
* @brief   Configure the Extended interface to egress packets tagged
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] portNum                  -   port number
* @param[in] vid                      -   vlanId to egress packet tagged
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHalBpePortExtendedUntaggedVlanDel
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_U32                              vid
)
{
    CPSS_PX_PORTS_BMP   portsBmp;    
    GT_STATUS           rc;
    
    /* Get VLAN tag state entry for target ports */ 
    rc = cpssPxEgressVlanTagStateEntryGet(devNum,vid,&portsBmp);  
    if(GT_OK != rc)
    {
        return rc;
    }    

    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum);  
     
    /* Set VLAN tag state entry for target ports */ 
    rc = cpssPxEgressVlanTagStateEntrySet(devNum,vid,portsBmp);        

    return rc;
}


/**
* @internal prvCpssPxHalBpeCncDumpClient function
* @endinternal
*
* @brief   Display CNC information for client.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] client                   -   CNC client
* @param[in] blockNum                 -   CNC block number
* @param[in] displayAll               -   GT_TRUE:  display all entries
*                                      GT_FALSE: display only valid entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
*/
static GT_STATUS prvCpssPxHalBpeCncDumpClient
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_CNC_CLIENT_ENT      client,
    IN  GT_U32                      blockNum,
    IN  GT_BOOL                     displayAll
)
{
    GT_STATUS   rc;
    GT_U32      ii, cncCounter, port, tc, drop;
    CPSS_PX_CNC_COUNTER_STC counter;
    CPSS_PX_TABLE_ENT table;

    table = (blockNum == 0) ? CPSS_PX_TABLE_CNC_0_COUNTERS_E : CPSS_PX_TABLE_CNC_1_COUNTERS_E;

    rc = cpssPxCfgTableNumEntriesGet(devNum,table,&cncCounter);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* print counters */
    if (client == CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E)
    {
        cpssOsPrintf("\n**************************************************\n");
        cpssOsPrintf("             Destination Portmap Table dump\n");
        cpssOsPrintf("**************************************************\n\n");
        cpssOsPrintf("NOTE: counter index equals the Dst_Idx\n");
        cpssOsPrintf("\n index      packetCount.l[0]    packetCount.l[1]    byteCount[0]    byteCount[1]");
        cpssOsPrintf("\n--------------------------------------------------------------------------------");


        for(ii = 0; ii < cncCounter; ii++)
        {

            cpssPxCncCounterGet(devNum,blockNum,ii,CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &counter);

            if (displayAll || counter.packetCount.l[0] > 0 || counter.packetCount.l[1] > 0)
            {
                cpssOsPrintf("\n %-10d\t0x%-14x\t0x%-10x\t0x%-10x\t0x%-10x"
                     ,ii,counter.packetCount.l[0],counter.packetCount.l[1],counter.byteCount.l[0],counter.byteCount.l[1]);
            }

        }

        cpssOsPrintf("\n\n");
    }

    if (client == CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E)
    {
        cpssOsPrintf("\n**************************************************\n");
        cpssOsPrintf("             Egress Queue Tail-Drop dump\n");
        cpssOsPrintf("**************************************************\n\n");
        cpssOsPrintf("NOTE: egress queue index: TXQ Port[4:0] TC[2:0] Pass/Drop[0]\n");
        cpssOsPrintf("\n index  port  TC  drop   packetCount.l[0]    packetCount.l[1]    byteCount[0]    byteCount[1]");
        cpssOsPrintf("\n--------------------------------------------------------------------------------");


            for(ii = 0; ii < cncCounter; ii++)
            {
	            cpssPxCncCounterGet(devNum,blockNum,ii,CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &counter);

	            if (displayAll || counter.packetCount.l[0] > 0 || counter.packetCount.l[1] > 0)
                {
	                port = (ii >> 4) & 0x1F;
	                tc = (ii >> 1) & 0x7;
	                drop = ii & 0x1;
	                cpssOsPrintf("\n %-4d   %-2d    %-2d  %-2d\t0x%-14x\t0x%-10x\t0x%-10x\t0x%-10x"
	                         ,ii, port, tc, drop, counter.packetCount.l[0],counter.packetCount.l[1],counter.byteCount.l[0],counter.byteCount.l[1]);
                }

            }


        cpssOsPrintf("\n\n");
    }

    return GT_OK;
}


/**
* @internal prvCpssPxHalBpeMulticastEChannelCreate function
* @endinternal
*
* @brief   Create Multicast E-Channel and add interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] cid                      -   Multicast E-Channel Id (APPLICABLE RANGE: maxBpeUnicastEChannels..maxBpeUnicastEChannels+maxBpeMulticastEChannels-1)
* @param[in] interfaceListPtr         -   (pointer to) interface list
* @param[in] isNew                    -   whehter this is a new E-Channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note cid of MC: <GRP[1:0],CID[11:0]>; GRP>0
*
*/
static GT_STATUS prvCpssPxHalBpeMulticastEChannelCreate
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              cid,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfaceListPtr,
    IN  GT_BOOL                             isNew
)
{
    GT_STATUS rc;

    rc = cpssPxHalBpeMulticastEChannelCreate(devNum,cid,interfaceListPtr);
    if (isNew)
    {
        devDbArr[devNum].numBpeMulticastEChannel++;
    }

    return rc;
}

/**
* @internal prvCpssPxHalBpeUpstreamQosMapSet function
* @endinternal
 *
* @brief   Set CoS mapping between E-PCP to QUEUE for Upstream port.
 *
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
 *
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -   device number 
* @param[in] portNum                  -   port number 
* @param[in] pcp                      -   class of service (APPLICABLE RANGE: 0..7).
* @param[in] queue                    -   egress  (APPLICABLE RANGE: 0..7).
*/
static GT_STATUS prvCpssPxHalBpeUpstreamQosMapSet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_PORT_NUM         portNum,    
    IN  GT_U32              pcp,
    IN  GT_U32              queue
)
{
    GT_STATUS   rc;
    GT_U32      l2Index, dei;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes;   

    for(dei=0; dei < CPSS_DEI_RANGE_CNS; dei++) {
        l2Index = (pcp << 1) | dei;    
        cosAttributes.userPriority = pcp;
        cosAttributes.dropEligibilityIndication = dei;
        cosAttributes.trafficClass = queue;
        cosAttributes.dropPrecedence = (cosAttributes.dropEligibilityIndication == 0) ? CPSS_DP_GREEN_E : CPSS_DP_YELLOW_E;
        /* Set CoS L2 Attributes mapping per Port */
        rc = cpssPxCosPortL2MappingSet(devNum, portNum, l2Index, &cosAttributes);
        if(GT_OK != rc)
        {
            return rc;
        } 
    }
    
    return GT_OK;
}

/**
* @internal cpssPxHalBpeInit function
* @endinternal
*
* @brief   Initialize the BPE package, called only after CPSS initialize is completed.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeInit
(
    IN  GT_SW_DEV_NUM                    devNum
)
{
    GT_STATUS                                       rc;
    GT_U32                                          portNum;
    CPSS_PX_PACKET_TYPE                             packetType;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC      keyData,maskData;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;
    CPSS_PX_PORTS_BMP                               portsBmp=0;
    CPSS_PX_INGRESS_TPID_ENTRY_STC                  tpidEntry;
    CPSS_PX_COS_FORMAT_ENTRY_STC                    cosFormatEntry;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT            sourcePortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT            destPortInfo;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC        portKey;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT      operationInfo;
    GT_U32                                          entryIndex, l2Index, pcp, dei;
    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC                *chPtr;
    CPSS_PX_COS_ATTRIBUTES_STC                      cosAttributes;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC                 ip2meEntry;

    /* Free previously allocated memory */
    if (devDbArr[devNum].valid == GT_TRUE)
    {
        while (devDbArr[devNum].channelDbPtr != NULL)
        {
            chPtr = devDbArr[devNum].channelDbPtr->nextChannelPtr;
            cpssOsFree(devDbArr[devNum].channelDbPtr);
            devDbArr[devNum].channelDbPtr = chPtr;
        }
    }

    /* update DB */
    cpssOsMemSet(&(devDbArr[devNum]), 0, sizeof(CPSS_PX_HAL_BPE_DEV_INFO_STC));
    devDbArr[devNum].valid = GT_TRUE;

    for (portNum=0; portNum<PRV_CPSS_PX_PORTS_NUM_CNS; portNum++) {
        devDbArr[devNum].portDbArr[portNum].mode = CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E;
        devDbArr[devNum].portDbArr[portNum].upstream.type = CPSS_PX_HAL_BPE_INTERFACE_NONE_E;
    }

    /* Update the packet type array. The order in this array defines the search order */
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E] = 0;
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E] = 1;
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_CPU_E] = 2;
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E] = 3;
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E] = 4;
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E] = 5;
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E] = 6;
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_1_E] = 7;
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_2_E] = 8;
    /* entries 9..29 are reserved for the user */
    for (packetType=PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E; packetType<(CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS+PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E); packetType++) {
        CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType] = PRV_CPSS_HAL_BPE_CTRL_DATA_RULE_CONVERT_CNS + (packetType - PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E);
    }
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E] = 30;
    CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E] = 31;

    /* Set default values source port profile Table */
    cpssOsMemSet(&portKey,0,sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));
    for (portNum=0; portNum<PRV_CPSS_PX_PORTS_NUM_CNS; portNum++) {

        rc = cpssPxIngressPortPacketTypeKeySet(devNum, portNum, &portKey);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* set CPU port source port profile */
    portKey.srcPortProfile = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CPU_E;
    rc = cpssPxIngressPortPacketTypeKeySet(devNum,PRV_CPSS_PX_PORTS_NUM_CNS-1,&portKey);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Set default values for src port map table - init state no filtering */
    portsBmp = 0x1FFFF;
    for (entryIndex = 0; entryIndex < _4K; entryIndex++)
    {
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,entryIndex,portsBmp,GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Set default values for dest port map table - init state no destination ports */
    portsBmp = 0;
    for (entryIndex = 0; entryIndex < _8K; entryIndex++)
    {
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,entryIndex,portsBmp,GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Set vlan tag state on all ports to "tagged", CPU port is not supported (ignored) */
    portsBmp = 0x0FFFF;      
    for (entryIndex = 0; entryIndex < _4K ; entryIndex++)
    {
        rc = cpssPxEgressVlanTagStateEntrySet(devNum,entryIndex,portsBmp);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Configure Designated port map table: no trunk members */
    for(entryIndex = 0 ; entryIndex <= PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS ; entryIndex++)
    {
        rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,entryIndex,0x1FFFF);
        if (rc != GT_OK)
            return rc;
    }
    /* LAG table mode = 'two tables' */
    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum)) 
    {
        for(entryIndex = PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS+1 ; entryIndex <= PRV_CPSS_PX_LAG_DESIGNATED_TABLE_MC_MAX_CNS ; entryIndex++)
        {
            rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,entryIndex,0x1FFFF);
            if (rc != GT_OK)
                return rc;
        }
    }

    cpssOsMemSet(&operationInfo,0,sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));

    /* init Header Alteration Table */
    for (portNum=0; portNum<PRV_CPSS_PX_PORTS_NUM_CNS; portNum++) {

        /* entries 9..29 are reserved for the user */
        for (packetType=PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E; packetType<(CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS+PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E); packetType++) {
            rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

            switch (packetType)
            {            
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:

                    if (portNum != PRV_CPSS_PX_PORTS_NUM_CNS-1)
                    {
                        rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,&operationInfo);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    break;

                default:
                    /* do nothing */
                    break;
            }
        }
    }
    
    
    /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,PRV_CPSS_PX_PORTS_NUM_CNS-1,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_CPU_E],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E,&operationInfo);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Calculate hash as CRC32 for all packet types */
    for (packetType=0; packetType<CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E; packetType++) {
        rc = cpssPxIngressHashPacketTypeHashModeSet(devNum,packetType,CPSS_PX_INGRESS_HASH_MODE_CRC32_E,NULL);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Seed value '0' used in the initialization of the CRC32 function */
    rc = cpssPxIngressHashSeedSet(devNum,0);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Set up LAG designated port table hash indexing mode for all ports */
    for (portNum=0; portNum<PRV_CPSS_PX_PORTS_NUM_CNS; portNum++) {
        rc = cpssPxIngressHashPortIndexModeSet(devNum,portNum,CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum)) 
    {
         /* Set global LAG Designated Port Table mode = two tables */
        rc = cpssPxIngressHashLagTableModeSet(devNum,CPSS_PX_INGRESS_HASH_LAG_TABLE_DUAL_MODE_E);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Set LAG table number for specific packet type */
        for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {
            switch (packetType) {
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:
                    rc = cpssPxIngressHashPacketTypeLagTableNumberSet(devNum,packetType,1); /* Lag table for MC */
                    break;
                default:
                    rc = cpssPxIngressHashPacketTypeLagTableNumberSet(devNum,packetType,0); /* Lag table for UC */
                    break;
            }
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        /* Set port filtering state disable for ingress */
        for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {
            rc = cpssPxIngressFilteringEnableSet(devNum,CPSS_DIRECTION_BOTH_E,packetType,GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }            
        }     
        
        /* Set port filtering state enable for egress MC */        
        rc = cpssPxIngressFilteringEnableSet(devNum,CPSS_DIRECTION_EGRESS_E,PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E,GT_TRUE);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsMemSet(&ip2meEntry,0,sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC)); 
        ip2meEntry.prefixLength = 1;         
        ip2meEntry.valid = GT_FALSE;
              
        /* Set IP2ME table entries to default */
        for (entryIndex=1; entryIndex<=7; entryIndex++)
        {            
            rc = cpssPxIngressIp2MeEntrySet(devNum,entryIndex,&ip2meEntry);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    /* Set packet type key table */
    for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

        cpssOsMemSet(&keyData,0,sizeof(keyData));
        cpssOsMemSet(&maskData,0,sizeof(maskData));
        maskData.profileIndex = 0x7F;
        maskData.etherType = 0xFFFF;

        switch (packetType) {
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
                keyData.etherType = 0x893F; /* E-Tag TPID */
                keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_UPSTREAM_E;
                keyData.udbPairsArr[0].udb[0] = 0; /* GRP = 0 */
                keyData.udbPairsArr[0].udb[1] = 0;
                maskData.udbPairsArr[0].udb[0] = 0x30; /* take only bits 4,5; L2 anchor, 16 bytes offset, [RE(2 bits) | GRP(2 bits) | PCID (bits 11.8)] */
                maskData.udbPairsArr[0].udb[1] = 0;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:
                keyData.etherType = 0x893F; /* E-Tag TPID */
                keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_UPSTREAM_E;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_CPU_E:
                keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_UPSTREAM_E;
                maskData.etherType = 0;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E:
                keyData.etherType = 0x893F; /* E-Tag TPID */
                keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CASCADE_E;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:
                keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CASCADE_E;
                maskData.etherType = 0;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E:
                keyData.etherType = 0x893F; /* E-Tag TPID */
                keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_EXTENDED_E;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
                keyData.etherType = 0x8100; /* C-Tag TPID */
                keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_EXTENDED_E;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:
                keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_EXTENDED_E;
                maskData.etherType = 0;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E:
                keyData.etherType = 0x893F; /* E-Tag TPID */
                keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CPU_E;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_1_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_2_E:
                /* do nothing */
                break;

            default:
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
                break;
        }

        rc = cpssPxIngressPacketTypeKeyEntrySet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],&keyData,&maskData);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Enable packet type table lookup for all packet types except reserved */
    for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {
        if ((packetType != PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_1_E) && (packetType != PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_2_E))
        {
            rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    /* Set Packet Type Source Format Table */
    for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

        cpssOsMemSet(&packetTypeFormat,0,sizeof(packetTypeFormat));

        switch (packetType) {

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:
                packetTypeFormat.indexConst = 0;
                packetTypeFormat.indexMax = _4K - 1;
                packetTypeFormat.bitFieldArr[0].byteOffset = 15; /* E-Tag<Ingress E-CID[7:0]> */
                packetTypeFormat.bitFieldArr[0].startBit = 0;
                packetTypeFormat.bitFieldArr[0].numBits = 8;
                packetTypeFormat.bitFieldArr[1].byteOffset = 14; /* E-Tag<Ingress E-CID[11:8]> */
                packetTypeFormat.bitFieldArr[1].startBit = 0;
                packetTypeFormat.bitFieldArr[1].numBits = 4;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_CPU_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_1_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_2_E:
                /* do nothing */
                break;

            default:
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
        }

        rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],&packetTypeFormat);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Set exception Destination port map; drop the packet */
    rc = cpssPxIngressForwardingPortMapExceptionSet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,portsBmp);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Set exception Source port map; drop the packet */
    rc = cpssPxIngressForwardingPortMapExceptionSet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,portsBmp);
    if(GT_OK != rc)
    {
        return rc;
    }

    portsBmp = 0x0;
    /* Set Destination port map for special entries: */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_DISCARDED_TRAFFIC_INDEX_CNS,portsBmp,GT_FALSE);
    if(GT_OK != rc)
    {
        return rc;
    }
    portsBmp = 0x10000; /* bit 16 is CPU port */
    rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_TRAFFIC_2_CPU_INDEX_CNS,portsBmp,GT_TRUE);
    if(GT_OK != rc)
    {
        return rc;
    }
    for (portNum=0; portNum<PRV_CPSS_PX_PORTS_NUM_CNS-1; portNum++) {
        portsBmp = 0;
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum);
        if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
        {
            /* For Upstream port LAG: The PE CPU should be able to direct control packets to specific port in the LAG,
               while other traffic should be load balanced. The LAG Designated Port selection is bypassed for entries range 3..18 traffic from CPU */
            rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+portNum,portsBmp,GT_TRUE);
        }
        else
        {
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+portNum,portsBmp,GT_FALSE);
        }
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Set global TPID table entry */
    tpidEntry.val = 0x893F; /* E-Tag */
    tpidEntry.size = 4; /* TPID size resolution is 2B. */
    tpidEntry.valid = GT_TRUE;
    rc = cpssPxIngressTpidEntrySet(devNum,0,&tpidEntry);
    if(GT_OK != rc)
    {
        return rc;
    }

    tpidEntry.val = 0x8100; /* VLAN-Tag*/
    tpidEntry.size = 2; /* TPID size resolution is 2B. */
    tpidEntry.valid = GT_TRUE;
    rc = cpssPxIngressTpidEntrySet(devNum,1,&tpidEntry);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Set CoS Mode and Attributes per Packet Type */
    for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

        cpssOsMemSet(&cosFormatEntry,0,sizeof(cosFormatEntry));

        switch (packetType) {
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E:
                cosFormatEntry.cosMode = CPSS_PX_COS_MODE_PACKET_L2_E;
                cosFormatEntry.cosByteOffset = 14; /* E-Tag<PCP,DEI> */
                cosFormatEntry.cosNumOfBits = 4;
                cosFormatEntry.cosBitOffset = 4;  
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_CPU_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E:
                cosFormatEntry.cosMode = CPSS_PX_COS_MODE_FORMAT_ENTRY_E;
                cosFormatEntry.cosAttributes.trafficClass = 7;
                cosFormatEntry.cosAttributes.dropPrecedence = CPSS_DP_GREEN_E;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:
                cosFormatEntry.cosMode = CPSS_PX_COS_MODE_PORT_E;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
                cosFormatEntry.cosMode = CPSS_PX_COS_MODE_PACKET_L2_E;
                cosFormatEntry.cosByteOffset = 14; /* C-Tag<PCP,DEI> */
                cosFormatEntry.cosNumOfBits = 4;
                cosFormatEntry.cosBitOffset = 4;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E: /* bad state; packet should have been dropped */
                /* do nothing */
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_1_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_2_E:
                /* do nothing */
                break;

            default:
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
        }

        rc = cpssPxCosFormatEntrySet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],&cosFormatEntry);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    cpssOsMemSet(&sourcePortInfo,0,sizeof(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT));
    cpssOsMemSet(&destPortInfo,0,sizeof(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT));

    /* Set default values for Source/Target Port Table Metadata */
    for (portNum=0; portNum<PRV_CPSS_PX_PORTS_NUM_CNS; portNum++) {

        rc = cpssPxEgressSourcePortEntrySet(devNum, portNum, CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E, &sourcePortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E, &destPortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Update Target Port Table Metadata for CPU port */
    destPortInfo.info_common.tpid = 0x893F;
    rc = cpssPxEgressTargetPortEntrySet(devNum, PRV_CPSS_PX_PORTS_NUM_CNS-1, CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E, &destPortInfo);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Set default port CoS L2 attributes */
    cpssOsMemSet(&cosAttributes, 0x00, sizeof(CPSS_PX_COS_ATTRIBUTES_STC));
    for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        rc = cpssPxCosPortAttributesSet(devNum, portNum, &cosAttributes);
        if(GT_OK != rc)
        {
            return rc;
        }
        for(pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
        {
            for(dei = 0; dei < CPSS_DEI_RANGE_CNS; dei++)
            {
                l2Index = (pcp << 1) | dei;
                rc = cpssPxCosPortL2MappingSet(devNum, portNum, l2Index, &cosAttributes);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;

}

/**
* @internal cpssPxHalBpeNumOfChannelsGet function
* @endinternal
*
* @brief   Get the number of E-channels supported by the BPE (unicast+multicast channels).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] numOfChannelsPtr         - (pointer to) number of E-channels
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxHalBpeNumOfChannelsGet
(
    IN  GT_SW_DEV_NUM                    devNum,
    OUT GT_U32                           *numOfChannelsPtr
)
{
     /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfChannelsPtr);

    *numOfChannelsPtr = _8K - PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS;

    return GT_OK;
}


/**
* @internal cpssPxHalBpeNumOfChannelsSet function
* @endinternal
*
* @brief   Set the maximum number of unicast E-channels and multicast E-channels.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] numOfUnicastChannels     - number of unicast E-channels (APPLICABLE RANGE: 0..4k-20)
* @param[in] numOfMulticastChannels   - number of multicast E-channels (APPLICABLE RANGE: 0..8K-20)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*
* @note This function must be called after the cpssPxHalBpeInit
*
*/
GT_STATUS cpssPxHalBpeNumOfChannelsSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          numOfUnicastChannels,
    IN  GT_U32                          numOfMulticastChannels
)
{
    CPSS_PX_PACKET_TYPE packetType;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;
    GT_STATUS rc;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(numOfUnicastChannels,_4K - PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS + 1);
    CPSS_PARAM_CHECK_MAX_MAC(numOfMulticastChannels,_8K - PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS + 1);
    CPSS_PARAM_CHECK_MAX_MAC(numOfUnicastChannels + numOfMulticastChannels, _8K - PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS + 1);

    if (devDbArr[devNum].numBpeUnicastEChannel > numOfUnicastChannels || devDbArr[devNum].numBpeMulticastEChannel > numOfMulticastChannels)
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "numOfUnicastChannels must be >= [%d]. numOfMulticastChannels must be >= [%d]",
                                     devDbArr[devNum].numBpeUnicastEChannel, devDbArr[devNum].numBpeMulticastEChannel);
    }

    devDbArr[devNum].maxBpeMulticastEChannels = numOfMulticastChannels;
    devDbArr[devNum].maxBpeUnicastEChannels = numOfUnicastChannels;

    /* Update offsets of Dst_Idx into destination portmap table */
    devDbArr[devNum].ucDstIndexCns = PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS;
    /* negative constant may be required for IEEE 802.1BR multicast, where the range of
       <GRP, E-CID> is <4096, 16383>, in order to locate the multicast entries in the Dst_PortMap
       Table at an index less than 4K */
    devDbArr[devNum].mcDstIndexCns = numOfUnicastChannels + PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS -_4K;

    /* Set Packet Type Destination Format Table */
    for (packetType=0; packetType<CPSS_PX_PACKET_TYPE_MAX_CNS; packetType++) {

        cpssOsMemSet(&packetTypeFormat,0,sizeof(packetTypeFormat));

        switch (packetType) {
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
                packetTypeFormat.indexConst = devDbArr[devNum].ucDstIndexCns;
                packetTypeFormat.indexMax = devDbArr[devNum].ucDstIndexCns + numOfUnicastChannels - 1;
                packetTypeFormat.bitFieldArr[0].byteOffset = 17;
                packetTypeFormat.bitFieldArr[0].startBit = 0;
                packetTypeFormat.bitFieldArr[0].numBits = 8;
                packetTypeFormat.bitFieldArr[1].byteOffset = 16;
                packetTypeFormat.bitFieldArr[1].startBit = 0;
                packetTypeFormat.bitFieldArr[1].numBits = 4;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:
                packetTypeFormat.indexConst = devDbArr[devNum].mcDstIndexCns;
                packetTypeFormat.indexMax = PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS + numOfUnicastChannels + numOfMulticastChannels - 1;
                packetTypeFormat.bitFieldArr[0].byteOffset = 17;
                packetTypeFormat.bitFieldArr[0].startBit = 0;
                packetTypeFormat.bitFieldArr[0].numBits = 8;
                packetTypeFormat.bitFieldArr[1].byteOffset = 16;
                packetTypeFormat.bitFieldArr[1].startBit = 0;
                packetTypeFormat.bitFieldArr[1].numBits = 6;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_CPU_E:
                packetTypeFormat.indexConst = PRV_CPSS_HAL_BPE_TRAFFIC_2_CPU_INDEX_CNS;
                packetTypeFormat.indexMax = PRV_CPSS_HAL_BPE_TRAFFIC_2_CPU_INDEX_CNS;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E:
                packetTypeFormat.indexConst = PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS;
                packetTypeFormat.indexMax = PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS + PRV_CPSS_PX_PORTS_NUM_CNS - 1;
                packetTypeFormat.bitFieldArr[0].byteOffset = 17;    /* E-Tag<E-CID[7:0]> */
                packetTypeFormat.bitFieldArr[0].startBit = 0;
                packetTypeFormat.bitFieldArr[0].numBits = 8;
                packetTypeFormat.bitFieldArr[1].byteOffset = 16;    /* E-Tag<E-CID[11:8]> */
                packetTypeFormat.bitFieldArr[1].startBit =  0;
                packetTypeFormat.bitFieldArr[1].numBits = 4;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E:
                packetTypeFormat.indexConst = PRV_CPSS_HAL_BPE_DISCARDED_TRAFFIC_INDEX_CNS;
                packetTypeFormat.indexMax = PRV_CPSS_HAL_BPE_DISCARDED_TRAFFIC_INDEX_CNS;
                break;

            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_1_E:
            case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_2_E:
            default: /* DATA extended->upstream */
                packetTypeFormat.indexConst = PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS;
                packetTypeFormat.indexMax = PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS;
                break;
        }

        rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],&packetTypeFormat);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpeDebugCncBlocksSet function
* @endinternal
*
* @brief   Set the bitmap of CNC Blocks that can be used by 802.1BR refHal for debug.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cncBitmap                - bitmap of CNC Blocks (APPLICABLE RANGE: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
*
* @note Default Configuration:
*       1 CNC block: Enable ingress-processing client
*       2 CNC blocks: Enable ingress-processing and egress-queue_pass_drop clients
*
*/
GT_STATUS cpssPxHalBpeDebugCncBlocksSet
(
    IN  GT_SW_DEV_NUM                    devNum,
    IN  GT_U32                           cncBitmap
)
{
    GT_STATUS rc;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_HAL_CNC_BMP_CHECK_MAC(cncBitmap);

    /* update DB with default configuration */
    switch (cncBitmap)
    {
        case 0:
            break;
        case 1:
            devDbArr[devNum].cncClientConfig[0] = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E;
            break;
        case 2:
            devDbArr[devNum].cncClientConfig[1] = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E;
            break;
        case 3:
            devDbArr[devNum].cncClientConfig[0] = CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E;
            devDbArr[devNum].cncClientConfig[1] = CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E;
            break;
    }
    devDbArr[devNum].cncBitmap = cncBitmap;

    /* configure CNC counters for debug */
    rc = prvCpssPxHalBpeCncConfig(devNum);
    if(GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal cpssPxHalBpeCncConfigClient function
* @endinternal
*
* @brief   Configure CNC for client.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] client                   -       CNC client; valid values:
*                                      CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E
*                                      CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E
* @param[in] blockNum                 -      CNC block number (APPLICABLE RANGE: 0..1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
*/
GT_STATUS cpssPxHalBpeCncConfigClient
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_CNC_CLIENT_ENT      client,
    IN  GT_U32                      blockNum
)
{

    GT_STATUS   rc;
    GT_U64      ranges;
    GT_U32      cncCounter, ii;  
    CPSS_PX_CNC_COUNTER_STC clearValue;
    CPSS_PX_TABLE_ENT table;
    CPSS_PX_CNC_CLIENT_ENT otherClient;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(blockNum,2);
    if (client == CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E)
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "client CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E is not supported");
    }

    blockNum == 0 ? (table = CPSS_PX_TABLE_CNC_0_COUNTERS_E) : (table = CPSS_PX_TABLE_CNC_1_COUNTERS_E);
    if (PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].cncBitmap, blockNum) == GT_FALSE)
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "block num [%d] is not enabled for debug",blockNum);
    }

    rc = cpssPxCfgTableNumEntriesGet(devNum,table,&cncCounter);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Un-bind client */
    otherClient = (client == CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E) ? CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E : CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E;
    rc = cpssPxCncBlockClientEnableSet(devNum, blockNum, otherClient, GT_FALSE);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Bind client */
    rc = cpssPxCncBlockClientEnableSet(devNum, blockNum, client, GT_TRUE);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Enable counter range 0..1K-1 for the block */
    ranges.l[0] = 0x1;
    ranges.l[1] = 0x0;
    rc = cpssPxCncBlockClientRangesSet(devNum, blockNum, ranges);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Enable clear by read */
    rc = cpssPxCncCounterClearByReadEnableSet(devNum, GT_TRUE);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Set clear by read value */
    cpssOsMemSet(&clearValue,0,sizeof(CPSS_PX_CNC_COUNTER_STC));
    rc = cpssPxCncCounterClearByReadValueSet(devNum, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &clearValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Set counter format */
    rc = cpssPxCncCounterFormatSet(devNum, blockNum, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E);
    if(GT_OK != rc)
    {
        return rc;
    }

    if (client == CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E)
    {
        /* Set Egress Queue client counting mode */
        rc = cpssPxCncEgressQueueClientModeSet(devNum,CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Clear all counters in block */
    for(ii = 0; ii < cncCounter; ii++)
    {
        cpssPxCncCounterGet(devNum, blockNum, ii, CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E, &clearValue);      
    }


    /* Update internal DB */
    devDbArr[devNum].cncClientConfig[blockNum] = client;

    return GT_OK;

}

/**
* @internal cpssPxHalBpeReset function
* @endinternal
*
* @brief   Clean all BPE configurations.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note set all ports to mode idle.
*       delete all E-channels Unicast and Multicast.
*
*/
GT_STATUS cpssPxHalBpeReset
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC *      chPtr;
    GT_U32                                  cid,i,entryIndex;
    GT_STATUS                               rc;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC      interface;
    CPSS_PX_PORTS_BMP                       portsBmp;
    CPSS_PX_PACKET_TYPE                     packetType;
    GT_TRUNK_ID                             trunkId;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC      portListPtr[PRV_CPSS_PX_PORTS_NUM_CNS];
    GT_U8                                   portListLen;
    GT_PORT_NUM                             portNum;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT    sourcePortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT    destPortInfo;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    /* delete all unicastEChannels */
    for (chPtr = devDbArr[devNum].channelDbPtr; chPtr; chPtr = chPtr->nextChannelPtr)
    {
        cid = chPtr->cid;
        rc = cpssPxHalBpeUnicastEChannelDelete(devNum, cid);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* delete all multicastEChannels */
    for (i=devDbArr[devNum].maxBpeUnicastEChannels; i<devDbArr[devNum].maxBpeUnicastEChannels+devDbArr[devNum].maxBpeMulticastEChannels; i++)
    {
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,i+PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS,0,GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* delete all non internal trunks */
    for (trunkId = 1; trunkId < PRV_CPSS_PX_TRUNKS_NUM_CNS; trunkId++)
    {
        if ((devDbArr[devNum].trunkDbArr[trunkId].valid == GT_TRUE) && (devDbArr[devNum].trunkDbArr[trunkId].trunkType != CPSS_PX_HAL_BPE_TRUNK_INTERNAL_E))
        {
            portListLen = 0;
            for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
            {
                if (PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum], portNum) == GT_TRUE)
                {
                    portListPtr[portListLen].type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
                    portListPtr[portListLen].devPort.devNum = devNum;
                    portListPtr[portListLen].devPort.portNum = portNum;
                    portListLen++;
                }
            }
            rc = cpssPxHalBpeTrunkPortsDelete(devNum, trunkId, portListPtr, portListLen);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssPxHalBpeTrunkRemove(devNum, trunkId);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    /* set all ports to IDLE mode */
    for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        switch (devDbArr[devNum].portDbArr[i].mode)
        {
            case CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E:
            case CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E:
            case CPSS_PX_HAL_BPE_PORT_MODE_TRUSTED_E:
            case CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E:
                interface.type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
                interface.devPort.devNum = devNum;
                interface.devPort.portNum = i;
                rc = cpssPxHalBpeInterfaceTypeSet(devNum,&interface,CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E);
                if(GT_OK != rc)
                {
                    return rc;
                }
                break;
            case CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E:
                /* do nothing */
                break;
            case CPSS_PX_HAL_BPE_PORT_MODE_INTERNAL_E:
            default:
                BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(devDbArr[devNum].portDbArr[i].mode);
        }
    }

    devDbArr[devNum].numBpeMulticastEChannel = 0;
    devDbArr[devNum].numBpeUnicastEChannel = 0;

    /* Set default values for src port map table - init state no filtering */
    portsBmp = 0x1FFFF;
    for (entryIndex = 0; entryIndex < _4K; entryIndex++)
    {
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,entryIndex,portsBmp,GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* disable entries 9..29 (rules for data traffic from extended to upstream with user-defined QoS) */
    for (packetType=PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E; packetType<(CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS+PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E); packetType++)
    {
        /* Disable packet type table lookup */
        rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Configure Designated port map table: no trunk members */
    portsBmp = 0x1FFFF;
    for(entryIndex = 0 ; entryIndex <= PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS ; entryIndex++)
    {
        rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,entryIndex,portsBmp);
        if (rc != GT_OK)
            return rc;
    }
    /* LAG table mode = 'two tables' */
    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum)) 
    {
        for(entryIndex = PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS+1 ; entryIndex <= PRV_CPSS_PX_LAG_DESIGNATED_TABLE_MC_MAX_CNS ; entryIndex++)
        {
            rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,entryIndex,portsBmp);
            if (rc != GT_OK)
                return rc;
        }
    }

    /* Set vlan tag state on all ports to "tagged", CPU port is not supported (ignored) */
    portsBmp = 0x0FFFF;  
    for (entryIndex = 0; entryIndex < _4K ; entryIndex++)
    {
        rc = cpssPxEgressVlanTagStateEntrySet(devNum,entryIndex,portsBmp);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Set default values for Source/Target Port Table Metadata */
    cpssOsMemSet(&sourcePortInfo,0,sizeof(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT));
    cpssOsMemSet(&destPortInfo,0,sizeof(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT));
    for (portNum=0; portNum<PRV_CPSS_PX_PORTS_NUM_CNS-1; portNum++) {

        rc = cpssPxEgressSourcePortEntrySet(devNum, portNum, CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E, &sourcePortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E, &destPortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal cpssPxHalBpeInterfaceTypeSet function
* @endinternal
*
* @brief   Set BPE interface mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number
* @param[in] interfacePtr             -  (pointer to) interface number
* @param[in] mode                     -  interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Changing mode: CASCADE->EXTENDED or EXTENDED->CASCADE:
*       it is the application responsiablitiy to check validity of E-channels.
*
*/
GT_STATUS cpssPxHalBpeInterfaceTypeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC      *interfacePtr,
    IN  CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT      mode
)
{
    GT_STATUS   rc;
    CPSS_PX_PORTS_BMP portsBmp;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT sourcePortInfo;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT sourceInfoType;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT destPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT destInfoType;
    CPSS_PX_PACKET_TYPE packetType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT operationInfo;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC portKey;
    GT_BOOL found = GT_FALSE;
    GT_U32 i, pcp;
    GT_PORT_NUM portNum = 0;
    GT_TRUNK_ID trunkId = 0;
    GT_BOOL bypassLagDesignatedBitmap;
    CPSS_PX_HAL_BPE_QOSMAP_STC qosMapArr[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(interfacePtr);
    switch (interfacePtr->type)
    {
        case CPSS_PX_HAL_BPE_INTERFACE_PORT_E:
            portNum = interfacePtr->devPort.portNum;
            PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(portNum);
            if (devNum != interfacePtr->devPort.devNum)
            {
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(interfacePtr->devPort.devNum));
            }
            break;

        case CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E:
            trunkId = interfacePtr->trunkId;
            PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_RANGE_MAC(trunkId);
            if (mode == CPSS_PX_HAL_BPE_INTERFACE_MODE_IDLE_E || mode == CPSS_PX_HAL_BPE_INTERFACE_MODE_TRUSTED_E)
            {
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(interfacePtr->type));
            }
            break;

        default:
            BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(interfacePtr->type));
    }

    cpssOsMemSet(&operationInfo,0,sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));

    if (interfacePtr->type == CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E)
    {
        /* TBD: The following type changes are allowed for a LAG:
            From Extended to Cascade
            From Cascade to Extended */
        return GT_NOT_SUPPORTED;
    }

    if (devDbArr[devNum].portDbArr[portNum].mode == CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E)
    {
        /* Update port's mode from IDLE -> mode */
        return prvCpssPxHalBpePortModeSet(devNum,portNum,mode);

    }
    else if (devDbArr[devNum].portDbArr[portNum].mode == CPSS_PX_HAL_BPE_PORT_MODE_TRUSTED_E && mode == CPSS_PX_HAL_BPE_INTERFACE_MODE_UPSTREAM_E) {

        /* TRUSTED -> UPSTREAM: all packets without E-tag are trapped to CPU ;all packet with E-tag are forward.
           Traffic without E-tag will have a match as packet type Upstream-to-CPU and dst format table will forward to CPU.
           Traffic with E-tag will have a match as packet type Upstream-to-Downstream-UC or Upstream-to-Downstream-MC; packet will be forward
        */

        devDbArr[devNum].numOfUpstreamPorts++;

        /* Set in DB port mode is UPSTREAM */
        prvCpssPxHalBpePortDbSet(devNum,portNum,mode);

        /* if exist TRUNK type Upstream --> don't add the new Upstream port to Dst port map table;
           all the traffic to Upstream ports should be through the designated port from the TRUNK */
        for (i=0; i<PRV_CPSS_PX_TRUNKS_NUM_CNS; i++)
        {
            if (devDbArr[devNum].trunkDbArr[i].trunkType == CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E && devDbArr[devNum].trunkDbArr[i].valid == GT_TRUE)
            {
                found = GT_TRUE;
                break;
            }
        }
        if (found == GT_FALSE)
        {
            rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,&portsBmp,
                                              &bypassLagDesignatedBitmap);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Set appropriate bit for portNum */
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum);

            /* Configure Dst port map table entry (UPSTREAM TRAFFIC) for the appropriate bit */
            rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,portsBmp,bypassLagDesignatedBitmap);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Set for all entries in LAG Designated port map table '1' for portNum */
            rc = prvCpssPxHalBpeLagDesignatedPortEntrySet(devNum,portNum,GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

         /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
        for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

            switch (packetType)
            {
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E:

                    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                    break;

                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E:

                    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                    break;

                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:

                    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                    break;

                default:
                    /* do nothing */
                    break;
            }
        }

    }
    else if (devDbArr[devNum].portDbArr[portNum].mode == CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E && mode == CPSS_PX_HAL_BPE_INTERFACE_MODE_TRUSTED_E) {

        /* UPSTREAM -> TRUSTED:  all packets without E-tag are trapped to CPU; all packets with E-tag are dropped.
           Traffic without E-tag will have a match as packet type Upstream-to-CPU and dst format table will forward to CPU.
           Traffic with E-tag will have a match as packet type Upstream-to-Downstream-UC or Upstream-to-Downstream-MC; packet will be dropped.                                                .
        */

        /* Set in DB port mode is TRUSTED */
        prvCpssPxHalBpePortDbSet(devNum,portNum,mode);

        /* Set src port profile as IDLE */
        cpssOsMemSet(&portKey,0,sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

        portKey.srcPortProfile = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_IDLE_E;
        rc = cpssPxIngressPortPacketTypeKeySet(devNum,portNum,&portKey);
        if(GT_OK != rc)
        {
            return rc;
        }

        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,&portsBmp,
                                          &bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Clear appropriate bit for portNum */
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,portNum);

        /* Delete appropriate bit from Dst port map table entry (UPSTREAM TRAFFIC) */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,portsBmp,bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* update DB */
        devDbArr[devNum].numOfUpstreamPorts--;

        /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
        for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

            switch (packetType)
            {

                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E:

                    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E,&operationInfo);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                    break;
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:

                    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,&operationInfo);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                    break;

                default:
                    /* do nothing */
                    break;
            }
        }
    }
    else if (devDbArr[devNum].portDbArr[portNum].mode == CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E && mode == CPSS_PX_HAL_BPE_INTERFACE_MODE_EXTENDED_E) {

        /* CASCADE -> EXTENDED: all packets with E-tag are dropped; all packets without E-tag are forward.
           Traffic with E-tag will have a match as packet type Extended-to-Upstream-E-Tagged and dst format table will drop the packet
           Traffic with C-tag will have a match as packet type Extended-to-Upstream-C-Tagged; packet will be forward
           Traffic untagged will have a match as packet type Extended-to-Upstream-Untagged; packet will be forward

           NOTE: it is the application responsiablitiy to check validity of E-channels.
        */

        /* Set in DB port mode is EXTENDED */
        prvCpssPxHalBpePortDbSet(devNum,portNum,mode);

        /* Set src port profile as EXTENDED */
        cpssOsMemSet(&portKey,0,sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

        portKey.srcPortProfile = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_EXTENDED_E;
        portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
        portKey.portUdbPairArr[0].udbByteOffset = 2; /* IP version and DSCP fields. the mask is done in Packet Type Key table */
        portKey.portUdbPairArr[1].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
        portKey.portUdbPairArr[1].udbByteOffset = 8; /* IPv6 next header (protocol). the mask is done in Packet Type Key table */
        portKey.portUdbPairArr[2].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
        portKey.portUdbPairArr[2].udbByteOffset = 10; /* IPv4 protocol. the mask is done in Packet Type Key table */
        portKey.portUdbPairArr[3].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
        portKey.portUdbPairArr[3].udbByteOffset = 0; /* L3 EtherType (for packet EthV2)/ LLC-SNAP PID (for packet LLC-SNAP) /
                                                        SSAP-DSAP (for packet LLC-NON-SNAP).
                                                        the mask is done in Packet Type Key table */          
        
        rc = cpssPxIngressPortPacketTypeKeySet(devNum,portNum,&portKey);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Set for all entries in LAG Designated port map table '1' for portNum */
        rc = prvCpssPxHalBpeLagDesignatedPortEntrySet(devNum,portNum,GT_TRUE);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Update Target Port Table Metadata */
        rc = cpssPxEgressTargetPortEntryGet(devNum, portNum, &destInfoType, &destPortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }

        destPortInfo.info_common.tpid = 0x8100;
        rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, destInfoType, &destPortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
        for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

            switch (packetType)
            {

                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:

                    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,&operationInfo);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                    break;

                default:
                    /* do nothing */
                    break;
            }
        }
    }
    else if (devDbArr[devNum].portDbArr[portNum].mode == CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E && mode == CPSS_PX_HAL_BPE_INTERFACE_MODE_CASCADE_E) {

        /* EXTENDED -> CASCADE: all packets with E-tag are forward; all packets without E-tag, E-tag is added then packet is forward.
           Traffic with E-tag will have a match as packet type Cascade-to-Upstream-E-Tagged; packet will be forward
           Traffic without E-tag will have a match as packet type Cascade-to-Upstream-Untagged; packet will be forward

           NOTE: it is the application responsiablitiy to check validity of E-channels.
        */

        /* Set in DB port mode is CASCADE */
        prvCpssPxHalBpePortDbSet(devNum,portNum,mode);

        /* Set src port profile as CASCADE */
        cpssOsMemSet(&portKey,0,sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

        portKey.srcPortProfile = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CASCADE_E;
        rc = cpssPxIngressPortPacketTypeKeySet(devNum,portNum,&portKey);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Set for all entries in LAG Designated port map table '1' for portNum */
        rc = prvCpssPxHalBpeLagDesignatedPortEntrySet(devNum,portNum,GT_TRUE);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
        for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

            switch (packetType)
            {
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
                case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:

                    rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E,&operationInfo);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                    break;

                default:
                    /* do nothing */
                    break;
            }
        }
    }
    else {

        /* move port to idle first */
        switch (devDbArr[devNum].portDbArr[portNum].mode) {

            /* wrong state */
            case CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E:

                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(devDbArr[devNum].portDbArr[portNum].mode));
                break;

            /* INTERNAL -> IDLE */
            case CPSS_PX_HAL_BPE_PORT_MODE_INTERNAL_E:
                /* TBD - will be implemented in phase 2 */
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(devDbArr[devNum].portDbArr[portNum].mode));
                break;

            /* UPSTREAM -> IDLE: all packets are dropped */
            case CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E:

                rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,&portsBmp,
                                                  &bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Clear appropriate bit for portNum */
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,portNum);

                /* Delete appropriate bit from Dst port map table entry (UPSTREAM TRAFFIC) */
                rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,portsBmp,bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* update DB */
                devDbArr[devNum].numOfUpstreamPorts--;

                /* Set in DB port mode is IDLE; Set src port profile as IDLE */
                rc = prvCpssPxHalBpePortDbIdleModeSet(devNum,portNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Update Source Port Table Metadata */
                rc = cpssPxEgressSourcePortEntryGet(devNum, portNum, &sourceInfoType, &sourcePortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

                sourcePortInfo.info_802_1br.pcid = 0;
                sourcePortInfo.info_802_1br.srcPortInfo.srcFilteringVector = 0;
                rc = cpssPxEgressSourcePortEntrySet(devNum, portNum, sourceInfoType, &sourcePortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

               /* Update Target Port Table Metadata */
                rc = cpssPxEgressTargetPortEntryGet(devNum, portNum, &destInfoType, &destPortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }
                destPortInfo.info_common.pcid = 0;
                destPortInfo.info_common.tpid = 0;
                rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, destInfoType, &destPortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
                for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

                    switch (packetType)
                    {

                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:

                            rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,&operationInfo);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                            break;

                        default:
                            /* do nothing */
                            break;
                    }
                }
                break;

            /* CASCADE -> IDLE: all packets are dropped */
            case CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E:

                /* Reset cascade E-PCP to queue mapping */
                for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
                {
                    rc = cpssPxHalBpeExtendedQosMapSet(devNum, portNum, pcp, 0);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }

                /* Reset cascade default E-PCP, E-DEI */
                rc = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(devNum, portNum, 0, 0);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Reset cascade C-PCP, C-DEI to E-PCP, E-DEI mapping */
                cpssOsMemSet(qosMapArr, 0x00, CPSS_PCP_RANGE_CNS*CPSS_DEI_RANGE_CNS*sizeof(CPSS_PX_HAL_BPE_QOSMAP_STC));
                rc = cpssPxHalBpePortExtendedQosMapSet(devNum, portNum, qosMapArr);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Set in DB port mode is IDLE; Set src port profile as IDLE */
                rc = prvCpssPxHalBpePortDbIdleModeSet(devNum,portNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Check no E-Channels are attached to this port */
                rc = prvCpssPxHalBpePortCheckChannel(devNum,portNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
                for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

                    switch (packetType)
                    {

                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:

                            rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,&operationInfo);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                            break;

                        default:
                            /* do nothing */
                            break;
                    }
                }
                break;

            /* EXTENDED -> IDLE: all packets are dropped */
            case CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E:

                /* Reset extended E-PCP to queue mapping */
                for (pcp = 0; pcp < CPSS_PCP_RANGE_CNS; pcp++)
                {
                    rc = cpssPxHalBpeExtendedQosMapSet(devNum, portNum, pcp, 0);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }

                /* Reset extended default E-PCP, E-DEI */
                rc = cpssPxHalBpePortExtendedQosDefaultPcpDeiSet(devNum, portNum, 0, 0);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Reset extended C-PCP, C-DEI to E-PCP, E-DEI mapping */
                cpssOsMemSet(qosMapArr, 0x00, CPSS_PCP_RANGE_CNS*CPSS_DEI_RANGE_CNS*sizeof(CPSS_PX_HAL_BPE_QOSMAP_STC));
                rc = cpssPxHalBpePortExtendedQosMapSet(devNum, portNum, qosMapArr);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Update Src PortMap Table */
                rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,devDbArr[devNum].portDbArr[portNum].pcid,&portsBmp,
                                                  &bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }
                /* Clear appropriate bit for portNum */
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum);

                /* Configure Src port map table entry pcid for the appropriate bit */
                rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,devDbArr[devNum].portDbArr[portNum].pcid,portsBmp,bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Update Source Port Table Metadata */
                rc = cpssPxEgressSourcePortEntryGet(devNum, portNum, &sourceInfoType, &sourcePortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Set in DB port mode is IDLE; Set src port profile as IDLE */
                rc = prvCpssPxHalBpePortDbIdleModeSet(devNum,portNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Check no E-Channels are attached to this port */
                rc = prvCpssPxHalBpePortCheckChannel(devNum,portNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Update Target Port Table Metadata */
                rc = cpssPxEgressTargetPortEntryGet(devNum, portNum, &destInfoType, &destPortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

                destPortInfo.info_common.tpid = 0;
                rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, destInfoType, &destPortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
                for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

                    switch (packetType)
                    {

                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E:

                            rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,&operationInfo);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                            break;

                        default:
                            /* do nothing */
                            break;
                    }
                }

                break;

            /* TRUSTED -> IDLE: all packets are dropped */
            case CPSS_PX_HAL_BPE_PORT_MODE_TRUSTED_E:

                /* Set in DB port mode is IDLE; Set src port profile as IDLE */
                rc = prvCpssPxHalBpePortDbIdleModeSet(devNum,portNum);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Update Source Port Table Metadata */
                rc = cpssPxEgressSourcePortEntryGet(devNum, portNum, &sourceInfoType, &sourcePortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

                sourcePortInfo.info_802_1br.pcid = 0;
                sourcePortInfo.info_802_1br.srcPortInfo.srcFilteringVector = 0;

                rc = cpssPxEgressSourcePortEntrySet(devNum, portNum, sourceInfoType, &sourcePortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

               /* Update Target Port Table Metadata */
                rc = cpssPxEgressTargetPortEntryGet(devNum, portNum, &destInfoType, &destPortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

                destPortInfo.info_common.pcid = 0;
                destPortInfo.info_common.tpid = 0;
                rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, destInfoType, &destPortInfo);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Update Header Alteration Table that maps <Packet Type, Egress port> tuple to the relevant firmware thread */
                for (packetType=0; packetType<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; packetType++) {

                    switch (packetType)
                    {

                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E:
                        case PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E:

                            rc = cpssPxEgressHeaderAlterationEntrySet(devNum,portNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[packetType],CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,&operationInfo);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                            break;

                        default:
                            /* do nothing */
                            break;
                    }
                }
                break;

            default:
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(devDbArr[devNum].portDbArr[portNum].mode));
                break;
        }

        /* update port mode; port is in state idle */
        rc = prvCpssPxHalBpePortModeSet(devNum,portNum,mode);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpeUpstreamQosMapSet function
* @endinternal
*
* @brief   Set CoS mapping between E-PCP to QUEUE for Upstream port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] pcp                      -   class of service (APPLICABLE RANGE: 0..7).
* @param[in] queue                    -   egress  (APPLICABLE RANGE: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeUpstreamQosMapSet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              pcp,
    IN  GT_U32              queue
)
{
    GT_STATUS   rc;    
    GT_PORT_NUM  portNum;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(pcp,BIT_3);
    CPSS_PARAM_CHECK_MAX_MAC(queue,BIT_3);

    /* For packet types:
       Upstream-to-Downstream-UC/MC : Mapping E-PCP of received frames on an upstream port to outgoing queue.
    */
    for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++) {

        if (devDbArr[devNum].portDbArr[portNum].mode != CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E)
            continue;

        rc = prvCpssPxHalBpeUpstreamQosMapSet(devNum,portNum,pcp,queue);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpeExtendedQosMapSet function
* @endinternal
*
* @brief   Set CoS mapping between E-PCP to QUEUE for Extended/Cascade port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] portNum                  -   port number
* @param[in] pcp                      -   class of service (APPLICABLE RANGE: 0..7).
* @param[in] queue                    -   egress  (APPLICABLE RANGE: 0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function must be called after cpssPxHalBpePortExtendedQosMapSet,
*       where egress PCP is updated.
*
*/
GT_STATUS cpssPxHalBpeExtendedQosMapSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U32          pcp,
    IN  GT_U32          queue
)
{
    GT_STATUS   rc;
    GT_U32      l2Index, dei, ingress_pcp;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(portNum);
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(pcp,BIT_3);
    CPSS_PARAM_CHECK_MAX_MAC(queue,BIT_3);
    PRV_CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum,portNum);

    /* For packet types:
        Extended-to-Upstream-Untagged
        Cascade-to-Upstream-Untagged
        Extended-to-Upstream-C-Tagged
        Cascade-to-Upstream-E-Tagged

        Mapping of E-Tag<PCP> (egress PCP) to egress queue.
    */

    for(ingress_pcp=0; ingress_pcp < CPSS_PCP_RANGE_CNS; ingress_pcp++) {
        for(dei=0; dei < CPSS_DEI_RANGE_CNS; dei++) {
            l2Index = (ingress_pcp << 1) | dei;
            rc = cpssPxCosPortL2MappingGet(devNum, portNum, l2Index, &cosAttributes);
            if(GT_OK != rc)
            {
                return rc;
            }
            if (cosAttributes.userPriority == pcp) {
                /* update queue */
                cosAttributes.trafficClass = queue;

                /* Set CoS L2 Attributes mapping per Port */
                rc = cpssPxCosPortL2MappingSet(devNum, portNum, l2Index, &cosAttributes);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* QoS mapping changed for the default mapping, update the port CoS attributes */
                if ((devDbArr[devNum].portDbArr[portNum].portQos.defaultQos.newPcp == ingress_pcp) &&
                    (devDbArr[devNum].portDbArr[portNum].portQos.defaultQos.newDei == dei)) {
                    /* Set CoS Attributes per Port from the L2 Mapping */
                    rc = cpssPxCosPortAttributesSet(devNum,portNum,&cosAttributes);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpeInterfaceExtendedPcidSet function
* @endinternal
*
* @brief   Set Extended/Cascade interface Pcid.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] interfacePtr             - (pointer to) interface number
* @param[in] pcid                     - E-channel Id to assign to ingress traffic on interface
*                                      (APPLICABLE RANGE: 0..maxBpeUnicastEChannels-1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeInterfaceExtendedPcidSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  GT_U32                              pcid
)
{
    CPSS_PX_PORTS_BMP                         portsBmp;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT sourceInfoType;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT      sourcePortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT destInfoType;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT      destPortInfo;
    GT_STATUS                                 rc;
    GT_PORT_NUM     portNum;
    GT_TRUNK_ID     trunkId;
    GT_U32          oldPcid,i;
    GT_BOOL         bypassLagDesignatedBitmap;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(interfacePtr);
    CPSS_PARAM_CHECK_MAX_MAC(pcid,devDbArr[devNum].maxBpeUnicastEChannels);
    switch (interfacePtr->type)
    {
        case CPSS_PX_HAL_BPE_INTERFACE_PORT_E:
            portNum = interfacePtr->devPort.portNum;
            PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(portNum);
            PRV_CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum,portNum);
            if (devNum != interfacePtr->devPort.devNum) 
            {
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(interfacePtr->devPort.devNum));
            }
            break;

        case CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E:
            trunkId = interfacePtr->trunkId;
            PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_RANGE_MAC(trunkId);
            PRV_CPSS_PX_HAL_BPE_TRUNK_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum,trunkId);            
            break;

        default:
            BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(interfacePtr->type));
    }

    if (interfacePtr->type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E)
    {
        oldPcid = devDbArr[devNum].portDbArr[portNum].pcid;
        /* Restore Src PortMap Table */
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,oldPcid,&portsBmp, &bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }
        /* Set appropriate bit for portNum */
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum);

        /* Configure Src port map table entry pcid for the appropriate bit */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,oldPcid,portsBmp,bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Update Src PortMap Table */
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,pcid,&portsBmp,
                                          &bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }
        /* Set appropriate bit for portNum */
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,portNum);

        /* Configure Src port map table entry pcid for the appropriate bit */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,pcid,portsBmp,bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Update Source Port Table Metadata */
        rc = cpssPxEgressSourcePortEntryGet(devNum, portNum, &sourceInfoType, &sourcePortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }
        /* '0' is also the inital value of the register */
        if(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E != sourceInfoType)
        {
            BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sourceInfoType);
        }
        sourcePortInfo.info_802_1br.pcid = pcid;
        sourcePortInfo.info_802_1br.srcPortInfo.srcFilteringVector = 1 << portNum;

        rc = cpssPxEgressSourcePortEntrySet(devNum, portNum, CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E, &sourcePortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Update Target Port Table Metadata */
        rc = cpssPxEgressTargetPortEntryGet(devNum, portNum, &destInfoType, &destPortInfo);
            if(GT_OK != rc)
        {
            return rc;
        }
        /* '0' is also the inital value of the register */
        if(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E != destInfoType)
        {
            BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(destInfoType);
        }
        destPortInfo.info_common.pcid = pcid;
        destPortInfo.info_common.tpid = 0x8100;
        rc = cpssPxEgressTargetPortEntrySet(devNum, portNum, CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E, &destPortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Update internal port DB */
        devDbArr[devNum].portDbArr[portNum].pcid = pcid;

    }
    else
    {        
        oldPcid = devDbArr[devNum].trunkDbArr[trunkId].cid;
        /* Restore Src PortMap Table */
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,oldPcid,&portsBmp, &bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }
        /* Set appropriate bit for all the ports in trunkId */
        for(i = 0; i < PRV_CPSS_PX_PORTS_NUM_CNS; i++)
        {
            if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum], i))
            {
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,i);
            }
        }

        /* Configure Src port map table entry pcid for the appropriate bit */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,oldPcid,portsBmp,bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Update Src PortMap Table */
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,pcid,&portsBmp,
                                          &bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }
        /* Set appropriate bit for all the ports in trunkId */
        for(i = 0; i < PRV_CPSS_PX_PORTS_NUM_CNS; i++)
        {
            if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum], i))
            {
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(portsBmp,i);
            }
        }        

        /* Configure Src port map table entry pcid for the appropriate bit */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,pcid,portsBmp,bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Update internal trunk DB */
        devDbArr[devNum].trunkDbArr[trunkId].cid = pcid;
    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpePortExtendedUpstreamSet function
* @endinternal
*
* @brief   Set Extended/Cascade port Upstream interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] portNum                  -       port number
* @param[in] upstreamPtr              -       (pointer to) upstream interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedUpstreamSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PORT_NUM                             portNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC      *upstreamPtr
)
{
    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(portNum);
    CPSS_NULL_PTR_CHECK_MAC(upstreamPtr);
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum,portNum);

    switch (upstreamPtr->type)
    {
        case CPSS_PX_HAL_BPE_INTERFACE_PORT_E:
            PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(upstreamPtr->devPort.devNum);
            PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(upstreamPtr->devPort.portNum);
            PRV_CPSS_PX_HAL_BPE_PORT_MODE_CHECK_MAC(upstreamPtr->devPort.devNum,upstreamPtr->devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E);
            break;
        case CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E:
            PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_RANGE_MAC(upstreamPtr->trunkId);
            break;
        default:
            BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(upstreamPtr->type);
    }

    /* update db */
    cpssOsMemCpy(&(devDbArr[devNum].portDbArr[portNum].upstream) ,upstreamPtr, sizeof(CPSS_PX_HAL_BPE_INTERFACE_INFO_STC));

    return GT_OK;

}


/**
* @internal cpssPxHalBpePortExtendedQosDefaultPcpDeiSet function
* @endinternal
*
* @brief   Set Extended/Cascade port default E-PCP and E-DEI.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] portNum                  -   port number
* @param[in] pcp                      -   class of service (APPLICABLE RANGE: 0..7).
* @param[in] dei                      -   drop eligibility indication (APPLICABLE RANGE: 0..1).
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedQosDefaultPcpDeiSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PORT_NUM     portNum,
    IN  GT_U32          pcp,
    IN  GT_U32          dei
)
{
    GT_STATUS   rc;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes;
    GT_U32      l2Index;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(portNum);
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum,portNum);
    CPSS_PARAM_CHECK_MAX_MAC(pcp,BIT_3);
    CPSS_PARAM_CHECK_MAX_MAC(dei,BIT_1);


    /* For packet types:
       packets with no E-tag and no C-tag; Extended-to-Upstream-Untagged: set default E-tag<PCP,DEI>.
       packets with no E-tag; Cascade-to-Upstream-Untagged: set default E-tag<PCP,DEI>.
    */

    l2Index = (pcp << 1) | dei;

    /* Get CoS L2 Attributes mapping per Port */
    rc = cpssPxCosPortL2MappingGet(devNum, portNum, l2Index, &cosAttributes);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Set CoS Attributes per Port from the L2 Mapping */
    rc = cpssPxCosPortAttributesSet(devNum,portNum,&cosAttributes);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Save default QoS in database so that when Extended port QoS mapping is changed
       for the default QoS mapping we would be able to update the port CoS attributes accordingly. */
    devDbArr[devNum].portDbArr[portNum].portQos.defaultQos.newPcp = pcp;
    devDbArr[devNum].portDbArr[portNum].portQos.defaultQos.newDei = dei;

    return rc;

}

/**
* @internal cpssPxHalBpePortExtendedQosMapSet function
* @endinternal
*
* @brief   Set CoS mapping between C-UP,C-CFI to E-PCP,E-DEI for Extended/Cascade port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] portNum                  -   port number
* @param[in] qosMapArr[CPSS_PCP_RANGE_CNS] -   qos mapping array
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedQosMapSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PORT_NUM                 portNum,
    IN  CPSS_PX_HAL_BPE_QOSMAP_STC  qosMapArr[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS]
)
{
    GT_STATUS   rc;
    GT_U32      l2Index;
    GT_U32      pcp,dei;
    CPSS_PX_COS_ATTRIBUTES_STC  cosAttributes;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(portNum);
    CPSS_NULL_PTR_CHECK_MAC(qosMapArr);
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum,portNum);

    /* For packet types:
        Extended-to-Upstream-C-Tagged: mapping C-tag <UP,CFI>  to E-tag<PCP,DEI>.
        Cascade-to-Upstream-E-Tagged:  mapping E-tag <PCP,DEI> to E-tag<PCP,DEI>.
    */

    for(pcp=0; pcp < CPSS_PCP_RANGE_CNS; pcp++) {
        for(dei=0; dei < CPSS_DEI_RANGE_CNS; dei++) {

            /* Validate qos parameters */
            CPSS_PARAM_CHECK_MAX_MAC(qosMapArr[pcp][dei].newPcp,BIT_3);
            CPSS_PARAM_CHECK_MAX_MAC(qosMapArr[pcp][dei].newDei,BIT_1);

            l2Index = (pcp << 1) | dei;

            /* Get CoS L2 Attributes mapping per Port */
            rc = cpssPxCosPortL2MappingGet(devNum, portNum, l2Index, &cosAttributes);
            if(GT_OK != rc)
            {
                return rc;
            }
            cosAttributes.userPriority = qosMapArr[pcp][dei].newPcp;
            cosAttributes.dropEligibilityIndication =  qosMapArr[pcp][dei].newDei;
            cosAttributes.dropPrecedence = (cosAttributes.dropEligibilityIndication == 0) ? CPSS_DP_GREEN_E : CPSS_DP_YELLOW_E;

            /* Set CoS L2 Attributes mapping per Port */
            rc = cpssPxCosPortL2MappingSet(devNum, portNum, l2Index, &cosAttributes);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* QoS mapping changed for the default mapping, update the port CoS attributes */
            if ((devDbArr[devNum].portDbArr[portNum].portQos.defaultQos.newPcp == pcp) &&
                (devDbArr[devNum].portDbArr[portNum].portQos.defaultQos.newDei == dei)) {
                /* Set CoS Attributes per Port from the L2 Mapping */
                rc = cpssPxCosPortAttributesSet(devNum,portNum,&cosAttributes);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }

        }
    }

    /* update db */
    cpssOsMemCpy(&(devDbArr[devNum].portDbArr[portNum].portQos.qosMap) ,qosMapArr, sizeof(CPSS_PX_HAL_BPE_QOSMAP_STC)*CPSS_PCP_RANGE_CNS*CPSS_DEI_RANGE_CNS);

    return GT_OK;
}

/**
* @internal cpssPxHalBpeUnicastEChannelCreate function
* @endinternal
*
* @brief   Create E-channel.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cid                      - E-channel Id (APPLICABLE RANGE: 0..maxBpeUnicastEChannels-1)
* @param[in] downStreamInterfacePtr   - downstream Interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_ALREADY_EXIST         - the E-Channel already was already created
*/
GT_STATUS cpssPxHalBpeUnicastEChannelCreate
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              cid,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *downStreamInterfacePtr
)
{
    CPSS_PX_PORTS_BMP portsBmp;
    GT_TRUNK_ID       trunkId;
    GT_PORT_NUM       portNum;
    GT_STATUS         rc;
    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC *chPtr;
    GT_BOOL           bypassLagDesignatedBitmap;

    /* Validate parameters */
    CPSS_NULL_PTR_CHECK_MAC(downStreamInterfacePtr);
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(cid,devDbArr[devNum].maxBpeUnicastEChannels);

    chPtr = prvCpssPxHalBpeGetChannel(&devDbArr[devNum],cid);
    if (chPtr) {
        return GT_ALREADY_EXIST;
    }

    rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+devDbArr[devNum].ucDstIndexCns,&portsBmp,
                                      &bypassLagDesignatedBitmap);
    if(GT_OK != rc)
    {
        return rc;
    }

    switch(downStreamInterfacePtr->type)
    {
    case CPSS_PX_HAL_BPE_INTERFACE_PORT_E:

        /* Validate parameters */
        portNum = downStreamInterfacePtr->devPort.portNum;
        PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(portNum);

        /* TBD - need to add support for INTERNAL in phase 2 */
        if (downStreamInterfacePtr->devPort.devNum != devNum) {
            BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(downStreamInterfacePtr->devPort.devNum));
        }

        if (devDbArr[devNum].portDbArr[portNum].mode == CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E) {

             /* Cascade port Unicast channel configuration depends on all the following conditions:
                - Cascade port have PCID configured
                - Channel contains the cascade port
                - Cascade port have a defined upstream.
            */
            if (prvCpssPxHalBpePortCascadeToUpstreamIsValid(&devDbArr[devNum],&devDbArr[devNum].portDbArr[portNum]) != GT_TRUE)
            {
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(devDbArr[devNum].portDbArr[portNum]));
            }
        }
        else
        {
            /* Extended port Unicast channel configuration depends on all the following conditions:
                - Extended port have PCID configured
                - Channel created with CID equal to PCID and channel contains the extended port
            */
            if (prvCpssPxHalBpePortExtendedToUpstreamIsValid(&devDbArr[devNum],&devDbArr[devNum].portDbArr[portNum],cid) != GT_TRUE)
            {
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(cid));
            }
        }

        /* Set appropriate bit for portNum */
        PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum);
        break;

    case CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E:

         /* Validate parameters */
        trunkId = downStreamInterfacePtr->trunkId;
        PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_RANGE_MAC(downStreamInterfacePtr->trunkId);
        PRV_CPSS_PX_HAL_BPE_TRUNK_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum,downStreamInterfacePtr->trunkId);       

        /* Set appropriate bits for trunkId */
        portsBmp |= devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum];
        break;

    default:
        BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(downStreamInterfacePtr->type);
    }

    /* add channel */
    chPtr = cpssOsMalloc(sizeof(CPSS_PX_HAL_BPE_CHANNEL_INFO_STC));
    if (chPtr == NULL)
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(chPtr));
    }

    chPtr->cid = cid;
    cpssOsMemCpy(&chPtr->interfce, downStreamInterfacePtr,sizeof(CPSS_PX_HAL_BPE_INTERFACE_INFO_STC));
    chPtr->nextChannelPtr = devDbArr[devNum].channelDbPtr;
    devDbArr[devNum].channelDbPtr = chPtr;
    devDbArr[devNum].numBpeUnicastEChannel++;

    /* Update Dest PortMap Table */
   if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        /* direct packets to specific port which is a member in a LAG. For UC E-channels of type PORT the LAG Designated Port selection is bypassed.
           For UC E-channels of type TRUNK the LAG Designated Port selection is applied. */
        bypassLagDesignatedBitmap = (downStreamInterfacePtr->type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E ? 1 : 0);
        rc = cpssPxIngressPortMapEntrySet(devNum, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, cid+devDbArr[devNum].ucDstIndexCns, portsBmp,bypassLagDesignatedBitmap);
    }
    else
    {
        rc = cpssPxIngressPortMapEntrySet(devNum, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, cid+devDbArr[devNum].ucDstIndexCns, portsBmp,GT_FALSE);
    }
    if(GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpeUnicastEChannelDelete function
* @endinternal
*
* @brief   Delete E-channel.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cid                      - E-channel Id (APPLICABLE RANGE: 0..maxBpeUnicastEChannels-1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeUnicastEChannelDelete
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cid
)
{
    GT_STATUS rc;
    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC *chPtr, *deleteChPtr;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(cid,devDbArr[devNum].maxBpeUnicastEChannels);

    chPtr = devDbArr[devNum].channelDbPtr;
    if (chPtr == NULL) {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(chPtr));
    }

    /* find Unicast E-Channel in DB */
    if (chPtr->cid == cid)
    {
        deleteChPtr = chPtr;
        devDbArr[devNum].channelDbPtr = chPtr->nextChannelPtr;
    }
    else
    {
        for (deleteChPtr = NULL; chPtr->nextChannelPtr; chPtr = chPtr->nextChannelPtr)
        {
            if (chPtr->nextChannelPtr->cid == cid)
            {
                    deleteChPtr = chPtr->nextChannelPtr;
                    chPtr->nextChannelPtr = chPtr->nextChannelPtr->nextChannelPtr;
                    break;
            }
        }
    }

    if (deleteChPtr == NULL)
    {
    BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(deleteChPtr));
    }

    /* delete channel */
    cpssOsFree(deleteChPtr);
    devDbArr[devNum].numBpeUnicastEChannel--;

    /* Update Dest PortMap Table */
    rc = cpssPxIngressPortMapEntrySet(devNum, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, cid+devDbArr[devNum].ucDstIndexCns, 0,GT_FALSE);
    if(GT_OK != rc)
    {
            return rc;
    }

    return GT_OK;
}


/**
* @internal cpssPxHalBpePortExtendedUntaggedVlanAdd function
* @endinternal
*
* @brief   Configure the Extended interface to egress packets untagged
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] interfacePtr             -   (pointer to) interface number
* @param[in] vid                      -   vlanId to egress packet untagged
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedUntaggedVlanAdd
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  GT_U32                              vid
)
{

    GT_STATUS                                 rc;
    GT_U32                                    i;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(interfacePtr);
    CPSS_PARAM_CHECK_MAX_MAC(vid,BIT_12);

    switch (interfacePtr->type)
    {
        case CPSS_PX_HAL_BPE_INTERFACE_PORT_E:
            PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(interfacePtr->devPort.portNum);
            PRV_CPSS_PX_HAL_BPE_PORT_MODE_CHECK_MAC(interfacePtr->devPort.devNum, interfacePtr->devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
            break;

        case CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E:
            PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_MAC(devNum, interfacePtr->trunkId);
            PRV_CPSS_PX_HAL_BPE_TRUNK_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum, interfacePtr->trunkId);
            break;

        default:
            BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(interfacePtr->type));
    }

   if (interfacePtr->type == CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E)
   {
       for(i = 0; i < PRV_CPSS_PX_PORTS_NUM_CNS; i++)
       {
           if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].trunkDbArr[interfacePtr->trunkId].portsBmpArr[devNum], i))
           {
               rc = prvCpssPxHalBpePortExtendedUntaggedVlanAdd(devNum, i, vid);
               if(GT_OK != rc)
               {
                   return rc;
               }
           }
       }
   }
   else
   {
       rc = prvCpssPxHalBpePortExtendedUntaggedVlanAdd(interfacePtr->devPort.devNum, interfacePtr->devPort.portNum, vid);
   }

    return rc;
}

/**
* @internal cpssPxHalBpePortExtendedUntaggedVlanDel function
* @endinternal
*
* @brief   Configure the Extended interface to egress packets tagged
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] interfacePtr             -   (pointer to) interface number
* @param[in] vid                      -   vlanId to egress packet tagged
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpePortExtendedUntaggedVlanDel
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfacePtr,
    IN  GT_U32                              vid
)
{
    GT_STATUS                                 rc;
    GT_U32                                    i;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(interfacePtr);
    CPSS_PARAM_CHECK_MAX_MAC(vid,BIT_12);    

    switch (interfacePtr->type)
    {
        case CPSS_PX_HAL_BPE_INTERFACE_PORT_E:
            PRV_CPSS_PX_HAL_BPE_PORT_NUM_CHECK_MAC(interfacePtr->devPort.portNum);
            PRV_CPSS_PX_HAL_BPE_PORT_MODE_CHECK_MAC(interfacePtr->devPort.devNum, interfacePtr->devPort.portNum, CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E);
            break;

        case CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E:
            PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_MAC(devNum, interfacePtr->trunkId);
            PRV_CPSS_PX_HAL_BPE_TRUNK_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum, interfacePtr->trunkId);
            break;

        default:
            BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(interfacePtr->type));
    }

    if (interfacePtr->type == CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E)
    {
       for(i = 0; i < PRV_CPSS_PX_PORTS_NUM_CNS; i++)
       {
           if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].trunkDbArr[interfacePtr->trunkId].portsBmpArr[devNum], i))
           {
               rc = prvCpssPxHalBpePortExtendedUntaggedVlanDel(devNum, i, vid);
               if(GT_OK != rc)
               {
                   return rc;
               }
           }
       }
    }
    else
    {
       rc = prvCpssPxHalBpePortExtendedUntaggedVlanDel(interfacePtr->devPort.devNum, interfacePtr->devPort.portNum, vid);
    }

    return rc;
}

/**
* @internal cpssPxHalBpeTrunkCreate function
* @endinternal
*
* @brief   Create a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -     device number
* @param[in] trunkId                  -     trunk ID
* @param[in] trunkType                -     type of trunk ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_ALREADY_EXIST         - the E-Channel was already 
*         created
*
* @note trunk contains ports from same trunk type
*
*/
GT_STATUS cpssPxHalBpeTrunkCreate
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_TRUNK_ID                     trunkId,
    IN  CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT  trunkType
)
{
    GT_STATUS rc;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_RANGE_MAC(trunkId);

    switch (trunkType)
    {
        case CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E:
        case CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E:
        case CPSS_PX_HAL_BPE_TRUNK_CASCADE_E:
            /* nothing to do */
            break;
        case CPSS_PX_HAL_BPE_TRUNK_INTERNAL_E:        
            /* TBD - need to add support in phase 2 */
            BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(trunkType);
            break;
        default:
            BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(trunkType);

    }

    if (devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum] || devDbArr[devNum].trunkDbArr[trunkId].valid == GT_TRUE)
    {
        return GT_ALREADY_EXIST;
    }

    devDbArr[devNum].trunkDbArr[trunkId].trunkType = trunkType;
    devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum] = 0;
    devDbArr[devNum].trunkDbArr[trunkId].cid = 0;
    devDbArr[devNum].trunkDbArr[trunkId].valid = GT_TRUE;

    /* if exist TRUNK type Upstream --> all the traffic to Upstream ports should be through the designated port from the TRUNK;
       delete all upstream ports from Dst port map table */
    if (trunkType == CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E)
    {
        /* Configure Dst port map table entry (UPSTREAM TRAFFIC) with '0' --> no upstream ports */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,0,GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpeTrunkRemove function
* @endinternal
*
* @brief   Remove a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] trunkId                  -       trunk ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on trunk doesn't exists or doesn't have cid defined
*/
GT_STATUS cpssPxHalBpeTrunkRemove
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_TRUNK_ID     trunkId
)
{
    CPSS_PX_PORTS_BMP                portsBmp;
    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC *chPtr;
    GT_U32                           portNum;
    GT_STATUS                        rc;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_RANGE_MAC(trunkId);

    portsBmp = devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum];
    if (portsBmp)
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "{devNum[%d],trunkId[%d]} has ports",(devNum),(trunkId));
    }
    if (devDbArr[devNum].trunkDbArr[trunkId].valid == GT_FALSE)
    {
        /* nothing to do */
        return GT_OK;
    }

    /* find channel of trunk */
    for (chPtr = devDbArr[devNum].channelDbPtr; chPtr; chPtr = chPtr->nextChannelPtr)
    {
        if (chPtr->interfce.type == CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E && chPtr->interfce.trunkId == trunkId)
        {
            BPE_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "{devNum[%d],trunkId[%d]} has E-Channel",(devNum),(trunkId));
            break;
        }
    }

    /* if TRUNK type Upstream is deleted --> all the traffic should be through Upstream ports;
       add all upstream ports to Dst port map table */
    if (devDbArr[devNum].trunkDbArr[trunkId].trunkType == CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E)
    {
        portsBmp = 0;
        for (portNum=0; portNum<PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
        {
            if (devDbArr[devNum].portDbArr[portNum].mode == CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E)
            {
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp,portNum);
            }
        }

        /* Configure Dst port map table entry (UPSTREAM TRAFFIC) with all upstream ports */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,portsBmp,GT_FALSE);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

     /* update DB */
    devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum] = 0;
    devDbArr[devNum].trunkDbArr[trunkId].valid = GT_FALSE;
    devDbArr[devNum].trunkDbArr[trunkId].cid = 0;

    return GT_OK;
}

/**
* @internal cpssPxHalBpeTrunkLoadBalanceModeSet function
* @endinternal
*
* @brief   Set the Load Balance Algorithm of the TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] loadBalanceMode          - load balance mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on offset in udbpArr is out of range
*/
GT_STATUS cpssPxHalBpeTrunkLoadBalanceModeSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT loadBalanceMode
)
{
    GT_STATUS   rc;               /* return code */
    CPSS_PX_INGRESS_HASH_UDBP_STC  udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    GT_U32      i;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    cpssOsMemSet(&udbpArr, 0, sizeof(CPSS_PX_INGRESS_HASH_UDBP_STC)*CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS);

    switch (loadBalanceMode){

        case CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_IP_TCP_E:

            if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
            {
                for (i=0; i<6; i++) {
                    cpssOsMemSet(&udbpArr[i].nibbleMaskArr, 0x1, sizeof(GT_BOOL)*4);
                }
                /* 21 User-Defined Byte Pair; 16 bit UDBP */
                udbpArr[0].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[0].offset = 0; /* macDA [47:32] */
                udbpArr[1].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[1].offset = 2; /* macDA [31:16] */
                udbpArr[2].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[2].offset = 4; /* macDA [15:0] */
                udbpArr[3].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[3].offset = 6; /* macSA [47:32] */
                udbpArr[4].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[4].offset = 8; /* macSA [31:16] */
                udbpArr[5].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[5].offset = 10; /* macSA [15:0] */
                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,udbpArr, GT_FALSE);
                if(GT_OK != rc)
                {
                    return rc;
                }

                for (i=0; i<11; i++) {
                    cpssOsMemSet(&udbpArr[i].nibbleMaskArr, 0x1, sizeof(GT_BOOL)*4);
                }

                /* 21 User-Defined Byte Pair; 16 bit UDBP */
                udbpArr[0].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E; 
                udbpArr[0].offset = 16; /* IPv4 DIP [31:16] - byte offset 16 (bytes 16,17) */
                udbpArr[1].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[1].offset = 18; /* IPv4 DIP [15:0] - byte offset 18 (bytes 18,19) */
                udbpArr[2].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[2].offset = 12; /* IPv4 SIP [31:16] - bytes offset 12 (bytes 12,13) */
                udbpArr[3].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[3].offset = 14; /* IPv4 SIP [15:0] - bytes offset 14 (bytes 14,15) */
                udbpArr[4].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[4].offset = 8; /* IPv4 protocol - byte offset 8 (bytes 8,9)*/                 
                udbpArr[5].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[5].offset = 0; /* macDA [47:32] */
                udbpArr[6].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[6].offset = 2; /* macDA [31:16] */
                udbpArr[7].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[7].offset = 4; /* macDA [15:0] */
                udbpArr[8].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[8].offset = 6; /* macSA [47:32] */
                udbpArr[9].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[9].offset = 8; /* macSA [31:16] */
                udbpArr[10].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[10].offset = 10; /* macSA [15:0] */

                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,udbpArr, GT_TRUE);
                if(GT_OK != rc)
                {
                    return rc;
                }

                for (i=0; i<13; i++) {
                    cpssOsMemSet(&udbpArr[i].nibbleMaskArr, 0x1, sizeof(GT_BOOL)*4);
                }

                /* 21 User-Defined Byte Pair; 16 bit UDBP */
                udbpArr[0].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E; 
                udbpArr[0].offset = 16; /* IPv4 DIP [31:16] - byte offset 16 (bytes 16,17) */
                udbpArr[1].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[1].offset = 18; /* IPv4 DIP [15:0] - byte offset 18 (bytes 18,19) */
                udbpArr[2].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[2].offset = 12; /* IPv4 SIP [31:16] - bytes offset 12 (bytes 12,13) */
                udbpArr[3].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[3].offset = 14; /* IPv4 SIP [15:0] - bytes offset 14 (bytes 14,15) */
                udbpArr[4].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[4].offset = 8; /* IPv4 protocol - byte offset 8 (bytes 8,9)*/ 
                udbpArr[5].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E;
                udbpArr[5].offset = 2; /* IPv4 Dst port - byte offset 2 (bytes 2,3)*/ 
                udbpArr[6].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E;
                udbpArr[6].offset = 0; /* IPv4 Src port - byte offset 0 (bytes 0,1)*/ 
                udbpArr[7].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[7].offset = 0; /* macDA [47:32] */
                udbpArr[8].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[8].offset = 2; /* macDA [31:16] */
                udbpArr[9].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[9].offset = 4; /* macDA [15:0] */
                udbpArr[10].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[10].offset = 6; /* macSA [47:32] */
                udbpArr[11].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[11].offset = 8; /* macSA [31:16] */
                udbpArr[12].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[12].offset = 10; /* macSA [15:0] */

                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E,udbpArr, GT_TRUE);
                if(GT_OK != rc)
                {
                    return rc;
                }

                for (i=0; i<19; i++) {
                    cpssOsMemSet(&udbpArr[i].nibbleMaskArr, 0x1, sizeof(GT_BOOL)*4);
                }

                /* 21 User-Defined Byte Pair; 16 bit UDBP */
                udbpArr[0].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E; 
                udbpArr[0].offset = 24; /* IPv6 DIP [95:80] - bytes offset 24 (bytes 24,25) */
                udbpArr[1].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[1].offset = 26; /* IPv6 DIP [79:64] - bytes offset 26 (bytes 26,27) */
                udbpArr[2].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[2].offset = 28; /* IPv6 DIP [63:48] - bytes offset 28 (bytes 28,29) */
                udbpArr[3].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[3].offset = 30; /* IPv6 DIP [47:32] - bytes offset 30 (bytes 30,31) */
                udbpArr[4].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[4].offset = 32; /* IPv6 DIP [31:16] - bytes offset 32 (bytes 32,33) */
                udbpArr[5].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[5].offset = 34; /* IPv6 DIP [15:0] - bytes offset 34 (bytes 34,35) */
                udbpArr[6].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E; 
                udbpArr[6].offset = 8; /* IPv6 SIP [95:80] - bytes offset 8 (bytes 8,9) */
                udbpArr[7].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[7].offset = 10; /* IPv6 SIP [79:64] - bytes offset 10 (bytes 10,11) */
                udbpArr[8].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[8].offset = 12; /* IPv6 SIP [63:48] - bytes offset 12 (bytes 12,13) */
                udbpArr[9].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[9].offset = 14; /* IPv6 SIP [47:32] - bytes offset 14 (bytes 14,15) */
                udbpArr[10].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[10].offset = 16; /* IPv6 SIP [31:16] - bytes offset 16 (bytes 16,17) */
                udbpArr[11].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[11].offset = 18; /* IPv6 SIP [15:0] - bytes offset 18 (bytes 18,19) */
                udbpArr[12].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[12].offset = 8; /* IPv6 protocol - byte offset 6 (bytes 6,7)*/                  
                udbpArr[13].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[13].offset = 0; /* macDA [47:32] */
                udbpArr[14].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[14].offset = 2; /* macDA [31:16] */
                udbpArr[15].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[15].offset = 4; /* macDA [15:0] */
                udbpArr[16].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[16].offset = 6; /* macSA [47:32] */
                udbpArr[17].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[17].offset = 8; /* macSA [31:16] */
                udbpArr[18].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[18].offset = 10; /* macSA [15:0] */   
                
                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E,udbpArr, GT_FALSE);
                if(GT_OK != rc)
                {
                    return rc;
                }

                for (i=0; i<21; i++) {
                    cpssOsMemSet(&udbpArr[i].nibbleMaskArr, 0x1, sizeof(GT_BOOL)*4);
                }

                /* 21 User-Defined Byte Pair; 16 bit UDBP */
                udbpArr[0].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E; 
                udbpArr[0].offset = 24; /* IPv6 DIP [95:80] - bytes offset 24 (bytes 24,25) */
                udbpArr[1].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[1].offset = 26; /* IPv6 DIP [79:64] - bytes offset 26 (bytes 26,27) */
                udbpArr[2].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[2].offset = 28; /* IPv6 DIP [63:48] - bytes offset 28 (bytes 28,29) */
                udbpArr[3].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[3].offset = 30; /* IPv6 DIP [47:32] - bytes offset 30 (bytes 30,31) */
                udbpArr[4].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[4].offset = 32; /* IPv6 DIP [31:16] - bytes offset 32 (bytes 32,33) */
                udbpArr[5].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[5].offset = 34; /* IPv6 DIP [15:0] - bytes offset 34 (bytes 34,35) */
                udbpArr[6].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E; 
                udbpArr[6].offset = 8; /* IPv6 SIP [95:80] - bytes offset 8 (bytes 8,9) */
                udbpArr[7].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[7].offset = 10; /* IPv6 SIP [79:64] - bytes offset 10 (bytes 10,11) */
                udbpArr[8].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[8].offset = 12; /* IPv6 SIP [63:48] - bytes offset 12 (bytes 12,13) */
                udbpArr[9].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[9].offset = 14; /* IPv6 SIP [47:32] - bytes offset 14 (bytes 14,15) */
                udbpArr[10].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[10].offset = 16; /* IPv6 SIP [31:16] - bytes offset 16 (bytes 16,17) */
                udbpArr[11].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[11].offset = 18; /* IPv6 SIP [15:0] - bytes offset 18 (bytes 18,19) */
                udbpArr[12].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[12].offset = 8; /* IPv6 protocol - byte offset 6 (bytes 6,7)*/  
                udbpArr[13].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E;
                udbpArr[13].offset = 2; /* IPv4 Dst port - byte offset 2 (bytes 2,3)*/ 
                udbpArr[14].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E;
                udbpArr[14].offset = 0; /* IPv4 Src port - byte offset 0 (bytes 0,1)*/ 
                udbpArr[15].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[15].offset = 0; /* macDA [47:32] */
                udbpArr[16].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[16].offset = 2; /* macDA [31:16] */
                udbpArr[17].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[17].offset = 4; /* macDA [15:0] */
                udbpArr[18].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[18].offset = 6; /* macSA [47:32] */
                udbpArr[19].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[19].offset = 8; /* macSA [31:16] */
                udbpArr[20].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[20].offset = 10; /* macSA [15:0] */   
                
                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E,udbpArr, GT_FALSE);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
            else
            {
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(loadBalanceMode));
            }
            break;
        case CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_PORT_BASE_E:

            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E,udbpArr, GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,udbpArr, GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E,udbpArr, GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E,udbpArr, GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,udbpArr, GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        case CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E:

            for (i=0; i<6; i++) {
                cpssOsMemSet(&udbpArr[i].nibbleMaskArr, 0x1, sizeof(GT_BOOL)*4);
            }
            /* 21 User-Defined Byte Pair; 16 bit UDBP */
            udbpArr[0].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
            udbpArr[0].offset = 0; /* macDA [47:32] */
            udbpArr[1].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
            udbpArr[1].offset = 2; /* macDA [31:16] */
            udbpArr[2].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
            udbpArr[2].offset = 4; /* macDA [15:0] */
            udbpArr[3].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
            udbpArr[3].offset = 6; /* macSA [47:32] */
            udbpArr[4].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
            udbpArr[4].offset = 8; /* macSA [31:16] */
            udbpArr[5].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
            udbpArr[5].offset = 10; /* macSA [15:0] */
            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E,udbpArr, GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,udbpArr, GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E,udbpArr, GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E,udbpArr, GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }
            rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,udbpArr, GT_FALSE);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        case CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_IP_E:
            
            if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
            {                
                for (i=0; i<5; i++) {
                    cpssOsMemSet(&udbpArr[i].nibbleMaskArr, 0x1, sizeof(GT_BOOL)*4);
                }

                /* 21 User-Defined Byte Pair; 16 bit UDBP */
                udbpArr[0].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E; 
                udbpArr[0].offset = 16; /* IPv4 DIP [31:16] - byte offset 16 (bytes 16,17) */
                udbpArr[1].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[1].offset = 18; /* IPv4 DIP [15:0] - byte offset 18 (bytes 18,19) */
                udbpArr[2].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[2].offset = 12; /* IPv4 SIP [31:16] - bytes offset 12 (bytes 12,13) */
                udbpArr[3].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[3].offset = 14; /* IPv4 SIP [15:0] - bytes offset 14 (bytes 14,15) */
                udbpArr[4].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[4].offset = 8; /* IPv4 protocol - byte offset 8 (bytes 8,9)*/ 
               
                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E,udbpArr, GT_FALSE);
                if(GT_OK != rc)
                {
                    return rc;
                }

                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,udbpArr, GT_FALSE);
                if(GT_OK != rc)
                {
                    return rc;
                }

                for (i=0; i<6; i++) {
                    cpssOsMemSet(&udbpArr[i].nibbleMaskArr, 0x1, sizeof(GT_BOOL)*4);
                }            

                /* 21 User-Defined Byte Pair; 16 bit UDBP */
                udbpArr[0].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[0].offset = 0; /* macDA [47:32] */
                udbpArr[1].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[1].offset = 2; /* macDA [31:16] */
                udbpArr[2].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[2].offset = 4; /* macDA [15:0] */
                udbpArr[3].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[3].offset = 6; /* macSA [47:32] */
                udbpArr[4].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[4].offset = 8; /* macSA [31:16] */
                udbpArr[5].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                udbpArr[5].offset = 10; /* macSA [15:0] */

                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,udbpArr, GT_FALSE);
                if(GT_OK != rc)
                {
                    return rc;
                }

                for (i=0; i<13; i++) {
                cpssOsMemSet(&udbpArr[i].nibbleMaskArr, 0x1, sizeof(GT_BOOL)*4);
                }

                /* 21 User-Defined Byte Pair; 16 bit UDBP */
                udbpArr[0].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E; 
                udbpArr[0].offset = 24; /* IPv6 DIP [95:80] - bytes offset 24 (bytes 24,25) */
                udbpArr[1].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[1].offset = 26; /* IPv6 DIP [79:64] - bytes offset 26 (bytes 26,27) */
                udbpArr[2].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[2].offset = 28; /* IPv6 DIP [63:48] - bytes offset 28 (bytes 28,29) */
                udbpArr[3].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[3].offset = 30; /* IPv6 DIP [47:32] - bytes offset 30 (bytes 30,31) */
                udbpArr[4].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[4].offset = 32; /* IPv6 DIP [31:16] - bytes offset 32 (bytes 32,33) */
                udbpArr[5].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[5].offset = 34; /* IPv6 DIP [15:0] - bytes offset 34 (bytes 34,35) */
                udbpArr[6].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E; 
                udbpArr[6].offset = 8; /* IPv6 SIP [95:80] - bytes offset 8 (bytes 8,9) */
                udbpArr[7].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[7].offset = 10; /* IPv6 SIP [79:64] - bytes offset 10 (bytes 10,11) */
                udbpArr[8].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[8].offset = 12; /* IPv6 SIP [63:48] - bytes offset 12 (bytes 12,13) */
                udbpArr[9].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[9].offset = 14; /* IPv6 SIP [47:32] - bytes offset 14 (bytes 14,15) */
                udbpArr[10].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[10].offset = 16; /* IPv6 SIP [31:16] - bytes offset 16 (bytes 16,17) */
                udbpArr[11].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[11].offset = 18; /* IPv6 SIP [15:0] - bytes offset 18 (bytes 18,19) */
                udbpArr[12].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                udbpArr[12].offset = 8; /* IPv6 protocol - byte offset 6 (bytes 6,7)*/     
                
                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E,udbpArr, GT_FALSE);
                if(GT_OK != rc)
                {
                    return rc;
                }

                rc = cpssPxIngressHashPacketTypeEntrySet(devNum,CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E,udbpArr, GT_FALSE);
                if(GT_OK != rc)
                {
                    return rc;
                }                                           
            }
            else
            {
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(loadBalanceMode));
            }            
            break;

        default:
            BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(loadBalanceMode);

    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpeTrunkPortsAdd function
* @endinternal
*
* @brief   Add ports to a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] trunkId                  -   trunk ID
* @param[in] portListPtr              -  (pointer to) list of ports to add to trunkId
* @param[in] portListLen              -  port list length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note TRUNK contains ports from same TRUNK type
*
*/
GT_STATUS cpssPxHalBpeTrunkPortsAdd
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_TRUNK_ID                           trunkId,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC    *portListPtr,
    IN  GT_U8                                 portListLen
)
{
    CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT  trunkType;
    CPSS_PX_PORTS_BMP               portsBmpGet,portsBmpDesignated,
                                    portsBmpDest,maskPortsBmp=0, portsBmp=0;
    CPSS_PX_HAL_BPE_PORT_MODE_ENT   portMode;
    GT_U32                          i,index,numOfPorts=0, numOfPortsRemote=0;
    GT_U32                          portsArr[PRV_CPSS_PX_PORTS_NUM_CNS] = {0};
    GT_U32                          cid;
    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC *chPtr;
    GT_STATUS                       rc;
    GT_BOOL                         bypassLagDesignatedBitmap;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_RANGE_MAC(trunkId);
    CPSS_NULL_PTR_CHECK_MAC(portListPtr);
    PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_MAC(devNum,trunkId);

    trunkType = devDbArr[devNum].trunkDbArr[trunkId].trunkType;
    portsBmpGet = devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum];    

    switch (trunkType) {
        case CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E:
            portMode = CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E;
            break;

        case CPSS_PX_HAL_BPE_TRUNK_CASCADE_E:
        case CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E:
            portMode = CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E;
            break;

        default:
            BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(trunkType);
    }

    /* verify only ports; build portsBmp */
    for(i = 0;i < portListLen; i++)
    {
        if(portListPtr[i].type != CPSS_PX_HAL_BPE_INTERFACE_PORT_E) {
            return GT_BAD_PARAM;
        }  

        /* check that all ports are from the same trunk type */
        if (portMode == CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E) {
            PRV_CPSS_PX_HAL_BPE_PORT_MODE_CHECK_MAC(portListPtr[i].devPort.devNum, portListPtr[i].devPort.portNum, portMode);              
        }
        else {
            PRV_CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(portListPtr[i].devPort.devNum,portListPtr[i].devPort.portNum);
        }

        /* check if this port is new in the trunk */
        if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[portListPtr[i].devPort.devNum], portListPtr[i].devPort.portNum) == 0)
        {
            if (devNum == portListPtr[i].devPort.devNum)
            {
                /* Create Array of local ports */
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp, portListPtr[i].devPort.portNum); 
            }
            else
            {              
                /* count number of remote ports */                               
                numOfPortsRemote++;           
                /* Update Array of ports in DB */
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[portListPtr[i].devPort.devNum], portListPtr[i].devPort.portNum);            
            }
        }
    }

    if (portsBmp == 0 && numOfPortsRemote == 0)
    {
        /* nothing to do */
        return GT_OK;
    }

    /* In general: For each entry in the LAG Designated Ports:
        - If port i is a non-LAG port, bit i is set to '1' --> done when port type is changed from idle to upstream/extended/cascade.
        - If port i is a LAG port, bit i is set to '1' only if it is the designated LAG port member for this
          table entry; otherwise this bit is set to '0'
    */

    /* For each port in portsBmp:
        - If the port is not a member in the trunkId -> For each entry in the LAG Designated Ports set '0'
          for all the ports that belong to trunkId; then choose a port from all the ports of trunkId to be
          designated LAG port and set it's bit to '1'.
        - If the port is already a member in the trunkId -> nothing to do.
    */

    portsBmpGet |= portsBmp;

    /* 1. create maskBitmap - for local ports in the trunk set '0'; for local ports not in the trunk set '1'
       2. count number of local ports in the trunk
       3. create array of local ports
    */
    for(i = 0; i < PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(portsBmpGet, i))
        {
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(maskPortsBmp, i);
            portsArr[numOfPorts] = i;
            numOfPorts++;
        }
        else
        {
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(maskPortsBmp, i);
        }
    }

    /* when adding Upstream port to TRUNK type Upstream: add the new Upstream port to Dst port map table;
       all the traffic to Upstream ports should be through the designated port from the TRUNK */
    if (trunkType == CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E)
    {
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,&portsBmpDest,
                                          &bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Set appropriate bits for trunkId */
        portsBmpDest |= portsBmp;

        /* Configure Dst port map table entry cid for the appropriate bits */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,portsBmpDest,bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else if ((trunkType == CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E) ||
             (trunkType == CPSS_PX_HAL_BPE_TRUNK_CASCADE_E)) {
         /* find all e-channels of trunk */
        for (chPtr = devDbArr[devNum].channelDbPtr; chPtr; chPtr = chPtr->nextChannelPtr)
        {
            if (chPtr->interfce.type == CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E && chPtr->interfce.trunkId == trunkId)
            {
                cid = chPtr->cid;

                rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS,&portsBmpDest,
                                                  &bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Set appropriate bits for trunkId */
                portsBmpDest |= portsBmp;

                /* Configure Dst port map table entry cid for the appropriate bits */
                rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS,portsBmpDest,bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }
      
    /* if no local ports in the trunk, no need to choose designated port */
    if (numOfPorts > 0)
    {
        /* Set for all entries in LAG Designated port map table */
        for (i=0; i<=PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS; i++) {

            rc = cpssPxIngressHashDesignatedPortsEntryGet(devNum,i,&portsBmpDesignated);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* mask all ports in bitmap; don't change ports that are not in bitmap */
            portsBmpDesignated &= maskPortsBmp;
            
            index = (i%numOfPorts);
            /* choose designated port and set it's bit in bitmap */
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmpDesignated, portsArr[index]);
            

            /* Configure LAG Designated port map table entries for the appropriate bit */
            rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,i,portsBmpDesignated);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
        {
            /* update number of remote ports */
            for (i=0; i<PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
            {
                if (i != devNum)
                {
                    numOfPortsRemote += devDbArr[devNum].trunkDbArr[trunkId].numOfPortsArr[i];
                }
            }            

            for (i=PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS+1; i<=PRV_CPSS_PX_LAG_DESIGNATED_TABLE_MC_MAX_CNS; i++) {

                rc = cpssPxIngressHashDesignatedPortsEntryGet(devNum,i,&portsBmpDesignated);
                if(GT_OK != rc)
                {
                    return rc;
                }

                 /* mask all local ports in bitmap; don't change ports that are not in bitmap */
                portsBmpDesignated &= maskPortsBmp;
                index = (i%(numOfPorts+numOfPortsRemote));
                if (index < numOfPorts)
                {            
                    /* choose designated local port and set it's bit in bitmap */
                    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmpDesignated, portsArr[index]);
                }

                /* Configure LAG Designated port map table entries for the appropriate bit */
                rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,i,portsBmpDesignated);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }

    /* Update DB */
    devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum] = portsBmpGet;

    return GT_OK;
}


/**
* @internal cpssPxHalBpeTrunkPortsDelete function
* @endinternal
*
* @brief   Delete ports from a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] trunkId                  -   trunk ID
* @param[in] portListPtr              -  (pointer to) list of ports to delete from trunkId
* @param[in] portListLen              -  port list length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device 
* @retval GT_BAD_STATE             - on invalid trunk 
*/
GT_STATUS cpssPxHalBpeTrunkPortsDelete
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_TRUNK_ID                           trunkId,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC    *portListPtr,
    IN  GT_U8                                 portListLen
)
{
    CPSS_PX_PORTS_BMP               portsBmpGet,portsBmpDesignated,
                                    portsBmpDest,maskPortsBmp=0, portsBmp=0;
    GT_U32                          i,index,numOfPorts=0, numOfPortsRemote=0;
    GT_U32                          portsArr[PRV_CPSS_PX_PORTS_NUM_CNS] = {0};
    GT_U32                          cid;
    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC *chPtr;
    GT_STATUS                       rc;
    GT_BOOL                         bypassLagDesignatedBitmap;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_RANGE_MAC(trunkId);
    CPSS_NULL_PTR_CHECK_MAC(portListPtr);

    if (devDbArr[devNum].trunkDbArr[trunkId].valid == GT_FALSE)
    {
        return GT_BAD_STATE;
    }

    portsBmpGet = devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum];

    /* verify only ports; build portsBmp */
    for(i = 0;i < portListLen; i++)
    {
        if(portListPtr[i].type != CPSS_PX_HAL_BPE_INTERFACE_PORT_E) {
            return GT_BAD_PARAM;
        }

        /* check if this port exists in the trunk */
        if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[portListPtr[i].devPort.devNum], portListPtr[i].devPort.portNum))
        {
            if (devNum == portListPtr[i].devPort.devNum)
            {
                /* Create Array of local ports */
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp, portListPtr[i].devPort.portNum); 
            }
            else
            {              
                /* count number of remote ports */                               
                numOfPortsRemote++;           
                /* Update Array of ports in DB */
                PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[portListPtr[i].devPort.devNum], portListPtr[i].devPort.portNum);            
            }
        }          
    }

    if (portsBmp == 0 && numOfPortsRemote == 0)
    {
        /* nothing to do */
        return GT_OK;
    }

    /* In general: For each entry in the LAG Designated Ports:
        - If port i is a non-LAG port, bit i is set to '1' --> done when port type is changed from idle to upstream/extended/cascade.
        - If port i is a LAG port, bit i is set to '1' only if it is the designated LAG port member for this
          table entry; otherwise this bit is set to '0'
    */

    /* For each port in portsBmp:
        - If the port is not a member in the trunkId -> For each entry in the LAG Designated Ports set '0'
          for all the ports that belong to trunkId; then choose a port from all the ports of trunkId to be
          designated LAG port and set it's bit to '1'.
        - If the port is already a member in the trunkId -> nothing to do.
    */

    portsBmpGet &= ~portsBmp;

    /* 1. create maskBitmap - for ports in the trunk set '0'; for ports not in the trunk set '1'
       2. count number of ports in the trunk
       3. create array of local ports
    */
    for(i = 0; i < PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        if(PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(portsBmpGet, i))
        {
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_CLEAR_MAC(maskPortsBmp, i);
            portsArr[numOfPorts] = i;
            numOfPorts++;
        }
        else
        {
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(maskPortsBmp, i);
        }
    }

    /* when removing Upstream port from TRUNK type Upstream: delete the new Upstream port from Dst port map table;
       all the traffic to Upstream ports should be through the designated port from the TRUNK */
    if (devDbArr[devNum].trunkDbArr[trunkId].trunkType == CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E)
    {
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,&portsBmpDest,
                                          &bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Set appropriate bits for trunkId */
        portsBmpDest &= ~portsBmp;

        /* Configure Dst port map table entry cid for the appropriate bits */
        rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS,portsBmpDest,bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }
    } 
    else if ((devDbArr[devNum].trunkDbArr[trunkId].trunkType == CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E) ||
             (devDbArr[devNum].trunkDbArr[trunkId].trunkType == CPSS_PX_HAL_BPE_TRUNK_CASCADE_E))
    {
         /* find all e-channels of trunk */
        for (chPtr = devDbArr[devNum].channelDbPtr; chPtr; chPtr = chPtr->nextChannelPtr)
        {
            if (chPtr->interfce.type == CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E && chPtr->interfce.trunkId == trunkId)
            {
                cid = chPtr->cid;
                rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS,&portsBmpDest,
                                                  &bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* Set appropriate bits for trunkId */
                portsBmpDest &= ~portsBmp;

                /* Configure Dst port map table entry cid for the appropriate bits */
                rc = cpssPxIngressPortMapEntrySet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,cid+PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS,portsBmpDest,bypassLagDesignatedBitmap);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }   
   
    /* if no local ports in the trunk, no need to choose designated port */
    if (numOfPorts > 0)
    {
        /* Set for all entries in LAG Designated port map table */
        for (i=0; i<=PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS; i++) {

            rc = cpssPxIngressHashDesignatedPortsEntryGet(devNum,i,&portsBmpDesignated);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* mask all ports in bitmap; don't change ports that are not in bitmap */
            portsBmpDesignated &= maskPortsBmp;
            /* add all deleted ports from trunk */
            portsBmpDesignated |= portsBmp;
                        
            index = (i%numOfPorts);
            /* choose designated port and set it's bit in bitmap */
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmpDesignated, portsArr[index]);            

            /* Configure LAG Designated port map table entries for the appropriate bit */
            rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,i,portsBmpDesignated);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
        {
            /* update number of remote ports */
            for (i=0; i<PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
            {
                if (i != devNum)
                {
                    numOfPortsRemote += devDbArr[devNum].trunkDbArr[trunkId].numOfPortsArr[i];
                }
            }        

            for (i=PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS+1; i<=PRV_CPSS_PX_LAG_DESIGNATED_TABLE_MC_MAX_CNS; i++) {

                rc = cpssPxIngressHashDesignatedPortsEntryGet(devNum,i,&portsBmpDesignated);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* mask all ports in bitmap; don't change ports that are not in bitmap */
                portsBmpDesignated &= maskPortsBmp;
                /* add all deleted local ports from trunk */
                portsBmpDesignated |= portsBmp;
               
                index = (i%(numOfPorts+numOfPortsRemote));
                if (index < numOfPorts)
                {
                    /* choose designated port and set it's bit in bitmap */
                    PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmpDesignated, portsArr[index]);
                }
                
                /* Configure LAG Designated port map table entries for the appropriate bit */
                rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum,i,portsBmpDesignated);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }

    /* Update DB */
    devDbArr[devNum].trunkDbArr[trunkId].portsBmpArr[devNum] = portsBmpGet;

    return GT_OK;
}

/**
* @internal cpssPxHalBpeMulticastEChannelCreate function
* @endinternal
*
* @brief   Create Multicast E-Channel and add interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] cid                      -   Multicast E-Channel Id (APPLICABLE RANGE: maxBpeUnicastEChannels..maxBpeUnicastEChannels+maxBpeMulticastEChannels-1)
* @param[in] interfaceListPtr         -   (pointer to) interface list
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note cid of MC: <GRP[1:0],CID[11:0]>; GRP>0
*
*/
GT_STATUS cpssPxHalBpeMulticastEChannelCreate
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              cid,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfaceListPtr
)
{
    GT_STATUS         rc;
    CPSS_PX_PORTS_BMP portsBmp=0;
    GT_U32            grp;


    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(interfaceListPtr);
   
    grp = ((cid & BIT_12) | (cid & BIT_13)) >> 12;
    
    if ( grp == 0 ||
         cid > devDbArr[devNum].maxBpeMulticastEChannels + devDbArr[devNum].maxBpeUnicastEChannels - 1 ||
         cid < devDbArr[devNum].maxBpeUnicastEChannels )    
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(cid));
    }

    /* limitation of dst portmap table is (0..8K-1) */
    if (cid+devDbArr[devNum].mcDstIndexCns > _8K - 1)
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(cid));
    }

    while (interfaceListPtr->type != CPSS_PX_HAL_BPE_INTERFACE_NONE_E)
    {
        if (interfaceListPtr->type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E)
        {
            /* all ports should be local */
            if (devNum != interfaceListPtr->devPort.devNum)
            {
                BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(interfaceListPtr->devPort.devNum));
            }
            /* check that port is EXTENDED/CASCADE */
            PRV_CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(interfaceListPtr->devPort.devNum, interfaceListPtr->devPort.portNum);
            PRV_CPSS_PX_HAL_PORTS_BMP_PORT_SET_MAC(portsBmp, interfaceListPtr->devPort.portNum);
        }
        else
        {
            PRV_CPSS_PX_HAL_BPE_TRUNK_CHECK_MAC(devNum,interfaceListPtr->trunkId);
            /* check that trunk is EXTENDED/CASCADE */            
            PRV_CPSS_PX_HAL_BPE_TRUNK_MODE_EXTENDED_OR_CASCADE_CHECK_MAC(devNum, interfaceListPtr->trunkId);
            portsBmp |= devDbArr[devNum].trunkDbArr[interfaceListPtr->trunkId].portsBmpArr[devNum];
        }

        /* move to the next interface */
        interfaceListPtr ++;
    }

    /* Update Dest PortMap Table */
    rc = cpssPxIngressPortMapEntrySet(devNum, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, cid+devDbArr[devNum].mcDstIndexCns, portsBmp,GT_FALSE);
    if(GT_OK != rc)
    {
        return rc;
    }

    devDbArr[devNum].numBpeMulticastEChannel++;

    return GT_OK;
}


/**
* @internal cpssPxHalBpeMulticastEChannelUpdate function
* @endinternal
*
* @brief   Update Multicast E-Channel and add interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] cid                      -   Multicast E-Channel Id (APPLICABLE RANGE: maxBpeUnicastEChannels..maxBpeUnicastEChannels+maxBpeMulticastEChannels-1)
* @param[in] interfaceListPtr         -   (pointer to) interface list to replace
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note cid of MC: <GRP[1:0],CID[11:0]>; GRP>0
*
*/
GT_STATUS cpssPxHalBpeMulticastEChannelUpdate
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U32                              cid,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *interfaceListPtr
)
{
    return prvCpssPxHalBpeMulticastEChannelCreate(devNum,cid,interfaceListPtr,GT_FALSE);
}

/**
* @internal cpssPxHalBpeMulticastEChannelDelete function
* @endinternal
*
* @brief   Delete multicast E-Channel.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -       device number
* @param[in] cid                      -   Multicast E-Channel Id (APPLICABLE RANGE: maxBpeUnicastEChannels..maxBpeUnicastEChannels+maxBpeMulticastEChannels-1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note cid of MC: <GRP[1:0],CID[11:0]>; GRP>0
*
*/
GT_STATUS cpssPxHalBpeMulticastEChannelDelete
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cid
)
{
    GT_STATUS         rc;
    GT_U32            grp;        

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    grp = ((cid & BIT_12) | (cid & BIT_13)) >> 12;
 
    if ( grp == 0 ||
         cid > devDbArr[devNum].maxBpeMulticastEChannels + devDbArr[devNum].maxBpeUnicastEChannels - 1 ||
         cid < devDbArr[devNum].maxBpeUnicastEChannels )
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(cid));
    }

    /* limitation of dst portmap table is (0..8K-1) */
    if (cid+devDbArr[devNum].mcDstIndexCns > _8K-1)
    {
        BPE_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(cid));
    }

    /* Update Dest PortMap Table */
    rc = cpssPxIngressPortMapEntrySet(devNum, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, cid+devDbArr[devNum].mcDstIndexCns, 0,GT_FALSE);
    if(GT_OK != rc)
    {
        return rc;
    }

    devDbArr[devNum].numBpeMulticastEChannel--;

    return GT_OK;
}

/**
* @internal cpssPxHalBpeDataControlQosRuleAdd function
* @endinternal
*
* @brief   Add or replace the existing rule (of any type) with the given index for Extended port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] ruleIndex                -   index of the rule (0 .. CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS)
* @param[in] qosMapPtr                -   qos map
* @param[in] encapsulationType        -   encapsulation type
* @param[in] maskPtr                  -   mask
* @param[in] patternPtr               -   pattern
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeDataControlQosRuleAdd
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_U32                                      ruleIndex,
    IN  CPSS_PX_HAL_BPE_QOSMAP_DATA_STC            *qosMapPtr,
    IN  CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_TYPE_ENT   encapsulationType,
    IN  CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC   *maskPtr,
    IN  CPSS_PX_HAL_BPE_CTRL_DATA_CLASSIFIER_STC   *patternPtr
)
{
    GT_STATUS   rc;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData,maskData;
    CPSS_PX_COS_FORMAT_ENTRY_STC                cosFormatEntry;
    CPSS_PX_UDB_PAIR_DATA_STC                   udbPairsArr[CPSS_PX_UDB_PAIRS_MAX_CNS];
    CPSS_PX_PACKET_TYPE                         internalRuleIndex;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC             ip2meEntry;    
    GT_U32                                      entryIndex;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(qosMapPtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);
    CPSS_NULL_PTR_CHECK_MAC(patternPtr);
    CPSS_PARAM_CHECK_MAX_MAC(qosMapPtr->newPcp,BIT_3);
    CPSS_PARAM_CHECK_MAX_MAC(qosMapPtr->newDei,BIT_1);
    CPSS_PARAM_CHECK_MAX_MAC(qosMapPtr->queue,BIT_3);
    CPSS_PARAM_CHECK_MAX_MAC(ruleIndex,CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS);

    /* convert ruleIndex 0..CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS-1 to internalRuleIndex 9..29 */
    internalRuleIndex = ruleIndex + PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E;

    cpssOsMemSet(&keyData,0,sizeof(keyData));
    cpssOsMemSet(&maskData,0,sizeof(maskData));
    cpssOsMemSet(&udbPairsArr,0,sizeof(udbPairsArr));

    cpssOsMemCpy(&keyData.macDa, &(patternPtr->macDa), sizeof(GT_ETHERADDR));
    cpssOsMemCpy(&maskData.macDa, &(maskPtr->macDa), sizeof(GT_ETHERADDR));
    keyData.profileIndex = PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_EXTENDED_E;
    maskData.profileIndex = 0x7F;

    /* UDBP[0] - byte offset 0 (bytes 0,1) - IP version and DSCP */
    /* UDBP[1] - byte offset 6 (bytes 6,7) - IPv6 next header (protocol) */
    /* UDBP[2] - byte offset 8 (bytes 8,9) - IPv4 protocol */
    /* UDBP[3] - byte offset 0 (bytes 0,1) - L3 EtherType (for packet EthV2)/ LLC-SNAP PID (for packet LLC-SNAP)/
                                             DSAP-SSAP (for packet LLC-NON-SNAP) */

    /* UDBP[3] - If the packet is EthernetV2:
                 EtherType is set to the 16-bit value at the EtherType/Len offset.
                 L3 offset is set to the EtherType/Len offset.
               - If the packet is LLC/SNAP:
                 EtherType is set to the 16-bit value at the EtherType/Len offset + 8B.
                 L3 offset is set to the EtherType/Len offset + 8B.
               - If the packet is LLC/Non-SNAP:
                 EtherType is the 16-bit DSAP-SSAP at EtherType/Len offset + 2B.
                 L3 offset is set to the EtherType/Len offset + 2B */

    switch (encapsulationType)
    {
        case CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ANY_E:
            break;

        case CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV4_E:
            if (maskPtr->dip.u32Ip != 0) {                
                if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
                {
                    /* IP2ME entry index for ruleIndex exist in internal DB */
                    if (devDbArr[devNum].ip2MeIndex[ruleIndex] != 0)
                    {
                        cpssOsMemSet(&ip2meEntry, 0, sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC));
                        ip2meEntry.isIpv6 = GT_FALSE;
                        cpssOsMemCpy(&ip2meEntry.ipAddr.ipv4Addr, &(patternPtr->dip), sizeof(GT_IPADDR));
                        rc = ipv4Mask2PrefixLen(maskPtr->dip, &ip2meEntry.prefixLength);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                        ip2meEntry.valid = GT_TRUE;

                        /* Set IP2ME table entry */
                        rc = cpssPxIngressIp2MeEntrySet(devNum,devDbArr[devNum].ip2MeIndex[ruleIndex],&ip2meEntry);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        for (entryIndex=1; entryIndex<=7; entryIndex++)
                        {
                            /* Get IP2ME table entry */
                            rc = cpssPxIngressIp2MeEntryGet(devNum,entryIndex,&ip2meEntry);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                            if (ip2meEntry.valid == GT_FALSE)
                            {
                                break;
                            }
                        }
                        if (entryIndex == 8)
                        {
                            BPE_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(encapsulationType));  
                        }

                        cpssOsMemSet(&ip2meEntry, 0, sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC));
                        ip2meEntry.isIpv6 = GT_FALSE;
                        cpssOsMemCpy(&ip2meEntry.ipAddr.ipv4Addr, &(patternPtr->dip), sizeof(GT_IPADDR));
                        rc = ipv4Mask2PrefixLen(maskPtr->dip, &ip2meEntry.prefixLength);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                        ip2meEntry.valid = GT_TRUE;

                        /* Set IP2ME table entry */
                        rc = cpssPxIngressIp2MeEntrySet(devNum,entryIndex,&ip2meEntry);
                        if(GT_OK != rc)
                        {                            
                            return rc;
                        }
                        /* save IP2ME entry index into internal DB */
                        devDbArr[devNum].ip2MeIndex[ruleIndex] = entryIndex;

                        keyData.ip2meIndex = entryIndex;
                        maskData.ip2meIndex = 0x7;
    
                    }
                }
                else
                {
                    BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(encapsulationType));  
                }
            }
            else /* maskPtr->dip.u32Ip == 0 */
            {
                if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
                {
                    /* IP2ME entry index for ruleIndex exist in internal DB; free this entry */
                    if (devDbArr[devNum].ip2MeIndex[ruleIndex] != 0)
                    {                        
                        cpssOsMemSet(&ip2meEntry, 0, sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC)); 
                        ip2meEntry.prefixLength = 1;         
                        ip2meEntry.valid = GT_FALSE;

                        /* Set IP2ME table entry */
                        rc = cpssPxIngressIp2MeEntrySet(devNum,devDbArr[devNum].ip2MeIndex[ruleIndex],&ip2meEntry);
                        if(GT_OK != rc)
                        {                            
                            return rc;
                        }

                        /* free IP2ME entry index from internal DB */
                        devDbArr[devNum].ip2MeIndex[ruleIndex] = 0;
                    }
                }
            }
            CPSS_PARAM_CHECK_MAX_MAC(patternPtr->dscp,BIT_6);
            CPSS_PARAM_CHECK_MAX_MAC(maskPtr->dscp,BIT_6);
            CPSS_PARAM_CHECK_MAX_MAC(patternPtr->ipProtocol,BIT_8);
            CPSS_PARAM_CHECK_MAX_MAC(maskPtr->ipProtocol,BIT_8);

            /* UDB[0] - IP version = 4 */
            /* UDB[1] - DSCP */
            keyData.udbPairsArr[0].udb[0] = 0x40;
            maskData.udbPairsArr[0].udb[0] = 0xF0;
            keyData.udbPairsArr[0].udb[1] = (patternPtr->dscp << 2);
            maskData.udbPairsArr[0].udb[1] = (maskPtr->dscp << 2);

            /* UDB[0] - unused */
            /* UDB[1] - IPv4 protocol */
            keyData.udbPairsArr[2].udb[1] = (GT_U8) patternPtr->ipProtocol;
            maskData.udbPairsArr[2].udb[1] = (GT_U8) maskPtr->ipProtocol;
            break;

        case CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_IPV6_E:
            if ((maskPtr->dipv6.u32Ip[0] != 0) || (maskPtr->dipv6.u32Ip[1] != 0) || (maskPtr->dipv6.u32Ip[2] != 0) || (maskPtr->dipv6.u32Ip[3] != 0)) {
                if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
                {
                    /* IP2ME entry index for ruleIndex exist in internal DB */
                    if (devDbArr[devNum].ip2MeIndex[ruleIndex] != 0)
                    {
                        cpssOsMemSet(&ip2meEntry, 0, sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC));
                        ip2meEntry.isIpv6 = GT_TRUE;
                        cpssOsMemCpy(&ip2meEntry.ipAddr.ipv6Addr, &(patternPtr->dipv6), sizeof(GT_IPV6ADDR));                        
                        rc = ipv6Mask2PrefixLen(maskPtr->dipv6, &ip2meEntry.prefixLength);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                        ip2meEntry.valid = GT_TRUE;

                        /* Set IP2ME table entry */
                        rc = cpssPxIngressIp2MeEntrySet(devNum,devDbArr[devNum].ip2MeIndex[ruleIndex],&ip2meEntry);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        for (entryIndex=1; entryIndex<=7; entryIndex++)
                        {
                            /* Get IP2ME table entry */
                            rc = cpssPxIngressIp2MeEntryGet(devNum,entryIndex,&ip2meEntry);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                            if (ip2meEntry.valid == GT_FALSE)
                            {
                                break;
                            }
                        }
                        if (entryIndex == 8)
                        {
                            BPE_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(encapsulationType));  
                        }

                        cpssOsMemSet(&ip2meEntry, 0, sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC));
                        ip2meEntry.isIpv6 = GT_TRUE;
                        cpssOsMemCpy(&ip2meEntry.ipAddr.ipv6Addr, &(patternPtr->dipv6), sizeof(GT_IPV6ADDR));
                        rc = ipv6Mask2PrefixLen(maskPtr->dipv6, &ip2meEntry.prefixLength);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                        ip2meEntry.valid = GT_TRUE;

                        /* Set IP2ME table entry */
                        rc = cpssPxIngressIp2MeEntrySet(devNum,entryIndex,&ip2meEntry);
                        if(GT_OK != rc)
                        {                            
                            return rc;
                        }
                        /* save IP2ME entry index into internal DB */
                        devDbArr[devNum].ip2MeIndex[ruleIndex] = entryIndex;

                        keyData.ip2meIndex = entryIndex;
                        maskData.ip2meIndex = 0x7;
    
                    }
                }
                else
                {
                    BPE_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, BPE_LOG_ERROR_ONE_PARAM_FORMAT_MAC(encapsulationType));  
                }
            }
            else /* (maskPtr->dipv6.u32Ip[0] == 0) && (maskPtr->dipv6.u32Ip[1] == 0) && ... */
            {
                if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
                {
                    /* IP2ME entry index for ruleIndex exist in internal DB; free this entry */
                    if (devDbArr[devNum].ip2MeIndex[ruleIndex] != 0)
                    {
                        cpssOsMemSet(&ip2meEntry, 0, sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC)); 
                        ip2meEntry.prefixLength = 1;         
                        ip2meEntry.valid = GT_FALSE;

                        /* Set IP2ME table entry */
                        rc = cpssPxIngressIp2MeEntrySet(devNum,devDbArr[devNum].ip2MeIndex[ruleIndex],&ip2meEntry);
                        if(GT_OK != rc)
                        {                            
                            return rc;
                        }

                        /* free IP2ME entry index from internal DB */
                        devDbArr[devNum].ip2MeIndex[ruleIndex] = 0;
                    }
                }
            }
            CPSS_PARAM_CHECK_MAX_MAC(patternPtr->dscp,BIT_6);
            CPSS_PARAM_CHECK_MAX_MAC(maskPtr->dscp,BIT_6);
            CPSS_PARAM_CHECK_MAX_MAC(patternPtr->ipProtocol,BIT_8);
            CPSS_PARAM_CHECK_MAX_MAC(maskPtr->ipProtocol,BIT_8);

            /* UDB[0] - IP version = 6 and DSCP (upper 4 bits) */
            /* UDB[1] - DSCP (lower 2 bits) */
            keyData.udbPairsArr[0].udb[0] = 0x60;
            maskData.udbPairsArr[0].udb[0] = 0xF0;
            if (maskPtr->dscp != 0) {
                keyData.udbPairsArr[0].udb[0] |= (patternPtr->dscp >> 2);
                maskData.udbPairsArr[0].udb[0] |= (maskPtr->dscp >> 2);
                keyData.udbPairsArr[0].udb[1] = ((patternPtr->dscp & 0x3) << 6);
                maskData.udbPairsArr[0].udb[1] = ((maskPtr->dscp & 0x3) << 6);
            }

            /* UDB[0] - IPv6 next header (protocol) */
            /* UDB[1] - unused */
            keyData.udbPairsArr[1].udb[0] = (GT_U8) patternPtr->ipProtocol;
            maskData.udbPairsArr[1].udb[0] = (GT_U8) maskPtr->ipProtocol;
            break;

        case CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_ETHERNET_OR_LLC_SNAP_E:            
            keyData.isLLCNonSnap = GT_FALSE;
            maskData.isLLCNonSnap = GT_TRUE;

            /* UDB[0] - L3 EtherType (for packet EthV2)/ LLC-SNAP PID (for packet LLC-SNAP) /
                        DSAP-SSAP (for packet LLC-NON-SNAP) (upper 8 bits) */
            /* UDB[1] - L3 EtherType (for packet EthV2)/ LLC-SNAP PID (for packet LLC-SNAP) /
                        DSAP-SSAP (for packet LLC-NON-SNAP) (lower 8 bits) */                                
            keyData.udbPairsArr[3].udb[0] = (GT_U8) (patternPtr->etherType >> 8);
            maskData.udbPairsArr[3].udb[0] = (GT_U8) (maskPtr->etherType >> 8);
            keyData.udbPairsArr[3].udb[1] = (GT_U8) (patternPtr->etherType & 0xFF);
            maskData.udbPairsArr[3].udb[1] = (GT_U8) (maskPtr->etherType & 0xFF);
            break;
        case CPSS_PX_HAL_BPE_CTRL_DATA_ENCAP_LLC_NOT_SNAP_E:            
            keyData.isLLCNonSnap = GT_TRUE;
            maskData.isLLCNonSnap = GT_TRUE;

            /* UDB[0] - L3 EtherType (for packet EthV2)/ LLC-SNAP PID (for packet LLC-SNAP) /
                        DSAP-SSAP (for packet LLC-NON-SNAP) (upper 8 bits) */
            /* UDB[1] - L3 EtherType (for packet EthV2)/ LLC-SNAP PID (for packet LLC-SNAP) /
                        DSAP-SSAP (for packet LLC-NON-SNAP) (lower 8 bits) */                                
            keyData.udbPairsArr[3].udb[0] = (GT_U8) (patternPtr->etherType >> 8);
            maskData.udbPairsArr[3].udb[0] = (GT_U8) (maskPtr->etherType >> 8);
            keyData.udbPairsArr[3].udb[1] = (GT_U8) (patternPtr->etherType & 0xFF);
            maskData.udbPairsArr[3].udb[1] = (GT_U8) (maskPtr->etherType & 0xFF);
            break;
        default:
            BPE_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(encapsulationType);
            break;
    }

    /* Set packet type key table */
    rc = cpssPxIngressPacketTypeKeyEntrySet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[internalRuleIndex],&keyData,&maskData);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Set CoS Mode and Attributes per Packet Type */
    cpssOsMemSet(&cosFormatEntry,0,sizeof(cosFormatEntry));
    cosFormatEntry.cosMode = CPSS_PX_COS_MODE_FORMAT_ENTRY_E;
    cosFormatEntry.cosAttributes.trafficClass = qosMapPtr->queue;
    cosFormatEntry.cosAttributes.dropPrecedence = (qosMapPtr->newDei == 0) ? CPSS_DP_GREEN_E : CPSS_DP_YELLOW_E;
    cosFormatEntry.cosAttributes.dropEligibilityIndication = qosMapPtr->newDei;
    cosFormatEntry.cosAttributes.userPriority = qosMapPtr->newPcp;
    rc = cpssPxCosFormatEntrySet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[internalRuleIndex],&cosFormatEntry);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Enable packet type table lookup */
    rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[internalRuleIndex],GT_TRUE);
    if(GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpeDataControlQosRuleDelete function
* @endinternal
*
* @brief   Delete the rule with the given index.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] ruleIndex                -   index of the rule (0 .. CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeDataControlQosRuleDelete
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      ruleIndex
)
{
    GT_STATUS   rc;
    CPSS_PX_PACKET_TYPE                         internalRuleIndex;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC             ip2meEntry;   

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(ruleIndex,CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS);

    /* convert ruleIndex 0..CPSS_PX_HAL_BPE_CTRL_DATA_RULE_MAX_CNS-1 to internalRuleIndex 9..29 */
    internalRuleIndex = ruleIndex + PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E;

    /* Disable packet type table lookup */
    rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[internalRuleIndex],GT_FALSE);
    if(GT_OK != rc)
    {
        return rc;
    }

    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        if (devDbArr[devNum].ip2MeIndex[ruleIndex])
        {
            cpssOsMemSet(&ip2meEntry,0,sizeof(CPSS_PX_INGRESS_IP2ME_ENTRY_STC));              
            ip2meEntry.prefixLength = 1;
            ip2meEntry.valid = GT_FALSE;
                   
            /* Unset IP2ME table entry */
            rc = cpssPxIngressIp2MeEntrySet(devNum,devDbArr[devNum].ip2MeIndex[ruleIndex],&ip2meEntry);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    /* delete IP2ME entry index for ruleIndex from internal DB */
    devDbArr[devNum].ip2MeIndex[ruleIndex] = 0;

    return GT_OK;
}

/**
* @internal cpssPxHalBpeSendPacketToInterface function
* @endinternal
*
* @brief   Send a packet to egress interface
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*                                      pcktParamsPtr- The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] buffList[]               - The packet data buffers list.
* @param[in] buffLenList[]            - A list of the buffers len in buffList.
* @param[in] numOfBufs                - Length of buffList.
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_HW_ERROR              - when after transmission last descriptor own bit wasn't
*                                       changed for long time.
* @retval GT_BAD_PARAM             - the data buffer is longer than allowed.
*                                       Buffer data can occupied up to the maximum number of descriptors defined.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
*/
GT_STATUS cpssPxHalBpeSendPacketToInterface
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           txQueue,
    IN  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC *egressInterface,
    IN  GT_U8            *buffList[],
    IN  GT_U32           buffLenList[],
    IN  GT_U32           numOfBufs
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PX_NET_TX_PARAMS_STC       pcktParams;
    GT_U32                          pcid;

    pcktParams.recalcCrc = GT_TRUE;
    pcktParams.txQueue   = txQueue;

    if(egressInterface->type != CPSS_PX_HAL_BPE_INTERFACE_PORT_E)
    {
        return GT_BAD_PARAM;
    }

    if(egressInterface->devPort.devNum != devNum)
    {
        return GT_BAD_PARAM;
    }

    pcid = egressInterface->devPort.portNum;

    /* Build the E-tag */
    cpssOsMemMove((GT_VOID *)(buffList[0]+12+8), (GT_VOID *)(buffList[0]+12), buffLenList[0]-12);
    buffList[0][12] = 0x89;
    buffList[0][13] = 0x3F;
    buffList[0][14] = 0xF0;
    buffList[0][15] = 0x00;
    buffList[0][16] = 0x00;
    buffList[0][17] = (GT_U8)(pcid & 0x0F);
    buffList[0][18] = 0x00;
    buffList[0][19] = 0x00;

    buffLenList[0] += 8;

    rc = cpssPxNetIfSdmaSyncTxPacketSend(devNum,
                                         &pcktParams,
                                         buffList,
                                         buffLenList,
                                         numOfBufs);
    buffLenList[0] -= 8;

    cpssOsMemMove((GT_VOID *)(buffList[0]+12), (GT_VOID *)(buffList[0]+12+8), buffLenList[0]-12);

    return rc;
}

/**
* @internal cpssPxHalBpeRxPacketGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] rxQueue                  - The queue from which this packet was received (APPLICABLE RANGES: 0..7).
* @param[in,out] numOfBuffPtr             Num of buffs in packetBuffsArrPtr.
*
* @param[out] ingressInterface         - the interface packet was recieved
* @param[in,out] numOfBuffPtr             - Num of used buffs in packetBuffsArrPtr.
* @param[out] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[out] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,rxQueue.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeRxPacketGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_U32                               rxQueue,
    OUT  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  *ingressInterface,
    INOUT GT_U32                             *numOfBuffPtr,
    OUT GT_U8                                *packetBuffsArrPtr[],
    OUT GT_U32                               buffLenArr[]
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    pcid;

    rc = cpssPxNetIfSdmaRxPacketGet(devNum,
                                    rxQueue,
                                    numOfBuffPtr,
                                    packetBuffsArrPtr,
                                    buffLenArr);
    if(rc == GT_OK) {
        /* Packet must be received with e-tag */
        if((packetBuffsArrPtr[0][12] != 0x89) || (packetBuffsArrPtr[0][13] != 0x3F))
        {
            return GT_DSA_PARSING_ERROR;
        }
        /* Parse the E-TAG and build the Ingress Interface */
        pcid = ((packetBuffsArrPtr[0][16] & 0xF) << 8) | (packetBuffsArrPtr[0][17] & 0xFF);

        ingressInterface->type = CPSS_PX_HAL_BPE_INTERFACE_PORT_E;
        ingressInterface->devPort.devNum = devNum;
        ingressInterface->devPort.portNum = pcid;

        /* Remove the E-tag */
        cpssOsMemMove((GT_VOID *)(packetBuffsArrPtr[0]+12), (GT_VOID *)(packetBuffsArrPtr[0]+12+8), buffLenArr[0]-12-8);
        buffLenArr[0] -= 8;
    }

    return rc;
}

/**
* @internal cpssPxHalBpeRxBufFree function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received (APPLICABLE RANGES: 0..7).
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] buffListLen              - Length of rxBufList.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeRxBufFree
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           rxQueue,
    IN GT_U8            *rxBuffList[],
    IN GT_U32           buffListLen
)
{
    return cpssPxNetIfRxBufFree(devNum,rxQueue,rxBuffList,buffListLen);
}


/**
* @internal cpssPxHalBpeCncDump function
* @endinternal
*
* @brief   Display CNC information for all clients.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] displayAll               -   GT_TRUE:  display all entries
*                                      GT_FALSE: display only valid entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
*/
GT_STATUS cpssPxHalBpeCncDump
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         displayAll
)
{
    GT_STATUS   rc;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    /* Dump CNC block 0 */
    if (PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].cncBitmap, 0) == GT_TRUE) {
        rc = prvCpssPxHalBpeCncDumpClient(devNum, devDbArr[devNum].cncClientConfig[0], 0, displayAll);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Dump CNC block 1 */
    if (PRV_CPSS_PX_HAL_PORTS_BMP_IS_PORT_SET_MAC(devDbArr[devNum].cncBitmap, 1) == GT_TRUE) {
        rc = prvCpssPxHalBpeCncDumpClient(devNum, devDbArr[devNum].cncClientConfig[1], 1, displayAll);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxHalBpeIngressTablesDump function
* @endinternal
*
* @brief   Display Ingress tables information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] displayAll               -   GT_TRUE:  display all entries
*                                      GT_FALSE: display only valid entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeIngressTablesDump
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         displayAll
)
{
    GT_STATUS rc;
    GT_U32 i,max;
    CPSS_PX_INGRESS_TPID_ENTRY_STC tpidEntry;
    GT_U16 etherType;
    GT_BOOL valid, enable;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC keyData,keyMask;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;
    CPSS_PX_PORTS_BMP portsBmp;
    GT_BOOL bypassLagDesignatedBitmap;
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC ip2meEntry;
    GT_U32 ecid;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    cpssOsPrintf("\n**************************************************\n");
    cpssOsPrintf("             Ingress TPID Table dump\n");
    cpssOsPrintf("**************************************************\n\n");
    cpssOsPrintf("\n index    value    size    valid");
    cpssOsPrintf("\n---------------------------------");
    for (i=0; i<CPSS_PX_INGRESS_TPID_TABLE_ENTRIES_MAX_CNS; i++)
    {
        rc = cpssPxIngressTpidEntryGet(devNum,i,&tpidEntry);
        if(GT_OK != rc)
        {
            return rc;
        }
        if (displayAll || tpidEntry.valid)
        {
            cpssOsPrintf("\n %-2d\t  0x%-5x   %-2d      %s",i,tpidEntry.val,tpidEntry.size,(tpidEntry.valid ? "T":"F"));
        }
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**************************************************\n");
    cpssOsPrintf("             Ingress EtherType Table dump\n");
    cpssOsPrintf("**************************************************\n\n");
    cpssOsPrintf("\n index    etherType    valid");
    cpssOsPrintf("\n-----------------------------");
    for (i=0; i<CPSS_PX_INGRESS_ETHERTYPE_LAST_E; i++)
    {
        rc = cpssPxIngressEtherTypeGet(devNum,i,&etherType,&valid);
        if(GT_OK != rc)
        {
            return rc;
        }
        if (displayAll || valid)
        {
            cpssOsPrintf("\n %-2d\t  0x%-5x\t%s",i,etherType,(valid ? "T":"F"));
        }
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n*******************************************************\n");
    cpssOsPrintf("             Ingress Port Packet Type Key Table dump\n");
    cpssOsPrintf("*******************************************************\n\n");
    cpssOsPrintf("\n portNum    src port profile     UDBP0       UDBP1       UDBP2       UDBP3");
    cpssOsPrintf("\n---------------------------------------------------------------------------");
    for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        rc = cpssPxIngressPortPacketTypeKeyGet(devNum,i,&portKey);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-2d\t\t%-10s\t{%-3s,%-2d}    {%-3s,%-2d}    {%-3s,%-2d}    {%-3s,%-2d}",
                     i,srcPortProfileStr(portKey.srcPortProfile),
                     udbAnchorTypeStr(portKey.portUdbPairArr[0].udbAnchorType),portKey.portUdbPairArr[0].udbByteOffset,
                     udbAnchorTypeStr(portKey.portUdbPairArr[1].udbAnchorType),portKey.portUdbPairArr[1].udbByteOffset,
                     udbAnchorTypeStr(portKey.portUdbPairArr[2].udbAnchorType),portKey.portUdbPairArr[2].udbByteOffset,
                     udbAnchorTypeStr(portKey.portUdbPairArr[3].udbAnchorType),portKey.portUdbPairArr[3].udbByteOffset);
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n*******************************************************\n");
    cpssOsPrintf("             Ingress Packet Type Key Table dump\n");
    cpssOsPrintf("*******************************************************\n\n");
    cpssOsPrintf("\n packetType       Data:       macDa         etherType   isLLCNonSnap    profileIndex     UDBP0      UDBP1      UDBP2      UDBP3      ip2meIndex");
    cpssOsPrintf("\n                  Mask:       macDa         etherType   isLLCNonSnap    profileIndex     UDBP0      UDBP1      UDBP2      UDBP3      ip2meIndex");
    cpssOsPrintf("\n------------------------------------------------------------------------------------------------------------------------------------------------");
    for (i=0; i<CPSS_PX_PACKET_TYPE_MAX_CNS; i++)
    {
        rc = cpssPxIngressPacketTypeKeyEntryEnableGet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[i],&enable);
        if((GT_OK != rc) || (enable == GT_FALSE && displayAll == GT_FALSE))
        {
            continue;
        }
        rc = cpssPxIngressPacketTypeKeyEntryGet(devNum, CPSS_PX_HAL_BPE_PKT_TYPE_ARR[i], &keyData, &keyMask);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-10s\t\t%02x:%02x:%02x:%02x:%02x:%02x    0x%-5x\t     %s\t\t    %-10d\t{%-2x,%-2x}    {%-2x,%-2x}    {%-2x,%-2x}    {%-2x,%-2x}    %d",
                     packetTypeStr(i),keyData.macDa.arEther[0],keyData.macDa.arEther[1],
                     keyData.macDa.arEther[2],keyData.macDa.arEther[3],keyData.macDa.arEther[4],keyData.macDa.arEther[5],
                     keyData.etherType, (keyData.isLLCNonSnap ? "T":"F"),keyData.profileIndex,
                     keyData.udbPairsArr[0].udb[0],keyData.udbPairsArr[0].udb[1],
                     keyData.udbPairsArr[1].udb[0],keyData.udbPairsArr[1].udb[1],
                     keyData.udbPairsArr[2].udb[0],keyData.udbPairsArr[2].udb[1],
                     keyData.udbPairsArr[3].udb[0],keyData.udbPairsArr[3].udb[1],
                     keyData.ip2meIndex);
        cpssOsPrintf("\n\t\t\t%02x:%02x:%02x:%02x:%02x:%02x    0x%-5x\t     %s\t\t    %-10d\t{%-2x,%-2x}    {%-2x,%-2x}    {%-2x,%-2x}    {%-2x,%-2x}    %d",
                     keyMask.macDa.arEther[0],keyMask.macDa.arEther[1],
                     keyMask.macDa.arEther[2],keyMask.macDa.arEther[3],keyMask.macDa.arEther[4],keyMask.macDa.arEther[5],
                     keyMask.etherType, (keyMask.isLLCNonSnap ? "T":"F"),keyMask.profileIndex,
                     keyMask.udbPairsArr[0].udb[0],keyMask.udbPairsArr[0].udb[1],
                     keyMask.udbPairsArr[1].udb[0],keyMask.udbPairsArr[1].udb[1],
                     keyMask.udbPairsArr[2].udb[0],keyMask.udbPairsArr[2].udb[1],
                     keyMask.udbPairsArr[3].udb[0],keyMask.udbPairsArr[3].udb[1],
                     keyMask.ip2meIndex);

    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Ingress Port Map Packet Type Source Format Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n packetType     BitField #1     BitField #2     BitField #3     BitField #4     indexConst  indexMax");
    cpssOsPrintf("\n-----------------------------------------------------------------------------------------------------");
    for (i=0; i<CPSS_PX_PACKET_TYPE_MAX_CNS; i++)
    {
        rc = cpssPxIngressPacketTypeKeyEntryEnableGet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[i],&enable);
        if((GT_OK != rc) || (enable == GT_FALSE && displayAll == GT_FALSE))
        {
            continue;
        }
        rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[i],&packetTypeFormat);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-10s\t{%-2d,%-1d,%-1d}\t{%-2d,%-1d,%-1d}\t{%-2d,%-1d,%-1d}\t{%-2d,%-1d,%-1d}\t    %-5d      %-5d",
                     packetTypeStr(i),
                     packetTypeFormat.bitFieldArr[0].byteOffset,packetTypeFormat.bitFieldArr[0].startBit,packetTypeFormat.bitFieldArr[0].numBits,
                     packetTypeFormat.bitFieldArr[1].byteOffset,packetTypeFormat.bitFieldArr[1].startBit,packetTypeFormat.bitFieldArr[1].numBits,
                     packetTypeFormat.bitFieldArr[2].byteOffset,packetTypeFormat.bitFieldArr[2].startBit,packetTypeFormat.bitFieldArr[2].numBits,
                     packetTypeFormat.bitFieldArr[3].byteOffset,packetTypeFormat.bitFieldArr[3].startBit,packetTypeFormat.bitFieldArr[3].numBits,
                     packetTypeFormat.indexConst,packetTypeFormat.indexMax);

    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("      Ingress Port Filtering Table dump (default: 0x1FFFF)\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n index     portsBmp");
    cpssOsPrintf("\n--------------------");
    for (i=0; i<_4K; i++)
    {
        rc = cpssPxIngressPortMapEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,i,&portsBmp,
                                          &bypassLagDesignatedBitmap);
        if(GT_OK != rc)
        {
            return rc;
        }
        if (displayAll || portsBmp != 0x1FFFF)
        {
            cpssOsPrintf("\n %-4d\t   0x%-16x",i,portsBmp);                     
        }

    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**************************************************************************\n");
    cpssOsPrintf("             Ingress Port Map Packet Type Destination Format Table dump\n");
    cpssOsPrintf("**************************************************************************\n\n");
    cpssOsPrintf("\n packetType     BitField #1     BitField #2     BitField #3     BitField #4     indexConst  indexMax");
    cpssOsPrintf("\n-----------------------------------------------------------------------------------------------------");
    for (i=0; i<CPSS_PX_PACKET_TYPE_MAX_CNS; i++)
    {
        rc = cpssPxIngressPacketTypeKeyEntryEnableGet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[i],&enable);
        if((GT_OK != rc) || (enable == GT_FALSE && displayAll == GT_FALSE))
        {
            continue;
        }
        rc = cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[i],&packetTypeFormat);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-10s\t{%-2d,%-1d,%-1d}\t{%-2d,%-1d,%-1d}\t{%-2d,%-1d,%-1d}\t{%-2d,%-1d,%-1d}\t    %-5ld      %-5d",
                     packetTypeStr(i),
                     packetTypeFormat.bitFieldArr[0].byteOffset,packetTypeFormat.bitFieldArr[0].startBit,packetTypeFormat.bitFieldArr[0].numBits,
                     packetTypeFormat.bitFieldArr[1].byteOffset,packetTypeFormat.bitFieldArr[1].startBit,packetTypeFormat.bitFieldArr[1].numBits,
                     packetTypeFormat.bitFieldArr[2].byteOffset,packetTypeFormat.bitFieldArr[2].startBit,packetTypeFormat.bitFieldArr[2].numBits,
                     packetTypeFormat.bitFieldArr[3].byteOffset,packetTypeFormat.bitFieldArr[3].startBit,packetTypeFormat.bitFieldArr[3].numBits,
                     packetTypeFormat.indexConst,packetTypeFormat.indexMax);

    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Ingress Port Map Destination Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n index     portsBmp   bypassLAG    description");
    cpssOsPrintf("\n-------------------------------------------------");
    for (i=0; i<_8K; i++)
    {
        if ((i == PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS) ||
            (i == devDbArr[devNum].ucDstIndexCns + devDbArr[devNum].maxBpeUnicastEChannels)) {
            cpssOsPrintf("\n-------------------------------------------------");
        }
        rc = cpssPxIngressPortMapEntryGet(devNum, CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, i, &portsBmp, &bypassLagDesignatedBitmap); 
        if(GT_OK != rc)
        {
            return rc;
        }
        if (displayAll || portsBmp)
        {
            if (i < PRV_CPSS_HAL_BPE_NUM_RESERVED_ENTRIES_CNS) {
                cpssOsPrintf("\n %-4d\t   0x%05x\t %s\t %s", i, portsBmp, (bypassLagDesignatedBitmap ? "T" : "F"), reservedDestinationStr(i));
            }
            else if (i < devDbArr[devNum].ucDstIndexCns + devDbArr[devNum].maxBpeUnicastEChannels) {
                cpssOsPrintf("\n %-4d\t   0x%05x\t %s\t UC E-CID %d", i, portsBmp, (bypassLagDesignatedBitmap ? "T" : "F"),
                             (i - devDbArr[devNum].ucDstIndexCns));
            }
            else {
                /* convert from consecutive to base format: <GRP,ECID_base> -> <GRP,ECID_ext,ECID_base> */
                ecid = (i - devDbArr[devNum].mcDstIndexCns);
                cpssOsPrintf("\n %-4d\t   0x%05x\t %s\t MC E-CID %d (%d)", i, portsBmp, (bypassLagDesignatedBitmap ? "T" : "F"), 
                             ecid, (((ecid & 0x3000) << 8) | (ecid & 0x000FFF)));
            }
        }

    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Lag Designated Portmap Table dump (default: 0x1FFFF)\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n index     portsBmp");
    cpssOsPrintf("\n--------------------");
    max = (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum) ? PRV_CPSS_PX_LAG_DESIGNATED_TABLE_MC_MAX_CNS : PRV_CPSS_PX_LAG_DESIGNATED_TABLE_UC_MAX_CNS);   
    for (i=0; i<=max; i++)
    {
        rc = cpssPxIngressHashDesignatedPortsEntryGet(devNum,i,&portsBmp);
        if(GT_OK != rc)
        {
            return rc;
        }
        if (displayAll || portsBmp != 0x1FFFF)
        {
            cpssOsPrintf("\n %-4d\t   0x%-16x",i,portsBmp);                     
        }

    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Ingress Forwarding PortMap Exception Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n TableType   PortBmp ");
    cpssOsPrintf("\n----------------------");
    rc = cpssPxIngressForwardingPortMapExceptionGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,&portsBmp);
    if(GT_OK != rc)
    {
        return rc;
    }
    cpssOsPrintf("\n %s\t      0x%-16x","SRC",portsBmp);
    rc = cpssPxIngressForwardingPortMapExceptionGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,&portsBmp);
    if(GT_OK != rc)
    {
        return rc;
    }
    cpssOsPrintf("\n %s\t      0x%-16x","DST",portsBmp);              

    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Ingress PacketType Key Entry Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n packetType     Status");
    cpssOsPrintf("\n-----------------------");
    for (i=0; i<CPSS_PX_PACKET_TYPE_MAX_CNS; i++)
    {
        rc = cpssPxIngressPacketTypeKeyEntryEnableGet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[i],&enable);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-10s\t%s",packetTypeStr(i),(enable ? "Enable":"Disable"));

    }

    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {        
        cpssOsPrintf("\n**********************************************************************\n");
        cpssOsPrintf("             Ingress IP2ME Table dump\n");
        cpssOsPrintf("**********************************************************************\n\n");
        cpssOsPrintf("\n index     valid    prefixLength     isIpv6      ipAddr");
        cpssOsPrintf("\n-------------------------------------------------------");
        for (i=1; i<=7; i++)
        {
            rc = cpssPxIngressIp2MeEntryGet(devNum,i,&ip2meEntry);           
            if((GT_OK != rc) || (ip2meEntry.valid == GT_FALSE && displayAll == GT_FALSE))
            {
                continue;
            }
            cpssOsPrintf("\n %-4d\t     %s\t\t%-4d\t\t%s",i,(ip2meEntry.valid ? "T":"F"),ip2meEntry.prefixLength,(ip2meEntry.isIpv6 ? "T":"F"));
            if (ip2meEntry.isIpv6 == GT_FALSE)
            {
                cpssOsPrintf("\t%d.%d.%d.%d",ip2meEntry.ipAddr.ipv4Addr.arIP[0],ip2meEntry.ipAddr.ipv4Addr.arIP[1],
                                             ip2meEntry.ipAddr.ipv4Addr.arIP[2],ip2meEntry.ipAddr.ipv4Addr.arIP[3]);
            }
            else
            {
                cpssOsPrintf("\t%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                             ip2meEntry.ipAddr.ipv6Addr.arIP[0],ip2meEntry.ipAddr.ipv6Addr.arIP[1],
                             ip2meEntry.ipAddr.ipv6Addr.arIP[2],ip2meEntry.ipAddr.ipv6Addr.arIP[3],
                             ip2meEntry.ipAddr.ipv6Addr.arIP[4],ip2meEntry.ipAddr.ipv6Addr.arIP[5],
                             ip2meEntry.ipAddr.ipv6Addr.arIP[6],ip2meEntry.ipAddr.ipv6Addr.arIP[7],
                             ip2meEntry.ipAddr.ipv6Addr.arIP[8],ip2meEntry.ipAddr.ipv6Addr.arIP[9],
                             ip2meEntry.ipAddr.ipv6Addr.arIP[10],ip2meEntry.ipAddr.ipv6Addr.arIP[11],
                             ip2meEntry.ipAddr.ipv6Addr.arIP[12],ip2meEntry.ipAddr.ipv6Addr.arIP[13],
                             ip2meEntry.ipAddr.ipv6Addr.arIP[14],ip2meEntry.ipAddr.ipv6Addr.arIP[15]);

            }
        }             
    }
    cpssOsPrintf("\n\n");

    return GT_OK;
}

/**
* @internal cpssPxHalBpeIngressErrorsDump function
* @endinternal
*
* @brief   Display Ingress Errors information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeIngressErrorsDump
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS rc;
    GT_U32 errorCounter;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC errorKey;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    cpssOsPrintf("\nIngress Packet Type Error:\n");
    cpssOsPrintf("\n ErrorCounter   ErrorKey:       macDa           etherType   isLLCNonSnap    profileIndex     UDBP0       UDBP1       UDBP2       UDBP3");
    cpssOsPrintf("\n---------------------------------------------------------------------------------------------------------------------------------------");
    rc = cpssPxIngressPacketTypeErrorGet(devNum,&errorCounter,&errorKey);
    if(GT_OK != rc)
    {
        return rc;
    }
    if (errorCounter != 0)
    {
        cpssOsPrintf("\n %-10d\t\t    %02x:%02x:%02x:%02x:%02x:%02x    0x%-5x         %s\t    %-10d\t{%-3s,%-2d}    {%-3s,%-2d}    {%-3s,%-2d}    {%-3s,%-2d}",
                     errorCounter,errorKey.macDa.arEther[0],errorKey.macDa.arEther[1],
                     errorKey.macDa.arEther[2],errorKey.macDa.arEther[3],errorKey.macDa.arEther[4],errorKey.macDa.arEther[5],
                     errorKey.etherType, (errorKey.isLLCNonSnap ? "T":"F"),errorKey.profileIndex,
                     udbAnchorTypeStr(errorKey.udbPairsArr[0].udb[0]),errorKey.udbPairsArr[0].udb[1],
                     udbAnchorTypeStr(errorKey.udbPairsArr[1].udb[0]),errorKey.udbPairsArr[1].udb[1],
                     udbAnchorTypeStr(errorKey.udbPairsArr[2].udb[0]),errorKey.udbPairsArr[2].udb[1],
                     udbAnchorTypeStr(errorKey.udbPairsArr[3].udb[0]),errorKey.udbPairsArr[3].udb[1]);
    }

    cpssOsPrintf("\n\n");

    cpssOsPrintf("\nIngress Port Map Exception Counter:\n");
    cpssOsPrintf("\n TableType   ErrorCounter ");
    cpssOsPrintf("\n---------------------------");
    rc = cpssPxIngressPortMapExceptionCounterGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,&errorCounter);
    if(GT_OK != rc)
    {
        return rc;
    }
    if (errorCounter != 0)
    {
        cpssOsPrintf("\n %s\t      %-16d","SRC",errorCounter);
    }

    rc = cpssPxIngressPortMapExceptionCounterGet(devNum,CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,&errorCounter);
    if(GT_OK != rc)
    {
        return rc;
    }
    if (errorCounter != 0)
    {
        cpssOsPrintf("\n %s\t      %-16d","DST",errorCounter);
    }

    cpssOsPrintf("\n\n");

    return GT_OK;
}

/**
* @internal cpssPxHalBpeEgressTablesDump function
* @endinternal
*
* @brief   Display Egress tables information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] displayAll               -   GT_TRUE:  display all entries
*                                      GT_FALSE: display only valid entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHalBpeEgressTablesDump
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         displayAll
)
{
    GT_STATUS rc;
    GT_U32 i,j;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT operationType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT operationInfo;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT srcInfoType;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT srcPortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT trgInfoType;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT trgPortInfo;
    CPSS_PX_PORTS_BMP portsBmp;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Egress Header Alteration Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n portNum    packetType  operationType   TPID    E_PCP   E_DEI  Ingress_E_CID_base   GRP   E_CID_base  Ingress_E_CID_ext   E_CID_ext");
    cpssOsPrintf("\n-------------------------------------------------------------------------------------------------------------------------------------");
    for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        cpssOsPrintf("\n %-2d\t    ",i);
        for (j=0; j<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; j++)
        {
            cpssOsMemSet(&operationInfo,0,sizeof(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT));

            rc = cpssPxEgressHeaderAlterationEntryGet(devNum,i,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[j],&operationType,&operationInfo);
            if(GT_OK != rc)
            {
                return rc;
            }

            cpssOsPrintf("%-10s\t%-10s\t 0x%-5x  %-3d\t  %d\t      0x%-12x %-2d       0x%-12x  0x%-8x    0x%-8x",
                         packetTypeStr(j),headerAlterationTypeStr(operationType),operationInfo.info_802_1br_E2U.eTag.TPID,
                         operationInfo.info_802_1br_E2U.eTag.E_PCP,operationInfo.info_802_1br_E2U.eTag.E_DEI,
                         operationInfo.info_802_1br_E2U.eTag.Ingress_E_CID_base,operationInfo.info_802_1br_E2U.eTag.GRP,
                         operationInfo.info_802_1br_E2U.eTag.E_CID_base,operationInfo.info_802_1br_E2U.eTag.Ingress_E_CID_ext,
                         operationInfo.info_802_1br_E2U.eTag.E_CID_ext);
            cpssOsPrintf("\n\t    ");

        }
        cpssOsPrintf("\n");
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Egress Source Port Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n portNum    infoType        PCID      srcFilteringVector");
    cpssOsPrintf("\n---------------------------------------------------------");
    for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        rc = cpssPxEgressSourcePortEntryGet(devNum,i,&srcInfoType,&srcPortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-2d\t    %-8s\t    %-12d 0x%-12x",i,sourcePortTypeStr(srcInfoType),        
                     srcPortInfo.info_802_1br.pcid,srcPortInfo.info_802_1br.srcPortInfo.srcFilteringVector);
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Egress Target Port Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n portNum      infoType        TPID        PCID    egressDelay");
    cpssOsPrintf("\n-------------------------------------------------------------");
    for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        rc = cpssPxEgressTargetPortEntryGet(devNum,i,&trgInfoType,&trgPortInfo);
        if(GT_OK != rc)
        {
            return rc;
        }
            
        cpssOsPrintf("\n %-2d\t      %-13s   0x%-5x\t  %-12d%-7d",i,targetPortTypeStr(trgInfoType),                                               
                     trgPortInfo.info_common.tpid, trgPortInfo.info_common.pcid,trgPortInfo.info_common.egressDelay); 
                
    }
    cpssOsPrintf("\n\n");


    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Egress Vlan Tag State Table dump (default: 0x0FFFF)\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n vlanId     portsBmp");
    cpssOsPrintf("\n--------------------");
    for (i=0; i<BIT_12; i++)
    {
        rc = cpssPxEgressVlanTagStateEntryGet(devNum,i,&portsBmp);
        if(GT_OK != rc)
        {
            return rc;
        }
        if (displayAll || (portsBmp != 0xFFFF))
        {
            cpssOsPrintf("\n %-4d\t   0x%04X",i,portsBmp);
        }        

    }
    cpssOsPrintf("\n\n"); 

    return GT_OK;
}

/**
* @internal cpssPxHalBpeCosTablesDump function
* @endinternal
*
* @brief   Display COS tables information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
*/
GT_STATUS cpssPxHalBpeCosTablesDump
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS rc;
    GT_U32 i,j;
    CPSS_PX_COS_FORMAT_ENTRY_STC cosFormatEntry;
    CPSS_PX_COS_ATTRIBUTES_STC cosAttributes;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             COS Format Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n packetType    mode        byteOffset  bitOffset   numOfBits   TC     DP   userPriority    DEI");
    cpssOsPrintf("\n-----------------------------------------------------------------------------------------------");
    for (i=0; i<PRV_CPSS_PX_HAL_BPE_PKT_TYPE_LAST_E; i++)
    {
        rc = cpssPxCosFormatEntryGet(devNum,CPSS_PX_HAL_BPE_PKT_TYPE_ARR[i],&cosFormatEntry);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-10s    %-10s      %d\t   %d\t\t%d\t%d    %-7s\t %d\t    %d",packetTypeStr(i),cosModeStr(cosFormatEntry.cosMode),
                     cosFormatEntry.cosByteOffset,cosFormatEntry.cosBitOffset,cosFormatEntry.cosNumOfBits,
                     cosFormatEntry.cosAttributes.trafficClass,dpLevelStr(cosFormatEntry.cosAttributes.dropPrecedence),
                     cosFormatEntry.cosAttributes.userPriority,cosFormatEntry.cosAttributes.dropEligibilityIndication);
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             COS Port Attributes Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n portNum      TC     DP   userPriority    DEI");
    cpssOsPrintf("\n--------------------------------------------------------------------------------------------");
    for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        rc = cpssPxCosPortAttributesGet(devNum,i,&cosAttributes);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-2d\t      %d    %-7s\t %d\t   %d",i,cosAttributes.trafficClass,dpLevelStr(cosAttributes.dropPrecedence),
                     cosAttributes.userPriority,cosAttributes.dropEligibilityIndication);
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             COS Port L2 Mapping Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n portNum    L2 index    TC     DP   userPriority   DEI");
    cpssOsPrintf("\n-------------------------------------------------------");
    for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        cpssOsPrintf("\n %-2d\t    ",i);
        for (j=0; j<BIT_4; j++)
        {
            rc = cpssPxCosPortL2MappingGet(devNum,i,j,&cosAttributes);
            if(GT_OK != rc)
            {
                return rc;
            }

            cpssOsPrintf("    %-2d\t%d    %-7s\t %d\t    %d",j,cosAttributes.trafficClass,dpLevelStr(cosAttributes.dropPrecedence),
                         cosAttributes.userPriority,cosAttributes.dropEligibilityIndication);
            cpssOsPrintf("\n\t    ");

        }
        cpssOsPrintf("\n");
    }
    cpssOsPrintf("\n\n");

    return GT_OK;
}

/**
* @internal cpssPxHalBpeIngressHashTablesDump function
* @endinternal
*
* @brief   Display Ingress Hash tables information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
*/
GT_STATUS cpssPxHalBpeIngressHashTablesDump
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS rc;
    GT_U32 i,j;
    CPSS_PX_PORTS_BMP portsBmp;
    CPSS_PX_INGRESS_HASH_MODE_ENT hashMode;
    GT_U32 dummy[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    GT_U32 crc32Seed;
    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT indexMode;
    CPSS_PX_INGRESS_HASH_UDBP_STC udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    GT_BOOL sourcePortEnable;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Ingress Hash Designated Port Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n index    portsBmp");
    cpssOsPrintf("\n-------------------");
    for (i=0; i<BIT_7; i++)
    {
        rc = cpssPxIngressHashDesignatedPortsEntryGet(devNum,i,&portsBmp);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-7d   0x%-16x",i,portsBmp);
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Ingress Hash Packet Type Hash Mode Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n packetType        mode");
    cpssOsPrintf("\n------------------------");
    for (i=0; i<CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E; i++)
    {
        rc = cpssPxIngressHashPacketTypeHashModeGet(devNum,i,&hashMode,&dummy[0]);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-15s   %-8s",ingressHashTypeStr(i),(hashMode == CPSS_PX_INGRESS_HASH_MODE_CRC32_E )? "CRC32":"SEL_BITS");
    }
    cpssOsPrintf("\n\n");

    rc = cpssPxIngressHashSeedGet(devNum,&crc32Seed);
    if(GT_OK != rc)
    {
        return rc;
    }
    cpssOsPrintf("\nIngress Hash Seed: [%d]\n\n",crc32Seed);

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Ingress Hash Port Index Mode Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n portNum    mode");
    cpssOsPrintf("\n-----------------");
    for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        rc = cpssPxIngressHashPortIndexModeGet(devNum,i,&indexMode);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-2d\t    %-4s",i,(indexMode == CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E )? "HASH":"PRNG");
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Ingress Hash Packet Type Table dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n packetType     sourcePortEnale     UDBP {anchor, offset, nibbleMaskEnable}");
    cpssOsPrintf("\n---------------------------------------------------------------------------");
    for (i=0; i<CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E; i++)
    {
        rc = cpssPxIngressHashPacketTypeEntryGet(devNum,i,&udbpArr[0],&sourcePortEnable);
        if(GT_OK != rc)
        {
            return rc;
        }

        cpssOsPrintf("\n %-15s      %s\t\t     ",ingressHashTypeStr(i),(sourcePortEnable ? "T":"F"));
        for (j=0; j<CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS; j++)
        {
            cpssOsPrintf("%d\t {%-3s,%-2d,%s %s %s %s}",j,udbAnchorTypeStr(udbpArr[j].anchor),udbpArr[j].offset,
                         (udbpArr[j].nibbleMaskArr[0] ? "T":"F"),(udbpArr[j].nibbleMaskArr[1] ? "T":"F"),
                         (udbpArr[j].nibbleMaskArr[2] ? "T":"F"),(udbpArr[j].nibbleMaskArr[3] ? "T":"F"));
            cpssOsPrintf("\n\t\t\t\t     ");
        }

    }
    cpssOsPrintf("\n\n");

    return GT_OK;
}


/**
* @internal cpssPxHalBpeInternalDbDump function
* @endinternal
*
* @brief   Display Internal Database information.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -   device number
* @param[in] displayAll               -   GT_TRUE:  display all entries
*                                      GT_FALSE: display only valid entries
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssPxHalBpeInternalDbDump
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         displayAll
)
{
    GT_U32 i,j,k;
    CPSS_PX_HAL_BPE_CHANNEL_INFO_STC *chPtr;

    /* Validate parameters */
    PRV_CPSS_PX_HAL_BPE_DEV_CHECK_MAC(devNum);

    if (devDbArr[devNum].valid)
    {
        cpssOsPrintf("\nnumOfUpstreamPorts: [%d]",devDbArr[devNum].numOfUpstreamPorts);
        cpssOsPrintf("\nmaxBpeUnicastEChannels: [%d]",devDbArr[devNum].maxBpeUnicastEChannels);
        cpssOsPrintf("\nmaxBpeMulticastEChannels: [%d]",devDbArr[devNum].maxBpeMulticastEChannels);
        cpssOsPrintf("\ncncBitmap: [%d]",devDbArr[devNum].cncBitmap);
        cpssOsPrintf("\ncncClientConfig[block0]: [%d]",devDbArr[devNum].cncClientConfig[0]);
        cpssOsPrintf("\ncncClientConfig[block1]: [%d]",devDbArr[devNum].cncClientConfig[1]);

        cpssOsPrintf("\n**********************************************************************\n");
        cpssOsPrintf("             Trunk Database dump\n");
        cpssOsPrintf("**********************************************************************\n\n");
        cpssOsPrintf("\n index      type        portsBmp");
        cpssOsPrintf("\n---------------------------------");
        for (i=0; i<PRV_CPSS_PX_TRUNKS_NUM_CNS; i++)
        {
            if (displayAll || devDbArr[devNum].trunkDbArr[i].portsBmpArr[devNum])
            {
                cpssOsPrintf("\n %-4d\t   %-8s\t%-16d",i,trunkTypeStr(devDbArr[devNum].trunkDbArr[i].trunkType),devDbArr[devNum].trunkDbArr[i].portsBmpArr[devNum]);
            }
        }
        cpssOsPrintf("\n\n");

        cpssOsPrintf("\n**********************************************************************\n");
        cpssOsPrintf("             Port Database dump\n");
        cpssOsPrintf("**********************************************************************\n\n");
        cpssOsPrintf("\n index      interface           mode        PCID        upstream");
        cpssOsPrintf("\n-----------------------------------------------------------------");
        for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
        {
            if (displayAll || devDbArr[devNum].portDbArr[i].interfce.type != CPSS_PX_HAL_BPE_INTERFACE_NONE_E)
            {
                cpssOsPrintf("\n %-2d\t      ",i);
                if (devDbArr[devNum].portDbArr[i].interfce.type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E)
                {
                    cpssOsPrintf("%-5s  {%-2d,%-2d}\t","PORT",devDbArr[devNum].portDbArr[i].interfce.devPort.devNum,devDbArr[devNum].portDbArr[i].interfce.devPort.portNum);
                }
                else
                {
                    cpssOsPrintf("%-5s  %-4d\t","TRUNK",devDbArr[devNum].portDbArr[i].interfce.trunkId);
                }
                cpssOsPrintf("%-8s    0x%-12x",portTypeStr(devDbArr[devNum].portDbArr[i].mode),devDbArr[devNum].portDbArr[i].pcid);
                if (devDbArr[devNum].portDbArr[i].upstream.type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E)
                {
                    cpssOsPrintf("%-5s  {%-2d,%-2d}\t","PORT",devDbArr[devNum].portDbArr[i].upstream.devPort.devNum,devDbArr[devNum].portDbArr[i].upstream.devPort.portNum);
                }
                else if(devDbArr[devNum].portDbArr[i].upstream.type == CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E)
                {
                    cpssOsPrintf("%-5s  %-4d\t","TRUNK",devDbArr[devNum].portDbArr[i].upstream.trunkId);
                }
                else
                {
                    cpssOsPrintf("%-5s  \t","NONE");
                }
            }
        }
        cpssOsPrintf("\n");
    }

    cpssOsPrintf("\n index         PCP     DEI");
    cpssOsPrintf("\n---------------------------");
    for (i=0; i<PRV_CPSS_PX_PORTS_NUM_CNS; i++)
    {
        if (displayAll || devDbArr[devNum].portDbArr[i].interfce.type != CPSS_PX_HAL_BPE_INTERFACE_NONE_E)
        {
            cpssOsPrintf("\n %-2d   default: %d       %d",i,devDbArr[devNum].portDbArr[i].portQos.defaultQos.newPcp,
                         devDbArr[devNum].portDbArr[i].portQos.defaultQos.newDei);
            for (j=0; j<CPSS_PCP_RANGE_CNS; j++)
            {
                for (k=0; k<CPSS_DEI_RANGE_CNS; k++)
                {
                    cpssOsPrintf("\n      {%d,%d}:   %d       %d",j,k,devDbArr[devNum].portDbArr[i].portQos.qosMap[j][k].newPcp,
                         devDbArr[devNum].portDbArr[i].portQos.qosMap[j][k].newDei);
                }
            }
        }
    }
    cpssOsPrintf("\n\n");

    cpssOsPrintf("\n**********************************************************************\n");
    cpssOsPrintf("             Channel Database dump\n");
    cpssOsPrintf("**********************************************************************\n\n");
    cpssOsPrintf("\n index      interface           CID");
    cpssOsPrintf("\n------------------------------------");
    i=1;
    for (chPtr = devDbArr[devNum].channelDbPtr; chPtr; chPtr = chPtr->nextChannelPtr)
    {
        cpssOsPrintf("\n %-2d\t    ",i++);
        if (chPtr->interfce.type == CPSS_PX_HAL_BPE_INTERFACE_PORT_E)
        {
            cpssOsPrintf("%-5s  {%-2d,%-2d}\t","PORT",chPtr->interfce.devPort.devNum,chPtr->interfce.devPort.portNum);
        }
        else
        {
            cpssOsPrintf("%-5s  %-4d\t","TRUNK",chPtr->interfce.trunkId);
        }
        cpssOsPrintf("0x%-12x\n",chPtr->cid);
    }
    cpssOsPrintf("\n\n");

    return GT_OK;
}



static char* srcPortProfileStr(PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_ENT profile)
{
    char *str;
    switch(profile)
    {
        case PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_IDLE_E:
            str = "IDLE";
            break;
        case PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_UPSTREAM_E:
            str = "UPSTREAM";
            break;
        case PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CASCADE_E:
            str = "CASCADE";
            break;
        case PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_EXTENDED_E:
            str = "EXTENDED";
            break;
        case PRV_CPSS_PX_HAL_BPE_SRC_PORT_PROFILE_CPU_E:
            str = "CPU";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char* udbAnchorTypeStr(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT  udbAnchorType)
{
    char *str;
    switch(udbAnchorType)
    {
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E:
            str = "L2";
            break;
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E:
            str = "L3";
            break;
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E:
            str = "L4";
            break;
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E:
            str = "PTP";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char* packetTypeStr(PRV_CPSS_PX_HAL_BPE_PKT_TYPE_ENT  packetType)
{
    if (packetType < CPSS_PX_PACKET_TYPE_MAX_CNS) {
        return packetTypeStrArr[packetType];
    }
    else {
        return packetTypeStrArr[CPSS_PX_PACKET_TYPE_MAX_CNS];
    }
}

static char* headerAlterationTypeStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  type)
{
    char *str;
    switch(type)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E:
            str = "802_1BR_E2U";
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E:
            str = "802_1BR_U2E";
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E:
            str = "802_1BR_U2C";
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E:
            str = "802_1BR_C2U";
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E:
            str = "802_1BR_U2CPU";
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E:
            str = "802_1BR_CPU2U";
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E:
            str = "DO_NOT_MODIFY";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char* sourcePortTypeStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  type)
{
    char *str;
    switch(type)
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E:
            str = "802_1BR";
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E:
            str = "NOT_USED";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char* targetPortTypeStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  type)
{
    char *str;
    switch(type)
    {
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E:
            str = "COMMON";
            break;
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E:
            str = "DO_NOT_MODIFY";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char* cosModeStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  cosMode)
{
    char *str;
    switch(cosMode)
    {
        case CPSS_PX_COS_MODE_PORT_E:
            str = "PORT";
            break;
        case CPSS_PX_COS_MODE_PACKET_L2_E:
            str = "PACKET_L2";
            break;
        case CPSS_PX_COS_MODE_PACKET_L3_E:
            str = "PACKET_L3";
            break;
        case CPSS_PX_COS_MODE_FORMAT_ENTRY_E:
            str = "FORMAT_ENT";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char* dpLevelStr(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  dpLevel)
{
    char *str;
    switch(dpLevel)
    {
        case CPSS_DP_GREEN_E:
            str = "GREEN";
            break;
        case CPSS_DP_YELLOW_E:
            str = "YELLOW";
            break;
        case CPSS_DP_RED_E:
            str = "RED";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char* ingressHashTypeStr(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT  ingressHash)
{
    char *str;
    switch(ingressHash)
    {
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E:
            str = "IPV4_TCP_UDP";
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E:
            str = "IPV6_TCP_UDP";
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E:
            str = "IPV4_NO_TCP_UDP";
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E:
            str = "IPV6_NO_TCP_UDP";
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_SINGLE_LABEL_E:
            str = "MPLS_SINGLE_LBL";
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_MULTI_LABEL_E:
            str = "MPLS_MULTI_LBL";
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E:
            str = "UDE1";
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E:
            str = "UDE2";
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E:
            str = "ETHERNET";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char* trunkTypeStr(CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT  trunkType)
{
    char *str;
    switch(trunkType)
    {
        case CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E:
            str = "UPSTREAM";
            break;
        case CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E:
        case CPSS_PX_HAL_BPE_TRUNK_CASCADE_E:
            str = "EXTENDED/CASCADE";
            break;
        case CPSS_PX_HAL_BPE_TRUNK_INTERNAL_E:
            str = "INTERNAL";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char* portTypeStr(CPSS_PX_HAL_BPE_PORT_MODE_ENT  portType)
{
    char *str;
    switch(portType)
    {
        case CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E:
            str = "IDLE";
            break;
        case CPSS_PX_HAL_BPE_PORT_MODE_TRUSTED_E:
            str = "TRUSTED";
            break;
        case CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E:
            str = "UPSTREAM";
            break;
        case CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E:
            str = "EXTENDED";
            break;
        case CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E:
            str = "CASCADE";
            break;
        case CPSS_PX_HAL_BPE_PORT_MODE_INTERNAL_E:
            str = "INTERNAL";
            break;
        default:
            str = "UNKNOWN";
    }
    return str;
}

static char * packetTypeStrArr[CPSS_PX_PACKET_TYPE_MAX_CNS+1] = {
    "U2DOWN_UC"     /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_UC_E*/,
    "U2DOWN_MC"     /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_DOWNSTREAM_MC_E*/,
    "U2CPU"         /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_UPSTREAM_TO_CPU_E*/,
    "C2U_E_TAG"     /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_E_TAGGED_E*/,
    "C2U_UNTAG"     /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CASCADE_TO_UPSTREAM_UNTAGGED_E*/,
    "E2U_E_TAG"     /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_E_TAGGED_E*/,
    "E2U_C_TAG"     /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_C_TAGGED_E*/,
    "E2U_UNTAG"     /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_EXTENDED_TO_UPSTREAM_UNTAGGED_E*/,
    "CPU2U"         /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CPU_TO_UPSTREAM_E*/,
    "RESERVED_1"    /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_1_E*/,
    "RESERVED_2"    /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_RESERVED_2_E*/,
    "DATA_0"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E*/,
    "DATA_1"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+1*/,
    "DATA_2"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+2*/,
    "DATA_3"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+3*/,
    "DATA_4"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+4*/,
    "DATA_5"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+5*/,
    "DATA_6"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+6*/,
    "DATA_7"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+7*/,
    "DATA_8"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+8*/,
    "DATA_9"        /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+9*/,
    "DATA_10"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+10*/,
    "DATA_11"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+11*/,
    "DATA_12"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+12*/,
    "DATA_13"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+13*/,
    "DATA_14"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+14*/,
    "DATA_15"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+15*/,
    "DATA_16"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+16*/,
    "DATA_17"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+17*/,
    "DATA_18"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+18*/,
    "DATA_19"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+19*/,
    "DATA_20"       /*PRV_CPSS_PX_HAL_BPE_PKT_TYPE_CTRL_DATA_FIRST_E+20*/,
    "UNKNOWN"       /*INVALID*/
};

static char* reservedDestinationStr(GT_U32  index)
{
    char *str;
    switch(index)
    {
        case PRV_CPSS_HAL_BPE_UPSTREAM_TRAFFIC_INDEX_CNS:
            str = "UPSTREAM_TRAFFIC";
            break;
        case PRV_CPSS_HAL_BPE_TRAFFIC_2_CPU_INDEX_CNS:
            str = "TO_CPU";
            break;
        case PRV_CPSS_HAL_BPE_DISCARDED_TRAFFIC_INDEX_CNS:
            str = "DISCARD";
            break;
        case PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS:
            str = "CPU_TO_PORT_0";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+1):
            str = "CPU_TO_PORT_1";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+2):
            str = "CPU_TO_PORT_2";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+3):
            str = "CPU_TO_PORT_3";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+4):
            str = "CPU_TO_PORT_4";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+5):
            str = "CPU_TO_PORT_5";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+6):
            str = "CPU_TO_PORT_6";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+7):
            str = "CPU_TO_PORT_7";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+8):
            str = "CPU_TO_PORT_8";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+9):
            str = "CPU_TO_PORT_9";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+10):
            str = "CPU_TO_PORT_10";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+11):
            str = "CPU_TO_PORT_11";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+12):
            str = "CPU_TO_PORT_12";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+13):
            str = "CPU_TO_PORT_13";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+14):
            str = "CPU_TO_PORT_14";
            break;
        case (PRV_CPSS_HAL_BPE_TRAFFIC_FROM_CPU_INDEX_CNS+15):
            str = "CPU_TO_PORT_15";
            break;
        default:
            str = "";
    }
    return str;
}

/* The function returns the position of the first (least
       significant) bit set in the word */
static GT_STATUS ipv4Mask2PrefixLen(GT_IPADDR  ipv4Mask, GT_U32 *prefixLenPtr)
{
   GT_U32 prefixLen=0;   

    if (ipv4Mask.u32Ip==0)
    {
        *prefixLenPtr = 0;
        return GT_OK;
    }
        
    ipv4Mask.u32Ip = CPSS_32BIT_BE(ipv4Mask.u32Ip);

    while (!(ipv4Mask.u32Ip & 1))
    {
        ipv4Mask.u32Ip >>= 1;
        ++prefixLen;
    }
    
    prefixLen = 32-prefixLen;        

    /* check that all the bits are set continuously */
    while (ipv4Mask.u32Ip & 1)
    {
        if (ipv4Mask.u32Ip == 1)
        {
            *prefixLenPtr = prefixLen;
            return GT_OK;
        }
        ipv4Mask.u32Ip >>=1;
    }

    return GT_NOT_SUPPORTED;    
} 

/* The function returns the position of the first (least
       significant) bit set in the word */
static GT_STATUS ipv6Mask2PrefixLen(GT_IPV6ADDR  ipv6Mask, GT_U32 *prefixLenPtr)
{
   GT_U32 prefixLen=0;
        
    if (ipv6Mask.u32Ip[0]==0 && ipv6Mask.u32Ip[1]==0 && ipv6Mask.u32Ip[2]==0 && ipv6Mask.u32Ip[3]==0)
    {
        *prefixLenPtr = 0;
        return GT_OK;    
    }

    ipv6Mask.u32Ip[0] = CPSS_32BIT_BE(ipv6Mask.u32Ip[0]);
    ipv6Mask.u32Ip[1] = CPSS_32BIT_BE(ipv6Mask.u32Ip[1]);
    ipv6Mask.u32Ip[2] = CPSS_32BIT_BE(ipv6Mask.u32Ip[2]);
    ipv6Mask.u32Ip[3] = CPSS_32BIT_BE(ipv6Mask.u32Ip[3]);

   while (!(ipv6Mask.u32Ip[0] & 1))
    {
        ipv6Mask.u32Ip[0] >>= 1;
        ++prefixLen;
    }

   if (prefixLen)
   {
       prefixLen = 32-prefixLen;       
       
       /* check that all the bits are set continuously */
       while (ipv6Mask.u32Ip[0] & 1)
       {
           if (ipv6Mask.u32Ip[0] == 1)
           {
               *prefixLenPtr = prefixLen;
               return GT_OK;
           }
           ipv6Mask.u32Ip[0] >>=1;
       } 
       return GT_NOT_SUPPORTED;  
   }

   while (!(ipv6Mask.u32Ip[1] & 1))
    {
        ipv6Mask.u32Ip[1] >>= 1;
        ++prefixLen;
    }

   if (prefixLen)
   {
       prefixLen = 64-prefixLen;  
       
       /* check that all the bits are set continuously */
       while (ipv6Mask.u32Ip[1] & 1)
       {
           if (ipv6Mask.u32Ip[1] == 1)
           {
               *prefixLenPtr = prefixLen;
               return GT_OK;
           }
           ipv6Mask.u32Ip[1] >>=1;
       } 
       return GT_NOT_SUPPORTED;               
   }

   while (!(ipv6Mask.u32Ip[2] & 1))
    {
        ipv6Mask.u32Ip[2] >>= 1;
        ++prefixLen;
    }

   if (prefixLen)
   {
       prefixLen = 96-prefixLen;        
       
       /* check that all the bits are set continuously */
       while (ipv6Mask.u32Ip[2] & 1)
       {
           if (ipv6Mask.u32Ip[2] == 1)
           {
               *prefixLenPtr = prefixLen;
               return GT_OK;
           }
           ipv6Mask.u32Ip[2] >>=1;
       } 
       return GT_NOT_SUPPORTED;       
   }

   while (!(ipv6Mask.u32Ip[3] & 1))
    {
        ipv6Mask.u32Ip[3] >>= 1;
        ++prefixLen;
    }   

   prefixLen = 128-prefixLen;

   /* check that all the bits are set continuously */
   while (ipv6Mask.u32Ip[3] & 1)
   {
       if (ipv6Mask.u32Ip[3] == 1)
       {
           *prefixLenPtr = prefixLen;
           return GT_OK;
       }
       ipv6Mask.u32Ip[3] >>=1;
   } 
   return GT_NOT_SUPPORTED;             
}
