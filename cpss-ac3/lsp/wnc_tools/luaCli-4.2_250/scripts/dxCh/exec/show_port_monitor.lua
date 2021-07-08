--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_port_monitor.lua
--*
--* DESCRIPTION:
--*       showing of port monitoring status
--*
--* FILE REVISION NUMBER:
--*       $Revision: 8 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChMirrorRxPortsListGet")

--constants


-- ************************************************************************
---
--  show_port_monitor_func
--        @description  show's of port monitoring status
--
--        @return       true on success, otherwise false and error message
--
local function show_port_monitor_func(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum, portNum, analyzer_entry_index
    local command_data = Command_Data()
    -- Command specific variables declaration
    local dev_port_analyzer_entries, analyzer_dev_ports
    local mirror_analyzer_entry, mirror_analyzer_device, mirror_analyzer_port
    local mirror_analyzer_device_port_string
    local rx_analyzer_device, rx_analyzer_port, rx_analyzer_device_port_string    
    local tx_analyzer_device, tx_analyzer_port, tx_analyzer_device_port_string 
    local header_string, footer_string   
    
    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllAvailableDevicesRange()
    command_data:initDevicesPortCounts()
    
    -- Command specific variables initialization  
    dev_port_analyzer_entries   = command_data:getEmptyDeviceNestedTable()
    analyzer_dev_ports          = command_data:getEmptyDeviceNestedTable()  
    header_string = 
        "\n" ..
        " Mirrored port     Analyzer Port   \n" ..
        "                   RX        TX    \n" .. 
        "--------------  -------------------\n"
    footer_string = "\n"       
    
    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, 
                                                     footer_string)     
    
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus() 
        
            command_data:clearLocalStatus()    
    
            devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet","(devNum)",devNum)
               
            if  is_device_xCat_or_higher(devFamily)  then
                -- Mirror analyser entries cycle
                command_data:initGivenDeviceMirrorAnalyzerEntries(devNum)                
                if true == command_data["local_status"] then
                    local iterator
                    for iterator, devNum, analyzer_entry_index in 
                            command_data:getMirrorAnalyzerEntriesIterator() do
                        command_data:clearEntryStatus()            
                        
                        mirror_analyzer_entry               = 
                            command_data["mirror-analyzer-entry"]
                        
                        mirror_analyzer_device              = 
                            mirror_analyzer_entry["interface"]["devPort"]["devNum"]                    
                        mirror_analyzer_port                = 
                            mirror_analyzer_entry["interface"]["devPort"]["portNum"]
                                
                        mirror_analyzer_device_port_string  = 
                            tostring(mirror_analyzer_device) .. "/" .. 
                            tostring(mirror_analyzer_port)                        
                      
                        dev_port_analyzer_entries[devNum][analyzer_entry_index] = 
                                            mirror_analyzer_device_port_string
                        
                        command_data:updateEntries() 
                    end        
                end               
              
                -- Rx mirrored ports iterating cycle.
                command_data:initGivenDevicePortCounts(devNum)
                command_data:initMirroredPorts("rx")
                if true == command_data["local_status"] then
                    local iterator
                    for iterator, devNum, portNum, analyzer_entry_index in 
                                    command_data:getMirroredPortsIterator() do
                        command_data:clearPortStatus() 
                     
                        analyzer_dev_ports[devNum][portNum] = 
                            mergeTablesRightJoin(
                                analyzer_dev_ports[devNum][portNum], 
                                { ["rx"] = dev_port_analyzer_entries[devNum][analyzer_entry_index],
                                  ["tx"] = ""                                                      })
                        
                        command_data:updatePorts()
                    end
                end
                
                -- Tx mirrored ports iterating cycle.
                command_data:initMirroredPorts("tx")
                if true == command_data["local_status"] then
                    local iterator
                    for iterator, devNum, portNum, 
                                    analyzer_entry_index in 
                                    command_data:getMirroredPortsIterator() do
                        command_data:clearPortStatus() 
                 
                        analyzer_dev_ports[devNum][portNum] = 
                            mergeTablesRightJoin(
                                analyzer_dev_ports[devNum][portNum], 
                                { ["tx"] = dev_port_analyzer_entries[devNum][analyzer_entry_index] })
                        
                        analyzer_dev_ports[devNum][portNum] = 
                            mergeTablesRightJoin(
                                { ["rx"] = "" },
                                analyzer_dev_ports[devNum][portNum])
                                 
                                 
                        command_data:updatePorts()
                    end
                end
            else       
                -- The analyzer port getting of rx mirrored ports.
                if true == command_data["local_status"]             then
                    result, values = 
                        myGenWrapper("cpssDxChMirrorRxAnalyzerPortGet",
                                     {{ "IN",  "GT_U8",         "dev",  devNum }, 
                                      { "OUT", "GT_PORT_NUM",   "analyzerPort" },
                                      { "OUT", "GT_HW_DEV_NUM", "analyzerDev"  }}) 

                    if        0 == result then
                        rx_analyzer_device  = values["analyzerDev"]                    
                        rx_analyzer_port    = values["analyzerPort"]
                    elseif 0x10 == result then
                        command_data:setFailPortStatus() 
                        command_data:addWarning("It is not allowed to get the " .. 
                                                "analyzer port of rx mirrored " ..
                                                "ports on device %d.", devNum)   
                    elseif    0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at the analyzer port " ..
                                              "getting of rx mirrored ports on " ..
                                              "device %d: %s", devNum, 
                                              returnCodes[result])
                    end    
                end   

                -- The Rx analyzer port data converting.
                if true == command_data["local_status"]             then
                    result, values, rx_analyzer_port = 
                        device_port_from_hardware_format_convert(
                            rx_analyzer_device, rx_analyzer_port)
                    if 0 == result                                  then       
                        rx_analyzer_device  = values
                    elseif 0x10 == result                   then
                        command_data:setFailPortAndLocalStatus() 
                        command_data:addError("Rx device id %d and port " ..
                                              "number %d converting is not " ..
                                              "allowed.", devNum, portNum)
                    elseif    0 ~= result                   then
                        command_data:setFailPortAndLocalStatus()                 
                        command_data:addError("Error at Rx device id %d and " ..
                                              "port number %d converting: %s.", 
                                              devNum, portNum, values)                    
                    end
                end
                
                -- The Rx analyzer port string forming.
                if true == command_data["local_status"]             then
                            rx_analyzer_device_port_string     = 
                                            tostring(rx_analyzer_device) .. "/" .. 
                                            tostring(rx_analyzer_port) 
                    else
                            rx_analyzer_device_port_string     = "n/a"
                    end                

                -- The analyzer port getting of tx mirrored ports.     
                if true == command_data["local_status"]             then
                    result, values = 
                        myGenWrapper("cpssDxChMirrorTxAnalyzerPortGet",
                                     {{ "IN",  "GT_U8",         "dev", devNum  }, 
                                      { "OUT", "GT_PORT_NUM",   "analyzerPort" },
                                      { "OUT", "GT_HW_DEV_NUM", "analyzerDev"  }}) 

                    if        0 == result then
                        tx_analyzer_device  = values["analyzerDev"]
                        tx_analyzer_port    = values["analyzerPort"]
                    elseif 0x10 == result then
                        command_data:setFailPortStatus() 
                        command_data:addWarning("It is not allowed to get the " .. 
                                                    "analyzer port of tx mirrored " ..
                                                    "ports on device %d.", devNum) 
                    elseif    0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                            command_data:addError("Error at the analyzer port " ..
                                                  "getting of tx mirrored ports " ..
                                                  "on device %d: %s", devNum, 
                                                  returnCodes[result])
                    end   
                end                  

                -- The Tx analyzer port data converting.
                if true == command_data["local_status"]             then
                    result, values, tx_analyzer_port = 
                        device_port_from_hardware_format_convert(
                            tx_analyzer_device, tx_analyzer_port)
                    if 0 == result                                  then       
                        tx_analyzer_device  = values
                    elseif 0x10 == result                   then
                        command_data:setFailPortAndLocalStatus() 
                        command_data:addError("Rx device id %d and port " ..
                                              "number %d converting is not " ..
                                              "allowed.", devNum, portNum)
                    elseif    0 ~= result                   then
                        command_data:setFailPortAndLocalStatus()                 
                        command_data:addError("Error at Rx device id %d and " ..
                                              "port number %d converting: %s.", 
                                              devNum, portNum, values)                    
                    end
                end                
                
                -- The Tx analyzer port string forming.
                if true == command_data["local_status"]             then 
                            tx_analyzer_device_port_string     = 
                                            tostring(tx_analyzer_device) .. "/" .. 
                                            tostring(tx_analyzer_port) 
                    else
                            tx_analyzer_device_port_string     = "n/a"
                    end      

                    -- Analyzer port adding.
                    if true == command_data["local_status"]         then                
                        analyzer_dev_ports[devNum]["rx"] = 
                            rx_analyzer_device_port_string
                        analyzer_dev_ports[devNum]["tx"] = 
                            tx_analyzer_device_port_string
                    end                     
                end                  
                              
            command_data:updateStatus()
            
            command_data:updateDevices() 
        end
    end    

    -- Common variables initialization
    command_data:initDevicesPortCounts()
    command_data:initMirroredPorts()

    -- Main mirrored port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum, analyzer_entry_index in 
                                    command_data:getMirroredPortsIterator() do
            command_data:clearPortStatus() 
      
            if  is_device_xCat_or_higher(devFamily)  then
               
                -- Resulting string formatting and adding.
                command_data["result"] = 
                    string.format(
                        "%-16s%-9s %-9s", 
                        alignLeftToCenterStr(devNum .. "/" .. portNum, 14),
                        alignLeftToCenterStr(analyzer_dev_ports[devNum][portNum]["rx"], 
                                             9),
                        alignLeftToCenterStr(analyzer_dev_ports[devNum][portNum]["tx"], 
                                             9))  
                command_data:addResultToResultArray()
            else
                -- Resulting string formatting and adding.
                command_data["result"] = 
                    string.format(
                        "%-16s%-9s %-9s", 
                        alignLeftToCenterStr(devNum .. "/" .. portNum, 14),
                        alignLeftToCenterStr(analyzer_dev_ports[devNum]["rx"], 
                                             9),
                        alignLeftToCenterStr(analyzer_dev_ports[devNum]["tx"], 
                                             9))  
                command_data:addResultToResultArray()            
            end
           
            command_data:updatePorts() 
        end
    end
   
    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnPortCount(header_string, command_data["result"], 
                                         footer_string,
                                         "There is no port monitor information to show.\n")    
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: show port monitor
--------------------------------------------    
CLI_addHelp("exec", "show port", "Show port configuration") 
CLI_addCommand("exec", "show port monitor", {
  func   = show_port_monitor_func,
  help   = "Show monitor port configuration"
})
