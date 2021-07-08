--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* flow_control_periodic.lua
--*
--* DESCRIPTION:
--*       enabling transmission of periodic fc messages
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes


-- ************************************************************************
---
--  flow_control_periodic_func
--        @description  enables/disables transmission of periodic fc 
--                      messages
--
--        @usage __global       - __global["ifRange"]: iterface range 
--
--        @return       true on success, otherwise false and error message
--
local function flow_control_periodic_func(params)
    local result, values
    local status, err
    local full_duplex_ports_count 
    local devFamily, devNum, portNum, periodic_flow_control_type 
    local command_data = Command_Data()   
    local all_ports
    local interval, speed, fc_intervals, periodic_flow_control_type_string
    
    -- Common variables initialization.    
    command_data:initAllDeviceRange(params) 
    
    full_duplex_ports_count = 0
    
	devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
	if ("CPSS_PP_FAMILY_DXCH_LION_E" == devFamily) then
        command_data:setFailDeviceStatus() 
        command_data:addWarning("Family of device %d does not " ..
                                 "supported.", devNum) 
	end
	
    all_ports = getGlobal("ifRange")	

    if nil == params.flagNo then
        periodic_flow_control_type_string = params.periodic_fc_type
		if "xon_xoff" == periodic_flow_control_type_string then
			periodic_flow_control_type = "CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E"
		elseif "xon_only" == periodic_flow_control_type_string then
			periodic_flow_control_type = "CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E"
		elseif "xoff_only" == periodic_flow_control_type_string then
			periodic_flow_control_type = "CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E"
		else 	
			periodic_flow_control_type = "CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E"
		end
    else
        periodic_flow_control_type = "CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E"
    end
    for devNum, portNums in pairs(all_ports) do
        for portIndex = 1, #portNums do    
            portNum = portNums[portIndex]    
            
            result, values = myGenWrapper("cpssDxChPortPeriodicFcEnableSet",
                                          {{ "IN", "GT_U8", "devNum", devNum},
                                           { "IN", "GT_U8", "portNum", portNum},
                                           { "IN", "CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT", "enable", periodic_flow_control_type}})																					  
            if 0 ~= result then 
                command_data:setFailDeviceAndLocalStatus()
                command_data:addError("Error at enabling transmits of periodic " ..
                                      "flow control on device %d: %s.", devNum, 
                                      returnCodes[result])
            end  
			
			if "CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E" ~= periodic_flow_control_type then
				result, values = myGenWrapper("cpssDxChPortSpeedGet",
												{{ "IN", "GT_U8"  , "devNum", devNum},								 -- devNum
												 { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", portNum},	 		     -- portNum
												 { "OUT","CPSS_PORT_SPEED_ENT" , "speedPtr"}})      				 -- data
				if 0 ~= result then 
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at getting port speed " ..
                                          " on device %d: %s.", devNum, 
                                          returnCodes[result])
				end  
                
				speed = values.speedPtr;
				if "CPSS_PORT_SPEED_NA" ~= speed then	
					fc_intervals = fc_intervals_get()
					interval = fc_intervals[speed]
					if nil ~= interval then
					    result, values = myGenWrapper("cpssDxChPortPeriodicFlowControlCounterSet",
					                                    {{ "IN", "GT_U8", "devNum", devNum},                                 -- devNum
					                                     { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", portNum},	 		     -- portNum
					                                     { "IN", "GT_U32", "value", interval}})                              -- interval
					    if 0 ~= result then 
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error at setting periodic " ..
                                                  "Flow Control interval on device %d: %s.", devNum, 
                                                  returnCodes[result])
					    end  
					end
				end
			end
        end      
    end
        
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
           
    return command_data:getCommandExecutionResults()
end

--------------------------------------------------
-- command registration: flow-control periodic
--------------------------------------------------

CLI_addCommand("interface", "flow-control periodic", {
  func=flow_control_periodic_func,
  help="Enabling transmission of periodic fc messages",
  params={
    { type="named",
      { format="xon_xoff",   name="periodic_fc_type", help="XON and XOFF enabled"},
      { format="xon_only",   name="periodic_fc_type", help="XON ONLY enabled"},
      { format="xoff_only",  name="periodic_fc_type", help="XOFF ONLY enabled"}
    }
  }
})

--------------------------------------------------
-- command registration: no flow-control periodic
--------------------------------------------------
CLI_addCommand("interface", "no flow-control periodic", {
  func=function(params)
      params.flagNo=true
      return flow_control_periodic_func(params)
  end,
  help="Disabling transmission of periodic fc messages"
})