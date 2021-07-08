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
* @file luaCLIEngine_genWrapper.c
*
* @brief a generic wrapper to CPSS APIs
*
* @version   1
********************************************************************************
*/

#define CPSS_GENWRAPPER_ENGINE
#include <lua.h>
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <extUtils/luaCLI/luaCLIEngine.h>

#define MAX_PARAMS 30
#define MAX_TYPE_LENGTH 30

typedef enum
{
    PARAM_DIRECTION_IN_E,
    PARAM_DIRECTION_OUT_E,
    PARAM_DIRECTION_INOUT_E,
    PARAM_DIRECTION_WRONG_E
} PARAM_DIRECTION_ENT;
typedef enum {
    PARAM_TYPE_VAL_U32_E, /* IN GT_U32 */
    PARAM_TYPE_VAL_U16_E, /* IN GT_U16 */
    PARAM_TYPE_VAL_U8_E,  /* IN GT_U8 */
    PARAM_TYPE_PTR_U32_E, /* OUT GT_U32* */
    PARAM_TYPE_PTR_U16_E, /* OUT GT_U16* */
    PARAM_TYPE_PTR_U8_E,  /* OUT GT_U8* */
    PARAM_TYPE_PTR_PTR_E, /* OUT GT_PTR* */
    PARAM_TYPE_PTR_E,     /* _STC*  ARR*  */
    PARAM_TYPE_ARR_NUM_E, /* Array of numbers */
    PARAM_TYPE_UNKNOWN_E
} PARAM_TYPE_ENT;

/*************************************************************************
* mgmType_to_c_int
* mgmType_to_c_GT_U8
* mgmType_to_c_GT_U16
* mgmType_to_c_GT_U32
*
* Description:
*       Convert to integer/GT_U8/GT_U16/GT_U32
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  number if successful
*
*************************************************************************/
static int mgmType_to_c_int(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        lua_pushnumber(L, 0);
    else if (!lua_isnumber(L, 1))
        lua_pushnil(L);
    else
        lua_pushvalue(L, 1);
    return 1;
}

/*************************************************************************
* mgmType_to_lua_int
* mgmType_to_lua_GT_U8
* mgmType_to_lua_GT_U16
* mgmType_to_lua_GT_U32
*
* Description:
*       Convert integer/GT_U8/GT_U16/GT_U32 to number
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  number if successful
*
*************************************************************************/
static int mgmType_to_lua_int(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        return 1;
    if (!lua_isnumber(L, 1))
        lua_pushnil(L);
    else
        lua_pushvalue(L, 1);
    return 1;
}

/*************************************************************************
* mgmType_to_c_bool
* mgmType_to_c_GT_BOOL
*
* Description:
*       Convert to integer(GT_BOOL)
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  number if successful
*
*************************************************************************/
static int mgmType_to_c_bool(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        lua_pushnumber(L, 0);
    else if (lua_isboolean(L, 1))
        lua_pushnumber(L, lua_toboolean(L, 1));
    else if (lua_isnumber(L, 1))
        lua_pushnumber(L, lua_tointeger(L, 1) == 0 ? 0 : 1);
    else
        lua_pushnumber(L, 0);
    return 1;
}

/*************************************************************************
* mgmType_to_lua_bool
* mgmType_to_lua_GT_BOOL
*
* Description:
*       Convert integer(GT_BOOL) to boolean
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  number if successful
*
*************************************************************************/
static int mgmType_to_lua_bool(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        return 1;
    if (!lua_isnumber(L, 1))
        lua_pushnil(L);
    else
        lua_pushboolean(L, lua_tointeger(L, 1) == 0 ? 0 : 1);
    return 1;
}

/*************************************************************************
* string_equal_to
*
* Description:
*       Compare string at the top of Lua's stack with constant string
*
* Parameters:
*       string at the top of
*
* Returns:
*  Nonzero if strings equal
*  zero if not equal
*
*************************************************************************/
static int string_equal_to(lua_State *L, const char *literal)
{
    int result;
    lua_pushstring(L, literal);
    result = lua_equal(L, -1, -2);
    lua_pop(L, 1);
    return result;
}

/*************************************************************************
* prvInterpretParameterDirection
*
* Description:
*       Convert parameter direction ("IN", "OUT", "INOUT") to enum
*
* Parameters:
*       string at the top of stack
*
* Returns:
*  PARAM_DIRECTION_WRONG_E if error
*  enum value if successful
*
*************************************************************************/
static PARAM_DIRECTION_ENT prvInterpretParameterDirection(lua_State *L)
{
    PARAM_DIRECTION_ENT ret = PARAM_DIRECTION_WRONG_E;
    if (string_equal_to(L, "IN"))
        ret = PARAM_DIRECTION_IN_E;
    if (string_equal_to(L, "OUT"))
        ret = PARAM_DIRECTION_OUT_E;
    if (string_equal_to(L, "INOUT"))
        ret = PARAM_DIRECTION_INOUT_E;
    lua_pop(L, 1);
    return ret;
}

/*************************************************************************
* prvCheckArraySuffix
*
* Description:
*       Check if type has array suffix [ddd] where ddd is number in decimal
*       remove suffix
*
* Parameters:
*       string at the top of stack
*
* Returns:
*  -1 if no suffix
*  -2 if error
*  number
*
*************************************************************************/
static int prvCheckArraySuffix(lua_State *L)
{
    GT_CHAR *p, *d;
    const char *s = lua_tostring(L, -1);
    int n = 0;
    p = cpssOsStrChr((const GT_CHAR*)s, '[');
    if (p == NULL)
        return -1;
    if (p[1] == ']')
        return -2;
    
    for (d = p+1; *d && *d != ']'; d++)
    {
        if (*d < '0' || *d > '9')
            return -2;
        n = n * 10 + (*d - '0');
    }
    if (d[0] != ']' || d[1] != 0)
        return -2;
    lua_pushlstring(L, s, p-s);
    lua_remove(L, -2);
    return n;
}

/*************************************************************************
* prvGetTypeCvtFunc
*
* Description:
*       
*
* Parameters:
*       typename string at the top of stack
*
* Returns:
*    1 if suceeded, function on the top of stack
*    0 if error
*  number
*
*************************************************************************/
static int prvGetTypeCvtFunc(
    lua_State               *L,
    LUA_CLI_GETFUNCBYNAME_T getFunctionByName,
    int                     param,
    int                     isLuaToC
)
{
    lua_CFunction cvtFunc;
    if (!lua_isstring(L, -1))
    {
        lua_pushfstring(L, "paramter type for parameter %d is not string", param);
        return 0;
    }
    if (isLuaToC)
        lua_pushliteral(L, "mgmType_to_c_");
    else
        lua_pushliteral(L, "mgmType_to_lua_");
    lua_insert(L, -2);
    lua_concat(L, 2);
    lua_getglobal(L, lua_tostring(L, -1));
    if (lua_isfunction(L, -1))
        return 1;
    lua_pop(L, 1);
    cvtFunc = (lua_CFunction) getFunctionByName(lua_tostring(L, -1));
    if (cvtFunc)
    {
        lua_pushcfunction(L, cvtFunc);
        return 1;
    }
    lua_pushfstring(L, "function '%s' is not defined", lua_tostring(L, -1));
    return 0;
}

/* one parameter description for I/O */
typedef struct PRV_PARAM_DATA_STCT {
    GT_UINTPTR          param;
    GT_UINTPTR          value;
    PARAM_DIRECTION_ENT typeIO;
    PARAM_TYPE_ENT      pType;
    int                 elemSize;
    int                 arraySize;
} PRV_PARAM_DATA_STC;

/*************************************************************************
* prvParseParam
*
* Description:
*       
*
* Input:
*   paramNo     - parameter number (1..)
*
* Output:
*   p           - parameter data (C value, type, etc)
*
* Returns:
*    1 if suceeded, function on the top of stack
*    0 if error
*  number
*
*************************************************************************/
static int prvParseParam(
    lua_State              *L,
    LUA_CLI_GETFUNCBYNAME_T getFunctionByName,
    int                     paramNo,
    PRV_PARAM_DATA_STC     *p
)
{
    int i;
    p->arraySize = -1;
    p->elemSize = 0;
    lua_rawgeti(L, 2, paramNo); /* params[param] */
    if (!lua_istable(L, -1))
    {
        lua_pushfstring(L, "params[%d] is not a table", paramNo);
        return 1;
    }
    lua_rawgeti(L, -1, 1); /* params[paramNo][1] */
    /* stk: params[paramNo][1]; params[param] */
    p->typeIO = prvInterpretParameterDirection(L);
    if (p->typeIO == PARAM_DIRECTION_WRONG_E)
    {
        lua_pushfstring(L, "bad parameter type for parameter %d, must be IN|OUT|INOUT", paramNo);
        return 1;
    }
    lua_rawgeti(L, -1, 2); /* params[paramNo][2] */
    /* stk: params[paramNo][2]; params[param] */
    if (!lua_isstring(L, -1))
    {
        lua_pushfstring(L, "no type specified for parameter %d", paramNo);
        return 1;
    }
    p->arraySize = prvCheckArraySuffix(L);
    if (p->arraySize == -2)
    {
        lua_pushfstring(L, "wrong type of parameter %d", paramNo);
        return 1;
    }
    /* stk: typename; params[paramNo] */
    /* TODO: handle arraySize==0 */
    if (p->typeIO == PARAM_DIRECTION_IN_E &&
            string_equal_to(L, "string"))
    {
        /* special case: `IN string' will be passed as `const char*' */
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 4);
        /* stk: value; params[paramNo] */
        if (!lua_isstring(L, -1))
        {
            lua_pushfstring(L, "parameter %d is string but the value doesn't", paramNo);
            return 1;
        }
        lua_remove(L, -2); /* params[paramNo] */
        /* stk: value */
        p->pType = PARAM_TYPE_PTR_E;
        p->value = (GT_UINTPTR)lua_tostring(L, -1);
        return 0;
    }
    /* stk: typename; params[paramNo] */
    if (p->arraySize > 0)
    {
        /* Array */
        p->pType = PARAM_TYPE_PTR_E;
        p->elemSize = -1;
        if (string_equal_to(L, "GT_U16"))
        {
            p->elemSize = 2;
            p->pType = PARAM_TYPE_ARR_NUM_E;
        }
        if (string_equal_to(L, "GT_U8"))
        {
            p->elemSize = 1;
            p->pType = PARAM_TYPE_ARR_NUM_E;
        }

        if (p->elemSize < 0)
        {
            if (!prvGetTypeCvtFunc(L, getFunctionByName, paramNo, 1))
                return 1;
            /* stk: cvt_func; typename; params[paramNo] */
            /* first call to detect element size */
            /* stk: cvt_func; typename; params[paramNo] */
            lua_pushvalue(L, -1); /* duplicate function */
            lua_pushnil(L);
            if (lua_pcall(L, 1, 1, 0))
            {
                lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -2));
                lua_insert(L, -2);
                lua_concat(L, 2);
                return 1;
            }
            /* stk: retVal; cvt_func; typename; params[paramNo] */
            if (lua_type(L, -1) == LUA_TNUMBER)
            {
                p->elemSize = sizeof(GT_U32);
                p->pType = PARAM_TYPE_ARR_NUM_E;
            }
            else if (lua_type(L, -1) == LUA_TUSERDATA)
            {
                p->elemSize = lua_objlen(L, -1);
            }
            else
            {
                lua_pushfstring(L, "p1: not supported parameter %d", paramNo);
                return 1;
            }
            lua_pop(L, 1);
            /* stk: cvt_func; typename; params[paramNo] */
        }
        else
        {
            lua_pushnil(L);
            /* stk: nil; typename; params[paramNo] */
        }
        lua_newuserdata(L, p->arraySize * p->elemSize);
        cpssOsMemSet(lua_touserdata(L, -1), 0, lua_objlen(L, -1));
        /* stk: userdata; nil/cvt_func; typename; params[paramNo] */
        lua_insert(L, -4);
        /* stk: nil/cvt_func; typename; params[paramNo]; userdata */
        p->value = (GT_UINTPTR)lua_touserdata(L, -4);
        if (p->typeIO == PARAM_DIRECTION_OUT_E)
        {
            lua_pop(L, 3);
            /* stk: userdata */
            return 0;
        }
        /* typeIO = IN || INOUT */
        /* stk: nil/cvt_func; typename; params[paramNo]; userdata */
        lua_rawgeti(L, -3, 4);
        if (!lua_istable(L, -1))
        {
            lua_pushfstring(L, "parameter %d value must be a table",paramNo);
            return 1;
        }
        /* stk: table; nil/cvt_func; typename; params[paramNo]; userdata */
        for (i = 0; i < p->arraySize; i++)
        {
            GT_UINTPTR elemPtr = p->value + p->elemSize * i;
            if (p->elemSize == 1 || p->elemSize == 2)
            {
                /* GT_U8 || GT_U16 */
                lua_rawgeti(L, -1, i);
                if (lua_type(L, -1) == LUA_TNUMBER)
                {
                    if (p->elemSize == 1)
                        *((GT_U8*)elemPtr) = (GT_U8)lua_tonumber(L, -1);
                    else /* p->elemSize == 2 */
                        *((GT_U16*)elemPtr) = (GT_U16)lua_tonumber(L, -1);
                }
                if (lua_type(L, -1) != LUA_TNUMBER && lua_type(L, -1) != LUA_TNIL)
                {
                    lua_pushfstring(L, "parameter %d[%d] expected to be number", paramNo, i);
                    return 1;
                }
                lua_pop(L, 1);
                continue;
            }
            lua_pushvalue(L, -2); /* duplicate function */
            /* stk: cvt_func; table; cvt_func; typename; params[paramNo]; userdata */
            lua_rawgeti(L, -2, i);
            /* stk: table[i]; cvt_func; table; cvt_func; typename; params[paramNo]; userdata */
            if (lua_pcall(L, 1, 1, 0))
            {
                lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -3));
                lua_insert(L, -2);
                lua_concat(L, 2);
                return 1;
            }
            /* stk: table[i](converted); table; cvt_func; typename; params[paramNo]; userdata */
            if (lua_type(L, -1) == LUA_TNUMBER)
            {
                *((GT_U32*)elemPtr) = (GT_U32)lua_tonumber(L, -1);
            }
            else if (lua_type(L, -1) == LUA_TUSERDATA)
            {
                cpssOsMemCpy((void*)elemPtr, lua_touserdata(L, -1), p->elemSize);
            }
            lua_pop(L, 1);
            /* stk: table; cvt_func; typename; params[paramNo]; userdata */
        }
        /* stk: table; nil/cvt_func; typename; params[paramNo]; userdata */
        lua_pop(L, 4);
        return 0;
    } /* array end */
    /* stk: typename; params[paramNo] */
    if (string_equal_to(L, "GT_U16") || string_equal_to(L, "GT_U8"))
    {
        p->value = 0;
        if (string_equal_to(L, "GT_U16"))
            p->pType = PARAM_TYPE_VAL_U16_E;
        else
            p->pType = PARAM_TYPE_VAL_U8_E;
        if (p->typeIO != PARAM_DIRECTION_OUT_E)
        {
            lua_rawgeti(L, -2, 4);
            if (lua_type(L, -1) == LUA_TNUMBER)
            {
                if (p->pType == PARAM_TYPE_VAL_U16_E)
                    p->value = (GT_U16)lua_tonumber(L, -1);
                else
                    p->value = (GT_U8)lua_tonumber(L, -1);
            }
            if (lua_type(L, -1) != LUA_TNUMBER && lua_type(L, -1) != LUA_TNIL)
            {
                lua_pushfstring(L, "parameter %d[4] expected to be number", paramNo);
                return 1;
            }
            lua_pop(L, 1);
        }
        if (p->typeIO != PARAM_DIRECTION_IN_E)
        {
            p->pType = (p->pType == PARAM_TYPE_VAL_U16_E)
                ? PARAM_TYPE_PTR_U16_E
                : PARAM_TYPE_PTR_U8_E;
        }
        lua_pop(L, 2);
        return 0;
    } /* GT_U16 || GT_U8 */
    /* stk: typename; params[paramNo] */
    if (!prvGetTypeCvtFunc(L, getFunctionByName, paramNo, 1))
        return 1;
    /* stk: cvt_func; typename; params[paramNo] */
    if (p->typeIO != PARAM_DIRECTION_OUT_E)
        lua_rawgeti(L, -3, 4);
    else
        lua_pushnil(L);
    /* stk: value/nil; cvt_func; typename; params[paramNo] */
    if (lua_pcall(L, 1, 1, 0))
    {
        lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -2));
        lua_insert(L, -2);
        lua_concat(L, 2);
        return 1;
    }
    /* stk: value(converted); typename; params[paramNo] */
    if (lua_type(L, -1) == LUA_TNUMBER)
    {
        p->value = (GT_UINTPTR)((GT_U32)lua_tonumber(L, -1));
        p->pType = (p->typeIO == PARAM_DIRECTION_IN_E)
            ? PARAM_TYPE_VAL_U32_E
            : PARAM_TYPE_PTR_U32_E;
        lua_pop(L, 3);
        return 0;
    }
    /* stk: value(converted); typename; params[paramNo] */
    if (lua_type(L, -1) == LUA_TLIGHTUSERDATA)
    {
        p->value = (GT_UINTPTR)lua_touserdata(L, -1);
        p->pType = (p->typeIO == PARAM_DIRECTION_IN_E)
            ? PARAM_TYPE_PTR_E
            : PARAM_TYPE_PTR_PTR_E;
        lua_pop(L, 2);
        return 0;
    }
    if (lua_type(L, -1) != LUA_TUSERDATA)
    {
        lua_pushfstring(L, "parameter %d: wrong value/converter", paramNo);
        return 1;
    }
    /* userdata */
    /* stk: value(converted); typename; params[paramNo] */
    lua_insert(L, -3);
    lua_pop(L, 2);
    /* stk: value(converted) */
    p->value = (GT_UINTPTR)lua_touserdata(L, -1);
    p->pType = PARAM_TYPE_PTR_E;
    p->elemSize = lua_objlen(L, -1);
    return 0;
}

/*************************************************************************
* cpssGenWrapper
*
* Description:
*       Call CPSS API function
*
* Parameters:
*  cpssAPIname     - Fx: "prvCpssDrvHwPpReadRegister"
*  params          - table:
*                    each row is table of 3 elements:
*                    1. direction (IN | OUT | INOUT)
*                    2. parameter type
*                       the valid types:
*                       "int"
*                       "bool"
*                       "GT_U8",
*                       "GT_U16",
*                       "GT_U32",
*                       "string"
*                       "custom_type_name" with a set of functions:
*                          mgmType_to_c_${custom_type_name}(param)
*                                  - convert lua variable/table to
*                                    userdata object
*                          mgmType_to_lua_${custom_type_name}(param)
*                                  - convert userdata object to lua
*                    3. parameter name
*                    4. parameter value(optional, for IN and INOUT only)
*  noLock          - if true then don't protect call with mutex
*
*
* Returns:
*  GT_STATUS, array of output values if successful
*  string if error
*
*
* Example
* status, values = cpssGenWrapper(
*                      "prvCpssDrvHwPpReadRegister", {
*                           { "IN", "GT_U8", "devNum", 0 },
*                           { "IN", "GT_U32","regAddr", 0x200000 },
*                           { "OUT","GT_U32","regData" }
*                      })
* regData = values.regData
*************************************************************************/
static int cpssGenWrapper(
    lua_State *L
)
{
    PRV_PARAM_DATA_STC      params[MAX_PARAMS];
    GT_STATUS               retval;
    typedef GT_STATUS (*CPSS_API_FUNC_PTR)(GT_UINTPTR, ...);
    CPSS_API_FUNC_PTR funcPtr;
    int nparams;
    int k,values;

    LUA_CLI_GETFUNCBYNAME_T  getFunctionByName;

    getFunctionByName = (LUA_CLI_GETFUNCBYNAME_T)
            luaCLIEngineGetData(L, "_getFunctionByName", 1);
    if (getFunctionByName == NULL)
    {
        lua_pushliteral(L, "_getFunctionByName not defined");
        return 1;
    }
    if (lua_gettop(L) < 2)
    {
        lua_pushliteral(L, "no enough parameters");
        return 1;
    }
    if (!lua_isstring(L, 1))
    {
        lua_pushliteral(L, "bad parameter 1");
        return 1;
    }
    if (!lua_istable(L, 2))
    {
        lua_pushliteral(L, "bad parameter 2");
        return 1;
    }

    lua_settop(L, 2); /* remove extra parameters */

    funcPtr = (CPSS_API_FUNC_PTR) getFunctionByName(lua_tostring(L, 1));
    if (!funcPtr)
    {
        lua_pushliteral(L, "function '");
        lua_pushvalue(L, 1);
        lua_pushliteral(L, "' not found");
        lua_concat(L, 3);
        return 1;
    }

    cpssOsMemSet(params, 0, sizeof(params));
    /* prepare parameters */
    nparams = (int)lua_objlen(L, 2);
    for (k = 0; k < nparams; k++)
    {
        PRV_PARAM_DATA_STC *p;
        void *valPtr;
        p = &(params[k]);
        valPtr = (void*)(&(p->value));
        if (prvParseParam(L, getFunctionByName, k+1, p) != 0)
        {
            return 1;
        }

        switch (p->pType)
        {
            case PARAM_TYPE_VAL_U32_E: /* IN GT_U32 */
            case PARAM_TYPE_VAL_U16_E: /* IN GT_U16 */
            case PARAM_TYPE_VAL_U8_E:  /* IN GT_U8 */
                p->param = p->value;
                break;
            case PARAM_TYPE_PTR_U32_E: /* OUT GT_U32* */
                *((GT_U32*)valPtr) = (GT_U32)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_U16_E: /* OUT GT_U16* */
                *((GT_U16*)valPtr) = (GT_U16)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_U8_E:  /* OUT GT_U8* */
                *((GT_U8*)valPtr) = (GT_U8)p->value;
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_PTR_E: /* OUT GT_PTR* */
                p->param = (GT_UINTPTR)(&(p->value));
                break;
            case PARAM_TYPE_PTR_E:     /* _STC*  ARR*  */
            case PARAM_TYPE_ARR_NUM_E: /* Array of numbers */
                p->param = p->value;
                break;
            case PARAM_TYPE_UNKNOWN_E:
                lua_pushfstring(L, "unknown parameter class for param %d", k+1);
                return 1;
        }
    }

    /* call function */
    retval = funcPtr(
        params[0].param,
        params[1].param,
        params[2].param,
        params[3].param,
        params[4].param,
        params[5].param,
        params[6].param,
        params[7].param,
        params[8].param,
        params[9].param,
        params[10].param,
        params[11].param,
        params[12].param,
        params[13].param,
        params[14].param,
        params[15].param,
        params[16].param,
        params[17].param,
        params[18].param,
        params[19].param,
        params[20].param,
        params[21].param,
        params[22].param,
        params[23].param,
        params[24].param,
        params[25].param,
        params[26].param,
        params[27].param,
        params[28].param,
        params[29].param);

    /* push results */
    lua_pushnumber(L, retval);
    lua_newtable(L);
    values = lua_gettop(L);

    for (k = 0; k < nparams; k++)
    {
        PRV_PARAM_DATA_STC *p;
        int paramDescIdx;
        void* valPtr;
        p = &(params[k]);
        valPtr = (void*)(&(p->value));
        if (p->typeIO == PARAM_DIRECTION_IN_E)
            continue;

        lua_rawgeti(L, 2, k+1); /* params[k+1] */
        paramDescIdx = lua_gettop(L); /* param = params[k+1] */
        /* stk: params[k+1] */

        lua_rawgeti(L, paramDescIdx, 2);
        prvCheckArraySuffix(L); /* strip "[ddd]" suffix */
        /* stk: typename; params[k+1] */

        if (p->arraySize > 0)
        {
            int i;
            /* Array */
            if (p->pType == 1 || p->elemSize == 2)
            {
                lua_pushnil(L);
            }
            else
            {
                if (!prvGetTypeCvtFunc(L, getFunctionByName, k+1, 0))
                    return 1;
            }
            /* stk: nil/cvt_func; typename; params[k+1] */
            lua_newtable(L);
            /* stk: ret_tbl; nil/cvt_func; typename; params[k+1] */
            for (i = 0; i < p->arraySize; i++)
            {
                GT_UINTPTR elemPtr = p->value + p->elemSize * i;
                if (p->elemSize == 1 || p->elemSize == 2)
                {
                    /* GT_U8 || GT_U16 */
                    if (p->elemSize == 1)
                        lua_pushnumber(L, *((GT_U8*)elemPtr));
                    else /* p->elemSize == 2 */
                        lua_pushnumber(L, *((GT_U16*)elemPtr));
                    lua_rawseti(L, -2, i);
                    continue;
                }
                lua_pushvalue(L, -2);
                /* stk: cvt_func; ret_tbl; cvt_func; typename; params[k+1] */
                if (p->pType == PARAM_TYPE_ARR_NUM_E)
                {
                    /* array elem is number */
                    lua_pushnumber(L, *((GT_U32*)elemPtr));
                }
                else
                {
                    lua_newuserdata(L, p->elemSize);
                    cpssOsMemCpy(lua_touserdata(L, -1), (void*)elemPtr, p->elemSize);
                }
                /* stk: value; cvt_func; ret_tbl; cvt_func; typename; params[k+1] */
                /* convert return values to lua */
                if (lua_pcall(L, 1, 1, 0))
                {
                    lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -4));
                    lua_insert(L, -2);
                    lua_concat(L, 2);
                    return 1;
                }
                /* stk: lua_value; ret_tbl; cvt_func; typename; params[k+1] */
                lua_rawseti(L, -2, i);
                /* stk: ret_tbl; cvt_func; typename; params[k+1] */
            }
            lua_rawgeti(L, paramDescIdx, 3);
            /* TODO: check name is string */
            /* stk: param_name; ret_tbl; cvt_func; typename; params[k+1] */
            lua_insert(L, -2);
            /* stk: ret_tbl; param_name; cvt_func; typename; params[k+1] */
            lua_setfield(L, values, lua_tostring(L, -2));
            /* stk: param_name; cvt_func; typename; params[k+1] */
            lua_pop(L, 4);
            continue;
        }

        /* Not array */
        if (!prvGetTypeCvtFunc(L, getFunctionByName, k+1, 0))
            return 1;
        /* stk: cvt_func; typename; params[k+1] */

        switch (p->pType)
        {
            case PARAM_TYPE_PTR_U32_E: /* OUT GT_U32* */
                lua_pushnumber(L, *((GT_U32*)valPtr));
                break;
            case PARAM_TYPE_PTR_U16_E: /* OUT GT_U16* */
                lua_pushnumber(L, *((GT_U16*)valPtr));
                break;
            case PARAM_TYPE_PTR_U8_E:  /* OUT GT_U8* */
                lua_pushnumber(L, *((GT_U8*)valPtr));
                break;
            case PARAM_TYPE_PTR_PTR_E: /* OUT GT_PTR* */
#if 0
                lua_pushlightuserdata(L, (void*)p->value);
                break;
#else
                lua_pop(L, 2);
                /* stk: params[k+1] */
                lua_rawgeti(L, -1, 3);
                /* TODO: check name is string */
                lua_pushlightuserdata(L, (void*)p->value);
                lua_setfield(L, values, lua_tostring(L, -2));
                /* stk: params[k+1][3]; params[k+1] */
                lua_pop(L, 2);
                continue;
#endif
            case PARAM_TYPE_PTR_E:     /* _STC* */
                lua_newuserdata(L, p->elemSize);
                cpssOsMemCpy(lua_touserdata(L, -1), (void*)p->param, p->elemSize);
                break;
            default:
                lua_pushnil(L);
                break;/* should never happen */
        }
        /* stk: value; cvt_func; typename; params[k+1] */
        /* convert return values to lua */
        if (lua_pcall(L, 1, 1, 0))
        {
            lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -2));
            lua_insert(L, -2);
            lua_concat(L, 2);
            return 1;
        }
        /* stk: lua_value; typename; params[k+1] */
        lua_rawgeti(L, paramDescIdx, 3);
        /* TODO: check name is string */
        /* stk: param_name; lua_value; typename; params[k+1] */
        lua_insert(L, -2);
        /* stk: lua_value; param_name; typename; params[k+1] */
        lua_setfield(L, values, lua_tostring(L, -2));
        /* stk: param_name; typename; params[k+1] */
        lua_pop(L, 3);
    }
    return 2;
}

/*************************************************************************
* cpssGenWrapperCheckParam
*
* Description:
*       mgmType_to_lua_${custom_type_name}(mgmType_to_c_${custom_type_name}(param))
*       This can be used for
*       1. Convert 
*
* Parameters:
*       custom_type_name - Fx: "CPSS_MAC_ENTRY_EXT_STC"
*       param            - A value to check
*
*
* Returns:
*       output value if successful
*       string if error
*
*
* Example
*       value = cpssGenWrapperCheckParam("CPSS_MAC_ENTRY_EXT_STC", nil)
*       value.isStatic = true
*       value = cpssGenWrapperCheckParam("CPSS_MAC_ENTRY_EXT_STC", value)
*
*************************************************************************/
static int cpssGenWrapperCheckParam(
    lua_State *L
)
{
    int i;
    LUA_CLI_GETFUNCBYNAME_T  getFunctionByName;

    getFunctionByName = (LUA_CLI_GETFUNCBYNAME_T)
            luaCLIEngineGetData(L, "_getFunctionByName", 1);
    if (getFunctionByName == NULL)
    {
        lua_pushliteral(L, "_getFunctionByName not defined");
        return 1;
    }
    if (!lua_isstring(L, 1))
    {
        lua_pushliteral(L, "wrong type name");
        return 1;
    }
    lua_settop(L, 2);
    /*
     * push "mgmType_to_lua_${custom_type_name}"
     * push function mgmType_to_lua_${custom_type_name}
     * push "mgmType_to_c_${custom_type_name}"
     * push function mgmType_to_c_${custom_type_name}
     */
    for (i = 0; i < 2; i++)
    {
        /* convert lua value to C */
        lua_pushstring(L, i == 0 ? "mgmType_to_lua_" : "mgmType_to_c_");
        lua_pushvalue(L, 1);
        lua_concat(L, 2);
        lua_getglobal(L, lua_tostring(L, -1));
        if (!lua_isfunction(L, -1))
        {
            /* try to lookup C function with the same name */
            lua_CFunction cvtFunc;
            cvtFunc = (lua_CFunction) getFunctionByName(lua_tostring(L, -2));
            if (cvtFunc)
            {
                lua_pop(L, 1);
                lua_pushcfunction(L, cvtFunc);
            }
        }
        if (!lua_isfunction(L, -1))
        {
            lua_pushfstring(L, "function '%s' is not defined", lua_tostring(L, -2));
            return 1;
        }
    }

    lua_pushvalue(L, 2);
     
    /*
     * call function mgmType_to_c_${custom_type_name} (param)
     * call function mgmType_to_lua_${custom_type_name}
     */
    for (i = 0; i < 2; i++)
    {
        if (lua_pcall(L, 1, 1, 0))
        {
            lua_pushfstring(L, "error while executing %s: ", lua_tostring(L, -2));
            lua_insert(L, -2);
            lua_concat(L, 2);
            return 1;
        }
        /* remove function name */
        lua_remove(L, -2);
    }
    return 1;
}

/*************************************************************************
* prv_lua_to_c_enum
*
* Description:
*       Convert lua value to enum
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       integer
*
*************************************************************************/
int prv_lua_to_c_enum(
    lua_State   *L,
    enumDescr   *descr
)
{
    if (lua_isnumber(L, -1))
        return (int)lua_tointeger(L, -1);
    if (!descr)
        return 0;
    if (lua_isstring(L, -1))
    {
        const char *s = lua_tostring(L, -1);
        int delta = 0;
        GT_CHAR *plu = cpssOsStrChr(s, '+');
        if (plu != NULL)
        {
            char *d = plu + 1;
            while (*d != 0 && *d >= '0' && *d <= '9')
            {
                delta = delta * 10 + ((*d)-'0');
                d++;
            }
            if (*d == 0)
            {
                *plu = 0;
            }
            else
            {
                delta = 0;
                plu = NULL;
            }
        }
        while(descr->string)
        {
            if (cpssOsStrCmp(s, descr->string) == 0)
            {
                if (plu != NULL)
                    *plu = '+';
                return descr->enumValue + delta;
            }
            descr++;
        }
        if (plu != NULL)
            *plu = '+';
    }
    return 0;
}

/*************************************************************************
* prv_c_to_lua_enum
*
* Description:
*       Convert enum to lua
*
* Parameters:
*       cmd
*
* Returns:
*       string at the top of stack
*
*************************************************************************/
void prv_c_to_lua_enum(
    lua_State   *L,
    enumDescr   *descr,
    int         enumValue
)
{
    if (descr)
    {
        for(; descr->string; descr++)
        {
            if (enumValue == descr->enumValue)
            {
                lua_pushstring(L, descr->string);
                return;
            }
        }
        /* couldn't find given enum value */
        lua_pushfstring(L, "value 0x%x", enumValue);
        return;
    }
    lua_pushnil(L);
}

#ifndef LUACLI_NOCPSS
/*******************************************************************************
* prvCpssGetEnumString
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Returns enum member string according to its enumerated value or NULL 
*       when enumerated value is wrong.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       enumValue             - enum enumerated value
*       descrPtr              - enum support struct array
*
* OUTPUTS:
*
* RETURNS:
*       enum member string or NULL when entry enumerated value is wrong
*
* COMMENTS:
*
*******************************************************************************/
const GT_CHAR* prvCpssGetEnumString
(   
    IN  GT_U32                  enumValue,
    IN  enumDescr               *descrPtr    
)
{
    if (NULL != descrPtr)
    {
        while(NULL != descrPtr->string)
        {
            if (enumValue == (GT_U32) descrPtr->enumValue)
                return (const GT_CHAR_PTR) descrPtr->string;
                
            descrPtr++;
        }    
    }   
    
    return NULL;          
}
#endif /* !defined(LUACLI_NOCPSS) */


/*************************************************************************
* prv_lua_check_enum
*
* Description:
*       Return true if L => 1st is a member of enum
*
* Parameters:
*       L => 1st    - string to check
*
* Returns:
*       integer
*
*************************************************************************/
int prv_lua_check_enum(
    lua_State   *L,
    enumDescr   *descr
)
{
    const char *str;

    if (lua_type(L, 1) != LUA_TSTRING)
    {
        lua_pushboolean(L, 0);
        return 0;
    }

    str = lua_tostring(L, 1);
    if (descr)
    {
        for(; descr->string; descr++)
        {
            if (cpssOsStrCmp(str, descr->string) == 0)
            {
                lua_pushboolean(L, 1);
                return 1;
            }
        }
    }
    lua_pushboolean(L, 0);
    return 1;
}

/*************************************************************************
* prv_lua_complete_enum
*
* Description:
*       Return the list of alternatives for autocomplete matching L => 1st
*
* Parameters:
*       L => 1st    - string to complete
*
* Returns:
*       integer
*
*************************************************************************/
int prv_lua_complete_enum(
    lua_State   *L,
    enumDescr   *descr
)
{
    const char *str;
    size_t      len;
    int         index = 0;

    if (lua_type(L, 1) != LUA_TSTRING)
        return 0;

    str = lua_tolstring(L, 1, &len);
    lua_newtable(L);
    if (descr)
    {
        for(; descr->string; descr++)
        {
            if (cpssOsStrNCmp(str, descr->string, (GT_U32)len) == 0)
            {
                lua_pushstring(L, descr->string);
                lua_rawseti(L, -2, ++index);
            }
        }
    }
    return 1;
}

#ifndef LUACLI_NOCPSS
/**
* @internal prv_swap_byte_array function
* @endinternal
*
* @brief   Swaps array in reversiveorder
*
* @param[in] input_data               - input byte array
* @param[in] dataLen                  - swapping bytes counmt
*
* @param[out] output_data              - output byte array
*                                       GT_OK
*/
GT_STATUS prv_swap_byte_array
(
    IN  const GT_U8_PTR     input_data,
    IN  GT_U32              dataLen,
    OUT GT_U8_PTR           output_data
)
{
    GT_U32 k;
    GT_U8  data;
    
    for (k = 0; k < dataLen / 2; k++)
    {
        data = input_data[k];
        output_data[k] = input_data[dataLen - k - 1];
        output_data[dataLen - k - 1] = data;
    }
    
    return GT_OK;
}


/**
* @internal prv_lua_apply_mask function
* @endinternal
*
* @brief   Compare two structures using ignore mask
*
* @param[in] input_data               - Input data
* @param[in] output_data              - Output data
* @param[in] appiedMask               - data mask: the bits which will be reseted
*                                      are set to 1
* @param[in] dataLen                  - Data length (the same for data1, data2, ignoreMask)
*                                       GT_OK
*/
GT_STATUS prv_lua_apply_mask(
    IN  const void* input_data,
    IN  const void* output_data,
    IN  const void* appiedMask,
    IN  GT_U32      dataLen
)
{
    GT_U32 k;
    for (k = 0; k < dataLen; k++)
    {
        char mask = (char) ~(((const char*)appiedMask)[k]);
        ((char*)output_data)[k] = (char) (((const char*)input_data)[k] & mask);
    }
    return GT_OK;
}


/**
* @internal prv_lua_compare_mask function
* @endinternal
*
* @brief   Compare two structures using ignore mask
*
* @param[in] data1                    - Input data
* @param[in] data2                    - Input data
* @param[in] ignoreMask               - data mask: the bits which will be ignored
*                                      are set to 1
* @param[in] dataLen                  - Data length (the same for data1, data2, ignoreMask)
*                                       1 if matched
*                                       0 if not matched
*/
int prv_lua_compare_mask(
    IN  const void* data1,
    IN  const void* data2,
    IN  const void* ignoreMask,
    IN  GT_U32      dataLen
)
{
    GT_U32 k;
    for (k = 0; k < dataLen; k++)
    {
        char mask = (char) ~(((const char*)ignoreMask)[k]);
        if (    ( ((const char*)data1)[k] & mask ) !=
                ( ((const char*)data2)[k] & mask ) )
        {
            return 0;
        }
    }
    return 1;
}


/*************************************************************************
* prv_lua_to_c_GT_U64
*
* Description:
*       Convert to "c" GT_U64
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       
*
*************************************************************************/
void prv_lua_to_c_GT_U64(
    lua_State *L,
    GT_U64  *val
)
{
    F_ARRAY_START(val, -1, l);
        F_ARRAY_NUMBER(val, l, 0, GT_U32);
        F_ARRAY_NUMBER(val, l, 1, GT_U32);
    F_ARRAY_END(val, -1, l);
}

/*************************************************************************
* prv_c_to_lua_GT_U64
*
* Description:
*       Convert GT_U64 to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_GT_U64(
    lua_State *L,
    GT_U64 *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, l);
        FO_ARRAY_NUMBER(val, l, 0, GT_U32);
        FO_ARRAY_NUMBER(val, l, 1, GT_U32);
    FO_ARRAY_END(val, t, l);
}

add_mgm_type(GT_U64);
#endif /* !defined(LUACLI_NOCPSS) */


/*************************************************************************
* mgmType_to_c_GT_PTR
*
* Description:
*       Convert to GT_PTR
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  LUA_TLIGHTUSERDATA if successful
*
*************************************************************************/
static int mgmType_to_c_GT_PTR(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        lua_pushlightuserdata(L, NULL);
    else if (!lua_islightuserdata(L, 1))
        lua_pushnil(L);
    else
        lua_pushvalue(L, 1);
    return 1;
}

/*************************************************************************
* mgmType_to_lua_GT_PTR
*
* Description:
*       Convert GT_PTR to Lua lightuserdata
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  LUA_TLIGHTUSERDATA if successful
*
*************************************************************************/
static int mgmType_to_lua_GT_PTR(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        return 1;
    if (!lua_islightuserdata(L, 1))
        lua_pushnil(L);
    else
        lua_pushvalue(L, 1);
    return 1;
}

/*************************************************************************
* mgmType_to_c_uintptr
* mgmType_to_c_GT_UINTPTR
*
* Description:
*       Convert to GT_PTR
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  LUA_TLIGHTUSERDATA if successful
*
*************************************************************************/
static int mgmType_to_c_uintptr(
    lua_State *L
)
{
    if (lua_isnumber(L, 1))
        lua_pushlightuserdata(L, (void*)((GT_UINTPTR)lua_tonumber(L, 1)));
    else if (lua_islightuserdata(L, 1))
        lua_pushvalue(L, 1);
    else
        lua_pushlightuserdata(L, NULL);
    return 1;
}

/*************************************************************************
* mgmType_to_lua_uintptr
* mgmType_to_lua_GT_UINTPTR
*
* Description:
*       Convert GT_PTR to Lua lightuserdata
*
* Parameters:
*  value
*
* Returns:
*  none if error
*  LUA_TLIGHTUSERDATA if successful
*
*************************************************************************/
static int mgmType_to_lua_uintptr(
    lua_State *L
)
{
    if (lua_isnil(L, 1))
        return 1;
    if (!lua_islightuserdata(L, 1))
        lua_pushnil(L);
    else
        lua_pushnumber(L, (lua_Number)((GT_UINTPTR)lua_touserdata(L, 1)));
    return 1;
}



/**
* @internal luaEngineGenWrapper_init function
* @endinternal
*
* @brief   initializing of CPSS contexted lua tipes
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; xCat2; Lion2.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval 0                        - no returned rarameters added no lua stack
*/
int luaEngineGenWrapper_init(lua_State *L)
{
    lua_register(L, "cpssGenWrapper", cpssGenWrapper);
    lua_register(L, "cpssGenWrapperCheckParam", cpssGenWrapperCheckParam);
    lua_register(L, "mgmType_to_c_int", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_U8", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_U16", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_TRUNK_ID",             mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_32",                   mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_U32", mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_PORT_NUM",             mgmType_to_c_int);
    lua_register(L, "mgmType_to_c_GT_PHYSICAL_PORT_NUM",    mgmType_to_c_int);
    lua_register(L, "mgmType_to_lua_int", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_U8", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_U16", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_32",                 mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_U32", mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_PORT_NUM",           mgmType_to_lua_int);
    lua_register(L, "mgmType_to_lua_GT_PHYSICAL_PORT_NUM",  mgmType_to_lua_int);
    lua_register(L, "mgmType_to_c_bool", mgmType_to_c_bool);
    lua_register(L, "mgmType_to_c_GT_BOOL", mgmType_to_c_bool);
    lua_register(L, "mgmType_to_lua_bool", mgmType_to_lua_bool);
    lua_register(L, "mgmType_to_lua_GT_BOOL", mgmType_to_lua_bool);
#ifndef LUACLI_NOCPSS
    lua_register(L, "mgmType_to_c_GT_U64", mgmType_to_c_GT_U64);
    lua_register(L, "mgmType_to_lua_GT_U64", mgmType_to_lua_GT_U64);
#endif /* !defined(LUACLI_NOCPSS) */
    lua_register(L, "mgmType_to_c_GT_PTR", mgmType_to_c_GT_PTR);
    lua_register(L, "mgmType_to_lua_GT_PTR", mgmType_to_lua_GT_PTR);
    lua_register(L, "mgmType_to_c_uintptr", mgmType_to_c_uintptr);
    lua_register(L, "mgmType_to_c_GT_UINTPTR", mgmType_to_c_uintptr);
    lua_register(L, "mgmType_to_lua_uintptr", mgmType_to_lua_uintptr);
    lua_register(L, "mgmType_to_lua_GT_UINTPTR", mgmType_to_lua_uintptr);
    return 0;
}

