--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* switchport_vlan_mapping.lua
--*
--* DESCRIPTION:
--*       enable/disable "vlan translation" feature on the port/ePort
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
--  vlanMappingEn
--
--  @description enable/disable ingress/egress VLAN Translation on a port
--
--  @param params   - parameters
--     - params["ingress"]   - setup port for incoming packets
--     - params["egress"]    - setup port for outgoing packets
--     - params["all"]       - setup port for incoming/outgoing packets
--     - params["direction"] - one of "ingress", "egress", "all". Not used.
--     - params["useSid"]    - use eVLAN<service-ID> instead of VLAN
--                             Translation Table
--     - params["vid0vid1"] - can be "vid0-vid1" or "vid0-only".
--
--
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function vlanMappingEn(params)

   local command_data = Command_Data()

   local func = function(command_data, devNum, portNum, params)
      local result, values
      local enable, enableStr
      local directions, directionStr

      -- init "directions" table
      if params.ingress then
         directions = {"CPSS_DIRECTION_INGRESS_E"}
      elseif params.egress then
         directions = {"CPSS_DIRECTION_EGRESS_E"}
      else --  Default
         directions = {"CPSS_DIRECTION_INGRESS_E",
                       "CPSS_DIRECTION_EGRESS_E"}
      end

      -- init "enable" value
      if params.flagNo then
         enable = "CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E"
      elseif params.useSid then
         if params.vid0vid1 == "vid0-vid1" then
            enable = "CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_VID1_E"
         else
            enable = "CPSS_DXCH_BRG_VLAN_TRANSLATION_SID_VID0_E"
         end
      else
         enable = "CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E"
      end

      enableStr = (enable=="CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E")
         and "disabling" or "enabling"

      for i=1, #directions do
         result, values = myGenWrapper(
            "cpssDxChBrgVlanPortTranslationEnableSet",
            {  {"IN", "GT_U8", "devNum", devNum},
               {"IN", "GT_U32", "portNum", portNum},
               {"IN", "CPSS_DIRECTION_ENT", "direction", directions[i]},
               {"IN", "CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT", "enable", enable}}
         )
         directionStr = (directions[i]== CPSS_DIRECTION_INGRESS_E) and "ingress" or "egress"

         command_data:handleCpssErrorDevPort(
            result, string.format("%s VLAN mapping %s ", directionStr, enableStr))
      end
   end -- func


   command_data:initInterfaceDevPortRange()

   command_data:iterateOverPorts(func, params)

   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: switchport vlan mapping enable
--------------------------------------------------------------------------------

CLI_addCommand({"interface", "interface_eport"}, "switchport vlan mapping enable", {
  func   = vlanMappingEn,
  help   = "enable a vlan mapping on the port(s).",
  params = {
     {type="named",
      {format = "ingress", help = "enable for incoming packets"},
      {format = "egress", help = "enable for outgoing packets"},
      {format = "all", help = "enable for incoming and outgoing packets. This is default."},
      {format = "use-sid", name = "useSid", help = "use VLAN<service-id>. If not specified a VLAN Translation Tables will be used."},
      {format = "vid0-only", name="vid0vid1", help = "use eVLAN<Service-ID>[23:12] as new tag0 VLAN-ID value. This is default."},
      {format = "vid0-vid1", name="vid0vid1", help = "use eVLAN<Service-ID>[23:12] as new tag0 VLAN-ID value and eVLAN<Service-ID>[11:0] as new tag1 VLAN-ID value."},

      alt = {direction = {"ingress", "egress", "all"}},

      requirements = {
         ["vid0vid1"] = {"useSid"},
         ["useSid"] = {"egress"}
      }
     },
  }
})


CLI_addCommand({"interface", "interface_eport"}, "no switchport vlan mapping enable", {
  func=function(params)
      params.flagNo = true
      return vlanMappingEn(params)
  end,
  help="Disable a vlan mapping",
  params = {
     {type="named",
      {format = "ingress", help = "disable for incoming packets"},
      {format = "egress", help = "disable for outgoing packets"},
      {format = "all", help = "disable for incoming and outgoing packets"..
                              " This is the default"},
      alt = {direction = {"ingress", "egress", "all"}},
     }
  }
})
