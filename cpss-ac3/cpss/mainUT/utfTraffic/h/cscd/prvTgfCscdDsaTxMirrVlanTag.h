
/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfCscdDsaTxMirrVlanTag.h
*
* @brief Check correctness of eDSA fields <eVLAN>, <CFI>, <UP>, <TPID> of
* of a tx-mirrored packet. There are 3 cases.
* 1) Configure analyzer port to be a cascade port. Send a packets to
* a network rx port. Packets will be forwarded to remote device
* (via FDB entry, a Device Map table and a cascade tx-port). Both
* packets are vlan-tagged. Whether to remove tag will decide
* tx port.
* And will be tx-mirrored to Analyzer port.
* 2) Send a DSA-tagged TO_ANALYZER packets to a cascade rx port.
* Packet should be received on the cascade analyzer port the same
* as it come to rx-port.
* 3) Setup an analyzer port to be a network port and send a DSA-tagged
* TO_ANALYZER packets on cascade rx port. Packets should be received
* on the analyzer port the same as they were sent on rx port
* in case 1.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCscdDsaTxMirrVlanTag
#define __prvTgfCscdDsaTxMirrVlanTag

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdDsaTxMirrVlanTagTest function
* @endinternal
*
* @brief   Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
*         <SrcTrg Tagged> inside a packet rx-mirrored to a remote port.
*/
GT_VOID prvTgfCscdDsaTxMirrVlanTagTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdDsaTxMirrVlanTag */

