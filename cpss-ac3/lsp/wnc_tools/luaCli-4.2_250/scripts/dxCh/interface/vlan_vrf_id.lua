--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_vrf_id.lua
--*
--* DESCRIPTION:
--*       add commands to set or reset a vrf-id of vlan
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--includes

--constants

-- ************************************************************************
--  vlanVrfId
--
--  @description set or reset a vrf-id value of vlan or range of vlans
--
--  @param params   - parameters
--     - params["vrf_id"]    - vrf-id value
--     - params["flagNo"]    - if true, reset vrf-id of vlan. Else set.
--
--  @return command status and error messages
--
-- ************************************************************************

local function vlanVrfId(params)
   local command_data = Command_Data()

   local vrfId  = params["vrf_id"]
   local flagNo = params["flagNo"] or false

   if flagNo then
      vrfId = 0
   end

   -----------------------------------------------------------------------------
   command_data:initInterfaceDevVlanRange()
   command_data:initInterfaceDeviceRange("dev_vlan_range")
   command_data:initDevVlanInfoIterator()


   -- iterate through valid VLANs
   command_data:clearVlanStatus()
   command_data:clearLocalStatus()
   for _, devNum, vlanId, vlanInfo in command_data:getDeviceVlanInfoIterator() do
      vlanInfo["vrfId"] = vrfId
      local rc, errMsg = vlan_info_set(devNum, vlanId, vlanInfo)
      if rc ~=0 then
         self:setFailVlanAndLocalStatus()

         if rc == 0x10 then
            self:addWarning("Information setting of vlan %d is not " ..
                               "allowed on device %d.", vlanId, devNum)
         else
            self:addError("Error at information setting of vlan %d " ..
                             "on device %d: %s.", vlanId, devNum, errMsg)
         end
      end
   end

   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()

end
--------------------------------------------------------------------------------
-- command registration: ip vrf-id
--------------------------------------------------------------------------------
CLI_addCommand({"vlan_configuration"}, "ip vrf-id", {
  func   = vlanVrfId,
  help   = "set vlan's Virtual Router ID",
  params = {{type="values", {format="%vrf_id_without_0", name = "vrf_id"}}}

})

--------------------------------------------------------------------------------
-- command registration: no ip vrf-id
--------------------------------------------------------------------------------
CLI_addCommand({"vlan_configuration"}, "no ip vrf-id", {
  func   = function(params)
               params.flagNo = true
               return vlanVrfId(params)
           end,
  help   = "clear vlan's binding to Virtual Router ID",
})
