--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ip_route.lua
--*
--* DESCRIPTION:
--*       showing of the global status of IP routing and the configured IP Routes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 8 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_ip_route_func
--        @description  show's the global status of IP routing and the 
--                      configured IP Routes
--
--        @param params             - params["all_device"]: all or given 
--                                    devices iterating property, could be 
--                                    irrelevant
--
--        @return       true on success, otherwise false and error message
-- 
function show_ip_route_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local enable_routing, enable_routing_string
    local routing_enabling_first_string_prefix 
    local routing_enabling_second_string_prefix
    local routing_enabling_third_string_prefix, routing_enabling_string_suffix
    local text_block_header_string, text_block_footer_string
    local ip_protocol, prefix_ip_address, prefix_length, vrId
    local prefix_string, prefix_output_string, prefix_showing, vrId_string
    local next_hop_id, next_hop_id_string, next_hop_id_output_string
    local next_hop_entry, next_hop_entry_showing, next_hop_entries
    local vlanId, vlanId_string
    local entry_interface, entry_interface_string
    local router_arp_index, mac_address, mac_address_string
    local table_header_string, table_footer_string
    local configured_ip_routs_table_string

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceRange(params)
    command_data:initIpPrefixEntries(params)
    
    -- Command specific variables initialization 
    text_block_header_string                = ""
    routing_enabling_first_string_prefix    = "ip routing"
    routing_enabling_third_string_prefix    = ":\t"
    text_block_footer_string                = "\n\n"
    ip_protocol                             = params["ip_protocol"] 
    is_ipv4_protocol                        = 
        isEquivalent(ip_protocol, "CPSS_IP_PROTOCOL_IPV4_E")
    routing_enabling_string_suffix          = ""
    table_header_string                     = 
        valueIfConditionTrue(
            is_ipv4_protocol,
            " vrId     prefix/mask    next-hop   vid     interface           mac        \n" ..
            "                            ID                                             \n" ..
            "-----  ----------------  --------  -----  -------------  ------------------\n",
            " vrId                   prefix/mask                  next-hop   vid     interface              mac          \n" .. -- IPv6 TO ADD
            "                                                        ID                                                  \n" ..
            "------  -------------------------------------------  --------  -----  --------------  ----------------------\n")            
    table_footer_string                       = "\n"  
    
    -- System specific data initialization.
    command_data:enablePausedPrinting()    
    command_data:setEmergencyPrintingHeaderAndFooter(table_header_string, 
                                                     table_footer_string)    
    
    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in  command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()  
                    
            command_data:clearLocalStatus()                    

            -- Routing enabling getting and its string forming.
            if true  == command_data["local_status"] then                        
                result, values = 
                    cpssPerDeviceParamGet("cpssDxChIpRoutingEnableGet", devNum, 
                                          "enableRouting", "GT_BOOL")
                                 
                if        0 == result then
                    enable_routing  = values["enableRouting"]
                elseif 0x10 == result then
                    command_data:setFailDeviceStatus() 
                    command_data:addWarning("Routing enabling getting is " ..
                                            "not allowed on device %d.", devNum)                     
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at routing enabling getting " ..
                                          "on device %d: %s.", 
                                          devNum, returnCodes[result])
                end  
                
                if        0 == result then
                    enable_routing_string   = boolEnabledLowerStrGet(enable_routing)
                else
                    enable_routing_string   = "n/a"
                end
            end
            
            -- Routing enabling second prefix forming.
            if true  == command_data["local_status"] then 
                    routing_enabling_second_string_prefix = 
                        stringValueIfConditionTrue(multiply_device_presence, 
                                                   " on device " .. 
                                                   tostring(devNum))
            end
            
            -- Routing enabling string formatting.
            command_data:setResultStr(routing_enabling_first_string_prefix, 
                                      routing_enabling_second_string_prefix, 
                                      routing_enabling_third_string_prefix)            
            command_data:setResultStr(command_data["result"],
                                      enable_routing_string,
                                      routing_enabling_string_suffix)            
            command_data:addResultToResultArray() 
            
            command_data:updateStatus()
                        
            command_data:updateDevices()            
        end
    end                     

    -- Resulting routing enabling string forming.
    enable_routing_string       = command_data:getResultArrayStr()
    enable_routing_string       =
        command_data:getStrOnDevicesCount(text_block_header_string, 
                                          enable_routing_string, 
                                          text_block_footer_string,
                                          "There is no routing enabling information to show.\n")     
    
    -- Common variables initialization
    command_data:clearResultArray()    
    command_data:addToEmergencyPrintingHeader(enable_routing_string)
    
    -- Main ip prefix handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, prefix_ip_address, prefix_length, vrId in 
                                        command_data:getIpPrefixIterator() do
            command_data:clearEntryStatus()

            command_data:clearLocalStatus()

            -- Prefix and next-hop id string forming.
            if true == command_data["local_status"]     then 
                prefix_string       = ipPrefixStrGet(ip_protocol, 
                                                     prefix_ip_address, 
                                               prefix_length)
                next_hop_id         = command_data["ip-unicast-entry-index"]
                next_hop_id_string  = tostring(next_hop_id)  
				vrId_string = tostring(vrId)
            end    
     
            -- Device handling cycle
            next_hop_entries    = {}
            if true == command_data["local_status"] then
                local device_iterator
                for device_iterator, devNum in 
                                            command_data:getDevicesIterator() do
                    command_data:clearDeviceStatus()  
                    
                    command_data:clearLocalStatus()                    
            
                    -- Next-hop entry vlan id and its interface getting.
                    if true  == command_data["local_status"] then 
                        result, values, next_hop_entry =wrLogWrapper("wrlCpssDxChNextIpUcRouteEntryGet", "(devNum, next_hop_id)", devNum, next_hop_id)
                        if        0 == result then
                            if next_hop_entry["type"] == "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E" then
                                vlanId              = 
                                    next_hop_entry["entry"]["regularEntry"]["nextHopVlanId"]
                                entry_interface     = 
                                    next_hop_entry["entry"]["regularEntry"]["nextHopInterface"]
                                router_arp_index    =
                                    next_hop_entry["entry"]["regularEntry"]["nextHopARPPointer"]
                            else
                                command_data:addError("next-hop entry %s is %s",
                                                    tostring(next_hop_entry["type"]),
                                                    to_string(next_hop_id))
                            end
                        elseif 0x10 == result then
                            command_data:setFailDeviceStatus() 
                            command_data:addWarning("Next-hop entry getting " ..
                                                    "is not  allowed on " ..
                                                    "device %d.", devNum)                     
                        elseif    0 ~= result then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error at next-hop entry " ..
                                                  "getting on device %d: %s.", 
                                                  devNum, returnCodes[result])
                        end
                    end

                    -- Next-hop entry vlan id and its interface string forming.
                    if true  == command_data["local_status"] then 
                        vlanId_string           = 
                            tostring(vlanId)
                        entry_interface_string  = 
                            interfaceInfoStrGet(entry_interface)
                    end
                       
                    -- Mac-address getting.
                    if true  == command_data["local_status"] then                        
                        result, values = 
                            myGenWrapper("cpssDxChIpRouterArpAddrRead", 
                                         {{ "IN",  "GT_U8",        "devNum", devNum }, 
                                          { "IN",  "GT_U32",       "routerArpIndex", 
                                                                   router_arp_index },
                                          { "OUT", "GT_ETHERADDR", "arpMacAddr"     }}) 
                        if        0 == result then
                            mac_address         = values["arpMacAddr"]
                        elseif 0x10 == result then
                            command_data:setFailDeviceStatus() 
                            command_data:addWarning("Arp mac-address getting " ..
                                                    "is not allowed on " ..
                                                    "device %d.", devNum)                     
                        elseif    0 ~= result then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error at arp mac-address " ..
                                                  "getting on device %d: %s.", 
                                                  devNum, returnCodes[result])
                        end                                          
                    end

                    -- Mac-address string forming.
                    if true  == command_data["local_status"] then 
                        mac_address_string = replaceStr(tostring(mac_address),
                                                        ":", "-")
                    end
                    
                    -- Detecting of output properties and storing output data.
                    if true  == command_data["local_status"] then 
                        next_hop_entry = 
                            {vlanId, entry_interface_string, mac_address_string}
                        next_hop_entry_showing = 
                            isItemNotInTable(next_hop_entry, next_hop_entries)
                        prefix_showing = isEmptyTable(next_hop_entries)
                        next_hop_entries[devNum] = next_hop_entry
                    end
                    
                    -- Prefix output data updating.
                    if true  == command_data["local_status"] then 
                        prefix_output_string        = 
                            stringValueIfConditionTrue(prefix_showing, prefix_string)
                        next_hop_id_output_string   = 
                            stringValueIfConditionTrue(prefix_showing, next_hop_id_string)
                    end
                    
                    command_data:updateStatus()
                       
                    -- Resulting string formatting and adding.
                    command_data["result"] = 
                        valueIfConditionTrue(
                            is_ipv4_protocol,
                            string.format(
                                "  %-4s%-18s%-10s%-7s%-14s%-24s",
								alignLeftToCenterStr(vrId_string, 2),
                                alignLeftToCenterStr(prefix_output_string, 18),
                                alignLeftToCenterStr(next_hop_id_output_string, 8),
                                alignLeftToCenterStr(vlanId_string, 5),
                                alignLeftToCenterStr(entry_interface_string, 14),
                                alignLeftToCenterStr(mac_address_string, 22)),
                            string.format(
                                "%-4s%-45s%-10s%-7s%-16s%-24s", 
								alignLeftToCenterStr(vrId_string, 2),
                                alignLeftToCenterStr(prefix_output_string, 43),
                                alignLeftToCenterStr(next_hop_id_output_string, 8),
                                alignLeftToCenterStr(vlanId_string, 5),
                                alignLeftToCenterStr(entry_interface_string, 14),
                                alignLeftToCenterStr(mac_address_string, 22)
								))                            
                    command_data:addResultToResultArrayOnCondition(
                        next_hop_entry_showing)                          
                        
                    command_data:updateDevices()            
                end
            end        
     
            command_data:updateStatus()            
            
            command_data:updateEntries()
        end
    end
  
    -- Resulting configured ip routs string forming.
    configured_ip_routs_table_string    = command_data["result_array"]
    configured_ip_routs_table_string    =
        command_data:getStrOnEntriesCount(table_header_string, 
                                          configured_ip_routs_table_string, 
                                          table_footer_string,
                                          "There is no configured ip routs to show.\n")  
   
    -- Resulting table string formatting
    command_data:setResultStr(enable_routing_string, 
                              configured_ip_routs_table_string)
   
    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()             
end


--------------------------------------------------------------------------------
-- command registration: show ip route 
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show ip", "IP Information")
CLI_addCommand("exec", "show ip route", {
  func   = function(params)
               params.ip_protocol = "CPSS_IP_PROTOCOL_IPV4_E"
               return show_ip_route_func(params)
           end,
  help   = "Current state of the ipv6 routing table",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})
