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
* @file wraplTgfTypes.h
*
* @brief Type definitions for RX/TX
* The following types are declared:
*
* TGF_PACKET_STC
* TGF_EXPECTED_EGRESS_INFO_STC
*
* @version   1
********************************************************************************
*/

#ifndef __wraplTgfTypes_h__
#define __wraplTgfTypes_h__

#include <cpss/common/cpssTypes.h>
#include <gtOs/gtOsMsgQ.h>
#include <cpss/common/networkIf/cpssGenNetIfTypes.h>
#ifdef CHX_FAMILY
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#endif /*CHX_FAMILY*/

/* Maximal egress ports to be inspected */
#define PRV_LUA_TGF_EGRESS_PORTS_ARR_MAX    4

use_prv_struct(TGF_PACKET_STC)
use_prv_struct(TGF_EXPECTED_EGRESS_INFO_STC)

/*************************************************************************
* prv_gc_TGF_PACKET_STC
*
* Description:
*       Garbage collector for TGF_PACKET_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_gc_TGF_PACKET_STC(
    TGF_PACKET_STC *val
);
/*************************************************************************
* prv_lua_to_c_TGF_PACKET_STC
*
* Description:
*       Convert to TGF_PACKET_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_TGF_PACKET_STC(
    lua_State *L,
    TGF_PACKET_STC *val
);

/*************************************************************************
* prv_c_to_lua_TGF_PACKET_STC
*
* Description:
*       Convert TGF_PACKET_STC to lua
*
* Parameters:
*
* Returns:
*       value at the top of stack
*
*************************************************************************/
void prv_c_to_lua_TGF_PACKET_STC(
    lua_State *L,
    TGF_PACKET_STC *val
);

/*************************************************************************
* prv_gc_TGF_EXPECTED_EGRESS_INFO_STC
*
* Description:
*       Garbage collector for TGF_EXPECTED_EGRESS_INFO_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_gc_TGF_EXPECTED_EGRESS_INFO_STC(
    TGF_EXPECTED_EGRESS_INFO_STC *val
);

/*************************************************************************
* prv_lua_to_c_TGF_EXPECTED_EGRESS_INFO_STC
*
* Description:
*       Convert to TGF_EXPECTED_EGRESS_INFO_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_TGF_EXPECTED_EGRESS_INFO_STC(
    lua_State *L,
    TGF_EXPECTED_EGRESS_INFO_STC *val
);

/*************************************************************************
* prv_c_to_lua_TGF_EXPECTED_EGRESS_INFO_STC
*
* Description:
*       Convert TGF_EXPECTED_EGRESS_INFO_STC to lua
*
* Parameters:
*
* Returns:
*       value at the top of stack
*
*************************************************************************/
void prv_c_to_lua_TGF_EXPECTED_EGRESS_INFO_STC(
    lua_State *L,
    TGF_EXPECTED_EGRESS_INFO_STC *val
);

#endif /* __wraplTgfTypes__ */

