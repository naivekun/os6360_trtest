--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_serdes_temperature.lua
--*
--* DESCRIPTION:
--*       display the Serdes Temperature
--*
--*
--********************************************************************************

local function Ret_Table_Update(Ret_Table, devNum, portNum, temperature)

	--local tuple    = {}
 
    if (Ret_Table ~= nil ) then
    	Ret_Table[devNum]                      = tableInsert(Ret_Table[devNum],nil)
    	Ret_Table[devNum][portNum]             = tableInsert(Ret_Table[devNum][portNum], nil)
    	Ret_Table[devNum][portNum].temperature = temperature
        	
    end
    return Ret_Table

end

-- ************************************************************************
---
--  show_interfaces_serdes_temperature
--        @description  show interfaces Serdes Temperature
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
function show_interfaces_serdes_temperature(params) 
    -- Common variables declaration

    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
	local GT_OK = 0
    local header_string1, footer_string1
	local ret, values
	local ifMode
	local startSerdesPtr
	local numOfSerdesLanesPtr
	local laneNum
	local laneNumS


    local result, values
    local command_data = Command_Data() 
    local Ret_Table = {}


    --print("params  = "..to_string(params))

    command_data:initAllInterfacesPortIterator(params)   
    command_data:clearResultArray()
     
    if params.all ~= nil then
        local dev, ports
        for dev,ports in pairs(command_data.dev_port_range) do
            table.insert(ports,"CPU")
        end
    end
    
    --result, values = show_interfaces_serdes_temperature_port_range(params, command_data)
    -- Command specific variables initialization.   
	header_string1 = 
				"\n" ..
                "Interface Temperature\n" ..
				"--------- --------\n"
	footer_string1 = ""  

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, 
                                                     footer_string) 

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do

			if portNum == "CPU" then portNum = 63 end
            -- DevNum/PortNum string forming.
            devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) .. 
                                                         '/' .. 
                                                         tostring(portNum), 9)

		
			ret,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)


			if( ret == GT_OK and numOfSerdesLanesPtr > 0) then

                command_data:clearPortStatus()     
                command_data:clearLocalStatus()

				ret, values = myGenWrapper("cpssDxChPortDiagTemperatureGet",{
					{"IN","GT_U8","devNum",devNum},
					{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
					{"OUT","GT_U32","temperaturePtr"}}) 
					
				if(ret ~= GT_OK) then
                    --command_data:setFailPortAndLocalStatus()
                    --command_data:addError("Error cpssDxChPortDiagTemperatureGet: device %d " ..
                    --                      "port %d: %s", devNum, portNum,
                    --                      returnCodes[ret])

                    print(devnum_portnum_string.." Error cpssDxChPortDiagTemperatureGet ret: "..to_string(ret).." "..returnCodes[ret])
				else
                    command_data:updateStatus()

					temperaturePtr= to_string(values.temperaturePtr)
    
                    command_data["result"] = 
                        string.format("%-10s%-8s", 
                                      devnum_portnum_string, 
                                      alignLeftToCenterStr(temperaturePtr, 7))
    				command_data:addResultToResultArray()
    				command_data:updatePorts()

                    Ret_Table = Ret_Table_Update(Ret_Table, devNum, portNum,values.temperaturePtr)

				end


		   end
		end

		command_data:setResultArrayToResultStr(1, 1)
		command_data:setResultStrOnPortCount(header_string1, command_data["result"], 
											footer_string1,
											"There is no port information to show.\n")
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        --return command_data:getCommandExecutionResults()
	
	end

    --print("Ret_Table  = "..to_string(Ret_Table))

    return command_data:getCommandExecutionResults(),Ret_Table

end

--------------------------------------------------------------------------------
-- command registration: show interfaces Serdes Temperature
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show interfaces serdes temperature", {
    func = show_interfaces_serdes_temperature,
    help = "Interface(s) serdes temperature",
    params = {
		{	type= "named", 	
							"#all_interfaces",	   
							mandatory={"all_interfaces"}
		}
	}	
})
    
