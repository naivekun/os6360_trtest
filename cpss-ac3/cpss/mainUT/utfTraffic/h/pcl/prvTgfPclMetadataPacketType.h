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
* @file prvTgfPclMetadataPacketType.h
*
* @brief IPCL Metadata Packet Type/Applicable Flow Sub-template test header file
*
* @version   2
********************************************************************************
*/
#ifndef __prvTgfPclMetadataPacketType
#define __prvTgfPclMetadataPacketType

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclMetadataPacketTypeTest function
* @endinternal
*
* @brief   IPCL Metadata Ingress UDB Packet Type/Applicable Flow Sub-template test
*/
GT_VOID prvTgfPclMetadataPacketTypeTest
(
    GT_VOID
);

/**
* @internal prvTgfPclMetadataPacketTypeIpOverMplsTest function
* @endinternal
*
* @brief   IPCL Metadata Ingress UDB Packet Type/Applicable Flow Sub-template test
*/
GT_VOID prvTgfPclMetadataPacketTypeIpOverMplsTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclMetadataPacketType */

