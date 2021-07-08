/**
********************************************************************************
* @file all_refhal_lua_c_type_wrappers.c
*
* @brief A lua type wrapper
* It implements support for the following types:
* enum  CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT
* struct CPSS_PX_HAL_BPE_QOSMAP_STC
* enum  CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT
* struct CPSS_PX_HAL_BPE_PORT_QOS
* enum  CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT
* enum  CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT
* enum  CPSS_PX_HAL_BPE_PORT_MODE_ENT
*
* @version   15
********************************************************************************
*/
/*************************************************************************
* ./wrappers/all_refhal_lua_c_type_wrappers.c
*
* DESCRIPTION:
*       A lua type wrapper
*       It implements support for the following types:
*           enum    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT
*           struct  CPSS_PX_HAL_BPE_QOSMAP_STC
*           enum    CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT
*           struct  CPSS_PX_HAL_BPE_PORT_QOS
*           enum    CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT
*           enum    CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT
*           enum    CPSS_PX_HAL_BPE_PORT_MODE_ENT
*
* DEPENDENCIES:
*
* COMMENTS:
*       Generated at Wed Jun  7 16:38:08 2017
*       /local/store/git/cpss_4.2/cpss.super/cpss/mainLuaWrapper/scripts/make_type_wrapper.py -i /local/store/git/cpss_4.2/cpss.super/cpss/compilation_root/type_info_file -p /local/store/git/cpss_4.2/cpss.super/cpss/mainLuaWrapper/scripts/already_implemented.lst -X /local/store/git/cpss_4.2/cpss.super/cpss/mainLuaWrapper/scripts/excludelist -X ./types_excludelist -C /local/store/git/cpss_4.2/cpss.super/cpss -M /local/store/git/cpss_4.2/cpss.super/cpss/mainLuaWrapper/scripts/lua_C_mapping -N -F NONE -I referenceHal -o ./wrappers/all_refhal_lua_c_type_wrappers.c
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
**************************************************************************/
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpssPxHalBpe.h>

/***** declarations ********/

use_prv_enum(CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT);
use_prv_struct(CPSS_PX_HAL_BPE_QOSMAP_STC);
use_prv_enum(CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT);
use_prv_struct(CPSS_PX_HAL_BPE_PORT_QOS);
use_prv_enum(CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT);
use_prv_enum(CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT);
use_prv_enum(CPSS_PX_HAL_BPE_PORT_MODE_ENT);

/***** declarations ********/

enumDescr enumDescr_CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_HAL_BPE_INTERFACE_MODE_IDLE_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_INTERFACE_MODE_TRUSTED_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_INTERFACE_MODE_UPSTREAM_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_INTERFACE_MODE_EXTENDED_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_INTERFACE_MODE_CASCADE_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_INTERFACE_MODE_INTERNAL_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT);

void prv_lua_to_c_CPSS_PX_HAL_BPE_QOSMAP_STC(
    lua_State *L,
    CPSS_PX_HAL_BPE_QOSMAP_STC *val
)
{
    F_NUMBER(val, -1, newPcp, GT_U32);
    F_NUMBER(val, -1, newDei, GT_U32);
}

void prv_c_to_lua_CPSS_PX_HAL_BPE_QOSMAP_STC(
    lua_State *L,
    CPSS_PX_HAL_BPE_QOSMAP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, newPcp, GT_U32);
    FO_NUMBER(val, t, newDei, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_HAL_BPE_QOSMAP_STC);

enumDescr enumDescr_CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_IP_TCP_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_PORT_BASE_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_IP_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT);

void prv_lua_to_c_CPSS_PX_HAL_BPE_PORT_QOS(
    lua_State *L,
    CPSS_PX_HAL_BPE_PORT_QOS *val
)
{
    F_STRUCT(val, -1, defaultQos, CPSS_PX_HAL_BPE_QOSMAP_STC);
/*
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
*/
#if 0
    unhandled line
CPSS_PX_HAL_BPE_QOSMAP_STC qosMap[8][2];
#endif
}

void prv_c_to_lua_CPSS_PX_HAL_BPE_PORT_QOS(
    lua_State *L,
    CPSS_PX_HAL_BPE_PORT_QOS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, defaultQos, CPSS_PX_HAL_BPE_QOSMAP_STC);
/*
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
*/
#if 0
    unhandled line
CPSS_PX_HAL_BPE_QOSMAP_STC qosMap[8][2];
#endif
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_HAL_BPE_PORT_QOS);

enumDescr enumDescr_CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_HAL_BPE_INTERFACE_PORT_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_INTERFACE_NONE_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_TRUNK_CASCADE_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_TRUNK_INTERNAL_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT);

enumDescr enumDescr_CPSS_PX_HAL_BPE_PORT_MODE_ENT[] = {
    ENUM_ENTRY(CPSS_PX_HAL_BPE_PORT_MODE_IDLE_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_PORT_MODE_TRUSTED_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_PORT_MODE_UPSTREAM_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_PORT_MODE_EXTENDED_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_PORT_MODE_CASCADE_E),
    ENUM_ENTRY(CPSS_PX_HAL_BPE_PORT_MODE_INTERNAL_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_PX_HAL_BPE_PORT_MODE_ENT);


