/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssPxPortControl.c
*
* DESCRIPTION:
*       port control functions wrapper
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 16 $
*******************************************************************************/
#include <string.h>
#include <generic/private/prvWraplGeneral.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/common/port/private/prvCpssPortCtrl.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpssCommon/wraplCpssExtras.h>
#include <cpssCommon/wrapCpssDebugInfo.h>
#include <cpss/px/port/cpssPxPortCtrl.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>


/*******************************************************************************
*   Trunk wrapper warnings                                                     *
*******************************************************************************/
static GT_CHAR wrlFunctionArgumentsFromLuaGettingErrorMessage[] =
                                    "Error at reading of function arguments";

static GT_CHAR wrlPortModeSpeedErrorMwssage[] =
                                "API error at setting of port mode and speed";


/***** declarations ********/

use_prv_enum(CPSS_PORT_INTERFACE_MODE_ENT);
use_prv_enum(CPSS_PORT_SPEED_ENT);
use_prv_print_struct(CPSS_PORTS_BMP_STC);

/***** declarations ********/

/*******************************************************************************
* wrlPxPortModeSpeedSet
*
* DESCRIPTION:
*        Configure Interface mode and speed on a specified port and execute on
*        port's serdeses power up sequence; or configure power down on port's
*        serdeses.
*
* APPLICABLE DEVICES:
*        pipe.
*
* NOT APPLICABLE DEVICES:
*
* INPUTS:
*       L                         - lua state
*       L[1]                      - physical device number
*       L[2]                      - physical port number (or CPU port)
*       L[3]                      - serdes power
*       L[4]                      - interface mode
*       L[5]                      - port data speed
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK is pused to lua stack if no errors occurs
*       error status and error message it error occures
*
* COMMENTS:
*
*******************************************************************************/
 int wrlPxPortModeSpeedSet
(
    IN lua_State *L
)
{
    GT_SW_DEV_NUM        devNum   = (GT_SW_DEV_NUM)lua_tonumber(L, 1);/* devId */
    GT_PORT_NUM          portNum  = (GT_PORT_NUM)lua_tonumber(L, 2);/* portNum */
    CPSS_PORTS_BMP_STC   portsBmp;
    GT_BOOL              powerUp  = (GT_BOOL)lua_toboolean(L, 3);/* powerUp */
    CPSS_PORT_SPEED_ENT  speed    = CPSS_PORT_SPEED_NA_E;
    GT_STATUS            status   = GT_OK;
    GT_CHAR_PTR          error_message  = NULL;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    P_CALLING_CHECKING;

    cpssOsMemSet(&portsBmp, 0, sizeof(portsBmp));
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

    status = pvrCpssPortInterfaceModeFromLuaGet(L, 4, &ifMode, &error_message);
    if (GT_OK != status)
    {
        error_message   = wrlFunctionArgumentsFromLuaGettingErrorMessage;
    }

    if (GT_OK == status)
    {
        status = pvrCpssPortSpeedFromLuaGet(L, 5, &speed, &error_message);
        if (GT_OK != status)
        {
            error_message   = wrlFunctionArgumentsFromLuaGettingErrorMessage;
        }
    }

    if (GT_OK == status)
    {
        status = cpssPxPortModeSpeedSet(devNum, &portsBmp, powerUp, ifMode, speed);
        P_CALLING_API(
            cpssPxPortModeSpeedSet,
            PC_NUMBER(devNum,     devNum,  GT_SW_DEV_NUM,
            PC_STRUCT(&portsBmp,  portsBmp,
                                              CPSS_PORTS_BMP_STC,
            PC_BOOL  (powerUp,    powerUp,
            PC_ENUM  (ifMode,     ifMode,
                                    CPSS_PORT_INTERFACE_MODE_ENT,
            PC_ENUM  (speed,      speed,     CPSS_PORT_SPEED_ENT,
            PC_LAST_PARAM))))),
            PC_STATUS);
        if (GT_OK != status)
        {
            error_message   = wrlPortModeSpeedErrorMwssage;
        }
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}

/*******************************************************************************
* wrlPxPortsModeSpeedSet
*
* DESCRIPTION:
*        Configure Interface mode and speed on a specified port bitmap and
*        execute on port's serdeses power up sequence; or configure power down
*        on port's serdeses.
*
* APPLICABLE DEVICES:
*        pipe.
*
* NOT APPLICABLE DEVICES:
*
* INPUTS:
*       L                         - lua state
*       L[1]                      - physical device number
*       L[2]                      - port bitmap
*       L[3]                      - serdes power
*       L[4]                      - interface mode
*       L[5]                      - port data speed
*
* OUTPUTS:
*
* RETURNS:
*       GT_OK is pused to lua stack if no errors occurs
*       error status and error message it error occures
*
* COMMENTS:
*
*******************************************************************************/
 int wrlPxPortsModeSpeedSet
(
    IN lua_State *L
)
{
    GT_SW_DEV_NUM        devNum    = (GT_SW_DEV_NUM)lua_tonumber(L, 1);/* devId */
    CPSS_PORTS_BMP_STC   portsBmp;
    GT_BOOL              powerUp   = (GT_BOOL)lua_toboolean(L, 3);/* powerUp */
    CPSS_PORT_SPEED_ENT  speed      = CPSS_PORT_SPEED_NA_E;
    GT_STATUS            status     = GT_OK;
    GT_CHAR_PTR          error_message  = NULL;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    P_CALLING_CHECKING;

    cpssOsMemSet(&portsBmp, 0, sizeof(portsBmp));
    status = pvrCpssPortInterfaceModeFromLuaGet(L, 4, &ifMode, &error_message);
    if (GT_OK != status)
    {
        error_message   = wrlFunctionArgumentsFromLuaGettingErrorMessage;
    }

    if (GT_OK == status)
    {
        status = pvrCpssPortSpeedFromLuaGet(L, 5, &speed, &error_message);
        if (GT_OK != status)
        {
            error_message   = wrlFunctionArgumentsFromLuaGettingErrorMessage;
        }
    }

    if (GT_OK == status)
    {
        status = pvrCpssPortsMembersFromLuaGet(L, 2, &portsBmp, &error_message);
        if (GT_OK != status)
        {
            error_message   = wrlFunctionArgumentsFromLuaGettingErrorMessage;
        }
    }

    if (GT_OK == status)
    {
        status = cpssPxPortModeSpeedSet(devNum, &portsBmp, powerUp, ifMode,
                                          speed);
        P_CALLING_API(
            cpssPxPortModeSpeedSet,
            PC_NUMBER(devNum,    devNum,   GT_SW_DEV_NUM,
            PC_STRUCT(&portsBmp, portsBmp, CPSS_PORTS_BMP_STC,
            PC_BOOL  (powerUp,   powerUp,
            PC_ENUM  (ifMode,    ifMode, CPSS_PORT_INTERFACE_MODE_ENT,
            PC_ENUM  (speed,     speed,  CPSS_PORT_SPEED_ENT,
            PC_LAST_PARAM))))),
            PC_STATUS);
        if (GT_OK != status)
        {
            error_message   = wrlPortModeSpeedErrorMwssage;
        }
    }

    return prvWrlCpssStatusToLuaPush(status, L, error_message);
}


/*******************************************************************************
* wrlPxPortLoopbackModeEnableGet
*
* DESCRIPTION:
*       get port loopback mode
*
* INPUTS:
*       devNum        - device number
*       portNum       - port number
*
* OUTPUTS:
*       enable        - enable/disable loopback
*
* RETURNS:
*       return code
*
* COMMENTS:
*
*******************************************************************************/
int wrlPxPortLoopbackModeEnableGet
(
    IN lua_State *L
)
{
    GT_SW_DEV_NUM        devNum;
    GT_PORT_NUM          portNum;
    GT_BOOL              enablePtr = GT_FALSE;
    GT_STATUS            rc = GT_OK;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    PARAM_NUMBER(rc, devNum,  1, GT_SW_DEV_NUM);
    PARAM_NUMBER(rc, portNum, 2, GT_PORT_NUM);

    rc = cpssPxPortInterfaceModeGet(devNum,portNum,&ifMode);
    if (ifMode != CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        rc = cpssPxPortInternalLoopbackEnableGet(devNum, portNum, &enablePtr);
    }
    else
    {
        rc = GT_NOT_INITIALIZED;
    }

    lua_pushinteger(L, rc);
    lua_pushinteger(L, enablePtr);

    return 2;
}


/*******************************************************************************
* wrlCpssPxPortNumberOfSerdesLanesGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Get number of quantity of SERDESes occupied by given
*       port.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                   - lua state (in stack should be device and port
*                             number)
*
* OUTPUTS:
*       number of returned to lua elements, quantity of SERDESes occupied to lua stack,
*       if exists, otherwise 0
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssPxPortNumberOfSerdesLanesGet(lua_State *L)
{
    GT_U32                  firstSerdes;
    GT_U32                  numOfSerdesLanes;
    GT_SW_DEV_NUM           devNum;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_STATUS               rc;

    if ((lua_gettop(L)==2) &&  (lua_isnumber(L, 1)) && (lua_isnumber(L, 2))  )
    {
        devNum          =  (GT_U8)      lua_tonumber(L, 1);
                                                                    /* devId */
        portNum         =  (GT_PHYSICAL_PORT_NUM)  lua_tonumber(L, 2);
                                                                    /* portNum */
    }
    else
    {
        lua_pushnil(L);
        return 1;
    }


    rc = prvCpssPxPortNumberOfSerdesLanesGet(devNum, portNum, &firstSerdes, &numOfSerdesLanes);
    if (rc !=GT_OK)
    {
        lua_pushinteger(L,rc);
        lua_pushnil(L);

        return 2;
    }
    lua_pushinteger(L,rc);
    lua_pushinteger(L,numOfSerdesLanes);
    return 2;
}


/*******************************************************************************
* wrlPxPortSerdesEyeGet
*
* DESCRIPTION:
*        Get vbtc, hbtc and Eye Matrix to represent in lua
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                         - lua state
*       L[1]                      - device number
*       L[2]                      - port number
*       L[3]                      - serdes number
*       L[4]                      - minimum dwell bits
*       L[5]                      - maximum dwell bits
*       L[6]                      - 1 - noeye, 0 - otherwize
*
* OUTPUTS:
*   if OK
*       number of columns
*       number of rows
*       result text string
*   if NOT OK
*       nil
*       message
*
* RETURNS:
*       push number
*
* COMMENTS:
*
*******************************************************************************/
#define MAX_STRING_LENGTH 100
int wrlPxPortSerdesEyeGet
(
    IN lua_State *L
)
{
    GT_STATUS rc;
    GT_U32    n_push = 0;
    GT_U32    len_vbtc;
    GT_U32    len_hbtc;
    GT_U32    len_matrix;
    GT_U32    len_result;
    GT_U32    len_height;
    GT_U32    len_width;
    GT_CHAR   *result;
    GT_U32    i,j;
    CPSS_PX_PORT_SERDES_EYE_RESULT_STC eye_results;
    CPSS_PX_PORT_SERDES_EYE_INPUT_STC eye_input;
    GT_CHAR str_height[MAX_STRING_LENGTH];
    GT_CHAR str_width[MAX_STRING_LENGTH];

    if (!(lua_isnumber(L,1)) || !(lua_isnumber(L,2)) || !(lua_isnumber(L,3)))
    {
        lua_pushnil(L);
        lua_pushstring(L,"All parameters must be integers");
        n_push = 2;
    }
    else
    {
        eye_results.matrixPtr = NULL;
        eye_results.vbtcPtr = NULL;
        eye_results.hbtcPtr = NULL;
        eye_input.min_dwell_bits = (GT_U32) lua_tonumber(L, 4);
        eye_input.max_dwell_bits = (GT_U32) lua_tonumber(L, 5);

        rc=cpssPxPortSerdesEyeMatrixGet((GT_U8) lua_tonumber(L, 1),(GT_PHYSICAL_PORT_NUM)lua_tointeger(L, 2),
                                    (GT_U32) lua_tonumber(L, 3), &eye_input, &eye_results);
        if(GT_OK == rc)
        {
            cpssOsSprintf(str_height,"\nSerDes No. %d: has an eye height of %d mV.\n", eye_results.globalSerdesNum, eye_results.height_mv);
            cpssOsSprintf(str_width,"\nSerDes No. %d: has an eye width of %d mUI.\n", eye_results.globalSerdesNum, eye_results.width_mui);
            len_height = cpssOsStrlen(str_height);
            len_width = cpssOsStrlen(str_width);
            if(NULL == eye_results.vbtcPtr)
            {
                len_vbtc = 0;
            }
            else
            {
                len_vbtc = cpssOsStrlen(eye_results.vbtcPtr);
            }
            if(NULL == eye_results.hbtcPtr)
            {
                len_hbtc = 0;
            }
            else
            {
                len_hbtc = cpssOsStrlen(eye_results.hbtcPtr);
            }
            if((NULL == eye_results.matrixPtr) || (1 == ((GT_U32) lua_tonumber(L, 6))))
            {
                len_matrix = 0;
            }
            else
            {
                len_matrix = cpssOsStrlen(eye_results.matrixPtr);
            }
            len_result = len_vbtc + len_hbtc + len_matrix + len_height + len_width;
            result = (GT_CHAR*)cpssOsMalloc(len_result + 1);
            j = 0;
            for(i = 0; i < len_height; i++)
            {
                result[j++] = str_height[i];
            }
            for(i = 0; i < len_width; i++)
            {
                result[j++] = str_width[i];
            }
            for(i = 0; i < len_vbtc; i++)
            {
                result[j++] = eye_results.vbtcPtr[i];
            }
            for(i = 0; i < len_hbtc; i++)
            {
                result[j++] = eye_results.hbtcPtr[i];
            }
            for(i = 0; i < len_matrix; i++)
            {
                result[j++] = eye_results.matrixPtr[i];
            }
            result[j] = 0;
            lua_pushnumber(L, eye_results.x_points);
            lua_pushnumber(L, eye_results.y_points);
            lua_pushstring(L, result);
            cpssOsFree(result);
            n_push = 3;
        }
        else
        {
            lua_pushnil(L);
            lua_pushstring(L,"Could not get eye matrix");
            n_push = 2;
        }
        if(NULL != eye_results.matrixPtr)
        {
            cpssOsFree(eye_results.matrixPtr);
            eye_results.matrixPtr = NULL;
        }
        if(NULL != eye_results.vbtcPtr)
        {
            cpssOsFree(eye_results.vbtcPtr);
            eye_results.vbtcPtr = NULL;
        }
        if(NULL != eye_results.hbtcPtr)
        {
            cpssOsFree(eye_results.hbtcPtr);
            eye_results.hbtcPtr = NULL;
        }
    }
    return n_push;
}
