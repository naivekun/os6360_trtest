--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* switchport_egress_filtering.lua
--*
--* DESCRIPTION:
--*       enable/disable "drop on VID1 mismatch" feature on the port/ePort
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
--  vid1EgressFiltering
--
--  @description enable/disable VID1 egress filtering on a port
--
--  @param params - params["vid1"] - VID1 associated with port
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************

local function vid1EgressFiltering(params)
   local command_data = Command_Data()
   local func = function(command_data, devNum, portNum, params)
      local result, values, enable, enableStr

      enable = not (params.flagNo==true)
      enableStr = enable and "enabling" or "disabling"

      if enable then
         result, values = cpssPerPortParamSet(
            "cpssDxChBrgEgrFltPortVid1Set", devNum, portNum,
            params.vid1, "vid1", "GT_U16")

         command_data:handleCpssErrorDevPort(result,
            string.format("setting a VID1 (%d) associated with port ", params.vid1),
            devNum, portNum)
      end

      result, values = cpssPerPortParamSet(
         "cpssDxChBrgEgrFltPortVid1FilteringEnableSet", devNum, portNum,
         enable, "enable", "GT_BOOL")

      command_data:handleCpssErrorDevPort(result,
         string.format("%s VID1 egress filtering ", enableStr),
         devNum, portNum)
   end -- func

   command_data:initInterfaceDevPortRange()

   command_data:iterateOverPorts(func, params)

   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: vid1 egress filtering
--------------------------------------------------------------------------------

CLI_addCommand({"interface", "interface_eport"}, "switchport egress-filtering vid1", {
  func   = vid1EgressFiltering,
  help   = "enable VID1 egress filtering. Packets with tag1 VID different "..
           "then specified <VID1> will be filtered",
  params = {
     {type="values", {format="%unchecked-vlan",
                      name = "vid1",
                      help = "VID1 (0..4095) associated with port"}
     }
  }
})

CLI_addCommand({"interface", "interface_eport"}, "no switchport egress-filtering vid1", {
  func=function(params)
      params.flagNo = true
      return vid1EgressFiltering(params)
  end,
  help="Disable VID1 egress filtering",
  params = {}
})

CLI_addHelp("interface", "switchport egress-filtering",
            "egress filtering")
