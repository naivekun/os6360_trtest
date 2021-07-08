--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_port_resources.lua
--*
--* DESCRIPTION:
--*       display the port resources list
--*
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssPxPortPhysicalPortMapShadowDBGet")
cmdLuaCLI_registerCfunction("wrlCpssPxPipePortResourseConfigGet")


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
--  print_header_pipe
--        @description  print header for pipe
--
--
local function print_header_pipe(parms)
    local header_string =
            "\n+---------------------------------------------------------------------------------------+"    ..
            "\n|                             Port resources                                            |"    ..
            "\n+----+------+----------+-------+--------------+-----------+-----+-----------+-----------+"    ..
            "\n|    |      |          |       |              |           |RXDMA|TXDMA SCDMA| TXFIFO    |"    ..
            "\n|    |      |          |       |              |           |-----|-----+-----|-----+-----|"    ..
            "\n| #  | Port | map type | Speed |      IF      |  dma txq  |  IF | Hdr | Pay | Out | Pay |"    ..
            "\n|    |      |          |       |              |           |Width|Thrsh|Load | Bus | Load|"    ..
            "\n|    |      |          |       |              |           |     |     |Thrsh|Width|Thrsh|"    ..
            "\n+----+------+----------+-------+--------------+-----------+-----+-----+-----+-----+-----+"
    print(header_string)
end


-- ************************************************************************
---
--  print_footer_pipe
--        @description  print footer for pipe
--
--
local function print_footer_pipe(parms)
    local footer_string =
              "+----+------+----------+-------+--------------+-----------+-----+-----+-----+-----+-----+"
    print(footer_string)
end

-- ************************************************************************
---
--  show_port_resources_pipe_func
--        @description  show port resources function
--
--        @param params         - params["devID"]: checked device number
--
--
local function show_port_resources_pipe_func(params)
    -- Common variables declaration
    local dev       = params["devID"]
    local devNum    = dev
    local GT_OK     = 0
    local GT_FALSE  = false
    local CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E = 1
    local rc
    local portIdx
    local portNum

    print_header_pipe(params)

    portIdx = 0;
    for portNum = 0, 15, 1 do
        rc, valid, mappingType, macNum, dmaNum, txqNum =
            wrlCpssPxPortPhysicalPortMapShadowDBGet(dev, portNum)
        if (rc ~= GT_OK) then
            local index = string.find(returnCodes[rc],"=")
            print("Error at show port resources (wrlCpssPxPortPhysicalPortMapShadowDBGet):", string.sub(returnCodes[rc],index + 1))
            do return end
        end

        if (valid ~= GT_FALSE) then
            if (mappingType ~= CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E) then
                rc, values = myGenWrapper(
                    "cpssPxPortSpeedGet", {
                        { "IN", "GT_SW_DEV_NUM", "dev", dev},                                           -- devNum
                        { "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},                     -- portNum
                        { "OUT","CPSS_PORT_SPEED_ENT" , "speedPtr"}                                 -- speed
                })
                if (rc ~= GT_OK) then
                    local index = string.find(returnCodes[rc],"=")
                    print("Error at show port resources (cpssPxPortSpeedGet):", string.sub(returnCodes[rc],index + 1))
                    do return end
                end
                speed = values["speedPtr"]
                rc, values = myGenWrapper(
                    "cpssPxPortInterfaceModeGet", {
                        { "IN", "GT_SW_DEV_NUM"  , "dev", dev},                                          -- devNum
                        { "IN", "GT_U32" , "GT_PHYSICAL_PORT_NUM", portNum},                     -- portNum
                        { "OUT","CPSS_PORT_INTERFACE_MODE_ENT" , "ifModePtr"}                    -- ifMode
                })
                if (rc ~= GT_OK) then
                    local index = string.find(returnCodes[rc],"=")
                    print("Error at show port resources (cpssPxPortInterfaceModeGet):", string.sub(returnCodes[rc],index + 1))
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
                    "| "       ..  string.format("%2d", portIdx)                                       ..
                    " | "      ..  string.format("%4d", portNum)                                       ..
                    " | "      ..  string.format("%-8s", map_type_func(mappingType))                   ..
                    " | "      ..  string.format("%5s", short_speed_func(string.sub(speed,17,-3)))     ..
                    " | "      ..  string.format("%12s", ifMode)                                       ..
                    " | "      ..  string.format("%5d", dmaNum)                                        ..
                    " "        ..  string.format("%3d", txqNum)                                        .. " |"

                rc, rxdmaScdmaIncomingBusWidth, txdmaScdmaHeaderTxfifoThrshold,
                txdmaScdmaPayloadTxfifoThrshold, txfifoScdmaShiftersOutgoingBusWidth,
                txfifoScdmaPayloadStartTransmThreshold = wrlCpssPxPipePortResourseConfigGet(dev, portNum)
                if (rc ~= GT_OK) then
                    local index = string.find(returnCodes[rc],"=")
                    print("Error at show port resources (wrlCpssPxPipePortResourseConfigGet):", string.sub(returnCodes[rc],index + 1))
                    do return end
                end

                table_info = table_info ..
                    string.format("%4s", rxdma_if_width_to_str_func(rxdmaScdmaIncomingBusWidth)) ..  " |"    ..
                    string.format("%4d", txdmaScdmaHeaderTxfifoThrshold)                         ..  " |"    ..
                    string.format("%4d", txdmaScdmaPayloadTxfifoThrshold)                        ..  " |"    ..
                    string.format("%4s", tx_fifo_if_width_to_str_func(txfifoScdmaShiftersOutgoingBusWidth))  ..  " |"    ..
                    string.format("%4d", txfifoScdmaPayloadStartTransmThreshold)                 ..  " |"

                print(table_info)
                portIdx = portIdx + 1
            end
        end
    end
    print_footer_pipe(parms)
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
        "cpssPxPortPizzaArbiterDevStateGet", {
            { "IN", "GT_SW_DEV_NUM"  , "dev", dev},                                -- devNum
            { "OUT","CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC" , "pizzaDeviceStatePtr"} -- data
        }
    )
    if (result ~= 0) then
        local index = string.find(returnCodes[result],"=")
        print("Error at show port resources (cpssPxPortPizzaArbiterDevStateGet):", string.sub(returnCodes[result],index + 1))
        do return end
    end
    OutValues = values["pizzaDeviceStatePtr"]
    devType = OutValues.devType

    devState    = OutValues.devState
    pipe        = devState.pipe
    unitList    = pipe.unitList

    local numberOfUnits = 0
    currentUnitName = string.sub(unitList[0],17,-3)
    while currentUnitName ~= "UNDEFINED" do
        numberOfUnits = numberOfUnits + 1
        currentUnitName     = string.sub(unitList[numberOfUnits],17,-3)
    end
    show_port_resources_pipe_func(params)            -- show port resources for - BobK
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
