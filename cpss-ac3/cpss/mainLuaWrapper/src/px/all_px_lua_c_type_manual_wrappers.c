/*************************************************************************
* all_px_lua_c_type_manual_wrappers.c
*
* DESCRIPTION:
*       A MANUAL lua type wrapper , for those that not generated 'automatically'
*       It implements support for the following types:
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/
/*#include <px/all_px_lua_c_type_wrappers.c>*/
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <mainLuaWrapper/wraplNetIf.h>
#include <cpss/common/diag/cpssCommonDiag.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>

typedef struct{

    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC           eventInfo;
    GT_U32                                          errorCounter;
    GT_U32                                          failedRow;
    GT_U32                                          failedSegment;
    GT_U32                                          failedSyndrome;

} CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC;
/***** declarations ********/
use_prv_struct(GT_IPV6ADDR);
use_prv_struct(GT_IPADDR);
use_prv_print_struct(GT_IPADDR);
use_prv_print_struct(GT_IPV6ADDR);
use_prv_struct(CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC);
use_prv_enum(CPSS_EVENT_MASK_SET_ENT);
use_prv_enum(CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);
use_prv_enum(CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT);
use_prv_enum(CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT);
use_prv_enum(CPSS_PX_LOGICAL_TABLE_ENT);
/************************/
#define PRV_UNION_LUA_TO_C_START(uniontype,member) \
void prv_lua_to_c_ ## uniontype ## _ ## member( \
    lua_State         *L, \
    uniontype         *val \
) \
{
#define PRV_UNION_LUA_TO_C_END() \
}
/************************/
#define PRV_UNION_C_TO_LUA_START(uniontype,member) \
void prv_c_to_lua_ ## uniontype ## _ ## member( \
    lua_State         *L, \
    uniontype         *val \
) \
{ \
    int t; \
    lua_newtable(L); \
    t = lua_gettop(L);
#define PRV_UNION_C_TO_LUA_END() \
}
/************************/
#define add_mgm_type_union(typeName,member,membertype) \
int mgmType_to_c_##membertype(lua_State *L); \
int mgmType_to_c_ ## typeName ## _ ## member( \
    lua_State *L \
) \
{                                        \
    return mgmType_to_c_##membertype(L); \
} \
 \
int mgmType_to_lua_ ## typeName ## _ ## member( \
    lua_State *L \
) \
{ \
    typeName *val; \
    if (lua_isnil(L, 1)) \
        return 1; \
    if (!lua_isuserdata(L, 1)) \
    { \
        lua_pushnil(L); \
        return 1; \
    } \
 \
    val = (typeName*)lua_touserdata(L, 1); \
    prv_c_to_lua_ ## typeName ## _ ## member(L, val); \
 \
    return 1; \
}
/************************/
#define UNION_MEMBER_STRUCT(typename,member,membertype) \
PRV_UNION_LUA_TO_C_START(typename,member) \
    F_STRUCT(val, -1, member, membertype); \
PRV_UNION_LUA_TO_C_END() \
 \
PRV_UNION_C_TO_LUA_START(typename,member) \
    FO_STRUCT(val, t, member, membertype); \
PRV_UNION_C_TO_LUA_END() \
add_mgm_type_union(typename,member,membertype);



/* start of function for union for 'C-to-lua' */
#define UNION_FO_IMPLEMENT_START(unionName)            \
    use_prv_struct(unionName);                         \
                                                       \
    void _prv_c_to_lua_##unionName(                     \
        lua_State *L,                                  \
        unionName *val                                 \
    )                                                  \
    {                                                  \
        int t;                                         \
        lua_newtable(L);                               \
        t = lua_gettop(L);

/* implement STC member in the union , to be used with : UNION_FO_IMPLEMENT_START */
#define UNION_FO_STC_MEMBER(memberType,mamberName) \
        FO_STRUCT(val, t, mamberName, memberType);

/* implement number member in the union , to be used with : UNION_FO_IMPLEMENT_START */
#define UNION_FO_NUMBER_MEMBER(memberType,mamberName) \
        FO_NUMBER(val, t, mamberName, memberType);

#define UNION_FO_IMPLEMENT_END       \
        lua_settop(L, t);            \
    }


/* start of function for union for 'LUA-to-C' */
#define UNION_F_IMPLEMENT_START(unionName)             \
    /* just prototype */                               \
    void prv_c_to_lua_##unionName(                     \
        lua_State *L,                                  \
        unionName *val                                 \
    );                                                 \
    /*implementation*/                                 \
    void _prv_lua_to_c_##unionName(                     \
        lua_State *L,                                  \
        unionName *val                                 \
    )                                                  \
    {                                                  \
        /* NOTE: not expected to be called , because caller should call to final*/ \
        /* member that defined by 'UNION_F_STC_MEMBER'                          */ \
        L = L;                                         \
        val = val;                                     \
    }                                                  \
    /*add_mgm_type(unionName);*/

/* implement STC member in the union , to be used with : UNION_F_IMPLEMENT_START */
#define UNION_F_STC_MEMBER(unionName,memberType,mamberName) \
    use_prv_struct(memberType); \
    UNION_MEMBER_STRUCT(unionName, mamberName, memberType)

/* implement number member in the union , to be used with : UNION_F_IMPLEMENT_START */
#define UNION_F_NUMBER_MEMBER(unionName,memberType,mamberName) \
    PRV_UNION_LUA_TO_C_START(unionName,mamberName) \
        F_NUMBER(val, -1, mamberName, memberType); \
    PRV_UNION_LUA_TO_C_END()                       \
                                                   \
    PRV_UNION_C_TO_LUA_START(unionName,mamberName) \
        FO_NUMBER(val, t, mamberName, memberType); \
    PRV_UNION_C_TO_LUA_END()                       \
    add_mgm_type_union(unionName,mamberName,memberType);

#define UNION_F_IMPLEMENT_END



/* start of function for union for 'LUA-to-C' */
#define STRUCT_F_IMPLEMENT_START(structName)                 \
    use_prv_struct(structName);                              \
    void prv_lua_to_c_##structName(                          \
        lua_State *L,                                        \
        structName *val                                      \
    )                                                        \
    {                                                        \
        int t = -1;

/* implement number member in the union , to be used with : STRUCT_IMPLEMENT_START */
#define F_NUMBER_MEMBER(memberType,mamberName) \
        F_NUMBER(val, t, mamberName, memberType);

#define STRUCT_F_IMPLEMENT_END       \
    }

/* start of function for union for 'LUA-to-C' */
#define STRUCT_FO_IMPLEMENT_START(structName)                \
    add_mgm_type(structName);                                \
    void prv_c_to_lua_##structName(                          \
        lua_State *L,                                        \
        structName *val                                      \
    )                                                        \
    {                                                        \
        int t;                                               \
        lua_newtable(L);                                     \
        t = lua_gettop(L);                                   \

/* implement number member in the union , to be used with : STRUCT_IMPLEMENT_START */
#define FO_NUMBER_MEMBER(memberType,mamberName) \
        FO_NUMBER(val, t, mamberName, memberType);

#define STRUCT_FO_IMPLEMENT_END       \
        lua_settop(L, t);             \
    }


/* start of enum support*/
#define ENUM_IMPLEMENT_START(enumName)      \
    use_prv_enum(enumName);                 \
    add_mgm_enum(enumName);                 \
    enumDescr enumDescr_##enumName[] =      \
    {

#define ENUM_IMPLEMENT_END                 \
        { 0, NULL }                        \
    };

#ifndef CHX_FAMILY
/*************************************************************************
* prv_lua_to_c_APP_DEMO_MICRO_INIT_STC
*
* Description:
*       Convert to "c" APP_DEMO_MICRO_INIT_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_APP_DEMO_MICRO_INIT_STC
(
    lua_State                               *L,
    APP_DEMO_MICRO_INIT_STC  *val
)
{
    F_NUMBER(val,   -1, deviceId, GT_U32);
    F_NUMBER(val,   -1, mngInterfaceType, GT_U32);
    F_NUMBER(val,   -1, coreClock, GT_U32);
    F_NUMBER(val,   -1, featuresBitmap, GT_U32);
}


/*************************************************************************
* prv_c_to_lua_APP_DEMO_MICRO_INIT_STC
*
* Description:
*       Convert APP_DEMO_MICRO_INIT_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_APP_DEMO_MICRO_INIT_STC(
    lua_State                               *L,
    APP_DEMO_MICRO_INIT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val,  t, deviceId, GT_U32);
    FO_NUMBER(val,  t, mngInterfaceType, GT_U32);
    FO_NUMBER(val,  t, coreClock, GT_U32);
    FO_NUMBER(val,  t, featuresBitmap, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(APP_DEMO_MICRO_INIT_STC);
#endif

#if !defined(CHX_FAMILY)
typedef enum { CPSS_NET_RX_CPU_CODE_ENT_E } CPSS_NET_RX_CPU_CODE_ENT;
enumDescr enumDescr_CPSS_NET_RX_CPU_CODE_ENT[] = {
    ENUM_ENTRY(CPSS_NET_RX_CPU_CODE_ENT_E),
    { 0, NULL }
};
add_mgm_enum(CPSS_NET_RX_CPU_CODE_ENT);
#endif /* !defined(CHX_FAMILY) */

void prv_lua_to_c_CPSS_PX_INGRESS_IP2ME_ENTRY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC *val
)
{
    F_BOOL(val, -1, valid);
    F_NUMBER(val, -1, prefixLength, GT_U32);
    F_BOOL(val, -1, isIpv6);
    /* done manually [[[ */
    if(val->isIpv6)
    {
        F_STRUCT_CUSTOM_N(&(val->ipAddr), -1, ipv6Addr, ipAddr, GT_IPV6ADDR)
    }
    else
    {
        F_STRUCT_CUSTOM_N(&(val->ipAddr), -1, ipv4Addr, ipAddr, GT_IPADDR)
    }
    /* ]]] done manually */
}

void prv_c_to_lua_CPSS_PX_INGRESS_IP2ME_ENTRY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, valid);
    FO_NUMBER(val, t, prefixLength, GT_U32);
    FO_BOOL(val, t, isIpv6);
    if(val->isIpv6)
    {
        FO_STRUCT_N(&(val->ipAddr), t, ipv6Addr, ipAddr, GT_IPV6ADDR);
    }
    else
    {
        FO_STRUCT_N(&(val->ipAddr), t, ipv4Addr, ipAddr, GT_IPADDR);
    }
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_IP2ME_ENTRY_STC);


void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, eventInfo, CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC);
    FO_NUMBER(val, t, errorCounter, GT_U32);
    FO_NUMBER(val, t, failedRow, GT_U32);
    FO_NUMBER(val, t, failedSegment, GT_U32);
    FO_NUMBER(val, t, failedSyndrome, GT_U32);
    lua_settop(L, t);
}
typedef struct{

    CPSS_PX_LOGICAL_TABLE_ENT                              table;
    GT_BOOL                                                errorCountEnable;
    CPSS_EVENT_MASK_SET_ENT                                eventMask;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT          errorType;
    GT_BOOL                                                injectEnable;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT         injectMode;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT protectionType;
}CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC;

void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, table, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_BOOL(val, t, errorCountEnable);
    FO_ENUM(val, t, eventMask, CPSS_EVENT_MASK_SET_ENT);
    FO_ENUM(val, t, errorType, CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);
    FO_BOOL(val, t, injectEnable);
    FO_ENUM(val, t, injectMode, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT);
    FO_ENUM(val, t, protectionType,  CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT);
    lua_settop(L, t);
}

typedef struct{

    CPSS_PX_LOGICAL_TABLE_ENT                              table;
    GT_U32                                                 tableSize;
}CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC;



void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC
(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, table, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_NUMBER(val, t, tableSize, GT_U32);

    lua_settop(L, t);
}




/*******************
    ENUM: CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT
*******************/
/*
ENUM_IMPLEMENT_START(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT)
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E),
    ENUM_ENTRY(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E),
ENUM_IMPLEMENT_END
*/
/*******************
    STRUCT: CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC
*******************/
/*
STRUCT_F_IMPLEMENT_START(CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC)
    F_NUMBER_MEMBER(GT_U32,pcid);
    F_NUMBER_MEMBER(GT_U32,srcPortNum);
STRUCT_F_IMPLEMENT_END
STRUCT_FO_IMPLEMENT_START(CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC)
    FO_NUMBER_MEMBER(GT_U32,pcid);
    FO_NUMBER_MEMBER(GT_U32,srcPortNum);
STRUCT_FO_IMPLEMENT_END
*/

/*******************
    STRUCT: CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC
*******************/
/*
STRUCT_F_IMPLEMENT_START(CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC)
    F_NUMBER_MEMBER(GT_U32,pvid);
    F_NUMBER_MEMBER(GT_U32,srcPortNum);
STRUCT_F_IMPLEMENT_END

STRUCT_FO_IMPLEMENT_START(CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC)
    FO_NUMBER_MEMBER(GT_U32,pvid);
    FO_NUMBER_MEMBER(GT_U32,srcPortNum);
STRUCT_FO_IMPLEMENT_END
*/

/*******************
    UNION: CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT
*******************/

/*
UNION_F_STC_MEMBER(     CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT,   CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC,info_802_1br)
UNION_F_STC_MEMBER(     CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT,   CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC,info_dsa)
*/

/*
void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC(
    lua_State         *L,
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT          *val);

int mgmType_to_c_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT_info_802_1br(
    lua_State *L
)
{
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT *val = NULL;
    lua_newuserdata(L, sizeof(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT));
    if (lua_isnil(L, 1))
    {
        return 1;
    }

    val = (CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT*)lua_touserdata(L, -1);
    cmdOsMemSet(val, 0, sizeof(*val));
    lua_pushvalue(L, 1);
    prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC(L, val);
    lua_pop(L, 1);
    return 1;
}
*/

/*
UNION_F_IMPLEMENT_START(    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT)
    UNION_F_STC_MEMBER(     CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT,   CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC,info_802_1br)
    UNION_F_STC_MEMBER(     CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT,   CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC,info_dsa)
    UNION_F_NUMBER_MEMBER(  CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT,   GT_U32,notNeeded)
UNION_F_IMPLEMENT_END
*/
/*
UNION_FO_IMPLEMENT_START(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT)
    UNION_FO_STC_MEMBER(CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC,info_802_1br)
    UNION_FO_STC_MEMBER(CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC,info_dsa)
    UNION_FO_NUMBER_MEMBER(GT_U32,notNeeded)
UNION_FO_IMPLEMENT_END
*/

/*DXCH : CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT*/
/*CPSS_DXCH_TUNNEL_START_CONFIG_UNT , CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC*/

use_prv_struct(PACKET_STC);
use_prv_print_struct(PACKET_STC);

/*******************************************/
/* PX_RX_DATA_STC                             */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_PX_RX_DATA_STC
*
* Description:
*       Convert to "c" PX_RX_DATA_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_PX_RX_DATA_STC(
    lua_State *L,
    PX_RX_DATA_STC *val
)
{
    F_NUMBER(val, -1, devNum, GT_U8);
    F_NUMBER(val, -1, portNum, GT_PHYSICAL_PORT_NUM);
    F_NUMBER(val, -1, originByteCount, GT_U32);
    F_NUMBER(val, -1, timestamp, GT_U32);
    /* special handling for packet: can be table, string, userdata */
    F_STRUCT_CUSTOM(val, -1, packet, PACKET_STC);
}

/*************************************************************************
* prv_c_to_lua_PX_RX_DATA_STC
*
* Description:
*       Convert PX_RX_DATA_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_PX_RX_DATA_STC(
    lua_State *L,
    PX_RX_DATA_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, devNum, GT_U8);
    FO_NUMBER(val, t, portNum, GT_PHYSICAL_PORT_NUM);
    FO_NUMBER(val, t, originByteCount, GT_U32);
    FO_NUMBER(val, t, timestamp, GT_U32);
    FO_STRUCT(val, t, packet, PACKET_STC);
    lua_settop(L, t);
}
add_mgm_type(PX_RX_DATA_STC);
