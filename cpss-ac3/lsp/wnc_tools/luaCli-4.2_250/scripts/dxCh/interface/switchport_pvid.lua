--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* switchport_pvid.lua
--*
--* DESCRIPTION:
--*       setting of the port PVID
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants
local default_pvid = 1


-- ************************************************************************
---
--  switchport_pvid_func
--        @description  set's the port PVID
--
--        @param params             - params["vlanID"]: configured vlan-id;
--                                    params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function switchport_pvid_func(params)
    local command_data = Command_Data()
    local vlanId

    command_data:initInterfaceDevPortRange()

    params.vlanId = command_data:getValueIfFlagNoIsNil(params, params["vlanId"],
                                                default_pvid)

    command_data:iterateOverPorts(
        function(command_data, devNum, portNum, params)
            local result
            result = cpssPerPortParamSet(
                    "cpssDxChBrgVlanPortVid"..params.subtype.."Set",
                    devNum, portNum, params.vlanId, "vlanId",
                    "GT_U16")
            command_data:handleCpssErrorDevPort(result,
                    "cpssDxChBrgVlanPortVid"..params.subtype.."Set()")

            if params.up ~= nil then
                result = myGenWrapper(
                    "cpssDxChPortDefaultUPSet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "GT_PORT_NUM", "portNum", portNum},
                        { "IN", "GT_U8", "defaultUserPrio", params.up}
                    })
                command_data:handleCpssErrorDevPort(result, "cpssDxChPortDefaultUPSet()")
            end
        end, params)

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------
-- command registration: switchport pvid
--------------------------------------------------------
CLI_type_dict["pvid_user_priority"] = {
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min=0,
    max=7,
    help = "User priority"
}

CLI_addCommand({"interface", "interface_eport"}, "switchport pvid", {
  func   = function(params)
    params.subtype=""
    return switchport_pvid_func(params)
  end,
  help   = "Configure port VLAN ID",
  params = {
      { type = "values",
          "%vlanId"
      },
      { type="named",
          { format="up %pvid_user_priority", name="up" }
      }
  }
})

CLI_addCommand({"interface", "interface_eport"}, "switchport pvid1", {
  func   = function(params)
    params.subtype="1"
    return switchport_pvid_func(params)
  end,
  help   = "Configure port VLAN ID",
  params = {
      { type = "values",
          "%vlanId"
      },
      { type="named",
          { format="up %pvid_user_priority", name="up" }
      }
  }
})

--------------------------------------------------------
-- command registration: no switchport pvid
--------------------------------------------------------
CLI_addCommand({"interface", "interface_eport"}, "no switchport pvid", {
  func   = function(params)
               params.flagNo = true
               params.subtype=""
               params.up = 1
               default_pvid = 1
               switchport_pvid_func(params)
           end,
  help   = "Configure port VLAN ID"
})

--------------------------------------------------------
-- command registration: no switchport pvid
--------------------------------------------------------
CLI_addCommand({"interface", "interface_eport"}, "no switchport pvid1", {
  func   = function(params)
               params.flagNo = true
               params.subtype="1"
               params.up = 1
               default_pvid = 0
               switchport_pvid_func(params)
           end,
  help   = "Configure port VLAN ID1 (only Bobcat2)"
})
