--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_multicast_ipv4_mode.lua
--*
--* DESCRIPTION:
--*       configuring of the multicast bridging mode for ipv4 multicast packets
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  bridge_multicast_ipv_mode_func
--        @description  configures the multicast bridging mode for ipv4/
--                      ipv6 multicast packets
--
--        @param params             - params[devId]: device number, 
--                                    params["multicast_group"]: multicast 
--                                    group mode
--
--        @usage common_global      - common_global[ifRange]: dev/vlan  
--                                    range
--
--        @return       true on success, otherwise false and error message
--
function bridge_multicast_ipv_mode_func(params)
    -- Common variables declaration
    local result, values
    local devNum, vlanId
    local command_data = Command_Data()
    -- Command specific variables declaration
    local ipVer, ipmMode, multicast_bridging_enabling
    
    -- Common variables initialization    
    command_data:initInterfaceDevVlanRange()

    -- Command specific variables initialization
    ipVer = valueIfConditionNil(params["ipv6"], "CPSS_IP_PROTOCOL_IPV4_E", 
                                                "CPSS_IP_PROTOCOL_IPV6_E")
    if    (nil ~= params["flagNo"])         or 
          (nil ~= params["mac-group"])      then
        multicast_bridging_enabling = false
    elseif nil ~= params["ip-group"]        then
        multicast_bridging_enabling = true
        ipmMode     = "CPSS_BRG_IPM_GV_E"   
    elseif nil ~= params["ip-src-group"]    then
        multicast_bridging_enabling = true
        ipmMode     = "CPSS_BRG_IPM_SGV_E"   
    end

    -- Main vlan handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, vlanId in 
                                    command_data:getInterfaceVlanIterator() do
            command_data:clearVlanStatus()
            
            -- IPv4 multicast bridging enabling.
            command_data:clearLocalStatus()
         
            if true == command_data["local_status"]         then
                result, values = 
                    myGenWrapper("cpssDxChBrgVlanIpmBridgingEnable", 
                                 {{ "IN", "GT_U8",      "devNum",   devNum}, 
							      { "IN", "GT_U16",     "vlanId",   vlanId},
							      { "IN", "CPSS_IP_PROTOCOL_STACK_ENT", 
                                                        "ipVer",    ipVer},
							      { "IN", "GT_BOOL", 
                                                        "ipmMode",  
                                                 multicast_bridging_enabling}})
                if     0x10 == result   then
                    command_data:setFailVidxStatus() 
                    command_data:addWarning("It is not allowed to enable IPv4 " ..
                                            "multicast bridging on vlan %d " ..
                                            "device %d.", vlanId, devNum) 
                elseif    0 ~= result   then
                    command_data:setFailVidxAndLocalStatus()
                    command_data:addError("Error at enabling of IPv4 multicast " ..
                                          "bridging  on vlan %d device %d: %s.", 
                                          vlanId, devNum, returnCodes[result])
                end         
            end              

            -- Setting of the Vlan IPM bridging mode.
            if (true == command_data["local_status"])       and
               (true == multicast_bridging_enabling)        then
                result, values = 
                    myGenWrapper("cpssDxChBrgVlanIpmBridgingModeSet", 
                                 {{ "IN", "GT_U8",      "devNum",   devNum}, 
							      { "IN", "GT_U16",     "vlanId",   vlanId},
							      { "IN", "CPSS_IP_PROTOCOL_STACK_ENT", 
                                                        "ipVer",    ipVer},
							      { "IN", "CPSS_BRG_IPM_MODE_ENT", 
                                                        "ipmMode",  ipmMode}})                                
                if     0x10 == result   then
                    command_data:setFailVidxStatus() 
                    command_data:addWarning("It is not allowed to set IPM " ..
                                            "bridging mode on vlan %d device %d.", 
                                            vlanId, devNum)                                                   
                elseif    0 ~= result   then
                    command_data:setFailVidxAndLocalStatus()
                    command_data:addError("Error at setting of IPM bridging " ..
                                          "mode on vlan %d device %d: %s.", 
                                          vlanId, devNum, returnCodes[result])
                end         
            end   
            
            command_data:updateStatus()
            
            command_data:updateVlans()
        end

        command_data:addWarningIfNoSuccessVlans(
            "Can not configure all processed vlans.")
    end
    
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()            
end


-------------------------------------------------
-- command registration: mac address-table static
-------------------------------------------------
CLI_addCommand("vlan_configuration", "bridge multicast ipv4 mode", {
  func   = bridge_multicast_ipv_mode_func,
  help   = "Configure IPv4 multicast bridging mode",
  params = {
      { type="named",
          { format="mac-group", 
                        help = "Multicast bridging is based on the packet's " ..
                               "VLAN and MAC address"                         },
          { format="ip-group",  
                        help = "Multicast bridging is based on the packet's " ..
                               "VLAN and the IPv6 destination address for " ..
                               "IPv6 packets"                                 },
          { format="ip-src-group", 
                        help = "Multicast bridging is based on the packet's " ..
                               "VLAN, IPv6 destination address and IPv6 " ..
                               "source address for IPv6 packets"              },
        alt       = { multicast_group = 
                        { "mac-group", "ip-group", "ip-src-group" }           }, 
        mandatory = { "multicast_group" }
    }
  }
})
CLI_addCommand("vlan_configuration", "no bridge multicast ipv4 mode", {
  func   = function(params)
               params.flagNo = true
               return bridge_multicast_ipv_mode_func(params)
           end,
  help   = "Setting to default of the multicast bridging mode for ipv4 " ..
           "multicast packets"
})
