
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
* @file prvTgfCscdDsaTxMirrSrcFlds.h
*
* @brief Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
* <SrcTrg Tagged> of a tx-mirrored packet. There are 3 cases. In every
* case we check two packets: vlan-tagged and vlan-untagged.
* 1) Configure analyzer port to be a cascade port. Send a packets to
* a network rx port. Packets will be fowrarded to remote device
* (via FDB entry, a Device Map table and a cascade tx-port). Both
* packets are vlan-tagged. Whether to remove tag will decide
* tx port.
* And will be tx-mirrored to Analyzer port.
* 2) Snd a DSA-tagged TO_ANALYZER packets to a cascade rx port.
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
#ifndef __prvTgfCscdDsaTxMirrSrcFlds
#define __prvTgfCscdDsaTxMirrSrcFlds

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdDsaTxMirrSrcFldsTest function
* @endinternal
*
* @brief   Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
*         <SrcTrg Tagged> in a packet mirrored (Tx-mirrired) to a remote port.
*/
GT_VOID prvTgfCscdDsaTxMirrSrcFldsTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdDsaTxMirrSrcFlds */

