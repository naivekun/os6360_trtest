/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplIncrement.c
*
* DESCRIPTION:
*       A lua wrapper to increment ipv4, ipv6 & mac addresses
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/
#include <lua.h>
#include <cpss/common/cpssTypes.h>
#include <cmdShell/common/cmdCommon.h>
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>

/* externs */

use_prv_struct(GT_ETHERADDR);
use_prv_struct(GT_IPADDR);
use_prv_struct(GT_IPV6ADDR);

/*******************************************************************************
* wraplMacIncrement: lua function
*
* DESCRIPTION:
*       increment Mac address
*
* INPUTS:
*       Mac address            - string
*       increment address       - string
*
* RETURNS:
*       string
*
* COMMENTS:
*       None
*
*******************************************************************************/
int wraplMacIncrement
(
    lua_State *L
)
{
    GT_ETHERADDR macAddr, incMacAddr;
    GT_32 i;
    GT_U32 sum, c = 0;

    if (lua_gettop(L) < 2) { /* not enough parameters */
        return 0; /* nil value */
    }
    lua_settop(L,2); /* keep only two values in stack */
    prv_lua_to_c_GT_ETHERADDR(L,&incMacAddr);
    lua_pop(L, 1);
    prv_lua_to_c_GT_ETHERADDR(L,&macAddr);

    for( i = 5 ; i >= 0 ; i-- ) {
        sum = (GT_U32)macAddr.arEther[i]+(GT_U32)incMacAddr.arEther[i]+c;
        c = (sum > 0xFF) ? 1 : 0;
        macAddr.arEther[i] = (GT_U8)sum;
    }

    prv_c_to_lua_GT_ETHERADDR(L, &macAddr);
    return 1;
}

/*******************************************************************************
* wraplIpv4Increment: lua function
*
* DESCRIPTION:
*       increment ipv4 address
*
* INPUTS:
*       ipv4 address            - string
*       increment address       - string
*
* RETURNS:
*       string
*
* COMMENTS:
*       None
*
*******************************************************************************/
int wraplIpv4Increment
(
    lua_State *L
)
{
    GT_IPADDR ipAddr, incIpAddr;
    GT_32 i;
    GT_U32 sum, c = 0;

    if (lua_gettop(L) < 2) { /* not enough parameters */
        return 0; /* nil value */
    }
    lua_settop(L,2); /* keep only two values in stack */
    prv_lua_to_c_GT_IPADDR(L,&incIpAddr);
    lua_pop(L, 1);
    prv_lua_to_c_GT_IPADDR(L,&ipAddr);

    for( i = 3 ; i >= 0 ; i-- ) {
        sum = (GT_U32)ipAddr.arIP[i]+(GT_U32)incIpAddr.arIP[i]+c;
        c = (sum > 0xFF) ? 1 : 0;
        ipAddr.arIP[i] = (GT_U8)sum;
    }

    prv_c_to_lua_GT_IPADDR(L,&ipAddr);
    return 1;
}

/*******************************************************************************
* wraplIpv6Increment: lua function
*
* DESCRIPTION:
*       increment ipv6 address
*
* INPUTS:
*       ipv6 address            - string
*       increment address       - string
*
* RETURNS:
*       string
*
* COMMENTS:
*       None
*
*******************************************************************************/
int wraplIpv6Increment
(
    lua_State *L
)
{
    GT_IPV6ADDR ipAddr, incIpAddr;
    GT_32 i;
    GT_U32 sum, c = 0;

    if (lua_gettop(L) < 2) { /* not enough parameters */
        return 0; /* nil value */
    }
    lua_settop(L,2); /* keep only two values in stack */
    prv_lua_to_c_GT_IPV6ADDR(L,&incIpAddr);
    lua_pop(L, 1);
    prv_lua_to_c_GT_IPV6ADDR(L,&ipAddr);

    for( i = 15 ; i >= 0 ; i-- ) {
        sum = (GT_U32)ipAddr.arIP[i]+(GT_U32)incIpAddr.arIP[i]+c;
        c = (sum > 0xFF) ? 1 : 0;
        ipAddr.arIP[i] = (GT_U8)sum;
    }

    prv_c_to_lua_GT_IPV6ADDR(L,&ipAddr);
    return 1;
}
