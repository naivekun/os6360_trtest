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
* @file prvTgfPclUdb10KeyCustomer.h
*
* @brief UDB Only Keys specific PCL features testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclUdb10KeyCustomerh
#define __prvTgfPclUdb10KeyCustomerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclUdb10KeyCustomerTest function
* @endinternal
*
* @brief   Test on Ingress UDB10 Only Key with customer scenario
*
* @note Fields:
*       pclId - replacedFld.pclId  - muxed with UDB0-1
*       Evlan - replacedFld.vid   - muxed with UDB2-3
*       Eport - replacedFld.srcPort - muxed with UDB4-5
*       VID1  - replacedFld.vid1  - muxed with UDB6-7
*       UP1  - replacedFld.up1   - muxed with UDB7
*       UP0  - UDB8 L2 + 14, mask 0xE0, patten - (up0 << 5)
*       DSCP  - UDB9 L3Minus2 + 3, mask 0xFC, patten - (dscp << 2)
*       UDB mapping 30-1 => 8-9
*
*/
GT_VOID prvTgfPclUdb10KeyCustomerTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclUdb10KeyCustomerh */



