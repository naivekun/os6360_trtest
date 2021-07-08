
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
* @file prvTgfCscdDsaTxMirrVidx.h
*
* @brief check DSA fields <Analyzer Use eVIDX>, <Analyzer eVIDX>
* of a tx-mirrored packet.
* 1) configure a tx-mirroring on ePort mapped to VIDX.
* 2) move a physical port (called here an "vidx-port") to this VIDX.
* 3) set HOP-BY-HOP forwarding mode. Assign ePort to a global tx Analyzer
* 4) generate traffic on rx port and check egressed packet on the
* vidx-port in three cases:
* 4.1) rx port is network port, analyzer port is cascade port
* 4.2) rx port is cascade port, analyzer port is cascade port
* 4.3) rx port is cascade port, analyzer port is network port
* 5) disable global tx Analyzer interface.
* 6) Setup END-TO-END forwarding mode. Make ePort to be an analyzer
* for tx port.
* 7) repeat steps 4.1 - 4.3. Before step 4.2 reconfigure analyzer for
* tx port to be a port!=vidx-port to be sure that mirroring is
* performed according to eDSA <VIDX>/<use eVIDX> settings
* rather than local setting. Packet should be received on vidx-port
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCscdDsaTxMirrVidx
#define __prvTgfCscdDsaTxMirrVidx

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdDsaTxMirrVidxTest function
* @endinternal
*
* @brief   Check correctress of eDSA fields <rx_sniff>, <Src Dev>, <src phy port>,
*         <SrcTrg Tagged> inside a packet rx-mirrored to a remote port.
*/
GT_VOID prvTgfCscdDsaTxMirrVidxTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdDsaTxMirrVidx */

