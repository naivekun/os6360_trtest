--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_service_id.lua
--*
--* DESCRIPTION:
--*       setup a eVLAN<service-id> value
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- *****************************************************************************
--  vlanServiceId
--
--  @description configure a service-id value assiciated with eVLAN.
--
--  @param params   - parameters
--                    - params["devNum"] - a device number
--                    - params["vid"]    - a VLAN id value.
--                    - params["sid"]    - a service-id value
--
--
--
--  @return  true on success, otherwise false and error message
--
-- *****************************************************************************
function vlanServiceId(params)
   local command_data = Command_Data()
   local result
   local devices
   local vid, sid

   -- check mandatory parameters
   if (params.devNum == nil or params.vid==nil or params.sid == nil) then
      local msg = "Some of necessary agruments (devNum, vid, sid) are not specified"
      print(msg)
      return nil, msg
   end

   -- init "devices" table
   devices = (params.devNum=="all") and wrLogWrapper("wrlDevList") or {params.devNum}

   vid = params.vid
   sid = params.sid
   for k, device in pairs(devices) do

      result = myGenWrapper(
         "cpssDxChTunnelStartEgessVlanTableServiceIdSet",
         {  {"IN", "GT_U8", "devNum", device},
            {"IN", "GT_U16", "vlanId", vid},
            {"IN", "GT_U32", "vlanServiceId", sid}
         }
      )
      command_data:handleCpssErrorDevice(
         result,
         string.format("setting a VLAN's %d service-id to %d", vid, sid),
         device)
   end
   command_data:updateStatus()
   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: switchport vlan mapping enable
--------------------------------------------------------------------------------

CLI_addCommand({"config"}, "vlan service-id", {
  func   = vlanServiceId,
  help   = "configure a service ID associated with VLAN",
  params = {
     {type = "named",
      {format="device %devID_all", name="devNum", help = "a device number"},
      {format="vid %unchecked-vlan", name="vid", help =  "a VLAN id number"},
      {format="sid %service_id", name="sid", help = "a service ID value"},
      mandatory = {"devNum", "vid", "sid"}
     },
  }
})

CLI_type_dict["service_id"] = {
    checker  = CLI_check_param_number,
    complete = CLI_complete_param_number,
    min = 0,
    max = 0xFFFFFF,
    help = "a service ID value (24-bit length)"
}
