--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlan_mapping.lua
--*
--* DESCRIPTION:
--*       configure VLAN mapping (i.e. ingress/egress VLAN Translation Tables)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- *****************************************************************************
--  vlanMapping
--
--  @description configure VLAN mapping
--
--  @param params   - parameters
--     - params["oldVid"]    - old VLAN id value.
--     - params["newVid"]    - new VLAN id value.
--     - params["devNum"]    - a device number
--     - params["ingress"]   - setup an ingress vlan mapping: oldVid->newVid
--     - params["egress"]    - setup an egress vlan mapping: oldVid->newVid
--     - params["all"]       - configure both ingress/egress mappings:
--                             ingress: oldVid->newVid,
--                             egress: newVid->oldVid
--     - params["direction"] - one of "ingress", "egress", "all". Not used.
--
--
--
--  @return  true on success, otherwise false and error message
--
-- *****************************************************************************
local function vlanMapping(params)
   local command_data = Command_Data()
   local result
   local devices
   local args
   local directionStr

   -- check necessary parameters
   if (params.oldVid == nil or params.newVid == nil or params.devNum==nil) then
      local msg = "Some of necessary agruments (devNum, oldVid, newVid) are not specified"
      print(msg)
      return nil, msg
   end

   -- init "devices" table
   devices = (params.devNum=="all") and wrLogWrapper("wrlDevList") or {params.devNum}

   -- init "args" table
   if params.ingress then
      args = {
         {direction = "CPSS_DIRECTION_INGRESS_E",
          oldVid = params.oldVid,
          newVid = params.newVid}
      }
   elseif params.egress then
      args = {
         {direction = "CPSS_DIRECTION_EGRESS_E",
          oldVid = params.oldVid,
          newVid = params.newVid}
      }
   else -- params.all or no one of ingress/egress/all
      args = {
         {direction = "CPSS_DIRECTION_INGRESS_E",
          oldVid = params.oldVid,
          newVid = params.newVid},

         {direction = "CPSS_DIRECTION_EGRESS_E",
          --oldVid and newVid are swapped
          oldVid = params.newVid,
          newVid = params.oldVid}
      }
   end


   for k, device in pairs(devices) do
      for k, arg in pairs(args) do
         directionStr = (arg.direction == "CPSS_DIRECTION_INGRESS_E") and "ingress" or "egress"

         result = myGenWrapper(
            "cpssDxChBrgVlanTranslationEntryWrite",
            {  {"IN", "GT_U8", "devNum", device},
               {"IN", "GT_U16", "vlanId", arg.oldVid},
               {"IN", "CPSS_DIRECTION_ENT", "direction", arg.direction},
               {"IN", "GT_U16", "transVlanId", arg.newVid}}
         )
         command_data:handleCpssErrorDevice(
            result,
            string.format("%s VLAN mapping from %d to %d",
                          directionStr, params.oldVid, params.newVid),
            device)
      end
   end

   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: vlan mapping
--------------------------------------------------------------------------------

CLI_addCommand({"config"}, "vlan mapping", {
  func   = vlanMapping,
  help   = "configure a vlan mapping (VLAN translation tables)",
  params = {
     {type = "named",
      {format = "ingress", help = "configure an ingress mapping: oldVid->newVid"},
      {format = "egress", help = "configure an egress mapping:  oldVid->newVid"},
      {format = "all", help = "configure an ingress (oldVid->newVid) and egress"..
                              "(newVid->oldVid) mapping. This is the default"},

      {format = "device %devID_all", name="devNum", help="a device number"},
      {format = "from %unchecked-vlan", name="oldVid", help="old VLAN id value<1-4095>"},
      {format = "to %unchecked-vlan", name="newVid", help="new VLAN id value<1-4095>"},
      alt = {direction = {"ingress", "egress", "all"}},
      mandatory = {"devNum", "oldVid", "newVid"}
     }
  }
})
