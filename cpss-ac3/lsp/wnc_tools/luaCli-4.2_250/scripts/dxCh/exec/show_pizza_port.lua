--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_pizza_port.lua
--*
--* DESCRIPTION:
--*       display the pizza port dump
--*
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssDxChPortPhysicalPortMapShadowDBGet")

-- ************************************************************************
---
--  port_validation
--        @description  port validation
--
--		  @params		dev 	- device number
--						portNum - port number
--
--        @return       true if port is valid, otherwise false
--
local function port_validation(dev, portNum)
	result, values = myGenWrapper(
		"cpssDxChPortPhysicalPortMapIsValidGet", {
			{ "IN", "GT_U8"  , "dev", dev},									 -- devNum
			{ "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},			 -- portNum
			{ "OUT","GT_BOOL" , "isValidPtr"}      							 -- isValidPtr
		}
	)
	if (result ~= 0) then
		print("Error at show pizza port: port_validation")
	end
	if(values["isValidPtr"] == false) then
		return false
	else
		return true
	end
end


-- ************************************************************************
---
--  port_initialized
--        @description  port initialized
--
--		  @params		dev 	- device number
--						portNum - port number
--
--        @return       speed and mode if port is initialized, otherwise false
--
local function port_initialized(dev, portNum)
	result, values = myGenWrapper(
			"cpssDxChPortSpeedGet", {
				{ "IN", "GT_U8"  , "dev", dev},									 -- devNum
				{ "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", portNum},	 		 -- portNum
				{ "OUT","CPSS_PORT_SPEED_ENT" , "speedPtr"}      				 -- data
			}
		)
		if (result ~= 0) then
			print("Error at show pizza port: port_initialized: PortSpeedGet")
		end
		portSpeed = string.sub(values["speedPtr"],17,-3)

		result, values = myGenWrapper(
			"cpssDxChPortInterfaceModeGet", {
				{ "IN", "GT_U8"  , "dev", dev},						  	-- devNum
				{ "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", portNum}, 	-- portNum
				{ "OUT","CPSS_PORT_INTERFACE_MODE_ENT" , "ifModePtr"} 	-- data
			}
		)
		if (result ~= 0) then
			print("Error at show pizza port: port_initialized: Port_Interface_Mode_Get")
		end
		portMode = string.sub(values["ifModePtr"],26,-3)
		if(portSpeed == "NA" or portMode == "NA") then
			return false
		else
			return true, portSpeed, portMode
		end

end




-- ************************************************************************
---
--  port_mapping
--        @description  port mapping
--
--		  @params		dev 	- device number
--						portNum - port number
--
--        @return       port mapping
--
local function port_mapping(dev, portNum)
	local ethernetType = 0 							-- CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E
	mapping = {}
	mapping.enable = false
	tmEnable	   = false
	firstPhysicalPortNumber = portNum
	local portMapShadowPtrPtr

	result, valid, mappingType, trafficManagerEn, portGroup, macNum, rxDmaNum,
   txDmaNum, txFifoPortNum, txqNum, ilknChannel, tmPortIdx = wrLogWrapper("wrlCpssDxChPortPhysicalPortMapShadowDBGet",
                                                                          "(dev, firstPhysicalPortNumber, portMapShadowPtrPtr)",
                                                                            dev, firstPhysicalPortNumber, portMapShadowPtrPtr)

	if (result ~= 0) then
		print("Error at show pizza port: port_mapping")
		return;
	end


	mapping.enable = false


	if(mappingType ~= ethernetType) then			-- Mapping type isn't Ethernet (currently not implemented)
		print("Error at show pizza port: Mapping type currently not implemented.")
	else
		mapping.enable = valid

		mapping[0] = {}
		mapping[0].name  = "RXDMA"
		mapping[0].value = rxDmaNum

		mapping[1] = {}
		mapping[1].name  = "TXDMA"
		mapping[1].value = txDmaNum
		mapping[1].tm    = trafficManagerEn

		mapping[2] = {}
		mapping[2].name  = "TX_FIFO"
		mapping[2].value = txFifoPortNum
		mapping[2].tm    = trafficManagerEn

		mapping[3] = {}
		mapping[3].name  = "TXQ"
		mapping[3].value = txqNum
		mapping[3].tm    = trafficManagerEn

		mapping[4] = {}
		mapping[4].name  = "RXDMA_1"
		mapping[4].value = rxDmaNum

		mapping[5] = {}
		mapping[5].name  = "TXDMA_1"
		mapping[5].value = txDmaNum
		mapping[5].tm    = trafficManagerEn

		mapping[6] = {}
		mapping[6].name  = "TX_FIFO_1"
		mapping[6].value = txFifoPortNum
		mapping[6].tm    = trafficManagerEn
	end

	return mapping
end


-- ************************************************************************
---
--  print_port_mapping
--        @description  print port mapping
--
--		  @params		dev 	- device number
--						portNum - port number
--						mapping - port local mapping
--						isFirst - is it the first port
--
--        @return       print port mapping
--
local function print_port_mapping(dev, portNum, mapping, isFirst)
	local header_string
	slices = {}
	port=to_string(portNum)



	if(string.len(portSpeed)>=4) then
		dec = portSpeed:sub(#portSpeed-2,#portSpeed-2)
		portSpeed =  portSpeed:sub(0,#portSpeed-3)
		if(dec ~= "0") then
			portSpeed = portSpeed .. "." .. dec
		end
		portSpeed = portSpeed .. "G"
	end
	if(portSpeed:sub(#portSpeed,#portSpeed) ~= "G") then
		portSpeed = portSpeed .. "M"
	end

	slices =                                                                  "|" ..
			string.format("%-6s",  alignLeftToCenterStr(port, 6)) 		   .. "|" ..
			string.format("%-11s", alignLeftToCenterStr(portMode, 11))     .. "|" ..
			string.format("%-7s",  alignLeftToCenterStr(portSpeed, 7))

	result, values = myGenWrapper(
		"cpssDxChPortPizzaArbiterDevStateGet", {
			{ "IN", "GT_U8"  , "dev", dev},                 						 -- devNum
			{ "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},					 -- portNum
			{ "OUT","CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
		}
	)
	if (result ~= 0) then
        local index = string.find(returnCodes[result],"=")
        print("Error at show pizza port:", string.sub(returnCodes[result],index + 1))
        do return end
	end
	OutValues = values["pizzaDeviceStatePtr"]
	devType = OutValues.devType

	if((devType == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") or
	    (devType == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
	    (devType == "CPSS_PP_FAMILY_DXCH_AC3X_E"))
	then  							-- Bobcat2/bobk/Aldrin/AC3X
		devState    = OutValues.devState
        bobK        = devState.bobK
        unitList    = bobK.unitList
        unitState   = bobK.unitState
		local first_pizza = true
        local currentPizza
        local currentPizzaName
        numberOfPizzas = table.getn(mapping)
        local j
		local numberOfUnits
        local mappingExist
		numberOfUnits = 0
		while currentUnitName ~= "UNDEFINED" do
			numberOfUnits = numberOfUnits + 1
			currentUnitName     = string.sub(unitList[numberOfUnits],19,-3)
		end
		for i = 0 , numberOfPizzas do
            currentUnitName     = string.sub(unitList[0],19,-3)
            currentUnitState    = unitState[0]
            j = 0
			portNum         = mapping[i].value
            mappingExist    = false
			while currentUnitName ~= "UNDEFINED" do
                if(currentUnitName == mapping[i].name) then
					currentPizza     = currentUnitState
                    currentPizzaName = currentUnitName
					if(numberOfUnits < 9 or (portNum>47 and i>=3) or (portNum<=47 and i<=3)) then
						mappingExist     = true
					end
                    break
                end
                j = j + 1
                currentUnitName     = string.sub(unitList[j],19,-3)
                currentUnitState    = unitState[j]
			end
            if(mappingExist) then
				if(first_pizza == false) then
					slices = slices .. "\n|      |           |       "
				end
				first_pizza 			= false
                totalConfiguredSlices   = currentPizza.totalConfiguredSlices
                totalSlicesOnUnit 		= currentPizza.totalSlicesOnUnit
                workConservingBit 		= currentPizza.workConservingBit
                pizzaMapLoadEnBit 		= currentPizza.pizzaMapLoadEnBit
                slice_enable 			= currentPizza.slice_enable
                slice_occupied_by 		= currentPizza.slice_occupied_by
                configNumber = 0
                slices = 	slices 														          	    .. "|" ..
                            string.format("%-11s", alignLeftToCenterStr(currentPizzaName, 11)) 			.. " "
                slicesNumbers = ""
                mappingNumber = ""
                if(mapping[i].tm == true) then
                    mappingNumber   = 	mappingNumber .. string.format("%4s", alignLeftToCenterStr(to_string(mapping[i].value), 4)) .. " (TM)"
                    slices          =   slices        .. mappingNumber .. "|"
                else
                    mappingNumber 	= 	mappingNumber .. string.format("%4s", alignLeftToCenterStr(to_string(mapping[i].value), 4)) .. "     "
                    slices 			=   slices 	      .. mappingNumber .. "|"
                    for j = 0, totalSlicesOnUnit do
                        if(slice_enable[j]==true and slice_occupied_by[j] == portNum) then
                            if(configNumber ~= 0 and configNumber %10 == 0) then
                                slicesNumbers = slicesNumbers .. "\n                                                       "
                            end
                            slicesNumbers = slicesNumbers .. " " .. string.format("%4s", j)
                            configNumber = configNumber + 1
                        end
                    end
                    configTotal = 	string.format("%4s", alignLeftToCenterStr(to_string(configNumber), 4))			..	" / " ..
									string.format("%-5s", alignLeftToCenterStr(to_string(totalConfiguredSlices), 5)) .. 	" "
                    slices = slices .. " " .. configTotal
                    slices = slices .. "-" .. slicesNumbers

                end

            end
		end
		print(slices)

	else
		print("Error at show pizza port:        Wrong Device.")
    end
end

-- ************************************************************************
---
--  print_header
--        @description  print port header
--
--
local function print_header(parms)
	header_string =
            "+------+-----------+-------+---------------------+------------------------\n"   ..
			"| Port | Interface | Speed |       Mapping       |     Pizza Arbiter      \n"   ..
            "|      |           |       |    Unit     Mapping | Conf / Total     List  \n"   ..
			"+------+-----------+-------+---------------------+------------------------"
	print(header_string)
end


-- ************************************************************************
---
--  print_footer
--        @description  print port footer
--
--
local function print_footer(parms)
	footer_string = "+------+-----------+-------+---------------------+------------------------"
	print(footer_string)
end

-- ************************************************************************
---
--  print_port
--        @description  print one port
--
--        @param params         - params["ethernet"]: checked interface
--                                name;
--
--                                dev				: device number
--
--                                portNum 			: port number
--
--                                i 				: port index
--
--
local function print_port(params, dev, portNum, i)
	portNum = params.ethernet[dev][i]
	if(port_validation(dev, portNum)  ~= true) then        			    		-- Port validation
		print("   " .. portNum, "No mapping.\n")
	else
		initialized, speed, mode = port_initialized(dev, portNum)
		if(initialized == false) then											-- Port initialized
			print("   " .. portNum, "Not initialized.\n")
		else
			mapping = port_mapping(dev, portNum)
			if(mapping.enable == false) then									-- Mapping isn't enable
				print("   " .. portNum, "Mapping isn't enable.\n")
			else
				if(i == 1) then
					print_port_mapping(dev, portNum, mapping, true)
				else
					print_port_mapping(dev, portNum, mapping, false)
				end
			end
		end
	end
	print_footer(parms)
end

-- ************************************************************************
---
--  pizza_port_dump
--        @description  show pizza port dump
--
--        @param params         - params["ethernet"]: checked interface
--                                name;
--
--
local function pizza_port_dump(params)
    -- Common variables declaration

    local command_data = Command_Data()
	local dev,i

	--dev = next(params.ethernet)
	dev             = 0
	portNum         = 0
	result, values = myGenWrapper(
	"cpssDxChPortPizzaArbiterDevStateGet", {
		{ "IN", "GT_U8"  , "dev", dev},                 						 -- devNum
		{ "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},					 -- portNum
		{ "OUT","CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
	}
	)
	if (result ~= 0) then
        local index = string.find(returnCodes[result],"=")
        print("Error at show pizza port:", string.sub(returnCodes[result],index + 1))
        do return end
	end
	OutValues = values["pizzaDeviceStatePtr"]
	devType = OutValues.devType
	if((devType == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") or
	    (devType == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
	    (devType == "CPSS_PP_FAMILY_DXCH_AC3X_E"))
	then
		print_header(parms)
		for i=1,#params.ethernet[dev] do
			print_port(params, dev, portNum, i)
		end
	else
		print("Error at show pizza port:        Wrong Device.")
    end
end


--------------------------------------------------------------------------------
-- command registration: show pizza port dump
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show pizza-arbiter-dump port", {
    func = pizza_port_dump,
    help = "Display pizza port dump",
    params={
		  {
			type="named",	{ format        =   "ethernet %remote-port-range",name="ethernet", help="dev/port" },
                              mandatory     =   {"ethernet"}
		  }
    }
})
