/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file wraplCpssPxBpe.c
*
* @brief A lua wrapper for BPE functions
*
* @version   1
********************************************************************************
*/


/* Feature specific includes. */
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpssPxHalBpe.h>

use_prv_struct(CPSS_PX_HAL_BPE_INTERFACE_INFO_STC);
use_prv_struct(CPSS_PX_HAL_BPE_QOSMAP_STC);
use_prv_enum(CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT);
use_prv_enum(CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT);


/**
* @internal wrlCpssPxHalBpeMulticastEChannelCreate function
* @endinternal
*
* @brief   Create Multicast E-Channel and add interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpeMulticastEChannelCreate
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum;
    GT_U32 cid, i;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  interfaceListArray[16];
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);
    PARAM_NUMBER(status, cid,            2, GT_U32);

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 3) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    for (i=0; i<16; i++)
    {
        lua_rawgeti(L, 3, i+1);  /* interfaceListArray[i] */
        if (!(lua_type(L, -1) == LUA_TTABLE))                 
        {
            lua_pop(L,1);
            lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
            return 1;
        }
        prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(interfaceListArray[i])); /* convert Lua structure to C */
        lua_pop(L,1);
        if (interfaceListArray[i].type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E)
        {
            break;
        }
    }

    if (i == 0 || i == 16)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    status = cpssPxHalBpeMulticastEChannelCreate(devNum,cid,interfaceListArray);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}


/**
* @internal wrlCpssPxHalBpeMulticastEChannelUpdate function
* @endinternal
*
* @brief   Update Multicast E-Channel and add interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpeMulticastEChannelUpdate
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum;
    GT_U32 cid, i;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  interfaceListArray[16];
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);
    PARAM_NUMBER(status, cid,            2, GT_U32);

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 3) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    for (i=0; i<16; i++)
    {
        lua_rawgeti(L, 3, i+1);  /* interfaceListArray[i] */
        if (!(lua_type(L, -1) == LUA_TTABLE))                 
        {
            lua_pop(L,1);
            lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
            return 1;
        }
        prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(interfaceListArray[i])); /* convert Lua structure to C */
        lua_pop(L,1);
        if (interfaceListArray[i].type == CPSS_PX_HAL_BPE_INTERFACE_NONE_E)
        {
            break;
        }
    }

    if (i == 0 || i == 16)
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    status = cpssPxHalBpeMulticastEChannelUpdate(devNum,cid,interfaceListArray);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}

/**
* @internal wrlCpssPxHalBpeTrunkPortsAdd function
* @endinternal
*
* @brief   Add ports to a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpeTrunkPortsAdd
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum;
    GT_U32 i;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  portListArray[16];
    GT_TRUNK_ID trunkId;    
    GT_U8 portListLen;
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);
    PARAM_NUMBER(status, trunkId,        2, GT_TRUNK_ID);
    PARAM_NUMBER(status, portListLen,    3, GT_U8);

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 4) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    for (i=0; i<portListLen; i++)
    {
        lua_rawgeti(L, 4, i+1);  /* portListArray[i] */
        if (!(lua_type(L, -1) == LUA_TTABLE))                 
        {
            lua_pop(L,1);
            lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
            return 1;
        }
        prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(portListArray[i])); /* convert Lua structure to C */
        lua_pop(L,1);        
    }    

    status = cpssPxHalBpeTrunkPortsAdd(devNum,trunkId,portListArray,portListLen);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}


/**
* @internal wrlCpssPxHalBpeTrunkPortsDelete function
* @endinternal
*
* @brief   Delete ports from a TRUNK.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpeTrunkPortsDelete
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum;
    GT_U32 i;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  portListArray[16];
    GT_TRUNK_ID trunkId;    
    GT_U8 portListLen;
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);
    PARAM_NUMBER(status, trunkId,        2, GT_TRUNK_ID);
    PARAM_NUMBER(status, portListLen,    3, GT_U8);

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 4) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    for (i=0; i<portListLen; i++)
    {
        lua_rawgeti(L, 4, i+1);  /* portListArray[i] */
        if (!(lua_type(L, -1) == LUA_TTABLE))                 
        {
            lua_pop(L,1);
            lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
            return 1;
        }
        prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(portListArray[i])); /* convert Lua structure to C */
        lua_pop(L,1);        
    }

    status = cpssPxHalBpeTrunkPortsDelete(devNum,trunkId,portListArray,portListLen);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}

/**
* @internal wrlCpssPxHalBpeUnicastEChannelCreate function
* @endinternal
*
* @brief   Create Unicat E-Channel and add downstream interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpeUnicastEChannelCreate
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum;
    GT_U32 cid;
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  interface;
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);
    PARAM_NUMBER(status, cid,            2, GT_U32);

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 3) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
        
    lua_rawgeti(L, 3, 1);  
    if (!(lua_type(L, -1) == LUA_TTABLE))                 
    {
        lua_pop(L,1);
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
    prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(interface)); /* convert Lua structure to C */    

    status = cpssPxHalBpeUnicastEChannelCreate(devNum,cid,&interface);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}


/**
* @internal wrlCpssPxHalBpeInterfaceTypeSet function
* @endinternal
*
* @brief   Set BPE interface mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpeInterfaceTypeSet
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum; 
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  interface;
    CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT  mode;
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 2) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
        
    lua_rawgeti(L, 2, 1);  
    if (!(lua_type(L, -1) == LUA_TTABLE))                 
    {
        lua_pop(L,1);
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
    prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(interface)); /* convert Lua structure to C */    

    PARAM_ENUM(status, mode,           3, CPSS_PX_HAL_BPE_INTERFACE_MODE_ENT);

    status = cpssPxHalBpeInterfaceTypeSet(devNum,&interface,mode);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }    

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}


/**
* @internal wrlCpssPxHalBpeInterfaceExtendedPcidSet function
* @endinternal
*
* @brief   Set Extended interface Pcid.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpeInterfaceExtendedPcidSet
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum; 
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  interface;
    GT_U32                              pcid;
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);    

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 2) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
        
    lua_rawgeti(L, 2, 1);  
    if (!(lua_type(L, -1) == LUA_TTABLE))                 
    {
        lua_pop(L,1);
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
    prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(interface)); /* convert Lua structure to C */    

    PARAM_NUMBER(status, pcid,           3, GT_U32);

    status = cpssPxHalBpeInterfaceExtendedPcidSet(devNum,&interface,pcid);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }    

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}


/**
* @internal wrlCpssPxHalBpePortExtendedUpstreamSet function
* @endinternal
*
* @brief   Set Extended port Upstream interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpePortExtendedUpstreamSet
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum; 
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  upstream;
    GT_U32                              portNum;
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);
    PARAM_NUMBER(status, portNum,        2, GT_U32);    

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 3) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
        
    lua_rawgeti(L, 3, 1);  
    if (!(lua_type(L, -1) == LUA_TTABLE))                 
    {
        lua_pop(L,1);
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
    prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(upstream)); /* convert Lua structure to C */    

    status = cpssPxHalBpePortExtendedUpstreamSet(devNum,portNum,&upstream);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }    

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}


/**
* @internal wrlCpssPxHalBpePortExtendedUntaggedVlanAdd function
* @endinternal
*
* @brief   Configure the Extended interface to egress packets untagged
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpePortExtendedUntaggedVlanAdd
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum; 
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  interface;
    GT_U32                              vid;
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);    

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 2) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
        
    lua_rawgeti(L, 2, 1);  
    if (!(lua_type(L, -1) == LUA_TTABLE))                 
    {
        lua_pop(L,1);
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
    prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(interface)); /* convert Lua structure to C */    

    PARAM_NUMBER(status, vid,           3, GT_U32);

    status = cpssPxHalBpePortExtendedUntaggedVlanAdd(devNum,&interface,vid);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }    

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}

/**
* @internal wrlCpssPxHalBpePortExtendedUntaggedVlanDel function
* @endinternal
*
* @brief   Configure the Extended interface to egress all packets tagged
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpePortExtendedUntaggedVlanDel
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM   devNum; 
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC  interface;
    GT_U32                              vid;
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);    

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 2) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
        
    lua_rawgeti(L, 2, 1);  
    if (!(lua_type(L, -1) == LUA_TTABLE))                 
    {
        lua_pop(L,1);
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
    prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(L, &(interface)); /* convert Lua structure to C */    

    PARAM_NUMBER(status, vid,           3, GT_U32);

    status = cpssPxHalBpePortExtendedUntaggedVlanDel(devNum,&interface,vid);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }    

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}

/**
* @internal wrlCpssPxHalBpePortExtendedQosMapSet function
* @endinternal
*
* @brief   Set CoS mapping between C-UP,C-CFI to E-PCP,E-DEI for Extended port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                        - lua state
*/
int wrlCpssPxHalBpePortExtendedQosMapSet
(
    IN lua_State *L 
)
{
    GT_SW_DEV_NUM devNum;
    GT_U32    portNum;
    GT_U32        i,j;    
    CPSS_PX_HAL_BPE_QOSMAP_STC  qosMapArr[CPSS_PCP_RANGE_CNS][CPSS_DEI_RANGE_CNS];  
    GT_STATUS status = GT_OK;

    PARAM_NUMBER(status, devNum,         1, GT_U8);
    PARAM_NUMBER(status, portNum,        2, GT_U32);

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
   
    if (!(lua_type(L, 3) == LUA_TTABLE))     
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }    

    for(i=0; i< CPSS_PCP_RANGE_CNS;i++)
    {
        lua_rawgeti(L, 3, i);
        if (lua_istable(L,-1))
        {
            for (j=0; j < CPSS_DEI_RANGE_CNS; j++)
            {
                lua_rawgeti(L, -1, j);
                if (lua_istable(L, -1))
                {
                    prv_lua_to_c_CPSS_PX_HAL_BPE_QOSMAP_STC(L, &(qosMapArr[i][j]));
                }
                lua_pop(L,1);
            }
        }
        lua_pop(L,1);
    }

    status = cpssPxHalBpePortExtendedQosMapSet(devNum,portNum,qosMapArr);    
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    lua_pushinteger(L, (lua_Integer)GT_OK);
    return 1;
}


