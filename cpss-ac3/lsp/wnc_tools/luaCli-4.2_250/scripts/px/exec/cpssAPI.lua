--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpssAPI.lua
--*
--* DESCRIPTION:
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlPxPortModeSpeedSet")
--constants

-- CPSS_PX_CNC_CLIENT_ENT;
  CLI_type_dict["cncClientEnum"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "CNC clients",
    enum = {
        ["CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E"]          =  { value="CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E",         help="CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E" },
        ["CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E"]           =  { value="CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E",          help="CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E" },
        ["CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E"]  =  { value="CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E", help="CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E" }
    }
  }



-- direct call of cpssPxPortModeSpeedSet ____________________________________

    local function direct_call_cpssPxPortModeSpeedSet(params)
        local ret

      ret = wrlPxPortModeSpeedSet(
          params.devID,
          params.port,
          params.powerUp,
          params.ifMode,
          params.speed)

      if (ret ~= 0) then
        printErr("cpssPxPortModeSpeedSet() failed: " .. returnCodes[ret])
        return false, "Error in cpssPxPortModeSpeedSet()"
      end
      return true
    end

    -- direct call of cpss api cpssPxPortModeSpeedSet
    CLI_addCommand("exec", "cpss-api call cpssPxPortModeSpeedSet",
    {
      help = "Direct call of function cpssPxPortModeSpeedSet",
      func = direct_call_cpssPxPortModeSpeedSet,
      params={{type= "named",
                {format="device %devID",                  name="devID",   help="The device number"},
                {format="portNum %GT_U32",                name="port",   help="The port number"},
                {format="powerUp  %bool",              name="powerUp", help="Serdes power up"},
                {format="ifMode  %port_interface_mode", name="ifMode",  help="Interface mode"},
                {format="speed  %port_speed",          name="speed",   help="Port speed"},
                requirements={ ["port"]={"devID"}, ["powerUp"]={"port"}, ["ifMode"]={"powerUp"}, ["speed"]={"ifMode"} },
                mandatory = {"devID", "port", "powerUp", "ifMode", "speed"}
             }}
    })

  -- end of cpssPxPortModeSpeedSet()_________________________________________
