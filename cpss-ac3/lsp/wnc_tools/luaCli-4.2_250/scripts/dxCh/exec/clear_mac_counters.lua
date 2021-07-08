--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* clear_mac_counters.lua
--*
--* DESCRIPTION:
--*       clearing of statistics on an interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  clear_mac_counters_func
--        @description  clears statistics on an interface
--
--        @param params         - params["all"]: all devices port or all 
--                                ports of given device cheking switch, 
--                                could be irrelevant;
--                                params["devID"]: checked device number, 
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil;
--                                params["port-channel"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil
--
--        @return       true on success, otherwise false and error message
--
function clear_mac_counters_func(params, printFlag)
    -- Common variables declaration
    local result, values
    local devNum, portNum
	local clerOnReadStatus
    local command_data = Command_Data()
    
    -- Common variables initialization
    command_data:initAllInterfacesPortIterator(params)
    
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        local port_skiped = false -- the port should be skiped

        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus() 
        
            -- Port mac counters clear property setting.
            command_data:clearLocalStatus()
  
            if true == command_data["local_status"] then
				result, values= myGenWrapper("cpssDxChPortMacCountersClearOnReadGet",{	
				{"IN","GT_U8","devNum",devNum},
				{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
				{"OUT","GT_BOOL","enablePtr"}
				})		
				
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to enable  " ..
                                            "port mac counters clearing for " ..
                                            "device %d port %d.", 
                                            boolPredicatStrGet(back_pressure), 
                                            devNum, portNum)                                                   
                elseif 4 == result and portNum == 63 then
                    -- skip CPU port
                    port_skiped = true
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port mac counters clear " ..
                                          "property setting of device %d " ..
                                          "port %d: %s", devNum, portNum, 
                                          returnCodes[result])
				else	
					clerOnReadStatus=values["enablePtr"]
                end 
            end    
        
            command_data:updateStatus() 

            if false == port_skiped and false == clerOnReadStatus and true == command_data["local_status"] then	--enable clear on read only if was disable
                result, values = 
                    cpssPerPortParamSet("cpssDxChPortMacCountersClearOnReadSet",
                                        devNum, portNum, true, "enable", 
                                        "GT_BOOL")   
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to enable  " ..
                                            "port mac counters clearing for " ..
                                            "device %d port %d.", 
                                            boolPredicatStrGet(back_pressure), 
                                            devNum, portNum)                                                   
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port mac counters clear " ..
                                          "property setting of device %d " ..
                                          "port %d: %s", devNum, portNum, 
                                          returnCodes[result])
                end 
            end    
        
            command_data:updateStatus() 
            
            -- Traffic statistic getting.    
            if false == port_skiped and true  == command_data["local_status"] then 
                result, values = 
                    cpssPerPortParamGet("cpssDxChPortMacCountersOnPortGet",
                                                     devNum, portNum, 
                                                     "portMacCounterSetArray", 
                                                     "CPSS_PORT_MAC_COUNTER_SET_STC")
                if     0x10 == result then
                    command_data:addWarning("Traffic statistic getting is " ..
                                            "not allowed")
                elseif 0 ~= result then
                    command_data:setFailLocalStatus()
                    command_data:addError("Error at traffic statistic getting")
                end         
            end
            
            command_data:updateStatus()
                       
            -- Port mac counters clear property resetting.
            command_data:clearLocalStatus()
            
            if false == port_skiped and false == clerOnReadStatus and true == command_data["local_status"] then
                result, values = 
                    cpssPerPortParamSet("cpssDxChPortMacCountersClearOnReadSet",
                                        devNum, portNum, false, "enable", 
                                        "GT_BOOL")   
                if     0x10 == result then
                    command_data:setFailPortStatus() 
                    command_data:addWarning("It is not allowed to disable " ..
                                            "port mac counters clearing for " ..
                                            "device %d port %d.", 
                                            boolPredicatStrGet(back_pressure), 
                                            devNum, portNum)                                                   
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port mac counters clear " ..
                                          "property resetting of device %d " ..
                                          "port %d: %s", devNum, portNum, 
                                          returnCodes[result])
                end 
            end    
        
            command_data:updateStatus() 
            
            command_data:updatePorts() 
        end     

        command_data:addWarningIfNoSuccessPorts("Can not clear statistic " ..
                                                "for all processed ports.")
    end
    
    command_data:analyzeCommandExecution()
     
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()       
end


--------------------------------------------
-- command registration: clear mac counters
--------------------------------------------
CLI_addCommand("exec", "clear mac counters", {
    func = clear_mac_counters_func,
    help = "Clearing of statistics on an interface",
    params = {
        { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
        }
    }  
})

