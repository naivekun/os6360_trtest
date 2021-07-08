 /*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssPxPortMapping.c
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
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/common/cpssTypes.h>
#include <generic/private/prvWraplGeneral.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpssCommon/wrapCpssDebugInfo.h>

use_prv_struct(CPSS_PX_PORT_MAP_STC)

#define QUIT_IF_NOT_OK(_rc) \
    if(GT_OK != _rc)\
    {\
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);\
        return 1;\
    }


/*******************************************************************************
* wrlCpssPxPortPhysicalPortMapSet
*
* DESCRIPTION:
*       configure ports mapping
*
* APPLICABLE DEVICES:
*        Pipe
*
* NOT APPLICABLE DEVICES:
*        None.
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
int wrlCpssPxPortPhysicalPortMapSet
(
    IN lua_State                            *L
)
{
    GT_STATUS                                   rc;
    GT_U32                                      i;
    GT_SW_DEV_NUM                               devNum;
    GT_U32                                      portMapArraySize;
    static CPSS_PX_PORT_MAP_STC                 *portMapArrayPtr = NULL;
    P_CALLING_CHECKING;


    devNum            = (GT_SW_DEV_NUM)                  lua_tonumber(L, 1);
    portMapArraySize  = (GT_U32)                         lua_tonumber(L, 2);

    portMapArrayPtr = cpssOsMalloc(portMapArraySize*sizeof(CPSS_PX_PORT_MAP_STC));
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
        prv_lua_to_c_CPSS_PX_PORT_MAP_STC(L, &(portMapArrayPtr[i]));
    }

    rc = cpssPxPortPhysicalPortMapSet(devNum, portMapArraySize, portMapArrayPtr);

    P_CALLING_API(
        cpssPxPortPhysicalPortMapSet,
        PC_NUMBER(devNum,                       devNum,                  GT_SW_DEV_NUM,
        PC_NUMBER(portMapArraySize,             portMapArraySize,        GT_U32,
        PC_LAST_PARAM)),
        rc);

    cpssOsFree(portMapArrayPtr);

    lua_pushinteger(L, rc);
    return 1;
}

/*******************************************************************************
* wrlCpssPxPipePortResourseConfigGet
*
* DESCRIPTION:
*       Get resources allocated to the port
*
* APPLICABLE DEVICES:
*       Pipe
*
* NOT APPLICABLE DEVICES:
*       None.
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
int wrlCpssPxPipePortResourseConfigGet
(
    IN lua_State                            *L
)
{
    GT_STATUS   status = GT_OK;
    GT_SW_DEV_NUM   devNum;
    PRV_CPSS_PX_PIPE_PORT_RESOURCE_STC  resource;
    GT_PHYSICAL_PORT_NUM    portNum;

    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_NUMBER(status, portNum, 2, GT_PHYSICAL_PORT_NUM);
    QUIT_IF_NOT_OK(status);

    status = prvCpssPxPipePortResourseConfigGet(devNum, portNum, &resource);
    QUIT_IF_NOT_OK(status);

    lua_pushinteger(L, status);
    lua_pushinteger(L, resource.rxdmaScdmaIncomingBusWidth);
    lua_pushinteger(L, resource.txdmaScdmaHeaderTxfifoThrshold);
    lua_pushinteger(L, resource.txdmaScdmaPayloadTxfifoThrshold);
    lua_pushinteger(L, resource.txfifoScdmaShiftersOutgoingBusWidth);
    lua_pushinteger(L, resource.txfifoScdmaPayloadStartTransmThreshold);

    return 6;
}

/*******************************************************************************
* wrlCpssDxChPortPhysicalPortMapShadowDBGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*
*
* INPUTS:
*        L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       1; error code and error message, if error occures.
*       12; GT_OK and port map shadows DB details.
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS wrlCpssPxPortPhysicalPortMapShadowDBGet
(
    IN lua_State            *L
)
{

    GT_STATUS   status = GT_OK;
    GT_SW_DEV_NUM devNum;
    GT_PHYSICAL_PORT_NUM firstPhysicalPortNumber;
    static CPSS_PX_DETAILED_PORT_MAP_STC portMapShadow;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr = &portMapShadow;

    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_NUMBER(status, firstPhysicalPortNumber, 2, GT_PHYSICAL_PORT_NUM);
    QUIT_IF_NOT_OK(status);

    status = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum,
        firstPhysicalPortNumber,
        &portMapShadowPtr);
    QUIT_IF_NOT_OK(status);

    lua_pushinteger(L, status);
    lua_pushboolean(L, portMapShadowPtr->valid);
    lua_pushinteger(L, portMapShadowPtr->portMap.mappingType);
    lua_pushinteger(L, portMapShadowPtr->portMap.macNum);
    lua_pushinteger(L, portMapShadowPtr->portMap.dmaNum);
    lua_pushinteger(L, portMapShadowPtr->portMap.txqNum);

    return 6;
}
