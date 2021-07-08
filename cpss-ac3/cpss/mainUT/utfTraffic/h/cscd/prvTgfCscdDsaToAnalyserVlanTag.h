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
* @file prvTgfCscdDsaToAnalyserVlanTag.h
*
* @brief Test a eVLAN CFI, UP, TPID fields in the eDsa tag (To Analyser command, Rx mirroring).
*
* Case0: The packet is sent from a network port to a cascade port
* Ingress port mirrored and not cascaded.
* MAC_DA learned in FDB to egress port different from Rx Analyzer.
* Rx Analyzer port is cascaded and captured
* Ckeck eDSA tag of packet egressed from Rx Analyzer.
*
* Case1: The packet is sent from a cascade port to a cascade port
* No FDB enries and no mirrored ports.
* Rx Analyzer configured on remote device.
* Remote device mapped to cascaded egress port.
* Egress port is checked: the packet must be egressed unchanged.
*
* Case2: The packet is sent from a cascade port to a network port
* No FDB enries and no mirrored ports.
* Egress port configured as Rx Analyzer.
* Egress port is checked: the packet must be egressed wthout eDSA tag.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCscdDsaToAnalyserVlanTag
#define __prvTgfCscdDsaToAnalyserVlanTag

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagNetworkToCascaded function
* @endinternal
*
* @brief   Test sending packet to network port and checking the Cascaded RX Analyser
*         output eDSA tag.
*/
GT_VOID prvTgfCscdDsaToAnalyzerVlanTagNetworkToCascaded
(
    GT_VOID
);

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagCascadedToNetwork function
* @endinternal
*
* @brief   Test sending packet to Cascaded port and checking the network RX Analyser
*         output VLAN tag.
*/
GT_VOID prvTgfCscdDsaToAnalyzerVlanTagCascadedToNetwork
(
    GT_VOID
);

/**
* @internal prvTgfCscdDsaToAnalyzerVlanTagCascadedToCascaded function
* @endinternal
*
* @brief   Test sending packet to Cascaded port and checking the Cascaded RX Analyser
*         output eDSA tag.
*/
GT_VOID prvTgfCscdDsaToAnalyzerVlanTagCascadedToCascaded
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdDsaToAnalyserVlanTag */


