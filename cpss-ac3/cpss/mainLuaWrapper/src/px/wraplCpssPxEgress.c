 /*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssPxEgress.c
*
* DESCRIPTION:
*       A lua wrapper for PX egress.
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
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/common/cpssTypes.h>
#include <generic/private/prvWraplGeneral.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpssCommon/wrapCpssDebugInfo.h>

#define QUIT_IF_NOT_OK(_rc) \
    if(GT_OK != _rc)\
    {\
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);\
        return 1;\
    }

use_prv_enum(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC);

/*******************************************************************************
* wrlCpssPxEgressSourcePortEntrySet
*
* DESCRIPTION:
*       Sets the entry per source port
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       L                   - lua state
*           1. GT_U8                                        devNum          device number
*           2. GT_PHYSICAL_PORT_NUM                         portNum         port number
*           3. CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT    infoType        the type of source port info
*           4. CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT         portInfo        the entry info
*
* OUTPUTS:
*       None
*
* RETURNS:
*       1 and result pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxEgressSourcePortEntrySet
(
    IN lua_State                            *L
)
{
    GT_STATUS                                   status;
    GT_SW_DEV_NUM                               devNum;
    GT_PHYSICAL_PORT_NUM                        portNum;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   infoType;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        portInfo;

    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_NUMBER(status, portNum, 2, GT_PHYSICAL_PORT_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, infoType, 3, CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT);
    QUIT_IF_NOT_OK(status);

    switch (infoType) 
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E:
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E:
            PARAM_STRUCT(status, &portInfo.info_802_1br, 4, CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC); 
            QUIT_IF_NOT_OK(status);
            break; 
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E:
            PARAM_STRUCT(status, &portInfo.info_dsa, 4, CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC); 
            QUIT_IF_NOT_OK(status);
            break; 
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E:
            PARAM_NUMBER(status, portInfo.notNeeded, 4, GT_U32);
            QUIT_IF_NOT_OK(status);
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E:
            PARAM_STRUCT(status, &portInfo.info_evb, 4, CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC);
            QUIT_IF_NOT_OK(status);
            break;

        default:
            break;
    }

    status = cpssPxEgressSourcePortEntrySet(devNum, portNum, infoType, &portInfo);

    lua_pushinteger(L, (lua_Integer)status);

    return 1;
}

/*******************************************************************************
* wrlCpssPxEgressSourcePortEntryGet
*
* DESCRIPTION:
*       Gets the entry per source port
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
*
* INPUTS:
*       L                   - lua state
*           1. GT_U8                                        devNum          device number
*           2. GT_PHYSICAL_PORT_NUM                         portNum         port number
*
* OUTPUTS:
*       L                   - lua state
*           1. CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT    infoType        the type of source port info
*           2. CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT         portInfo        the entry info
*
* RETURNS:
*       1, 2 or 3 and result, infoType, portInfo 
*           pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxEgressSourcePortEntryGet
(
    IN lua_State                            *L
)
{
    GT_STATUS                                   status;
    GT_SW_DEV_NUM                               devNum;
    GT_PHYSICAL_PORT_NUM                        portNum;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   infoType;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        portInfo;
    int t;

    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_NUMBER(status, portNum, 2, GT_PHYSICAL_PORT_NUM);
    QUIT_IF_NOT_OK(status);

    status = cpssPxEgressSourcePortEntryGet(devNum, portNum, &infoType, &portInfo);
    lua_pushinteger(L, (lua_Integer)status);
    if(GT_OK != status)
    {
        return 1;
    }

    prv_c_to_lua_enum(L, enumDescr_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT, infoType);

    switch (infoType) 
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E:
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E:
            prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC(L, &portInfo.info_802_1br);
            break; 
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E:
            prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC(L, &portInfo.info_dsa);
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E:
            lua_newtable(L);
            t = lua_gettop(L);
            FO_NUMBER(&portInfo, t, notNeeded, GT_U32);
            lua_settop(L, t);
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E:
            prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC(L, &portInfo.info_evb);
            break;
        default:
            break;
    }

    return 3;
}

