 /*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChPortMapping.c
*
* DESCRIPTION:
*       A lua wrapper for port mapping.
*
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/cpssTypes.h>
#include <generic/private/prvWraplGeneral.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpssCommon/wrapCpssDebugInfo.h>

use_prv_struct(CPSS_DXCH_PORT_MAP_STC)


/*******************************************************************************
* wrlCpssDxChPortPhysicalPortMapSet
*
* DESCRIPTION:
*       configure ports mapping
*
* APPLICABLE DEVICES:
*        Bobcat2
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; Lion2; xCat2.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       1 and result pushed to lua stack
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPortPhysicalPortMapSet
(
    IN lua_State                            *L
)
{
    GT_STATUS                                   rc;
    GT_U32                                      i;
    GT_U8                                       devNum;
    GT_U32                                      portMapArraySize;
    static CPSS_DXCH_PORT_MAP_STC               *portMapArrayPtr = NULL;
    P_CALLING_CHECKING;


    devNum            = (GT_U8)                          lua_tonumber(L, 1);
    portMapArraySize  = (GT_U32)                         lua_tonumber(L, 2);

    portMapArrayPtr = cpssOsMalloc(portMapArraySize*sizeof(CPSS_DXCH_PORT_MAP_STC));
    if (portMapArrayPtr == NULL)
    {
        /* Could not allocate memory */
        lua_pushinteger(L, GT_OUT_OF_CPU_MEM);
        return 1;
    }

    for (i = 0; (i < portMapArraySize); i++)
    {
        lua_pushinteger(L, i+1);
        lua_gettable(L, 3);                   /* portMapArrayPtr[i] */
        prv_lua_to_c_CPSS_DXCH_PORT_MAP_STC(L, &(portMapArrayPtr[i]));
        /* override Lua => C conversion to allow negative values */
        /* -1 can be passed from lua as double float containig   */
        /* -1 or 0xFFFFFFFF                                      */
        /* lua_tointeger converts -1 to 0xFFFFFFF                */
        /* lua_tointeger converts 0xFFFFFFF to 0x7FFFFFFF        */
        {
            GT_U32 val;
            lua_getfield(L, -1, "txqPortNumber");
            val = (GT_U32)lua_tointeger(L, -1);
            if ((val & 0x7FFFFFFF) == 0x7FFFFFFF) val = 0xFFFFFFFF;
            portMapArrayPtr[i].txqPortNumber = val;
            lua_pop(L, 1);
            lua_getfield(L, -1, "physicalPortNumber");
            val = (GT_U32)lua_tointeger(L, -1);
            if ((val & 0x7FFFFFFF) == 0x7FFFFFFF) val = 0xFFFFFFFF;
            portMapArrayPtr[i].physicalPortNumber = val;
            lua_pop(L, 1);
            lua_getfield(L, -1, "interfaceNum");
            val = (GT_U32)lua_tointeger(L, -1);
            if ((val & 0x7FFFFFFF) == 0x7FFFFFFF) val = 0xFFFFFFFF;
            portMapArrayPtr[i].interfaceNum = val;
            lua_pop(L, 1);
        }
    }

    rc = cpssDxChPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArrayPtr);

    P_CALLING_API(
        cpssDxChPortPhysicalPortMapSet,
        PC_NUMBER(devNum,                       devNum,                  GT_U8,
        PC_NUMBER(portMapArraySize,             portMapArraySize,        GT_U32,
        PC_LAST_PARAM)),
        rc);

    cpssOsFree(portMapArrayPtr);

    lua_pushinteger(L, rc);
    return 1;
}

/*******************************************************************************
* wrlPrvCpssDxChPortIsRemote
*
* DESCRIPTION:
*       check whether port is remote
*
* APPLICABLE DEVICES:
*        Any
*
* NOT APPLICABLE DEVICES:
*        None
*
* INPUTS:
*       L                   - lua state
*       contains 1. device number 2. port number
*
* OUTPUTS:
*       true if port is remote, false otherwise
*
* RETURNS:
*       1 and result pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlPrvCpssDxChPortIsRemote
(
    IN lua_State                            *L
)
{
    GT_U8 devNum     = (GT_U8) lua_tointeger(L, 1);
    GT_U32 port      = (GT_U32)lua_tointeger(L, 2);
    GT_BOOL rc       = prvCpssDxChPortRemotePortCheck(devNum, port);

    lua_pushboolean(L, rc);
    return 1;
}
