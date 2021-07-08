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
* @file prvTgfPclEgrOrgVid.h
*
* @brief Ingress PCL Double tagged packet Egress Pcl Original VID field
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclEgrOrgVid
#define __prvTgfPclEgrOrgVid

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclEgrOrgVidTrafficTest function
* @endinternal
*
* @brief   Full Egress Original VID field test
*/
GT_VOID prvTgfPclEgrOrgVidTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEgrExtIpv4RaclVaclSipDipTest function
* @endinternal
*
* @brief   Egress PASS/DROP test on SIp and DIP of key : Egress Ext Ipv4 Racl/Vacl
*/
GT_VOID prvTgfPclEgrExtIpv4RaclVaclSipDipTest
(
    GT_VOID
);

/**
* @internal prvTgfPclEgrUltraIpv6RaclVaclSipDipTest function
* @endinternal
*
* @brief   Egress PASS/DROP test on SIP and DIP of key : Egress Ultra Ipv6 Racl/Vacl
*/
GT_VOID prvTgfPclEgrUltraIpv6RaclVaclSipDipTest
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclEgrOrgVid */

