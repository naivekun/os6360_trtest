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
* @file prvTgfBrgVplsBasicTest.h
*
* @brief basic VPLS configuration of device with 2 ports in MPLS domain , and 2 ports of different service providers.
* 1. test check 'flooding' from service provider port to the other 3 ports.(TS to the MPLS domain)
* a. check SA learning (reassigned src eport)
* 2. test check 'flooding' from mpls domain port only to service provider ports (filter the other MPLS domain port). (TT of ingress packet - Ethernet passenger)
* a. check SA learning (reassigned src eport)
* 3. test 'Known UC' from service provider port to MPLS domain.
* 3. test 'Known UC' from MPLS domain port to service provider port.
*
*
* presentation of system is in Documentum :
* Cabinets/SWITCHING/CPSS and PSS/R&D Internal/CPSS/sip5 device (Lion3,BC2)/Test design/VPLS/Bobcat2_VPLS_Demo.pptx
* http://docil.marvell.com/webtop/drl/objectId/0900dd88801954ba
*
* ePort A is assigned for EFP1 - port to Ethernet domain - service provider 1 with outer vlan tag 10
* ePort B is assigned for EFP2 - port to Ethernet domain - service provider 2 with outer vlan tag 20
* ePort C is assigned for PW1 - port to MPLS domain , LABEL 50 - PW port 1 - PW label 60
* ePort D is assigned for PW2 - port to MPLS domain , LABEL 50 - PW port 2 - PW label 70
*
* eVlan - VPLS domain - the eVlan that represents the 'VPLS domain'
*
* @version   2.
********************************************************************************
*/
#ifndef __prvTgfBrgVplsBasicTest
#define __prvTgfBrgVplsBasicTest

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfBrgVplsBasicTest function
* @endinternal
*
* @brief   see file description.
*/
GT_VOID prvTgfBrgVplsBasicTest
(
    GT_VOID
);

/**
* @internal prvTgfBrgVplsBasicTest1 function
* @endinternal
*
* @brief   similar to file prvTgfBrgVplsBasicTest.c but with next changes:
*         1. supports 3 modes :
*         a. 'pop tag' - the mode that is tested in prvTgfBrgVplsBasicTest.c
*         b. 'raw mode' + 'QinQ terminal'(delete double Vlan)
*         c. 'tag mode' + 'add double vlan tag'
*         2. in RAW mode ethernet packets come with 2 tags and the passenger on MPLS
*         tunnels is without vlan tags.
*         3. in TAG mode the ethernet packets come with one tag but considered untagged
*         when become passenger on MPLS tunnels and so added additional 2 vlan tags.
*/
GT_VOID prvTgfBrgVplsBasicTest1
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgVplsBasicTest */



