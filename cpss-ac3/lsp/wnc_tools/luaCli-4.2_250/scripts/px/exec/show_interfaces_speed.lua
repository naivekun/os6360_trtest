--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_speed.lua
--*
--* DESCRIPTION:
--*       display all interfaces_speed
--*
--*         * Add new interface                     - at: get_interfaces_func()
--*         * Add new speed                         - at: get_speeds_func()
--*         * Add new connection interface->speed   - at: get_interface_speeds_func(interface_name)
--*
--*
--********************************************************************************


-- ************************************************************************
---
--  get_interfaces_func
--        @description  get all interfaces
--
--        @return       all interfaces
--
local function get_interfaces_func()
    local interfaces = {}
    table.insert(interfaces, "SGMII")
    table.insert(interfaces, "XGMII")
    table.insert(interfaces, "1000BASE_X")
    table.insert(interfaces, "QX")
    table.insert(interfaces, "HX")
    table.insert(interfaces, "RXAUI")
    table.insert(interfaces, "QSGMII")
    table.insert(interfaces, "KR")
    table.insert(interfaces, "HGL")
    table.insert(interfaces, "CHGL_12")
    table.insert(interfaces, "SR_LR")
    table.insert(interfaces, "XHGS")
    table.insert(interfaces, "XHGS_SR")

    table.sort(interfaces)
    return interfaces
end


-- ************************************************************************
---
--  get_speeds_func
--        @description  get all speeds
--
--        @return       all speeds
--
local function get_speeds_func()
    local speeds = {}
    table.insert(speeds, 10)        -- 10 Mbps
    table.insert(speeds, 100)       -- 100 Mbps
    table.insert(speeds, 1000)      -- 1 Gbps
    table.insert(speeds, 2500)      -- 2.5 Gbps
    table.insert(speeds, 5000)      -- 5 Gbps
    table.insert(speeds, 10000)     -- 10 Gbps
    table.insert(speeds, 11800)     -- 11.8 Gbps
    table.insert(speeds, 12000)     -- 12 Gbps
    table.insert(speeds, 13600)     -- 13.6 Gbps
    table.insert(speeds, 15000)     -- 15 Gbps
    table.insert(speeds, 16000)     -- 16 Gbps
    table.insert(speeds, 20000)     -- 20 Gbps
    table.insert(speeds, 22000)     -- 22 Gbps
    table.insert(speeds, 40000)     -- 40 Gbps
    table.insert(speeds, 47200)     -- 47.2 Gbps
    table.insert(speeds, 50000)     -- 50 Gbps
    table.insert(speeds, 75000)     -- 75 Gbps
    table.insert(speeds, 100000)    -- 100 Gbps
    table.insert(speeds, 140000)    -- 140 Gbps

    table.sort(speeds)
    return speeds
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
--  header_speeds_func
--        @description  get primary header to print
--
--        @params       speeds          - all speeds
--                      speeds_size     - speeds count
--                      is_new_header   - is it a table header(true) of line header(false)
--
--        @return       primary header
--
local function header_speeds_func(speeds, speeds_size, is_new_header)
    local header = string.format("%-11s", alignLeftToCenterStr("Interface", 11))
    local under_line = "-----------"
    for key, value in pairs (speeds) do
        local current_speed = short_speed_func(to_string(value))
        header      = header .. " ".. string.format("%-5s" ,  alignLeftToCenterStr(current_speed, 5))
        under_line  = under_line .. " " .. "-----"
    end
    header = header .. "\n" .. under_line
    if(is_new_header == false) then
        return under_line
    end
    return header
end


-- ************************************************************************
---
--  get_interface_speeds_func
--        @description  get speeds of specific interface
--
--        @params       interface_name      - the interface name
--
--        @return       speeds of specific interface
--
local function get_interface_speeds_func(interface_name)
    local interface_speeds = {}

    if(interface_name == "SGMII") then                          -- SGMII
        table.insert(interface_speeds, 1000)
        table.insert(interface_speeds, 2500)

    elseif(interface_name == "XGMII") then                      -- XGMII
        table.insert(interface_speeds, 10000)
        table.insert(interface_speeds, 12000)
        table.insert(interface_speeds, 16000)
        table.insert(interface_speeds, 20000)

    elseif(interface_name == "1000BASE_X") then                 -- 1000BASE_X
        table.insert(interface_speeds, 1000)

    elseif(interface_name == "QX") then                         -- QX
        table.insert(interface_speeds, 2500)
        table.insert(interface_speeds, 5000)

    elseif(interface_name == "HX") then                         -- HX
        table.insert(interface_speeds, 5000)
        table.insert(interface_speeds, 10000)

    elseif(interface_name == "RXAUI") then                      -- RXAUI
        table.insert(interface_speeds, 10000)

    elseif(interface_name == "QSGMII") then                     -- QSGMII
        table.insert(interface_speeds, 1000)

    elseif(interface_name == "KR") then                         -- KR
        table.insert(interface_speeds, 10000)
        table.insert(interface_speeds, 12000)
        table.insert(interface_speeds, 20000)
        table.insert(interface_speeds, 40000)
        table.insert(interface_speeds, 100000)

    elseif(interface_name == "HGL") then                        -- HGL
        table.insert(interface_speeds, 15000)
        table.insert(interface_speeds, 16000)
        table.insert(interface_speeds, 40000)

    elseif(interface_name == "CHGL_12") then                    -- CHGL_12
        table.insert(interface_speeds, 100000)

    elseif(interface_name == "SR_LR") then                      -- SR_LR
        table.insert(interface_speeds, 5000)
        table.insert(interface_speeds, 10000)
        table.insert(interface_speeds, 12000)
        table.insert(interface_speeds, 20000)
        table.insert(interface_speeds, 40000)

    elseif(interface_name == "XHGS") then                       -- XHGS
        table.insert(interface_speeds, 11800)
        table.insert(interface_speeds, 47200)

    elseif(interface_name == "XHGS_SR") then                    -- XHGS_SR
        table.insert(interface_speeds, 11800)
        table.insert(interface_speeds, 22000)
        table.insert(interface_speeds, 47200)
    end
    return interface_speeds
end


-- ************************************************************************
---
--  fill_interfaces_speeds_func
--        @description  fill table with interfaces and speeds
--
--        @params       interfaces      - interfaces types
--                      interfaces_size - interfaces count
--
--        @return       specific interface's speeds
--
local function fill_interfaces_speeds_func(interfaces, interfaces_size)
    local interface_to_speeds = {}
    for key, value in pairs (interfaces) do
        interface_to_speeds[value] = get_interface_speeds_func(value)
    end
    return interface_to_speeds
end


-- ************************************************************************
---
--  remove_unused_speed_func
--        @description  remove unused speeds
--
--        @params       filledInterfaceToSpeeds      - interface-speeds table
--                      speeds                       - speeds
--
--        @return       filtered speeds
--
local function remove_unused_speed_func(filledInterfaceToSpeeds, speeds)
    local new_speeds = {}
    local to_remove = true
    for key, value in pairs (speeds) do
        to_remove = true
        for key2, value2 in pairs (filledInterfaceToSpeeds) do
            for key3, value3 in pairs (value2) do
                if(value3 == value) then
                    to_remove = false
                end
            end
        end
        if(to_remove == false) then
            table.insert(new_speeds, value)
        end
    end
    return new_speeds
end


-- ************************************************************************
---
--  show_interfaces_speed
--        @description  show interfaces speed
--
--
local function show_interfaces_speed_func(params)

    local GT_OK             = 0
    local GT_FAIL           = 1
    local status            = GT_OK

    -- get interfaces and speed information
    local interfaces        = get_interfaces_func()
    local speeds            = get_speeds_func()

    -- get size
    local interfaces_size   = table.getn(interfaces)
    local interface_speeds_size

    -- index interface to speeds
    local filledInterfaceToSpeeds = fill_interfaces_speeds_func(interfaces, interfaces_size)

    -- remove unused speeds
    speeds = remove_unused_speed_func(filledInterfaceToSpeeds, speeds)

    -- get size
    local speeds_size       = table.getn(speeds)

        -- get headers
    local header_speeds     = header_speeds_func(speeds, speeds_size, true)
    local header_under_line = header_speeds_func(speeds, speeds_size, false)
    local interface_speeds_string

    -- print header
    print(header_speeds)

    -- print information
    local flag = false;
    for key, value in pairs (filledInterfaceToSpeeds) do
        interface_speeds_string = string.format("%-11s" ,  alignLeftToCenterStr(key, 11))
        local interface_speeds = value
        for key2, value2 in pairs (speeds) do
            flag = false
            for key3, value3 in pairs (interface_speeds) do
                if(value2 == value3) then
                    interface_speeds_string = interface_speeds_string .. " " .. string.format("%-5s" ,  alignLeftToCenterStr("X", 5))
                    flag = true;
                end
            end
            if(flag == false) then
                interface_speeds_string = interface_speeds_string .. "      "
            end
        end
        print(interface_speeds_string)
        print(header_under_line)
        interface_speeds_string = ""
    end
end


--------------------------------------------------------------------------------
-- command registration: show interfaces speed
--------------------------------------------------------------------------------
CLI_addCommand("exec", "show interfaces speed", {
    func = show_interfaces_speed_func,
    help = "Interfaces speed",
})

