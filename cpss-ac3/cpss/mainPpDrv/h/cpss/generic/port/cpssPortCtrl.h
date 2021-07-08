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
* @file cpssPortCtrl.h
*
* @brief CPSS definitions for port configurations.
*
* @version   32
********************************************************************************
*/

#ifndef __cpssPortCtrlh
#define __cpssPortCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/port/cpssPortCtrl.h>
/**
* @enum CPSS_PORT_MRU_SIZE_ENT
 *
 * @brief Enumeration of Maximal Receive packet sizes
*/
typedef enum{

    /** Accepts packets up to 1518 bytes in length */
    CPSS_PORT_MRU_1518_E = 0,

    /** Accepts packets up to 1522 bytes in length */
    CPSS_PORT_MRU_1522_E,

    /** Accepts packets up to 1526 bytes in length */
    CPSS_PORT_MRU_1526_E,

    /** Accepts packets up to 1536 bytes in length */
    CPSS_PORT_MRU_1536_E,

    /** Accepts packets up to 1552 bytes in length */
    CPSS_PORT_MRU_1552_E,

    /** Accepts packets up to 1632 bytes in length */
    CPSS_PORT_MRU_1632_E,

    /** Accepts packets up to 1664 bytes in length */
    CPSS_PORT_MRU_1664_E,

    /** Accepts packets up to 2048 bytes in length */
    CPSS_PORT_MRU_2048_E,

    /** Accepts packets up to 9022 bytes in length */
    CPSS_PORT_MRU_9022_E,

    /** Accepts packets up to 9192 bytes in length */
    CPSS_PORT_MRU_9192_E,

    /** Accepts packets up to 9216 bytes in length */
    CPSS_PORT_MRU_9216_E,

    /** Accepts packets up to 9220 bytes in length */
    CPSS_PORT_MRU_9220_E,

    /** Accepts packets up to 10240 bytes in length */
    CPSS_PORT_MRU_10240_E

} CPSS_PORT_MRU_SIZE_ENT;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPortCtrlh */


