--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_routing.lua
--*
--* DESCRIPTION:
--*       enabling/disabling of routing on an interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  interface_ip_routing_func
--        @description  enables/disables routing on an interface
--
--        @param params         - params["flagNo"]: no-flag
--
--        @return       true on success, otherwise false and error message
--
local function interface_ip_routing_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local enable_routing
    local uni_or_multicast;

    uni_or_multicast = "CPSS_IP_UNICAST_E";
    if params.mc_routing then
        uni_or_multicast = "CPSS_IP_MULTICAST_E";
    end

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()

    -- Command specific variables initialization.
    enable_routing  = getFalseIfNotNil(params["flagNo"])

    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()

            -- enable PCL lookup1 on port (done only when Policy Based Routing used)
            pbr_port_enable(
                devNum, portNum, enable_routing --[[enable--]], 0 --[[pcl_id--]]);

            local devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum);

            if devFamily ~= "CPSS_PP_FAMILY_CHEETAH_E" and
                devFamily ~= "CPSS_PP_FAMILY_DXCH_XCAT2_E"
            then
                -- Routing enabling setting.
                if true == command_data["local_status"] then
                result, values =
                    myGenWrapper("cpssDxChIpPortRoutingEnable", {
                                 { "IN", "GT_U8",       "devNum",         devNum                                       },
                                 { "IN", "GT_PORT_NUM", "portNum",        portNum                                      },
                                 { "IN", "CPSS_IP_UNICAST_MULTICAST_ENT", "ucMcEnable",    uni_or_multicast          },
                                 { "IN", "CPSS_IP_PROTOCOL_STACK_ENT",    "protocolStack", "CPSS_IP_PROTOCOL_IPV4V6_E" },
                                 { "IN", "GT_BOOL",     "enableRouting",  enable_routing                               }})

                    if     0x10 == result then
                        command_data:setFailDeviceStatus()
                        command_data:addWarning("Routing %s is not allowed on " ..
                                                "device %d port %d.",
                                                boolEnablingLowerStrGet(enable_routing),
                                                devNum, portNum)
                    elseif    0 ~= result then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at routing %s on device %d " ..
                                              "port %d: %s.",
                                              boolEnablingLowerStrGet(enable_routing),
                                              devNum, portNum, returnCodes[result])
                    end
                end
            end

            command_data:updateStatus()

            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts(
            "Can not %s routing on all processed ports.",
            boolEnableLowerStrGet(enable_routing))
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: ip routing
--------------------------------------------------------------------------------
CLI_addHelp({"interface", "interface_eport"}, "ip", "IP configuration commands")
CLI_addCommand({"interface", "interface_eport"}, "ip routing", {
  func   = interface_ip_routing_func,
  help   = "Enabling of routing on an interface"
})

CLI_addCommand({"interface", "interface_eport"}, "ip mc-routing", {
  func   = function(params)
      params.mc_routing = true;
      interface_ip_routing_func(params);
  end,
  help   = "Enabling of multicast routing on an interface"
})

--------------------------------------------------------------------------------
-- command registration: no ip routing
--------------------------------------------------------------------------------
CLI_addHelp({"interface", "interface_eport"}, "no ip", "IP configuration commands")
CLI_addCommand({"interface", "interface_eport"}, "no ip routing", {
  func   = function(params)
               params.flagNo = true
               return interface_ip_routing_func(params)
           end,
  help   = "Disabling of routing on an interface"
})

CLI_addCommand({"interface", "interface_eport"}, "no ip mc-routing", {
  func   = function(params)
               params.flagNo = true
               params.mc_routing = true;
               return interface_ip_routing_func(params)
           end,
  help   = "Disabling of multicast routing on an interface"
})

local function enableIpRoutingVlan(devNum,vlanId, enable, mc_routing)
    local ret, values
    ret,values=cpssGenWrapper("cpssDxChBrgVlanEntryRead",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", vlanId },
      { "OUT", "CPSS_PORTS_BMP_STC", "portsMembers"},
      { "OUT", "CPSS_PORTS_BMP_STC", "portsTagging"},
      { "OUT", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo" },
      { "OUT", "GT_BOOL", "isValid" },
      { "OUT", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd" }
    })
    if ret ~= 0 then return false end
    if not values.isValid then return false end
    if mc_routing then
        values.vlanInfo.ipv4McastRouteEn = enable
        values.vlanInfo.ipv6McastRouteEn = enable
    else
        values.vlanInfo.ipv4UcastRouteEn = enable
        values.vlanInfo.ipv6UcastRouteEn = enable
    end
    ret=cpssGenWrapper("cpssDxChBrgVlanEntryWrite",{
      { "IN", "GT_U8", "devNum", devNum },
      { "IN", "GT_U16","vlanId", vlanId },
      { "IN", "CPSS_PORTS_BMP_STC", "portsMembers", values.portsMembers },
      { "IN", "CPSS_PORTS_BMP_STC", "portsTagging", values.portsTagging },
      { "IN", "CPSS_DXCH_BRG_VLAN_INFO_STC", "vlanInfo", values.vlanInfo},
      { "IN", "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC", "portsTaggingCmd", values.portsTaggingCmd}
    })
    return ret == 0
end

local function ip_routing_vlan(params)
    local command_data = Command_Data()
    local macAddr=myMac

    if macAddr == nil then
        print("Error. Please configure mac address first.")
        return false
    end

     -- Common variables initialization
    command_data:initInterfaceDevVlanRange()

    -- Main vlan handling cycle
    if true == command_data["status"] then
        local iterator, devNum, vlanId

        for iterator, devNum, vlanId in
                                    command_data:getInterfaceVlanIterator() do
            command_data:clearVlanStatus()
            command_data:clearLocalStatus(command_data)

            local macEntry = {
                isStatic=true,
                key={
                    entryType="CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
                    key = {
                      macVlan = {
                            macAddr=macAddr,
                            vlanId=vlanId
                      }
                    },
                },
                dstInterface={
                    type="CPSS_INTERFACE_PORT_E",
                    devPort={
                        devNum=device_to_hardware_format_convert(devNum),
                        portNum=63
                    }
                },
                daRoute = true
            }


            local result, values = mac_entry_set(devNum, macEntry)
            if     0x10 == result then
                command_data:setFailDeviceStatus()
                command_data:addWarning("Mac address entry setting is not " ..
                                        "allowed on device %d.", devNum)
            elseif 0 ~= result then
                command_data:setFailDeviceAndLocalStatus()
                command_data:addError("Error at setting of mac address " ..
                                      "entry on device %d: %s.", devNum,
                                      returnCodes[result])
            end

            enableIpRoutingVlan(devNum, vlanId, true, params.mc_routing)

            command_data:updateStatus()

            command_data:updateVlans()
        end

        command_data:addWarningIfNoSuccessVlans(
            "Can not configure ports of all processed vlans.")
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
  return command_data:getCommandExecutionResults()
end


local function no_ip_routing_vlan(params)
    local command_data = Command_Data()

     -- Common variables initialization
    command_data:initInterfaceDevVlanRange()

    -- Main vlan handling cycle
    if true == command_data["status"] then
        local iterator, devNum, vlanId

        for iterator, devNum, vlanId in command_data:getInterfaceVlanIterator() do
            command_data:clearVlanStatus()
            command_data:clearLocalStatus(command_data)

            enableIpRoutingVlan(devNum, vlanId, false, params.mc_routing)

            command_data:updateStatus()

            command_data:updateVlans()
        end

        command_data:addWarningIfNoSuccessVlans(
            "No one vlan was affected.")
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
  return command_data:getCommandExecutionResults()
end

--------------------------------------------------------------------------------
-- command registration: ip routing for vlan
--------------------------------------------------------------------------------
CLI_addCommand("vlan_configuration", "ip routing", {
  func   = ip_routing_vlan,
  help   = "Configure ip routing for vlan"
})

CLI_addCommand("vlan_configuration", "ip mc-routing", {
  func   = function(params)
      params.mc_routing = true;
      ip_routing_vlan(params);
  end,
  help   = "Configure ip multicast routing for vlan"
})

--------------------------------------------------------------------------------
-- command registration: no ip routing for vlan
--------------------------------------------------------------------------------
CLI_addCommand("vlan_configuration", "no ip routing", {
  func   = no_ip_routing_vlan,
  help   = "Disable an IP routing for the VLAN"
})

CLI_addCommand("vlan_configuration", "no ip mc-routing", {
  func   = function(params)
      params.mc_routing = true;
      no_ip_routing_vlan(params);
  end,
  help   = "Disable an IP multicast routing for the VLAN"
})

