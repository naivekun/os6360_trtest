/*******************************************************************************
*              Copyright 2003, RADLAN Computer Communications, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* GALILEO TECHNOLOGY, INC. (GTI) AND RADLAN Computer Communications, LTD.
********************************************************************************
*/
/**
********************************************************************************
* @file miiInf.c
*
* @brief Interface for ethernet communication to MII-interface based ASICS.
*
* @version   1
********************************************************************************
*/

#include "miiInf.h"
#include "miiInf.h"


#define MII_PORT         1

/**
* @internal miiInfInit function
* @endinternal
*
* @brief   This function inits the internal data structure of the ethernet port
*/
GT_STATUS miiInfInit
(
    void
)
{
    return  GT_OK;
}

/**
* @internal miiInfOutput function
* @endinternal
*
* @brief   This function transmits an Ethenet packet to the Packet processor
*
* @param[in] header_PTR               - pointer the the 14 bytes packet header.
* @param[in] paylaod_PTR              - pointer to the reset of the packet.
* @param[in] payloadLen               - length ot the payload.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS miiInfOutput
(
    IN GT_U8    *header_PTR,
    IN GT_U8    *paylaod_PTR,
    IN GT_U32    payloadLen
)
{
    return GT_OK;
}

/**
* @internal miiInfInputHookAdd function
* @endinternal
*
* @brief   This bind the user Rx callback
*
* @param[in] userRxFunc               - the user Rx callbak function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS miiInfInputHookAdd
(
    IN GT_Rx_FUNCP userRxFunc
)
{
    return GT_OK;
}

/**
* @internal miiInfInputHookDelete function
* @endinternal
*
* @brief   This unbinds the user Rx callback
*/
GT_STATUS miiInfInputHookDelete
(
    void
)
{
    return GT_OK;
}

