/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssPxIngressHash.c
*
* DESCRIPTION:
*       A lua wrapper for ingress hash calculation.
*
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/common/cpssTypes.h>
#include <generic/private/prvWraplGeneral.h>

#include <cpss/px/ingress/cpssPxIngressHash.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpssCommon/wrapCpssDebugInfo.h>

#define QUIT_IF_NOT_OK(_rc) \
    if(GT_OK != _rc)\
    {\
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);\
        return 1;\
    }

use_prv_struct(CPSS_PX_INGRESS_HASH_UDBP_STC);
use_prv_enum(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);
use_prv_enum(CPSS_PX_INGRESS_HASH_MODE_ENT);

/*******************************************************************************
* wrlCpssPxIngressHashPacketTypeHashModeSet
*
* DESCRIPTION:
*       Sets up hash calculation mode.
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
*           1. GT_U8                                devNum         device number
*           2. CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType hash packet type
*           3. CPSS_PX_INGRESS_HASH_MODE_ENT        hashMode       hash calculation mode
*           4. GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] array of offsets
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
int wrlCpssPxIngressHashPacketTypeHashModeSet
(
    IN lua_State                            *L
)
{
    GT_STATUS                            status;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    CPSS_PX_INGRESS_HASH_MODE_ENT        hashMode;
    GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    GT_U32                               ii;

    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, hashPacketType, 2, CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, hashMode, 3, CPSS_PX_INGRESS_HASH_MODE_ENT);
    QUIT_IF_NOT_OK(status);

    /* Create array if bit offsets */
    if(CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E == hashMode)
    {
        for(ii = 1; ii <= CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS; ii++)
        {
            lua_pushinteger(L, ii);
            lua_gettable(L, 4);
            if (!lua_isnumber(L, -1))
            {
                lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
                return 1;
            }
            bitOffsetsArr[ii - 1] = lua_tonumber(L, -1);
        }
    }

    status = cpssPxIngressHashPacketTypeHashModeSet(devNum, hashPacketType,
        hashMode, bitOffsetsArr);
    lua_pushinteger(L, (lua_Integer)status);
    return 1;
}

/*******************************************************************************
* wrlCpssPxIngressHashPacketTypeHashModeGet
*
* DESCRIPTION:
*       Get hash calculation mode.
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
*           1. GT_U8                                devNum         device number
*           2. CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType hash packet type
*
* OUTPUTS:
*       L                   - lua state
*           1. CPSS_PX_INGRESS_HASH_MODE_ENT        hashMode       hash calculation mode
*           2. GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] array of offsets
*
* RETURNS:
*       1, 2 or 3 and result, hashMode (if without errors), bitOffsetsArr (if selected bits)
*           pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxIngressHashPacketTypeHashModeGet
(
    IN lua_State                            *L
)
{
    GT_STATUS                            status;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    CPSS_PX_INGRESS_HASH_MODE_ENT        hashMode;
    GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS];
    GT_U32                               ii;

    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, hashPacketType, 2, CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);
    QUIT_IF_NOT_OK(status);

    status = cpssPxIngressHashPacketTypeHashModeGet(devNum, hashPacketType,
        &hashMode, bitOffsetsArr);

    lua_pushinteger(L, (lua_Integer)status);
    if(GT_OK != status)
    {
        return 1;
    }

    lua_pushinteger(L, (lua_Integer)hashMode);
    if(CPSS_PX_INGRESS_HASH_MODE_CRC32_E == hashMode)
    {
        return 2;
    }

    for(ii = 0; ii < CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS; ii++)
    {
        lua_createtable(L, CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS, 0);
        for (ii = 0; ii < CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS; ii++)
        {
            lua_pushinteger(L, bitOffsetsArr[ii]);
            lua_rawseti(L, -2, ii + 1);
        }
    }
    return 3;
}

/*******************************************************************************
* wrlCpssPxIngressHashPacketTypeEntrySet
*
* DESCRIPTION:
*       Get packet hash key configuration for selected packet type.
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
*           1. GT_U8                                devNum         device number
*           2. CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType hash packet type
*           3. CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr          - user-defined byte pairs array of 21 elements.
*           4. GT_BOOL                              sourcePortEnable - enable usage of source port in packet hash.
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
int wrlCpssPxIngressHashPacketTypeEntrySet
(
    IN lua_State                            *L
)
{
    GT_STATUS                            status;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    GT_BOOL                              sourcePortEnable;
    GT_U32                               ii;

    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, hashPacketType, 2, CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);
    QUIT_IF_NOT_OK(status);
    PARAM_BOOL(status, sourcePortEnable, 4);
    QUIT_IF_NOT_OK(status);

    for(ii = 0; ii < CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS; ii++)
    {
        lua_pushinteger(L, ii + 1);
        lua_gettable(L, 3);
        prv_lua_to_c_CPSS_PX_INGRESS_HASH_UDBP_STC(L, &udbpArr[ii]);
    }

    status = cpssPxIngressHashPacketTypeEntrySet(devNum, hashPacketType,
        udbpArr, sourcePortEnable);

    lua_pushinteger(L, (lua_Integer)status);
    return 1;
}

/*******************************************************************************
* wrlCpssPxIngressHashPacketTypeEntryGet
*
* DESCRIPTION:
*       Get packet hash key configuration for selected packet type.
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
*           1. GT_U8                                devNum         device number
*           2. CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType hash packet type
*
* OUTPUTS:
*       L                   - lua state
*           1. CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr          - user-defined byte pairs array of 21 elements.
*           2. GT_BOOL                              sourcePortEnable - enable usage of source port in packet hash.
*
* RETURNS:
*       1 or 3 and result, udbpArr (if without errors), sourcePortEnable(if without errors)
*           pushed to lua stack (see OUTPUTS)
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxIngressHashPacketTypeEntryGet
(
    IN lua_State                            *L
)
{
    GT_STATUS                            status;
    GT_SW_DEV_NUM                        devNum;
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType;
    CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];
    GT_BOOL                              sourcePortEnable;
    GT_U32                               ii;
    GT_U32                               t;

    status = GT_OK;
    PARAM_NUMBER(status, devNum, 1, GT_SW_DEV_NUM);
    QUIT_IF_NOT_OK(status);
    PARAM_ENUM(status, hashPacketType, 2, CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);
    QUIT_IF_NOT_OK(status);

    status = cpssPxIngressHashPacketTypeEntryGet(devNum, hashPacketType,
        udbpArr, &sourcePortEnable);
    lua_pushinteger(L, (lua_Integer)status);
    if(GT_OK != status)
    {
        return 1;
    }

    lua_createtable(L, CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS, 0);
    t = lua_gettop(L);
    for(ii = 0; ii < CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS; ii++)
    {
        prv_c_to_lua_CPSS_PX_INGRESS_HASH_UDBP_STC(L, &udbpArr[ii]);
        lua_rawseti(L, t, ii + 1);
    }

    if(GT_TRUE == sourcePortEnable)
    {
        lua_pushboolean(L, 1);
    }
    else
    {
        lua_pushboolean(L, 0);
    }
    return 3;
}
