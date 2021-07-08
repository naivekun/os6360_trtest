/*******************************************************************************
*              (c), Copyright 2011, Marvell International Ltd.                 *
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
* @file luaTbufWrapper.h
*
* @brief Lua <=> C interface for tbuf
*
* @version   2
********************************************************************************
*/
#ifndef __luaTbufWrapper_h__
#define __luaTbufWrapper_h__

#include <lua.h>
#include <generic/cider/tbuf.h>

/**
* @internal prvLuaToTbuf function
* @endinternal
*
* @brief   Get tbuf stored in Lua object placed on stack at L_index
*
* @param[in] L                        - lua state
* @param[in] L_index                  - tbuf object index in lua stack
*                                       tbuf    (tbuf.ptr == NULL if error)
*/
tbuf prvLuaToTbuf(
    lua_State       *L,
    int             L_index
);

/**
* @internal prvLuaPushTbuf function
* @endinternal
*
* @brief   Push tbuf to Lua stack
*
* @param[in] L                        - lua state
* @param[in] buf                      - t object
*                                       tbuf    (tbuf.ptr == NULL if error)
*/
void prvLuaPushTbuf(
    lua_State       *L,
    tbuf            buf
);

/**
* @internal prvLuaPushTbufAsString function
* @endinternal
*
* @brief   Convert tbuf object to Lua string
*/
void prvLuaPushTbufAsString(lua_State *L, tbuf buf);

/**
* @internal prvLuaTbuf_register function
* @endinternal
*
* @brief   register tbuf functions
*/
int prvLuaTbuf_register(lua_State *L);

#endif /* __luaTbufWrapper_h__ */

