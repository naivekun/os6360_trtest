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
* @file prvTgfCscdDsaTxMirrAnInfo.h
*
* @brief check eDSA (TO_ANALYZER) fields inside a tx-mirrored packet
* - <Analyzer phy port is valid>
* - <Analyzer target device>
* - <Analyzer target physical port>
* - <Analyzer target ePort>
* Test check 3 cases in two modes HOP_BY_HOP and END_TO_END
* 1) rx port is a network port and the mirrored packet egresses
* to cascade port.
* 2) rx port is a cascade port and the mirrored packet egresses
* to cascade port.
* 3) rx port is a casccade port and the mirrored packet egresses to
* network port (target analyzer port).
* In 1st case we check three subcases: analyzer ePort is consequentially
* mapped to local port, to remote port and to trunk (with a remote port
* as trunk's member)
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCscdDsaTxMirrAnInfo
#define __prvTgfCscdDsaTxMirrAnInfo

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCscdDsaTxMirrAnInfoTest function
* @endinternal
*
* @brief   Check correctress of next eDSA fields inside a tx-mirrored packet
*         - <Analyzer phy port is valid>
*         - <Analyzer target device>
*         - <Analyzer target physical port>
*         - <Analyzer target ePort>
*/
GT_VOID prvTgfCscdDsaTxMirrAnInfoTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCscdDsaTxMirrAnInfo */

