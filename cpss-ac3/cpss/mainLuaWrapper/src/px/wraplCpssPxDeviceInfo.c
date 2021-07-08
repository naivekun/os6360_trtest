/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssPxDeviceInfo.c
*
* DESCRIPTION:
*       PX device info wrapper
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <px/wraplCpssPxDeviceInfo.h>
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpss/px/version/cpssPxVersion.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>


/*******************************************************************************
* wrlMvPxVersionGet
*
* DESCRIPTION:
*       Getting of cpss px version name.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       L   - lua state
*
* OUTPUTS:
*       HW version
*
* RETURNS:
*       Count of put lua_State parameters
*
*******************************************************************************/
int wrlMvPxVersionGet
(
    IN lua_State    *L
)
{
    GT_STATUS   rc;
    GT_VERSION  versionInfo;

    rc = cpssPxVersionGet(&versionInfo);
    if (rc != GT_OK)
    {
        return 0; /* nil */
    }

    lua_pushstring(L, (char*)(versionInfo.version));


#ifdef SHARED_MEMORY
    lua_pushboolean(L, 1);
#else
    lua_pushboolean(L, 0);
#endif /* SHARED_MEMORY */

#ifdef LINUX_NOKM
    lua_pushboolean(L, 1);
#else
    lua_pushboolean(L, 0);
#endif /* LINUX_NOKM */

#ifdef IS_64BIT_OS
    lua_pushboolean(L, 1);
#else
    lua_pushboolean(L, 0);
#endif /* IS_64BIT_OS */

    return 4;
}

/*******************************************************************************
* wrlPxRevisionA1AndAboveCheck
*
* DESCRIPTION:
*       Check if PIPE os of revision A1 and above
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       L   - lua state
*
* OUTPUTS:
*       HW version
*
* RETURNS:
*       Count of put lua_State parameters
*
*******************************************************************************/
int wrlPxRevisionA1AndAboveCheck
(
    IN lua_State    *L
)
{
    GT_SW_DEV_NUM dev;

    if (lua_isnumber(L, 1))
    {
        dev = (GT_SW_DEV_NUM)lua_tonumber(L, 1);
    }
    else
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(dev))
    {
        lua_pushinteger(L, (lua_Integer)GT_BAD_PARAM);
        return 1;
    }
    if(0 == PRV_CPSS_PX_FAMILY_CHECK_MAC(dev))
    {
        lua_pushinteger(L, (lua_Integer)GT_NOT_APPLICABLE_DEVICE);
        return 1;
    }

    lua_pushinteger(L, (lua_Integer)GT_OK);
    if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(dev))
    {
        lua_pushboolean(L, 1);
    }
    else
    {
        lua_pushboolean(L, 0);
    }
    return 2;
}