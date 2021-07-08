/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
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
* @file luaCLIEngine_C_API.h
*
* @brief a C API for LUA CLI Engine
*
* @version   1
********************************************************************************
*/
#ifndef __luaCLIEngine_C_API_h__
#define __luaCLIEngine_C_API_h__

/***** Includes  ********************************************************/
#include <extUtils/luaCLI/luaCLIEngine.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal luaCLIEngineNewInstance function
* @endinternal
*
* @brief   Init new Lua state, enable minimal functionality
*         Should be used to create luaCLI and for new Lua task
* @param[in] fs                       - FS API pointer (optional)
*                                      This file system should be implemented
*                                      to allow Lua load .lua scripts
*                                      Also it is used by scripts to store/load files
*                                      If this parameter is ommited (NULL) then
*                                      Lua will try to open files using fopen()/fread()...
* @param[in] getFunctionByName        - (optional)
*                                      This function is used to convert function name
*                                      to pointer.
*                                      Marvell's luaCLI implementation (the complete set
*                                      of Lua scripts) widely use it to don't create
*                                      wrapper for each CPSS API
*
* @param[out] luaPtr                   - Lua instance
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngineNewInstance
(
    IN  genFS_PTR                fs,
    IN  LUA_CLI_GETFUNCBYNAME_T  getFunctionByName,
    OUT lua_State              **luaPtr
);

/**
* @internal luaCLIEngineDestroy function
* @endinternal
*
* @brief   Destroy luaCLI instance
*
* @param[in] L                        - Lua instance
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngineDestroy
(
    IN lua_State    *L
);

/**
* @internal luaCLIEngineExec function
* @endinternal
*
* @brief   Execute luaCLI command
*
* @param[in] L                        - Lua instance
* @param[in] cmd                      - luaCLI command
*                                      For example: "show version"
* @param[in,out] outBufferLength          - output buffer length
*
* @param[out] outBuffer                - output buffer
* @param[in,out] outBufferLength          - output buffer length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngineExec
(
    IN  lua_State  *L,
    IN  const char *cmd,
    OUT char       *outBuffer,
    INOUT GT_U32   *outBufferLength
);

/**
* @internal luaCLIEngineDoLua function
* @endinternal
*
* @brief   Execute lua script
*
* @param[in] L                        - Lua instance
* @param[in] luaScript                - lua script
*                                      For example: "print(to_string({1,2,3}))"
* @param[in,out] outBufferLength          - output buffer length
*
* @param[out] outBuffer                - output buffer
* @param[in,out] outBufferLength          - output buffer length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS luaCLIEngineDoLua
(
    IN  lua_State  *L,
    IN  const char *luaScript,
    OUT char       *outBuffer,
    INOUT GT_U32   *outBufferLength
);

/**
* @internal luaCLIEngineLoadModule function
* @endinternal
*
* @brief   Load new luaCLI module
*         This command executes require("module_name")
*         module_name.lua must be in genFS
* @param[in] L                        - Lua instance
* @param[in] moduleName               - Module name (w/o .lua extension)
* @param[in,out] outBufferLength          - output buffer length
*
* @param[out] outBuffer                - output buffer
* @param[in,out] outBufferLength          - output buffer length
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note 
*       EXAMPLE:
*       luaCLIEngineLoadModule(L, "examples/run", NULL, 0);
*
*/
GT_STATUS luaCLIEngineLoadModule
(
    IN  lua_State  *L,
    IN  const char *moduleName,
    OUT char       *outBuffer,
    INOUT GT_U32   *outBufferLength
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __luaCLIEngine_C_API_h__ */

