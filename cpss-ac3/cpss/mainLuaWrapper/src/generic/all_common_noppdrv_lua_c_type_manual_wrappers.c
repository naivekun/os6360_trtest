/*************************************************************************
* all_common_noppdrv_lua_c_type_manual_wrappers.c
*
* DESCRIPTION:
*       A lua type wrapper
*       It implements support for the following types:
*           enum    GT_BOOL
*           struct  WRAP_DEV_PORT_STC
*           struct  GT_ETHERADDR
*           struct  CPSS_INTERFACE_INFO_STC
*           struct  GT_IPADDR
*           struct  GT_IPV6ADDR
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/

#include <generic/private/prvWraplGeneral.h>
#include <cpssCommon/wraplCpssDeviceInfo.h>
#include <cpssCommon/wrapCpssDebugInfo.h>
#include <cpss/common/config/cpssGenCfg.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/common/port/private/prvCpssPortManagerLuaTypes.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <mainLuaWrapper/wraplNetIf.h>
#include <string.h>



/***** MACROSES *****/
#define F_UNION_MEMBER_NUMBER(unionObject,Lindex,field,fieldtype) \
    lua_getfield(L, Lindex, #field); \
    if (lua_type(L, -1) == LUA_TNUMBER) \
    { \
        unionObject.field = (fieldtype)lua_tointeger(L, -1); \
    } \
    else \
    { \
        unionObject.field = (fieldtype)0; \
    } \
    lua_pop(L, 1);

#define F_UNION_MEMBER_STRUCT(unionObject,Lindex,field,fieldtype) \
    lua_getfield(L, Lindex, #field); \
    if (lua_type(L, -1) == LUA_TTABLE) \
    { \
        prv_lua_to_c_ ## fieldtype(L, &(unionObject.field)); \
    } \
    lua_pop(L, 1);

#define FO_UNION_MEMBER_NUMBER(unionObject,Lindex,field,fieldtype) \
    lua_pushnumber(L, unionObject.field); \
    lua_setfield(L, Lindex, #field);

#define FO_UNION_MEMBER_STRUCT(unionObject,Lindex,field,fieldtype) \
    prv_c_to_lua_ ## fieldtype(L, &(unionObject.field)); \
    lua_setfield(L, Lindex, #field);



#define F_UNION_MEMBER_ENUM(unionObject, Lindex, field, fieldtype) \
    lua_getfield(L, Lindex, #field); \
    unionObject.field = (fieldtype)prv_lua_to_c_enum(L, enumDescr_ ## fieldtype); \
    lua_pop(L, 1);

#define F_UNION_MEMBER_BOOL(unionObject, Lindex, field) \
    lua_getfield(L, Lindex, #field); \
    if (lua_type(L, -1) == LUA_TBOOLEAN) \
    { \
        unionObject.field = lua_toboolean(L, -1) ? GT_TRUE : GT_FALSE; \
    } \
    else \
    { \
        unionObject.field = GT_FALSE; \
    } \
    lua_pop(L, 1);


#define FO_UNION_MEMBER_ENUM(unionObject, Lindex, field, fieldtype) \
    prv_c_to_lua_enum(L, enumDescr_ ## fieldtype, unionObject.field); \
    if (lua_isnil(L, -1)) \
        lua_pop(L, 1); \
    else \
        lua_setfield(L, Lindex, #field);

#define FO_UNION_MEMBER_BOOL(unionObject, Lindex, field) \
    lua_pushboolean(L, (unionObject.field == GT_TRUE) ? 1 : 0); \
    lua_setfield(L, Lindex, #field);


#define PRV_UNION_LUA_TO_C_START(uniontype,member) \
void prv_lua_to_c_ ## uniontype ## _ ## member( \
    lua_State         *L, \
    uniontype         *val \
) \
{
#define PRV_UNION_LUA_TO_C_END() \
}

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


#define add_mgm_type_union(typeName,member) \
int mgmType_to_c_ ## typeName ## _ ## member( \
    lua_State *L \
) \
{ \
    typeName *val = NULL; \
    lua_newuserdata(L, sizeof(typeName)); \
    if (lua_isnil(L, 1)) \
    { \
        return 1; \
    } \
 \
    val = (typeName*)lua_touserdata(L, -1); \
    cmdOsMemSet(val, 0, sizeof(*val)); \
    lua_pushvalue(L, 1); \
    prv_lua_to_c_ ## typeName ## _ ## member(L, val); \
    lua_pop(L, 1); \
    return 1; \
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


#define UNION_MEMBER_STRUCT(typename,member,membertype) \
PRV_UNION_LUA_TO_C_START(typename,member) \
    F_STRUCT(val, -1, member, membertype); \
PRV_UNION_LUA_TO_C_END() \
 \
PRV_UNION_C_TO_LUA_START(typename,member) \
    FO_STRUCT(val, t, member, membertype); \
PRV_UNION_C_TO_LUA_END() \
 \
add_mgm_type_union(typename,member);

#ifndef CHX_FAMILY

/* Slice bounds of IP v6 array  */
#define GT_IPV6_BEGIN_OCTET     0
#define GT_IPV6_END_OCTET       3

/***** declarations ********/
use_prv_enum(CPSS_INTERFACE_TYPE_ENT);

use_prv_struct(WRAP_DEV_PORT_STC);
use_prv_struct(GT_ETHERADDR);
use_prv_struct(CPSS_INTERFACE_INFO_STC);
use_prv_struct(CPSS_PORTS_BMP_STC);
use_prv_struct(GT_IPADDR);
use_prv_struct(GT_IPV6ADDR);
use_prv_struct(CPSS_GEN_CFG_DEV_INFO_STC);

use_prv_print_struct(CPSS_INTERFACE_INFO_STC);
use_prv_print_struct(CPSS_PORTS_BMP_STC)
use_prv_print_struct(GT_ETHERADDR);
use_prv_print_struct(GT_IPADDR);
use_prv_print_struct(GT_IPV6ADDR);

/**
use_prv_enum(CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT);
use_prv_enum(CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
use_prv_enum(CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT); */


enumDescr enumDescr_GT_BOOL[] = {
    ENUM_ENTRY(GT_FALSE),
    ENUM_ENTRY(GT_TRUE),
    { 0, NULL }
};
add_mgm_enum(GT_BOOL);

static int hexdigit2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
        return -2;
    return -1;
}

/*******************************************/
/* PACKET_STC                              */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_PACKET_STC
*
* Description:
*       Convert to "c" PACKET_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_PACKET_STC(
    lua_State *L,
    PACKET_STC *val
)
{
    int ltype;
    int actual_len = 0;
    ltype = lua_type(L, -1);
    if (ltype != LUA_TTABLE && ltype != LUA_TSTRING && ltype != LUA_TUSERDATA)
    {
        return;
    }
    if (ltype == LUA_TTABLE)
    {
        /* table: { "len": LUA_TNUMBER, "data": LUA_TSTRING in hex or LUA_TUSERDATA } */
        lua_getfield(L, -1, "data");
        if (lua_type(L, -1) != LUA_TSTRING && lua_type(L, -1) != LUA_TUSERDATA)
        {
            lua_pop(L, -1);
            F_NUMBER(val, -1, len, int);
            if (val->len > PACKET_MAX_SIZE)
                val->len = PACKET_MAX_SIZE;
            return;
        }
        F_NUMBER(val, -1, len, int);
    }
    if (lua_type(L, -1) == LUA_TUSERDATA)
    {
        /* userdata, copy */
        val->len = (int)lua_objlen(L, -1);
        if (val->len > PACKET_MAX_SIZE)
            val->len = PACKET_MAX_SIZE;
        cmdOsMemCpy(val->data, lua_touserdata(L, -1), val->len);
        actual_len = val->len;
    }
    if (lua_type(L, -1) == LUA_TSTRING)
    {
        /* hexdump, decode */
        const char *str;
        size_t strLen, p;
        int byte = 0, s, digit;

        val->len = 0;
        str = lua_tolstring(L, -1, &strLen);
        s = 0;
        for (p = 0; p < strLen && val->len < PACKET_MAX_SIZE; p++)
        {
            digit = hexdigit2int(str[p]);
            if (digit == -2 || str[p] == '-') /* space or comment */
            {
                if (s)
                    val->data[val->len++] = (GT_U8) byte;
                s = 0;
                if (str[p] == '-')
                {
                    /* comment, skip to EOL */
                    while (p < strLen && str[p] != '\n')
                        p++;
                }
                continue;
            }
            if (digit == -1) /* wrong character */
                break;
            if (s)
                val->data[val->len++] = (GT_U8) (byte * 16 + digit);
            else
                byte = digit;
            s ^= 1;
        }
        actual_len = val->len;
    }
    if (ltype == LUA_TTABLE)
    {
        lua_pop(L, 1);
        /* table: { LUA_TNUMBER "len", LUA_TSTRING "data" in hex } */
        F_NUMBER(val, -1, len, int);
        if (val->len > PACKET_MAX_SIZE)
            val->len = PACKET_MAX_SIZE;
        /* pad with zeros */
        for (; actual_len < val->len; actual_len++)
            val->data[actual_len] = 0;
    }
}

/*************************************************************************
* prv_c_to_lua_PACKET_STC
*
* Description:
*       Convert PACKET_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_PACKET_STC(
    lua_State *L,
    PACKET_STC *val
)
{
    int t;
    int i;
    char buf[66];
    int p = 0;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, len, int);

    lua_pushliteral(L, "");
    for (i = 0; i < val->len && i < PACKET_MAX_SIZE; i++)
    {
        cmdOsSprintf(buf + p, "%02X", (unsigned char)(val->data[i]));
        p += 2;
        if (p >= 64)
        {
            lua_pushlstring(L, buf, p);
            lua_concat(L, 2);
            p = 0;
        }
    }
    if (p > 0)
    {
        lua_pushlstring(L, buf, p);
        lua_concat(L, 2);
    }
    lua_setfield(L, t, "data");
}

add_mgm_type(PACKET_STC);

/*************************************************************************
* prv_lua_to_c_DEV_PORT_STC
*
* Description:
*       Convert to "c" WRAP_DEV_PORT_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_WRAP_DEV_PORT_STC(
    lua_State         *L,
    WRAP_DEV_PORT_STC *val
)
{
    F_NUMBER(val, -1, hwDevNum,   GT_HW_DEV_NUM);
    F_NUMBER(val, -1, portNum,  GT_PORT_NUM);
}

/*************************************************************************
* prv_c_to_lua_DEV_PORT_STC
*
* Description:
*       Convert WRAP_DEV_PORT_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_WRAP_DEV_PORT_STC(
    lua_State         *L,
    WRAP_DEV_PORT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, hwDevNum,   GT_HW_DEV_NUM);
    FO_NUMBER(val, t, portNum,  GT_PORT_NUM);
}

/*******************************************/
/* GT_ETHERADDR                            */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_GT_ETHERADDR
*
* Description:
*       Convert to GT_ETHERADDR
*
* Parameters:
*       value at the top of stack
*       in lua it can be CLI_types["mac-address"]                   or
*                        "01:02:03:04:05:06"                        or
*                        {[0]=0, [1]=1, [2]=2, [3]=3, [4]=4, [5]=5} or
*                        {arEther =
*                           {[0]=0, [1]=1, [2]=2, [3]=3, [4]=4, [5]=5}}
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_GT_ETHERADDR(
    lua_State *L,
    GT_ETHERADDR *e
)
{
    int k;
    GT_BOOL                 pop_flag = GT_FALSE;
    const GT_CHAR           *s;
    cpssOsMemSet(e, 0, sizeof(*e));

    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "string");

        if (lua_isstring(L, -1))
        {
            pop_flag = GT_TRUE;
        }
        else
        {
            lua_pop(L, 1);
        }
    }

    if (lua_type(L, -1) != LUA_TSTRING)
    {
        if (lua_istable(L, -1))
        {
            if (5 > lua_objlen(L, -1))
            {
                lua_getfield(L, -1, "arEther");
                pop_flag = GT_TRUE;
            }

            for (k = 0; k < 6; k++)
            {
                F_ARRAY_NUMBER(e, arEther, k, GT_U8);
            }
        }
        /*F_ARRAY_END(e, -1, arEther);*/
    }
    else
    {
        int state = 0;
        GT_U32 sum = 0;
        int d;

        s = lua_tostring(L, -1);

        for (k=0; *s && k < 6; s++)
        {
            if (*s == ':')
            {
                e->arEther[k++] = (GT_U8)sum;
                sum = 0;
                state = 0;
                continue;
            }
            d = hexdigit2int(*s);
            if (d < 0)
                break;

            sum = sum * 16 + d;
            state++;
            if (state == 2)
            {
                e->arEther[k++] = (GT_U8)sum;
                sum = 0;
                state = 0;
                if (s[1] == ':')
                    s++;
            }
        }
        if (state && k < 6)
        {
            e->arEther[k] = (GT_U8)sum;
        }
    }

    if (GT_TRUE == pop_flag)
    {
        lua_pop(L, 1);
    }
}


/*************************************************************************
* prv_lua_to_c_mask_GT_ETHERADDR
*
* Description:
*       Convert lua GT_ETHERADDR comparision mask to "C"
*
* Parameters:
*       value at the top of stack
*       in lua it can be params["mac-address"]                      or
*                        params["mac-address"]["string"]            or
*                        {[0]=*, [1]=*, [2]=*, [3]=*, [4]=*, [5]=*} or
*                        {arEther =
*                           {[0]=*, [1]=*, [2]=*, [3]=*, [4]=*, [5]=*}}
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_mask_GT_ETHERADDR(
    lua_State               *L,
    GT_ETHERADDR            *val
)
{
    GT_BOOL                 pop_flag = GT_FALSE;
    GT_BOOL                 isMasked = GT_FALSE;

    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "string");

        if (lua_isstring(L, -1))
        {
            isMasked = GT_TRUE;
            pop_flag = GT_TRUE;
        }
        else
        {
            lua_pop(L, 1);
        }
    }

    if (0 == lua_isstring(L, -1))
    {
        if (lua_istable(L, -1))
        {
            if (5 > lua_objlen(L, -1))
            {
                lua_getfield(L, -1, "arEther");
                pop_flag = GT_TRUE;
            }

            if (5 > lua_objlen(L, -1))
            {
                isMasked = GT_TRUE;
            }
        }
    }
    else
    {
        isMasked = GT_TRUE;
    }

    if (GT_TRUE == pop_flag)
    {
        lua_pop(L, 1);
    }

    if (GT_TRUE == isMasked)
    {
        cpssOsMemSet(val, 0x00, sizeof(*val));
    }
    else
    {
        cpssOsMemSet(val, 0xff, sizeof(*val));
    }
}


/*************************************************************************
* prv_c_to_lua_GT_ETHERADDR
*
* Description:
*       Convert GT_ETHERADDR to lua
*
* Parameters:
*
* Returns:
*       value at the top of stack
*
*************************************************************************/
void prv_c_to_lua_GT_ETHERADDR(
    lua_State *L,
    GT_ETHERADDR *e
)
{
    char s[32];
    cpssOsSprintf(s, "%02x:%02x:%02x:%02x:%02x:%02x",
            e->arEther[0], e->arEther[1], e->arEther[2],
            e->arEther[3], e->arEther[4], e->arEther[5]);
    lua_pushstring(L, s);
}
add_mgm_type(GT_ETHERADDR);


/*************************************************************************
* prv_lua_c_print_GT_ETHERADDR
*
* Description:
*       Prints GT_ETHERADDR fields
*
* Parameters:
*       name                  - structure variable name or path to
*                               structure field
*       nameExtension         - structure field name (could be NULL)
*       val                   - pointer to structure
*
*
*************************************************************************/
void prv_lua_c_print_GT_ETHERADDR
(
    const GT_CHAR_PTR           name,
    const GT_CHAR_PTR           nameExtension,
    const GT_ETHERADDR          *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    if (fullName)
    {
        cpssOsPrintf("%s\n", fullName);
    }
    P_MESSAGE6("%02x:%02x:%02x:%02x:%02x:%02x",
               val->arEther[0], val->arEther[1], val->arEther[2],
               val->arEther[3], val->arEther[4], val->arEther[5]);
}


/*******************************************/
/* CPSS_INTERFACE_INFO_STC                 */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_CPSS_INTERFACE_INFO_STC
*
* Description:
*       Convert to CPSS_INTERFACE_INFO_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_CPSS_INTERFACE_INFO_STC(
    lua_State *L,
    CPSS_INTERFACE_INFO_STC *info
)
{
    F_ENUM(info, -1, type, CPSS_INTERFACE_TYPE_ENT);
    lua_getfield(L, -1, "devPort");
    if (lua_istable(L, -1))
    {
        F_NUMBER_N(&(info->devPort), -1, hwDevNum, devNum, GT_HW_DEV_NUM);
        F_NUMBER(&(info->devPort), -1, portNum, GT_PORT_NUM);
    }
    lua_pop(L, 1);
    F_NUMBER(info, -1, trunkId, GT_TRUNK_ID);
    F_NUMBER(info, -1, vidx, GT_U16);
    F_NUMBER(info, -1, vlanId, GT_U16);
    F_NUMBER_N(info, -1, hwDevNum, devNum, GT_HW_DEV_NUM);
    F_NUMBER(info, -1, fabricVidx, GT_U16);
    F_NUMBER(info, -1, index, GT_U32);

}

/*************************************************************************
* prv_lua_to_c_mask_CPSS_INTERFACE_INFO_STC
*
* Description:
*       Convert lua CPSS_INTERFACE_INFO_STC comparision mask to "C"
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_lua_to_c_mask_CPSS_INTERFACE_INFO_STC(
    lua_State *L,
    CPSS_INTERFACE_INFO_STC *val
)
{
    cpssOsMemSet(val, 0xff, sizeof(*val));
    FM_ENUM(val, -1, type, CPSS_INTERFACE_TYPE_ENT);
    lua_getfield(L, -1, "devPort");
    if (lua_istable(L, -1))
    {
        FM_NUMBER_N(&(val->devPort), -1, hwDevNum, devNum, GT_HW_DEV_NUM);
        FM_NUMBER(&(val->devPort), -1, portNum, GT_PORT_NUM);
    }
    lua_pop(L, 1);
    FM_NUMBER(val, -1, trunkId, GT_TRUNK_ID);
    FM_NUMBER(val, -1, vidx, GT_U16);
    FM_NUMBER(val, -1, vlanId, GT_U16);
    FM_NUMBER_N(val, -1, hwDevNum, devNum, GT_HW_DEV_NUM);
    FM_NUMBER(val, -1, fabricVidx, GT_U16);
    FM_NUMBER(val, -1, index, GT_U32);
}

/*************************************************************************
* prv_c_to_lua_CPSS_INTERFACE_INFO_STC
*
* Description:
*       Convert CPSS_INTERFACE_INFO_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_CPSS_INTERFACE_INFO_STC(
    lua_State *L,
    CPSS_INTERFACE_INFO_STC *info
)
{
    int t, t1;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(info, t, type, CPSS_INTERFACE_TYPE_ENT);
    lua_newtable(L);
    t1 = lua_gettop(L);
        FO_NUMBER_N(&(info->devPort), t1, hwDevNum, devNum,  GT_HW_DEV_NUM);
        FO_NUMBER(&(info->devPort), t1, portNum, GT_PORT_NUM);
    lua_setfield(L, t, "devPort");
    FO_NUMBER(info, t, trunkId, GT_TRUNK_ID);
    FO_NUMBER(info, t, vidx, GT_U16);
    FO_NUMBER(info, t, vlanId, GT_U16);
    FO_NUMBER_N(info, t, hwDevNum, devNum, GT_HW_DEV_NUM);
    FO_NUMBER(info, t, fabricVidx, GT_U16);
    FO_NUMBER(info, t, index, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_INTERFACE_INFO_STC)


/*************************************************************************
* prv_lua_c_print_CPSS_INTERFACE_INFO_STC
*
* Description:
*       Prints CPSS_INTERFACE_INFO_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_INTERFACE_INFO_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_INTERFACE_INFO_STC   *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ENUM(val, fullName, type, CPSS_INTERFACE_TYPE_ENT);

    switch(val->type)
    {
        case CPSS_INTERFACE_PORT_E:
            P_NAME_CONVERSIONS_BEGIN(devPortName, fullName, "devPort");
            {
                P_NUMBER_N(&(val->devPort), devPortName, hwDevNum, devNum,
                                                                GT_HW_DEV_NUM);
                P_NUMBER(&(val->devPort),   devPortName, portNum, GT_PORT_NUM);
            }
            P_NAME_CONVERSIONS_END(devPortName, fullName, "devPort");
            break;

        case CPSS_INTERFACE_TRUNK_E:
            P_NUMBER(val,   fullName, trunkId, GT_TRUNK_ID);
            break;

        case CPSS_INTERFACE_VIDX_E:
            P_NUMBER(val,   fullName, vidx, GT_U16);
            break;

        case CPSS_INTERFACE_VID_E:
            P_NUMBER(val,   fullName, vlanId, GT_U16);
            break;

        case CPSS_INTERFACE_DEVICE_E:
            P_NUMBER_N(val,   fullName, hwDevNum, devNum, GT_HW_DEV_NUM);
            break;

        case CPSS_INTERFACE_FABRIC_VIDX_E:
            P_NUMBER(val,   fullName, fabricVidx, GT_U16);
            break;

        case CPSS_INTERFACE_INDEX_E:
            P_NUMBER(val,   fullName, index, GT_U32);
            break;

        default:
            P_MESSAGE("Wrong interface type");
            break;
    }
}


/*************************************************************************
* prv_lua_to_c_mask_CPSS_PORTS_BMP_STC
*
* Description:
*       Convert lua CPSS_PORTS_BMP_STC comparision mask to "C"
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_lua_to_c_mask_CPSS_PORTS_BMP_STC(
    lua_State                               *L,
    CPSS_PORTS_BMP_STC                      *val
)
{
    GT_U32 portMaskByte = 0;

    cpssOsMemSet(val, 0xff, sizeof(*val));

    FM_ARRAY_START(val, -1, ports);
    {
        int idx;
        for (idx = 0; idx < CPSS_MAX_PORTS_BMP_NUM_CNS; idx++)
        {
            lua_rawgeti(L, -1, idx);
            if (lua_type(L, -1) == LUA_TNUMBER)
            {
                portMaskByte    = (GT_U32) lua_tointeger(L, -1);

                if (0 != portMaskByte)
                {
                    val->ports[idx] = (GT_U32) 0;
                }
            }
            lua_pop(L, 1);
        }
    }
    FM_ARRAY_END(val, -1, ports);
}


/*************************************************************************
* prv_lua_c_print_CPSS_PORTS_BMP_STC
*
* Description:
*       Prints CPSS_PORTS_BMP_STC fields
*
* Parameters:
*       name                  - structure variable name or path to
*                               structure field
*       nameExtension         - structure field name (could be NULL)
*       val                   - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_PORTS_BMP_STC
(
    const GT_CHAR_PTR           name,
    const GT_CHAR_PTR           nameExtension,
    const CPSS_PORTS_BMP_STC    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ARRAY_START(val, fullName, ports);
    {
        int idx;
        for (idx = 0; idx < 8; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, ports, idx, GT_U32);
        }
    }
    P_ARRAY_END(val, fullName, ports);
}



/*******************************************/
/* GT_IPV4                                   */
/*******************************************/
static void prv_lua_to_c_string_to_IPV4
(
    lua_State   *L,
    GT_U8_PTR   ipPtr
)
{
    /* parse dotted IP string */
    int k;
    int n;
    const char *s;

    if (lua_type(L, -1) != LUA_TSTRING)
        return; /* bad param */

    s = lua_tostring(L, -1);

    k = 0;
    for (; *s && k < 4; s++)
    {
        if (*s == '.')
        {
            k++;
        }
        else if ((*s) >= '0' && (*s) <= '9')
        {
            n = ipPtr[k] * 10 + (*s) - '0';
            if (n > 255)
                break;
            ipPtr[k] = (GT_U8) n;
        }
        else
        {
            break;
        }
    }
}

/*************************************************************************
* prv_lua_to_c_GT_IPV4
*
* Description:
*       Convert to IP v4 array
*
* Parameters:
*       value at the top of stack:
*           { string="1.2.3.4" }
*           "1.2.3.4"
*           { 1, 2, 3, 4 }
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_GT_IPV4
(
    lua_State   *L,
    GT_U8_PTR   ipPtr
)
{
    cpssOsMemSet(ipPtr, 0, sizeof(ipPtr));
    if (lua_istable(L, -1))
    {
        int i;
        lua_getfield(L, -1, "string");

        if (lua_isstring(L, -1))
        {
            prv_lua_to_c_string_to_IPV4(L, ipPtr);
            lua_pop(L, 1);
            return;
        }
        lua_pop(L, 1);
        for (i = 0; i < 4; i++)
        {
            lua_rawgeti(L, -1, i+1);
            if (lua_isnumber(L, -1))
                ipPtr[i] = (GT_U8)lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
        return;
    }

    if (lua_type(L, -1) == LUA_TSTRING)
    {
        prv_lua_to_c_string_to_IPV4(L, ipPtr);
        return;
    }
}


/*************************************************************************
* prv_lua_to_c_mask_GT_IPV4
*
* Description:
*       Convert to IP v4 array
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_mask_GT_IPV4
(
    lua_State   *L,
    GT_U8_PTR   ipPtr
)
{
    GT_BOOL     isMasked = GT_FALSE;

    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "string");

        if (lua_isstring(L, -1))
        {
            isMasked = GT_TRUE;
        }

        lua_pop(L, 1);
    }

    if (lua_type(L, -1) == LUA_TSTRING)
    {
        isMasked = GT_TRUE;
    }


    if (GT_TRUE == isMasked)
    {
        cpssOsMemSet(ipPtr, 0x00, sizeof(ipPtr));
    }
    else
    {
        cpssOsMemSet(ipPtr, 0xff, sizeof(ipPtr));
    }
}


/*************************************************************************
* prv_c_to_lua_GT_IPV4
*
* Description:
*       Convert IP v4 array to lua
*
* Parameters:
*
* Returns:
*       value at the top of stack
*
*************************************************************************/
void prv_c_to_lua_GT_IPV4
(
    lua_State   *L,
    GT_U8_PTR   ipPtr
)
{
    char s[16];
    cpssOsSprintf(s, "%d.%d.%d.%d",
            ipPtr[0], ipPtr[1],
            ipPtr[2], ipPtr[3]);
    lua_pushstring(L, s);
}


/*************************************************************************
* prv_lua_c_print_GT_IPV4
*
* Description:
*       Prints IP v4 array
*
* Parameters:
*
* Returns:
*       name                  - array variable name or path to array field
*       nameExtension         - array field name
*       val                   - pointer to array
*
*************************************************************************/
void prv_lua_c_print_GT_IPV4
(
    const GT_CHAR_PTR           name,
    const GT_CHAR_PTR           nameExtension,
    const GT_U8                 *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    if (fullName)
    {
        cpssOsPrintf("%s\n", fullName);
    }
    P_MESSAGE4("%d.%d.%d.%d", val[0], val[1], val[2], val[3]);
}


/*******************************************/
/* GT_IPV6                                   */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_GT_IPV6
*
* Description:
*       Convert to IP v6 array
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_GT_IPV6
(
    lua_State   *L,
    GT_U8_PTR   ipPtr
)
{
    GT_IPV6ADDR ip;
    GT_U16      octetIndex;

    prv_lua_to_c_GT_IPV6ADDR(L, &ip);

    cpssOsMemSet(ipPtr, 0, sizeof(ipPtr));

    for (octetIndex = GT_IPV6_BEGIN_OCTET;
         octetIndex <= GT_IPV6_END_OCTET;
         octetIndex++)
    {
        ipPtr[GT_IPV6_END_OCTET - (octetIndex - GT_IPV6_BEGIN_OCTET)] =
                                                            ip.arIP[octetIndex];
    }
}


/*************************************************************************
* prv_lua_to_c_mask_GT_IPV6
*
* Description:
*       Convert to IP v6 array
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_mask_GT_IPV6
(
    lua_State   *L,
    GT_U8_PTR   ipPtr
)
{
    GT_BOOL     isMasked = GT_FALSE;

    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "string");

        if (lua_isstring(L, -1))
        {
            isMasked = GT_TRUE;
        }

        lua_pop(L, 1);
    }

    if (lua_type(L, -1) == LUA_TSTRING)
    {
        isMasked = GT_TRUE;
    }


    if (GT_TRUE == isMasked)
    {
        cpssOsMemSet(ipPtr, 0x00, sizeof(ipPtr));
    }
    else
    {
        cpssOsMemSet(ipPtr, 0xff, sizeof(ipPtr));
    }
}


/*************************************************************************
* prv_c_to_lua_GT_IPV6
*
* Description:
*       Convert IP v6 array to lua
*
* Parameters:
*
* Returns:
*       value at the top of stack
*
*************************************************************************/
void prv_c_to_lua_GT_IPV6
(
    lua_State   *L,
    GT_U8_PTR   ipPtr
)
{
    GT_IPV6ADDR ip;
    GT_U16      octetIndex;

    cpssOsMemSet(&ip, 0, sizeof(ip));

    for (octetIndex = GT_IPV6_BEGIN_OCTET;
         octetIndex <= GT_IPV6_END_OCTET;
         octetIndex++)
    {
        ip.arIP[octetIndex] =
            ipPtr[GT_IPV6_END_OCTET - (octetIndex - GT_IPV6_BEGIN_OCTET)];
    }

    prv_c_to_lua_GT_IPV6ADDR(L, &ip);
}


/*************************************************************************
* prv_lua_c_print_GT_IPV6
*
* Description:
*       Prints IP v6 array
*
* Parameters:
*
* Returns:
*       name                  - array variable name or path to array field
*       nameExtension         - array field name
*       val                   - pointer to array
*
*************************************************************************/
void prv_lua_c_print_GT_IPV6
(
    const GT_CHAR_PTR           name,
    const GT_CHAR_PTR           nameExtension,
    const GT_U8                 *val
)
{
    GT_IPV6ADDR ip;
    GT_U16      octetIndex;

    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    cpssOsMemSet(&ip, 0, sizeof(ip));

    for (octetIndex = GT_IPV6_BEGIN_OCTET;
         octetIndex <= GT_IPV6_END_OCTET;
         octetIndex++)
    {
        ip.arIP[octetIndex] =
            val[GT_IPV6_END_OCTET - (octetIndex - GT_IPV6_BEGIN_OCTET)];
    }

    prv_lua_c_print_GT_IPV6ADDR(fullName, NULL, &ip);
}

/*******************************************/
/* GT_IPADDR                               */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_GT_IPADDR
*
* Description:
*       Convert to GT_IPADDR
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_GT_IPADDR(
    lua_State *L,
    GT_IPADDR *ip
)
{
    cpssOsMemSet(ip, 0, sizeof(*ip));
    if (lua_type(L, -1) == LUA_TNUMBER)
    {
        ip->u32Ip = (GT_U32)lua_tointeger(L, -1);
    }
    else
    {
        prv_lua_to_c_GT_IPV4(L, ip->arIP);
    }
}


/*************************************************************************
* prv_c_to_lua_GT_IPADDR
*
* Description:
*       Convert GT_IPADDR to lua
*
* Parameters:
*
* Returns:
*       value at the top of stack
*
*************************************************************************/
void prv_c_to_lua_GT_IPADDR(
    lua_State *L,
    GT_IPADDR *ip
)
{
    prv_c_to_lua_GT_IPV4(L, ip->arIP);
}
add_mgm_type(GT_IPADDR);

/*************************************************************************
* prv_lua_c_print_GT_IPADDR
*
* Description:
*       Prints GT_IPADDR fields
*
* Parameters:
*       name                  - structure variable name or path to
*                               structure field
*       nameExtension         - structure field name (could be NULL)
*       val                   - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_GT_IPADDR
(
    const GT_CHAR_PTR           name,
    const GT_CHAR_PTR           nameExtension,
    const GT_IPADDR             *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    prv_lua_c_print_GT_IPV4(fullName, "arIP", val->arIP);
}

/*******************************************/
/* GT_IPV6ADDR                             */
/*******************************************/

/*************************************************************************
* prv_string_to_IPV6_arr
*
* DESCRIPTION:
*       Convert to IPV6 string to GT_U16[8]
*
* INPUTS:
*       s   - string in form "2001:0db8:11a3:09d7:1f34:8a2e:07a0:765d"
*             or "2001:db8::ae21:ad12" or "::ae21:ad12"
*
* OUTPUTS:
*       addr - GT_U16[8] array
*
* RETURNS:
*       None
*
*
*************************************************************************/
void prv_string_to_IPV6_arr(
  IN  const char    *s,
  OUT GT_U16        *addr
)
{
    int k;
    int state = 0;
    GT_U32 sum = 0;
    int i;
    int ddot = -1;

    for (k = 0; k < 8; k++)
        addr[k] = 0;
    for (k = 0; *s && k < 8; s++)
    {
        if (*s == ':')
        {
            if (state)
            {
                if (ddot == -1)
                {
                    addr[k++] = (GT_U16)sum;
                }
                else
                {
                    for (i = ddot; i < 7; i++)
                        addr[i] = addr[i+1];
                    addr[7] = (GT_U16)sum;
                }
                sum = 0;
                state = 0;
                if (s[1] != ':')
                    continue;
            }
            if (s[1] == ':')
            {
                if (ddot != -1)
                    break; /* "::" twice */
                ddot = k;
                s++;
                continue;
            }
            else
            {
                break; /* ":" without digits */
            }
        }
        i = hexdigit2int(*s);
        if (i < 0)
            break;
        sum = sum * 16 + (unsigned)i;
        if (sum > 0xffff)
            break;
        state = 1;
    }
    if (state && k < 8)
    {
        if (ddot == -1)
        {
            addr[k++] = (GT_U16)sum;
        }
        else
        {
            for (i = ddot; i < 7; i++)
                addr[i] = addr[i+1];
            addr[7] = (GT_U16)sum;
        }
    }
}

/*************************************************************************
* prv_IPV6_arr_to_string
*
* DESCRIPTION:
*       Convert IPV6 GT_U16[8] to string
*
* INPUTS:
*       addr - GT_U16[8] array
*       useShortForm    - use short form (remove zeroes
*
* OUTPUTS:
*       s   - string in form "2001:0db8:11a3:09d7:1f34:8a2e:07a0:765d"
*             or "2001:db8::ae21:ad12" or "::ae21:ad12"
*
* RETURNS:
*       None
*
*
*************************************************************************/
void prv_IPV6_arr_to_string(
  IN  GT_U16        *addr,
  IN  GT_BOOL       useShortForm,
  OUT char          *s
)
{
    int k;
    int zind = -1;
    int zlen = 0;

    if (useShortForm == GT_TRUE)
    {
        int l;
        for (k = 0; k < 8;k++)
        {
            for (l = 0; k+l < 8; l++)
                if (addr[k+l] != 0)
                    break;
            if (l > zlen)
            {
                zind = k;
                zlen = l;
                k += l-1;
            }
        }
    }
    s[0] = 0;
    for (k = 0; k < 8; k++)
    {
        if (k == zind)
        {
            strcat(s, "::");
            k += zlen - 1;
            continue;
        }
        cpssOsSprintf(strchr(s, 0), "%x", addr[k]);
        if ((k < 7) && (k+1 != zind))
        {
            strcat(s, ":");
        }
    }
}


/*************************************************************************
* prv_lua_to_c_GT_IPV6ADDR
*
* Description:
*       Convert to GT_IPV6ADDR
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_GT_IPV6ADDR
(
    lua_State *L,
    GT_IPV6ADDR *ip
)
{
    GT_U16      ipArr[8];
    int         i;
    GT_BOOL     pop_flag            = GT_FALSE;

    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "string");

        if (lua_isstring(L, -1))
        {
            pop_flag = GT_TRUE;
        }
        else
        {
            lua_pop(L, 1);
        }
    }

    cpssOsMemSet(ip, 0, sizeof(*ip));
    if (lua_type(L, -1) != LUA_TSTRING)
    {
        return;
    }
    prv_string_to_IPV6_arr(lua_tostring(L, -1), ipArr);

    for (i = 0; i < 8; i++)
    {
        ip->arIP[i+i] = (GT_U8)((ipArr[i] >> 8) & 0x00ff);
        ip->arIP[i+i+1] = (GT_U8)(ipArr[i] & 0x00ff);
    }

    if (GT_TRUE == pop_flag)
    {
        lua_pop(L, 1);
    }
}


/*************************************************************************
* prv_c_to_lua_GT_IPV6ADDR
*
* Description:
*       Convert GT_IPV6ADDR to lua
*
* Parameters:
*
* Returns:
*       value at the top of stack
*
*************************************************************************/
void prv_c_to_lua_GT_IPV6ADDR
(
    lua_State *L,
    GT_IPV6ADDR *ip
)
{
    GT_CHAR         s[40];
    GT_U16          ipArr[8];
    int             i;
    for (i = 0; i < 8; i++)
    {
        ipArr[i] = ip->arIP[i+i] << 8 | ip->arIP[i+i+1];
    }

    prv_IPV6_arr_to_string(ipArr, GT_TRUE, s);

    lua_pushstring(L, s);
}
add_mgm_type(GT_IPV6ADDR)


/*************************************************************************
* prv_lua_c_print_GT_IPV6ADDR
*
* Description:
*       Prints GT_IPV6ADDR fields
*
* Parameters:
*       name                          - structure variable name or path
*                                       to structure field
*       nameExtension                 - structure field name (could be
*                                       NULL)
*       val                           - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_GT_IPV6ADDR
(
    const GT_CHAR_PTR                   name,
    const GT_CHAR_PTR                   nameExtension,
    const GT_IPV6ADDR                   *val
)
{
    GT_CHAR                         s[40];
    GT_U16          ipArr[8];
    int             i;
    for (i = 0; i < 8; i++)
    {
        ipArr[i] = val->arIP[i+i] << 8 | val->arIP[i+i+1];
    }

    prv_IPV6_arr_to_string(ipArr, GT_TRUE, s);

    P_MESSAGE1("%s", s);
}


use_prv_enum(CPSS_PORT_FEC_MODE_ENT);
use_prv_enum(CPSS_PORT_AP_FLOW_CONTROL_ENT);
use_prv_enum(CPSS_PORT_MANAGER_PORT_TYPE_ENT);
use_prv_enum(CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT);
use_prv_enum(CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT);

void prv_lua_to_c_CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *val
);

void prv_c_to_lua_CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *val
);


void prv_lua_to_c_CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC *val
);

void prv_c_to_lua_CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC *val
);

void prv_lua_to_c_CPSS_PORT_MANAGER_CALIBRATION_MODE_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CALIBRATION_MODE_STC *val
);

void prv_c_to_lua_CPSS_PORT_MANAGER_CALIBRATION_MODE_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CALIBRATION_MODE_STC *val
);

use_prv_enum(CPSS_PORT_INTERFACE_MODE_ENT);
use_prv_enum(CPSS_PORT_SPEED_ENT);
use_prv_enum(CPSS_PORT_FEC_MODE_ENT);
use_prv_enum(CPSS_PORT_AP_FLOW_CONTROL_ENT);
use_prv_enum(CPSS_PORT_MANAGER_PORT_TYPE_ENT);
use_prv_enum(CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT);
use_prv_enum(CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT);

void prv_lua_to_c_CPSS_PM_PORT_PARAMS_STC(
    lua_State *L,
    CPSS_PM_PORT_PARAMS_STC *val
);
void prv_c_to_lua_CPSS_PM_PORT_PARAMS_STC(
    lua_State *L,
    CPSS_PM_PORT_PARAMS_STC *val
);

void prv_lua_to_c_CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *val
);
void prv_c_to_lua_CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *val
);

void prv_c_to_lua_CPSS_PORT_MANAGER_CALIBRATION_MODE_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CALIBRATION_MODE_STC *val
);
void prv_lua_to_c_CPSS_PORT_MANAGER_CALIBRATION_MODE_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CALIBRATION_MODE_STC *val
);

void prv_c_to_lua_CPSS_PORT_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_SERDES_RX_CONFIG_STC *val
);

void prv_lua_to_c_CPSS_PORT_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_SERDES_RX_CONFIG_STC *val
);

void prv_c_to_lua_CPSS_PORT_SERDES_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_SERDES_TX_CONFIG_STC *val
);

void prv_lua_to_c_CPSS_PORT_SERDES_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_SERDES_TX_CONFIG_STC *val
);

void prv_c_to_lua_CPSS_PM_PORT_REG_PARAMS_STC(
    lua_State *L,
    CPSS_PM_PORT_REG_PARAMS_STC *val
);

void prv_lua_to_CPSS_PM_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_PORT_ATTR_STC *val
);

void prv_c_to_lua_CPSS_PM_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_PORT_ATTR_STC *val
);

void prv_lua_to_c_CPSS_PM_ET_OVERRIDE_CFG_STC(
    lua_State *L,
    CPSS_PM_ET_OVERRIDE_CFG_STC  *val
);

void prv_c_to_lua_CPSS_PM_ET_OVERRIDE_CFG_STC(
    lua_State *L,
    CPSS_PM_ET_OVERRIDE_CFG_STC  *val
);

void prv_c_to_lua_CPSS_PM_AP_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_ATTR_STC *val
);

void prv_lua_to_c_CPSS_PM_AP_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_ATTR_STC *val
);

void prv_lua_to_c_CPSS_PM_AP_PORT_PARAMS_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_PARAMS_STC *val
);

void prv_c_to_lua_CPSS_PM_AP_PORT_PARAMS_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_PARAMS_STC *val
);

void prv_lua_to_c_CPSS_PM_ET_OVERRIDE_CFG_STC(
    lua_State *L,
    CPSS_PM_ET_OVERRIDE_CFG_STC  *val
)
{
    F_NUMBER(val, -1, minLF, GT_U8);
    F_NUMBER(val, -1, maxLF, GT_U8);
}
void prv_c_to_lua_CPSS_PM_ET_OVERRIDE_CFG_STC(
    lua_State *L,
    CPSS_PM_ET_OVERRIDE_CFG_STC  *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, minLF, GT_U8);
    FO_NUMBER(val, t, maxLF, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_ET_OVERRIDE_CFG_STC);


void prv_lua_to_c_CPSS_PM_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_PORT_ATTR_STC *val
)
{
/*
  GT_U32                                        validAttrsBitMask;
  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT           trainMode;
  GT_BOOL                                       adaptRxTrainSupp;
  GT_BOOL                                       edgeDetectSupported;
  CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC         loopback;
  CPSS_PM_ET_OVERRIDE_CFG_STC                   etOverride;
  CPSS_PORT_FEC_MODE_ENT                        fecMode;
  CPSS_PORT_MANAGER_CALIBRATION_MODE_STC        calibrationMode;
  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT      unMaskEventsMode;
*/
    F_NUMBER(val, -1, validAttrsBitMask, GT_U32);
    F_ENUM(val,   -1, trainMode, CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT);
    F_NUMBER(val, -1, adaptRxTrainSupp, GT_BOOL);
    F_NUMBER(val, -1, edgeDetectSupported, GT_BOOL);
    F_STRUCT(val, -1, loopback,          CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC);
    F_STRUCT(val, -1, etOverride,        CPSS_PM_ET_OVERRIDE_CFG_STC);
    F_ENUM(val,   -1, fecMode,           CPSS_PORT_FEC_MODE_ENT);
    F_STRUCT(val, -1, calibrationMode,   CPSS_PORT_MANAGER_CALIBRATION_MODE_STC);
    F_ENUM(val,   -1, unMaskEventsMode,  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT);
}
void prv_c_to_lua_CPSS_PM_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_PORT_ATTR_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, validAttrsBitMask, GT_U32);
    FO_ENUM(val, t, trainMode,         CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT);
    FO_NUMBER(val, t, adaptRxTrainSupp,  GT_BOOL);
    FO_NUMBER(val, t, edgeDetectSupported, GT_BOOL);
    FO_STRUCT(val, t, loopback,          CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC);
    FO_STRUCT(val, t, etOverride,        CPSS_PM_ET_OVERRIDE_CFG_STC);
    FO_ENUM(val, t, fecMode,           CPSS_PORT_FEC_MODE_ENT);
    FO_STRUCT(val, t, calibrationMode,   CPSS_PORT_MANAGER_CALIBRATION_MODE_STC);
    FO_ENUM(val, t, unMaskEventsMode,  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_PORT_ATTR_STC);


void prv_lua_to_c_CPSS_PM_MODE_LANE_PARAM_STC(
    lua_State *L,
    CPSS_PM_MODE_LANE_PARAM_STC *val
)
{
    F_NUMBER(val, -1, validLaneParamsBitMask, GT_U32);
    F_NUMBER(val, -1, globalLaneNum, GT_U16);
    F_STRUCT(val, -1, txParams, CPSS_PORT_SERDES_TX_CONFIG_STC);
    F_STRUCT(val, -1, rxParams, CPSS_PORT_SERDES_RX_CONFIG_STC);
}
void prv_c_to_lua_CPSS_PM_MODE_LANE_PARAM_STC(
    lua_State *L,
    CPSS_PM_MODE_LANE_PARAM_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, validLaneParamsBitMask, GT_U32);
    FO_NUMBER(val, t, globalLaneNum, GT_U16);
    FO_STRUCT(val, t, txParams, CPSS_PORT_SERDES_TX_CONFIG_STC);
    FO_STRUCT(val, t, rxParams, CPSS_PORT_SERDES_RX_CONFIG_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_MODE_LANE_PARAM_STC);


void prv_lua_to_c_CPSS_PM_PORT_REG_PARAMS_STC(
    lua_State *L,
    CPSS_PM_PORT_REG_PARAMS_STC *val
)
{
  F_ENUM(val, -1, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
  F_ENUM(val, -1, speed, CPSS_PORT_SPEED_ENT);
  F_ARRAY_START(val, -1, laneParams);
  {
        int idx;
        for (idx = 0; idx < 7; idx++) {
            F_ARRAY_STRUCT(val, laneParams, idx, CPSS_PM_MODE_LANE_PARAM_STC);
        }
  }
  F_ARRAY_END(val, -1, laneParams);
  F_STRUCT(val, -1, portAttributes, CPSS_PM_PORT_ATTR_STC);
}
void prv_c_to_lua_CPSS_PM_PORT_REG_PARAMS_STC(
    lua_State *L,
    CPSS_PM_PORT_REG_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    FO_ENUM(val, t, speed, CPSS_PORT_SPEED_ENT);
    FO_ARRAY_START(val, t, laneParams);
    {
       int idx;
       for (idx = 0; idx < 7; idx++) {
          FO_ARRAY_STRUCT(val, laneParams, idx, CPSS_PM_MODE_LANE_PARAM_STC);
       }
    }
    FO_ARRAY_END(val, t, laneParams);
    FO_STRUCT(val, t, portAttributes, CPSS_PM_PORT_ATTR_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_PORT_REG_PARAMS_STC);


/*
void prv_c_to_lua_<REPLACE>(
    lua_State *L,
    <REPLACE> *val
)
{
}
void prv_lua_to_c_<REPLACE>(
    lua_State *L,
    <REPLACE> *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);

    lua_settop(L, t);
}
add_mgm_type(<REPLACE>);
*/


void prv_lua_to_c_CPSS_PM_AP_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_ATTR_STC *val
)
{
/*
  GT_U32                                  validAttrsBitMask;

  GT_BOOL                                 nonceDisable;
  GT_BOOL                                 fcPause;
  CPSS_PORT_AP_FLOW_CONTROL_ENT           fcAsmDir;
  GT_U32                                  negotiationLaneNum;
  CPSS_PM_AP_LANE_OVERRIDE_PARAM_STC      overrideLaneParams [CPSS_PM_MAX_LANE_OVERRIDE_SPEEDS][CPSS_PM_MAX_LANES];
*/

    F_NUMBER(val, -1, validAttrsBitMask, GT_U32);
    F_BOOL(val, -1, nonceDisable);
    F_BOOL(val, -1, fcPause);
    F_ENUM(val, -1, fcAsmDir, CPSS_PORT_AP_FLOW_CONTROL_ENT);
    F_NUMBER(val, -1, negotiationLaneNum, GT_U32);
}
void prv_c_to_lua_CPSS_PM_AP_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_ATTR_STC *val
)
{
  int t;
  lua_newtable(L);
  t = lua_gettop(L);
  FO_NUMBER(val, t, validAttrsBitMask, GT_U32);
  FO_BOOL(val, t, nonceDisable);
  FO_BOOL(val, t, fcPause);
  FO_ENUM(val, t, fcAsmDir, CPSS_PORT_AP_FLOW_CONTROL_ENT);
  FO_NUMBER(val, t, negotiationLaneNum, GT_U32);
  lua_settop(L, t);
}
add_mgm_type(CPSS_PM_AP_PORT_ATTR_STC);

void prv_lua_to_c_CPSS_PM_PORT_PARAMS_STC(
    lua_State *L,
    CPSS_PM_PORT_PARAMS_STC *val
)
{
    F_ENUM(val, -1, portType, CPSS_PORT_MANAGER_PORT_TYPE_ENT);
    F_NUMBER(val, -1, magic, GT_U32);
    lua_getfield(L, -1, "portParamsType");
    if (lua_istable(L, -1))
    {
        if (val->portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
        {
            F_STRUCT(&(val->portParamsType), -1, regPort, CPSS_PM_PORT_REG_PARAMS_STC);
        }
        else
        {
            F_STRUCT(&(val->portParamsType), -1, apPort, CPSS_PM_AP_PORT_PARAMS_STC);
        }
    }
    lua_pop(L, 1);
}
void prv_c_to_lua_CPSS_PM_PORT_PARAMS_STC(
    lua_State *L,
    CPSS_PM_PORT_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, portType, CPSS_PORT_MANAGER_PORT_TYPE_ENT);
    FO_NUMBER(val, t, magic, GT_U32);
    {
        int t1;
        lua_newtable(L);
        t1 = lua_gettop(L);
            FO_STRUCT(&(val->portParamsType), t1, regPort, CPSS_PM_PORT_REG_PARAMS_STC);
            FO_STRUCT(&(val->portParamsType), t1, apPort, CPSS_PM_AP_PORT_PARAMS_STC);
        lua_setfield(L, t, "portParamsType");
    }
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_PORT_PARAMS_STC);


#endif
