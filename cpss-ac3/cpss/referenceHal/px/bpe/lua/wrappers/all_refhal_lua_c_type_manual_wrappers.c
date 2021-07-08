/**
********************************************************************************
* @file all_refhal_lua_c_type_manual_wrappers.c
*
* @brief A lua type wrapper
* It implements support for the following types:
* struct CPSS_PX_HAL_BPE_INTERFACE_INFO_STC
*
* @version   1
********************************************************************************
*/
/*************************************************************************
* ./wrappers/all_refhal_lua_c_type_manual_wrappers.c
*
* DESCRIPTION:
*       A lua type wrapper
*       It implements support for the following types:
*           struct  CPSS_PX_HAL_BPE_INTERFACE_INFO_STC
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpssPxHalBpe.h>

/***** declarations ********/
use_prv_struct(CPSS_PX_HAL_BPE_INTERFACE_INFO_STC);
use_prv_enum(CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT);


/*************************************************************************
* prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC
*
* Description:
*       Convert to CPSS_PX_HAL_BPE_INTERFACE_INFO_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(
    lua_State *L,
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC *info
)
{
    F_ENUM(info, -1, type, CPSS_PX_HAL_BPE_INTERFACE_TYPE_ENT);
    switch(info->type)
    {
        case  CPSS_PX_HAL_BPE_INTERFACE_PORT_E:
            lua_getfield(L, -1, "devPort");
            if (lua_istable(L, -1))
            {
                F_NUMBER(&(info->devPort), -1, devNum, GT_SW_DEV_NUM);
                F_NUMBER(&(info->devPort), -1, portNum, GT_PORT_NUM);
            }
            break;
        case CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E:
            F_NUMBER(info, -1, trunkId, GT_TRUNK_ID);
            break;
        case CPSS_PX_HAL_BPE_INTERFACE_NONE_E:
            /* do nothing */
            break;   
        default:
            break;   
    }    
}

void prv_c_to_lua_CPSS_PX_HAL_BPE_INTERFACE_INFO_STC(
    lua_State *L,
    CPSS_PX_HAL_BPE_INTERFACE_INFO_STC *val
)
{
    lua_pushnil(L);
}


add_mgm_type(CPSS_PX_HAL_BPE_INTERFACE_INFO_STC);

