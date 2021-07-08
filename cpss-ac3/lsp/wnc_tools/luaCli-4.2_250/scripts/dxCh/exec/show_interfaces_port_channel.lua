--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_port_channel .lua
--*
--* DESCRIPTION:
--*       showing of port-channel information for all port channels or for a 
---*      specific port channel 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 10 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  show_interfaces_port_channel_func_one_trunk
--        @description  shows of port-channel information for all port 
--                      channels or for a specific port channel 
--
--        @param params         - command params, does not used
--        @param  command_data  - command execution data object 
--        @param trunkID        - port-channel number
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_port_channel_func_one_trunk(params, command_data, trunkID)
    -- Common variables declaration
    local result, values
    -- Command  specific variables declaration   
    local trunkId, trunk_dev_ports, trunk_dev_ports_str
    local header_string, footer_string
    
    -- Command specific variables initialization.  
    trunkId       = trunkID
    header_string =
        "\n" ..
        "Channel          Ports         \n".. 
        "-------   ---------------------\n"
    footer_string = "\n"  
               
    -- Trunk dev/port list getting.
    if true == command_data["status"] then   
        result, values = get_trunk_device_port_list(trunkId)
        if     0 == result then
            trunk_dev_ports = values
        elseif 0x10 == result then
            command_data:setFailStatus()
            command_data:addWarning("Getting of %d trunk information is not " ..
                                    "allowed: %s.", trunkId, values)            
        elseif 0 ~= result then
            command_data:setFailStatus()
            command_data:addError("Error at getting of %d trunk information: %s.",
                                  trunkId, values)
        end     

        if 0 == result then       
            trunk_dev_ports_str = tblToSortedKeyValueStr(trunk_dev_ports, ", ")
        else
            trunk_dev_ports_str = "n/a"
        end             
    end
    
    command_data["result"] = 
        string.format("  %-8s %-21s", trunkId, trunk_dev_ports_str) 
    command_data:setResultStr(header_string, command_data["result"], 
                              footer_string)   
    
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()  
end


-- ************************************************************************
---
--  show_interfaces_port_channel_func_all_trunks
--        @description  shows of port-channel information for all port 
--                      channels or for an all port channels 
--
--        @param params         - not used
--        @param command_data   - command execution data object 
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_port_channel_func_all_trunks(params, command_data)
    -- Common variables declaration
    local result, values
    -- Command  specific variables declaration    
    local trunkId, trunk_dev_ports    
    local header_string, footer_string
    
    -- Command specific variables initialization.  
    header_string =
        "Channel          Ports         \n".. 
        "-------   ---------------------\n"
    footer_string = "\n"  
    
    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, 
                                                     footer_string)     
    
    if true == command_data["status"] then
        local iterator
        for iterator, trunkId, trunk_dev_ports in 
                                    command_data:getTrunkDevPortsIterator() do           
            command_data:clearTrunkStatus()
            
            command_data:clearLocalStatus()
            
            -- Resulting string formatting and adding.
            command_data["result"] = 
                string.format("  %-8s %-21s", 
                              trunkId, 
                              tblToKeyValueStr(trunk_dev_ports, ", "))
            command_data:addResultToResultArray()            

            command_data:updateStatus()            
           
            command_data:updateTrunks()
        end
    end    
    
    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()    
    command_data:setResultStrOnTrunksCount(header_string, command_data["result"], 
                                           footer_string,
                                           "There is no trunk information to show")    
    
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()    
end


-- ************************************************************************
---
--  show_interfaces_port_channel_func
--        @description  shows of port-channel information for all port 
--                      channels or for a specific port channel or for an 
--                      all port channels 
--
--        @param params         - params["trunkID"]: trunk, could be 
--                                irrelevant
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_port_channel_func(params) 
    -- Common variables declaration
    local command_data = Command_Data() 
    local trunkID
    local result, values
    -- Common variables initialization
    command_data:clearResultArray()   
    
    -- Command specific variables initialization.  
    trunkID = params["port_channel"]
    
    if true == isNil(trunkID)   then
        result, values = 
        show_interfaces_port_channel_func_all_trunks(params, command_data)
    else
        result, values = 
            show_interfaces_port_channel_func_one_trunk(params, command_data, 
                                                        trunkID)
    end
    
    return result, values
end


-------------------------------------------------------
-- command registration: show interfaces port-channel
-------------------------------------------------------
CLI_addCommand("exec", "show interfaces port-channel", {
  func=show_interfaces_port_channel_func,
  help   = "Port-channel member ports information",
  params={
      { type = "named",    
          "#port_channel_or_nil"
    }
  }
})