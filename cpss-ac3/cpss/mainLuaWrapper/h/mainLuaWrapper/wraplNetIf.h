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
* @file wraplNetIf.h
*
* @brief type definitions for RX/TX
* The following types are declared here:
*
* PACKET_STC
* TX_PARAMS_STC
*
* @version   3
********************************************************************************
*/

#ifndef __wraplNetIf_h__
#define __wraplNetIf_h__

#include <cpss/common/cpssTypes.h>
#include <gtOs/gtOsMsgQ.h>
#include <cpss/common/networkIf/cpssGenNetIfTypes.h>
#ifdef CHX_FAMILY
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#endif /*CHX_FAMILY*/

/************* Defines ********************************************************/
#define PACKET_MAX_SIZE 13000

/************* Typedefs *******************************************************/
/* packet data */
typedef struct {
    int     len;
    GT_U8    data[PACKET_MAX_SIZE];
} PACKET_STC;

typedef struct {
    GT_MSGQ_ID  replyQ;
    CPSS_INTERFACE_INFO_STC srcInterface;
    PACKET_STC  packet;
} TX_PARAMS_STC;

typedef struct {
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_BOOL     isTagged;
    GT_U16      vlanId;
    GT_BOOL     packetIsLooped;

    GT_BOOL     srcIsTrunk;
    GT_TRUNK_ID srcTrunkId;

    GT_BOOL     hasCpuCode;
#ifdef CHX_FAMILY
    CPSS_NET_RX_CPU_CODE_ENT     cpuCode;
#endif /*CHX_FAMILY*/
    GT_BOOL     wasTruncated;
    GT_U32      originByteCount;
    GT_U32      timestamp;

    PACKET_STC  packet;
} RX_DATA_STC;

typedef struct {
    GT_U8       devNum;
    GT_PHYSICAL_PORT_NUM   portNum;
    GT_U32      originByteCount;
    GT_U32      timestamp;
    PACKET_STC  packet;
} PX_RX_DATA_STC;

#endif /* __wraplNetIf_h__ */

