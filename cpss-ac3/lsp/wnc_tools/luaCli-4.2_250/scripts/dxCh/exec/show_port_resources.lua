--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_port_resources.lua
--*
--* DESCRIPTION:
--*       display the port resources list
--*
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssDxChPortPhysicalPortMapShadowDBGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChCaelumTmPortResourceConfigGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChBcat2PortResoursesStateGet")


-- ************************************************************************
---
--  tx_fifo_if_width_to_str_func
--        @description  convert tx fifo if width to str (0 -> 1B, 3 -> 8B ..)
--
--        @return       if width as str
--
local function tx_fifo_if_width_to_str_func(tx_fifo)
	if(tx_fifo == 0) then
		return "1B"
	end
	if(tx_fifo == 3) then
		return "8B"
	end
	if(tx_fifo == 5) then
		return "32B"
	end
	if(tx_fifo == 6) then
		return "64B"
	end
	return "-1"
end


-- ************************************************************************
---
--  rxdma_if_width_to_str_func
--        @description  convert rxdma if width to str
--
--        @return       if width as str
--
local function rxdma_if_width_to_str_func(rxdma)
    if(rxdma == 0) then
		return "64b"
	end
	if(rxdma == 2) then
		return "256b"
	end
	if(rxdma == 3) then
		return "512b"
	end
	return "-1"
end


-- ************************************************************************
---
--  short_speed_func
--        @description  shorter the speed value (1000 -> 1G)
--
--        @return       shorter speed
--
local function short_speed_func(speed)
    if(string.len(speed)>=4) then
		dec = speed:sub(#speed-2,#speed-2)
		speed =  speed:sub(0,#speed-3)
		if(dec ~= "0") then
			speed = speed .. "." .. dec
		end
		speed = speed .. "G"
	end
	if(speed:sub(#speed,#speed) ~= "G") then
		speed = speed .. "M"
	end
    return speed
end


-- ************************************************************************
---
--  map_type_func
--        @description  map the enum type
--
--        @return       map type
--
local function map_type_func(map_type)
    if(map_type == 0) then
		return "ETHERNET"
	end
	if(map_type == 1) then
		return "CPU_SDMA"
	end
	if(map_type == 2) then
		return "ILKN"
	end
	if(map_type == 3) then
		return "PHYSICAL"
	end
	if(map_type == 4) then
		return "MAX"
	end
	return "INVALID"
end


-- ************************************************************************
---
--  print_header_bobk
--        @description  print header for bobk
--
--
local function print_header_bobk(parms)
	local header_string =
            "\n+------------------------------------------------------------------------------------------------------------------+"    ..
            "\n|                             Port resources                                                                       |"    ..
            "\n+----+------+----------+-------+--------------+--------------------+-----+-----------------+-----------+-----------+"    ..
            "\n|    |      |          |       |              |                    |RXDMA|  TXDMA SCDMA    | TXFIFO    | Eth_TXFIFO|"    ..
            "\n|    |      |          |       |              |                    |-----|-----+-----+-----|-----+-----|-----+-----|"    ..
            "\n| #  | Port | map type | Speed |    IF        | rxdma txq txdma tm |  IF |TxQ  | Hdr | Pay | Out | Pay | Out | Pay |"    ..
            "\n|    |      |          |       |              |                    |Width|Descr|Thrsh|Load | Bus | Load| Bus | Load|"    ..
            "\n|    |      |          |       |              |                    |     |     |     |Thrsh|Width|Thrsh|Width|Thrsh|"    ..
            "\n+----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+"
	print(header_string)
end


-- ************************************************************************
---
--  print_footer_bobk
--        @description  print footer for bobk
--
--
local function print_footer_bobk(parms)
	local footer_string =
            "+----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+"
	print(footer_string)
end


-- ************************************************************************
---
--  print_header_bobcat2
--        @description  print header for bobcat2
--
--
local function print_header_bobcat2(params)
	local header_string =
            "\n+------------------------------------------------------------------------------------------------------------------------------+"    ..
            "\n|                             Port resources                                                                                   |"    ..
            "\n+----+------+----------+-------+--------------+--------------------+-----+-----------------------------+-----------+-----------+"    ..
            "\n|    |      |          |       |              |                    |RXDMA|      TXDMA SCDMA            | TX-FIFO   |Eth-TX-FIFO|"    ..
            "\n|    |      |          |       |              |                    |-----|-----+-----+-----+-----------|-----+-----|-----+-----|"    ..
            "\n| #  | Port | map type | Speed |    IF        | rxdma txq txdma tm |  IF | TxQ |Burst|Burst| TX-FIFO   | Out | Pay | Out | Pay |"    ..
            "\n|    |      |          |       |              |                    |Width|Descr| All | Full|-----+-----|Going| Load|Going| Load|"    ..
            "\n|    |      |          |       |              |                    |     |     | Most|Thrsh| Hdr | Pay | Bus |Thrsh| Bus |Thrsh|"    ..
            "\n|    |      |          |       |              |                    |     |     | Full|     |Thrsh|Load |Width|     |Width|     |"    ..
            "\n|    |      |          |       |              |                    |     |     |Thrsh|     |     |Thrsh|     |     |     |     |"    ..
            "\n+----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+"
	print(header_string)
end


-- ************************************************************************
---
--  print_footer_bobcat2
--        @description  print footer for bobcat2
--
--
local function print_footer_bobcat2(params)
	local footer_string =
            "+----+------+----------+-------+--------------+--------------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+"
	print(footer_string)
end


-- ************************************************************************
---
--  show_port_resources_bobcat2_func
--        @description  show port resources function
--
--        @param params         - params["devID"]: checked device number
--
--
local function show_port_resources_bobcat2_func(params)
	local GT_OK 	= 0
	local GT_FALSE  = false
	local CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E = 1
	local enums_mapping
	outValues  =wrLogWrapper("wrlCpssDxChBcat2PortResoursesStateGet","(dev, 0)", dev, 0)

	rc = outValues["status"]
	if(rc ~= GT_OK)then
		local index = string.find(returnCodes[rc],"=")
		print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
		do return end
	end
	bc2_res_arr   = outValues["arr"]

    rc, values = myGenWrapper(
        "cpssDxChCfgTableNumEntriesGet", {
            { "IN", "GT_U8"  , "dev", dev},                                          -- devNum
            { "IN", "CPSS_DXCH_CFG_TABLES_ENT", "table", "CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E"},
            { "OUT","GT_U32" , "maxPortNum"}                                         -- maxPortNum
    })
	if(rc ~= GT_OK)then
		local index = string.find(returnCodes[rc],"=")
		print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
		do return end
	end

	local maxPortNum = values["maxPortNum"]
	print_header_bobcat2(params)

    local portIdx = 0;
    for portNum = 0, maxPortNum - 1 do
		rc, valid, mappingType, trafficManagerEn, portGroup, macNum, rxDmaNum,
    txDmaNum, txFifoPortNum, txqNum, ilknChannel, tmPortIdx = wrLogWrapper("wrlCpssDxChPortPhysicalPortMapShadowDBGet",
                                                                           "(dev, portNum, portMapShadowPtrPtr)",
                                                                            dev, portNum, portMapShadowPtrPtr)

		if (rc ~= GT_OK) then
			local index = string.find(returnCodes[rc],"=")
            print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
			do return end
		end

        if (valid ~= GT_FALSE) then
			if (mappingType ~= CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E) then
                rc, values = myGenWrapper(
                    "cpssDxChPortSpeedGet", {
                        { "IN", "GT_U8"  , "dev", dev},                      	    			 -- devNum
                        { "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},					 -- portNum
                        { "OUT","CPSS_PORT_SPEED_ENT" , "speedPtr"}                            	 -- speed
                })
                if (rc ~= GT_OK) then
                    local index = string.find(returnCodes[rc],"=")
                    print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
                    do return end
                end
                speed = values["speedPtr"]
                rc, values = myGenWrapper(
                    "cpssDxChPortInterfaceModeGet", {
                        { "IN", "GT_U8"  , "dev", dev},                 						 -- devNum
                        { "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},					 -- portNum
                        { "OUT","CPSS_PORT_INTERFACE_MODE_ENT" , "ifModePtr"}                    -- ifMode
                })
                if (rc ~= GT_OK) then
                    local index = string.find(returnCodes[rc],"=")
                    print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
                    do return end
                end

				ifMode = values["ifModePtr"]
            else    -- CPU
                speed  = "CPSS_PORT_SPEED_1000_E"
                ifMode = "CPSS_PORT_INTERFACE_MODE_NA_E"
            end

            if (speed ~= "CPSS_PORT_SPEED_NA_E") then
				outValues 	= wrLogWrapper("wrlCpssDxChBcat2PortResoursesStateGet", "(dev, portNum)",dev, portNum)

				rc = outValues["status"]
				if(rc ~= GT_OK)then
					local index = string.find(returnCodes[rc],"=")
					print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
					do return end
				end
				bc2_res_arr = outValues["arr"]

				ifMode = string.sub(ifMode,26,-3)
				if(ifMode == "NA") then
					ifMode = "------------"
				end

                table_info =
                    "| "       ..  string.format("%2d", portIdx)           							..
                    " | "      ..  string.format("%4d", portNum)           							..
                    " | "      ..  string.format("%-8s", map_type_func(mappingType)) 				..
					" | "      ..  string.format("%5s", short_speed_func(string.sub(speed,17,-3))) 	..
					" | "      ..  string.format("%12s", ifMode)    								..
					" | "      ..  string.format("%5d", rxDmaNum)          							..
                    " "        ..  string.format("%3d", txqNum)            							..
					" "        ..  string.format("%5d", txDmaNum)          							..
					" "        ..  string.format("%2d", tmPortIdx)   	   							.. " |"

				number_of_variables						= 3
				value_offset							= 2
				rxdmaScdmaIncomingBusWidth				= bc2_res_arr[0 * number_of_variables + value_offset]
				txdmaCreditValue						= bc2_res_arr[1 * number_of_variables + value_offset]
				txdmaBurstAlmostFullThreshold			= bc2_res_arr[4 * number_of_variables + value_offset]
				txdmaBBurstFullThreshold				= bc2_res_arr[5 * number_of_variables + value_offset]
				txfifoHeaderCounterThresholdScdma		= bc2_res_arr[6 * number_of_variables + value_offset]
				txfifoPayloadCounterThresholdScdma		= bc2_res_arr[7 * number_of_variables + value_offset]
				txfifoScdmaShiftersOutgoingBusWidth		= bc2_res_arr[8 * number_of_variables + value_offset]
				txfifoScdmaPayloadThreshold				= bc2_res_arr[10 * number_of_variables + value_offset]

				table_info = table_info ..
                    string.format("%4s", rxdma_if_width_to_str_func(rxdmaScdmaIncomingBusWidth))        		..  " |"	..
                    string.format("%4d", txdmaCreditValue)           											..  " |"	..
                    string.format("%4d", txdmaBurstAlmostFullThreshold)           								..  " |"	..
					string.format("%4d", txdmaBBurstFullThreshold)           									..  " |"	..
					string.format("%4d", txfifoHeaderCounterThresholdScdma)     								..  " |"	..
					string.format("%4d", txfifoPayloadCounterThresholdScdma)    								..  " |"	..
					string.format("%4s", tx_fifo_if_width_to_str_func(txfifoScdmaShiftersOutgoingBusWidth))   	..  " |"	..
					string.format("%4d", txfifoScdmaPayloadThreshold)          									..  " |"

				if(trafficManagerEn == GT_FALSE) then
					table_info = table_info ..
						string.format("%4s", "--")      						..  " |"	..
						string.format("%4s", "--")      						..  " |"
				else
					ethTxfifoOutgoingBusWidth		= bc2_res_arr[11 * number_of_variables + value_offset]
					ethTxfifoScdmaPayloadThreshold	= bc2_res_arr[13 * number_of_variables + value_offset]
					table_info = table_info ..
						string.format("%4s", tx_fifo_if_width_to_str_func(ethTxfifoOutgoingBusWidth))     	..  " |"	..
						string.format("%4d", ethTxfifoScdmaPayloadThreshold)   								..  " |"
				end
				print(table_info)
				portIdx = portIdx + 1
			end
		end
	end
	print_footer_bobcat2(params)
end


-- ************************************************************************
---
--  show_port_resources_bobk_func
--        @description  show port resources function
--
--        @param params         - params["devID"]: checked device number
--
--
local function show_port_resources_bobk_func(params)
    -- Common variables declaration
	local dev       = params["devID"]
	local devNum    = dev
    local GT_OK     = 0
    local GT_FALSE  = false
	local CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E = 1
    local rc
    local portIdx
    local portNum

    print_header_bobk(params)

    portIdx = 0;
    for portNum = 0, 255, 1 do

        rc, valid, mappingType, trafficManagerEn, portGroup, macNum, rxDmaNum,
    txDmaNum, txFifoPortNum, txqNum, ilknChannel, tmPortIdx = wrLogWrapper("wrlCpssDxChPortPhysicalPortMapShadowDBGet",
                                                                           "(dev, portNum, portMapShadowPtrPtr)",
                                                                           dev, portNum, portMapShadowPtrPtr)

		if (rc ~= GT_OK) then
			local index = string.find(returnCodes[rc],"=")
            print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
			do return end
		end

        if (valid ~= GT_FALSE) then
            if (mappingType ~= CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E) then
                rc, values = myGenWrapper(
                    "cpssDxChPortSpeedGet", {
                        { "IN", "GT_U8"  , "dev", dev},                      	    			 -- devNum
                        { "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},					 -- portNum
                        { "OUT","CPSS_PORT_SPEED_ENT" , "speedPtr"}                            	 -- speed
                })
                if (rc ~= GT_OK) then
                    local index = string.find(returnCodes[rc],"=")
                    print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
                    do return end
                end
                speed = values["speedPtr"]
                rc, values = myGenWrapper(
                    "cpssDxChPortInterfaceModeGet", {
                        { "IN", "GT_U8"  , "dev", dev},                 						 -- devNum
                        { "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},					 -- portNum
                        { "OUT","CPSS_PORT_INTERFACE_MODE_ENT" , "ifModePtr"}                    -- ifMode
                })
                if (rc ~= GT_OK) then
                    local index = string.find(returnCodes[rc],"=")
                    print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
                    do return end
                end
				ifMode = values["ifModePtr"]
            else    -- CPU
                speed  = "CPSS_PORT_SPEED_1000_E"
                ifMode = "CPSS_PORT_INTERFACE_MODE_NA_E"
            end

			ifMode = string.sub(ifMode,26,-3)
			if(ifMode == "NA") then
				ifMode = "------------"
			end

            if (speed ~= "CPSS_PORT_SPEED_NA_E") then
                table_info =
                    "| "       ..  string.format("%2d", portIdx)           							..
                    " | "      ..  string.format("%4d", portNum)           							..
                    " | "      ..  string.format("%-8s", map_type_func(mappingType)) 				..
					" | "      ..  string.format("%5s", short_speed_func(string.sub(speed,17,-3))) 	..
					" | "      ..  string.format("%12s", ifMode)    								..
					" | "      ..  string.format("%5d", rxDmaNum)          							..
                    " "        ..  string.format("%3d", txqNum)            							..
					" "        ..  string.format("%5d", txDmaNum)          							..
					" "        ..  string.format("%2d", tmPortIdx)   	   							.. " |"

				rc, txdmaCreditValue, txfifoHeaderCounterThresholdScdma, txfifoPayloadCounterThresholdScdma, txfifoScdmaPayloadThreshold,
						txfifoScdmaShiftersOutgoingBusWidth, rxdmaScdmaIncomingBusWidth, ethTxfifoOutgoingBusWidth, ethTxfifoScdmaPayloadThreshold =
						                                              wrLogWrapper("wrlCpssDxChCaelumTmPortResourceConfigGet","(dev, portNum)",dev, portNum)

				if (rc ~= GT_OK) then
                    local index = string.find(returnCodes[rc],"=")
                    print("Error at show port resources:", string.sub(returnCodes[rc],index + 1))
                    do return end
                end

				table_info = table_info ..
                    string.format("%4s", rxdma_if_width_to_str_func(rxdmaScdmaIncomingBusWidth))        		..  " |"	..
                    string.format("%4d", txdmaCreditValue)           			..  " |"	..
                    string.format("%4d", txfifoHeaderCounterThresholdScdma)     ..  " |"	..
					string.format("%4d", txfifoPayloadCounterThresholdScdma)    ..  " |"	..
					string.format("%4s", tx_fifo_if_width_to_str_func(txfifoScdmaShiftersOutgoingBusWidth))   	..  " |"	..
					string.format("%4d", txfifoScdmaPayloadThreshold)          	..  " |"

				if(trafficManagerEn == GT_FALSE) then
					table_info = table_info ..
						string.format("%4s", "--")      						..  " |"	..
						string.format("%4s", "--")      						..  " |"
				else
					table_info = table_info ..
						string.format("%4s", tx_fifo_if_width_to_str_func(ethTxfifoOutgoingBusWidth))     	..  " |"	..
						string.format("%4d", ethTxfifoScdmaPayloadThreshold)   								..  " |"
				end

				print(table_info)
				portIdx = portIdx + 1
			end
        end
    end
    print_footer_bobk(parms)
end


-- ************************************************************************
---
--  show_port_resources_func
--        @description  show port resources function
--
--        @param params         - params["devID"]: checked device number
--
--
local function show_port_resources_func(params)
	result, values = myGenWrapper(
		"cpssDxChPortPizzaArbiterDevStateGet", {
			{ "IN", "GT_U8"  , "dev", dev},                 						 -- devNum
			{ "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},					 -- portNum
			{ "OUT","CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
		}
	)
	if (result ~= 0) then
        local index = string.find(returnCodes[result],"=")
        print("Error at show port resources:", string.sub(returnCodes[result],index + 1))
        do return end
	end
	OutValues = values["pizzaDeviceStatePtr"]
	devType = OutValues.devType

	if( type(OutValues.devState.bobK) ~= "table") then
		print("Error at show port resources:	Device not supported.")
		do return end
	end
	devState    = OutValues.devState
	bobK        = devState.bobK
    unitList    = bobK.unitList

	local numberOfUnits = 0
	currentUnitName = string.sub(unitList[0],19,-3)
	while currentUnitName ~= "UNDEFINED" do
		numberOfUnits = numberOfUnits + 1
		currentUnitName     = string.sub(unitList[numberOfUnits],19,-3)
	end
	if(numberOfUnits < 7) then
		show_port_resources_bobcat2_func(params)		-- show port resources for - Bobcat2
	else
		show_port_resources_bobk_func(params)			-- show port resources for - BobK
	end
end

--------------------------------------------------------------------------------
-- command registration: show port resources
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show port resources", {
    func = show_port_resources_func,
    help = "Display the port resources list",
    params={
		  {
			type="named",   { format="device %devID",name="devID", help="The device number" },
                            mandatory={"devID"}
		  }
    }
})
