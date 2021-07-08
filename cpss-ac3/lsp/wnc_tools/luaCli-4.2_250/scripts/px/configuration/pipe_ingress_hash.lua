--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pipe_ingress_hash.lua
--*
--* DESCRIPTION:
--*       Pipe device: Packet hash calculation
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssPxIngressHashPacketTypeHashModeSet")
cmdLuaCLI_registerCfunction("wrlCpssPxIngressHashPacketTypeHashModeGet")
cmdLuaCLI_registerCfunction("wrlCpssPxIngressHashPacketTypeEntrySet")
cmdLuaCLI_registerCfunction("wrlCpssPxIngressHashPacketTypeEntryGet")

local hash_packet_type_help = "Hash packet type"
local hash_packet_type_enum = {
    ["ipv4-tcp/udp"]     = { value = 0, help = "IPv4 with TCP or UDP" },
    ["ipv6-tcp/udp"]     = { value = 1, help = "IPv6 with TCP or UDP" },
    ["ipv4-no-tcp/udp"]  = { value = 2, help = "IPv4 without TCP or UDP" },
    ["ipv6-no-tcp/udp"]  = { value = 3, help = "IPv6 without TCP or UDP" },
    ["mpls"]             = { value = 4, help = "MPLS with single label stack" },
    ["mpls-multilabel"]  = { value = 5, help = "MPLS with multiple label stack" },
    ["ude1"]             = { value = 6, help = "User-defined ethertype 1" },
    ["ude2"]             = { value = 7, help = "User-defined ethertype 2" },
    ["ethernet"]         = { value = 8, help = "Other ethernet frames" }
}
CLI_addParamDictAndType_enum("hash_packet_type_enum","Hash packet type", hash_packet_type_help, hash_packet_type_enum)
local hptypes = {[0] = "IPv4-TCP/UDP", [1] = "IPv6-TCP/UDP", [2] = "IPv4-no-TCP/UDP",
           [3] = "IPv6-no-TCP/UDP", [4] = "MPLS", [5] = "MPLS-multilabel",
           [6] = "UDE1", [7] = "UDE2", [8] = "ethernet"}

local hash_packet_type_c_enum =
    {[0] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E",
     [1] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E",
     [2] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E",
     [3] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E",
     [4] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_SINGLE_LABEL_E",
     [5] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_MULTI_LABEL_E",
     [6] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E",
     [7] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E",
     [8] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E"}

local help_ingress_hash = "Hash calculation commands"
local help_ingress_lag = "LAG configuration commands"
local help_ingress_hash_ude = "Configure user-defined ethertype "

CLI_addHelp("config", "ingress hash ude1", help_ingress_hash_ude.."1")
CLI_addHelp("config", "ingress hash ude2", help_ingress_hash_ude.."2")
CLI_addHelp("config", "ingress hash", help_ingress_hash)
CLI_addHelp("config", "ingress lag", help_ingress_lag)
CLI_addHelp("config", "no ingress hash", help_ingress_hash)

function luaWrap_cpssPxIngressHashUdeEthertypeSet(command_data, devNum, packetType, etherType, valid)
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxIngressHashUdeEthertypeSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT", "hashPacketType", packetType},
            { "IN", "GT_U16", "etherType", etherType },
            { "IN", "GT_BOOL", "valid", valid }
        })
    if isError then
        local error_string = "ERROR calling function cpssPxIngressHashUdeEthertypeSet"
        command_data:addErrorAndPrint(error_string)
    end
end

function hash_packet_type_ude_set_func(params)
    local command_data = Command_Data()
    local devices, devNum
    local hash_packet_type = params["hashPacketType"]
    local valid = true
    local ethertype

    if (params["devID"]=="all") then
        devices = generic_getDeviceOfFamilyList(params["devID"],
            {"CPSS_PX_FAMILY_PIPE_E"});
    else
        devices={params["devID"]}
    end

    if params["flagNo"] == true then
        valid = false
        ethertype = 0x0000
    else
        ethertype = tonumber(params["ether_type"], 16)
    end

    for j=1, #devices do
        devNum = devices[j]
        luaWrap_cpssPxIngressHashUdeEthertypeSet(command_data, devNum,
            hash_packet_type, ethertype, valid)
    end
end

--------------------------------------------
-- command registration: ingress hash ude1
--------------------------------------------
CLI_addCommand("config", "ingress hash ude1", {
    func   = function(params)
                params["hashPacketType"] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E"
                return hash_packet_type_ude_set_func(params)
            end,
    help   = help_ingress_hash_ude.."1",
    params = {
        { type = "named",
            { format= "device %devID_all",    name = "devID",      help = "The device number" },
            { format= "ethertype %ethertype", name = "ether_type", help = "Ethertype"},
            mandatory = { "devID", "ether_type"}
        }
    }
})

--------------------------------------------
-- command registration: ingress hash ude2
--------------------------------------------
CLI_addCommand("config", "ingress hash ude2", {
    func   = function(params)
                params["hashPacketType"] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E"
                return hash_packet_type_ude_set_func(params)
            end,
    help   = help_ingress_hash_ude.."2",
    params = {
        { type = "named",
            { format= "device %devID_all",    name = "devID",      help = "The device number" },
            { format= "ethertype %ethertype", name = "ether_type", help = "Ethertype"},
            mandatory = { "devID", "ether_type"}
        }
    }
})

--------------------------------------------
-- command registration: no ingress hash ude1
--------------------------------------------
CLI_addCommand("config", "no ingress hash ude1", {
    func=function(params)
        params["flagNo"]= true
        params["hashPacketType"] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E"
        return hash_packet_type_ude_set_func(params)
    end,
    help="Invalidate user-defined ethertype 1",
    params = {
        { type = "named",
            { format= "device %devID_all", name = "devID", help="The device number" },
            mandatory = { "devID" }
        }
    }
})

--------------------------------------------
-- command registration: no ingress hash ude1
--------------------------------------------
CLI_addCommand("config", "no ingress hash ude2", {
    func=function(params)
        params["flagNo"]= true
        params["hashPacketType"] = "CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E"
        return hash_packet_type_ude_set_func(params)
    end,
    help="Invalidate user-defined ethertype 2",
    params = {
        { type = "named",
            { format= "device %devID_all", name = "devID", help="The device number" },
            mandatory = { "devID" }
        }
    }
})

local help_show_ingress_hash_ude = "Show user-defined ethertypes"
local help_string_show_ingress_hash = "Show hash calculation configuration"
local help_string_show_ingress_lag = "Show LAG configuration"

CLI_addHelp("exec", "show ingress hash", help_string_show_ingress_hash)
CLI_addHelp("exec", "show ingress hash ude", help_show_ingress_hash_ude)
CLI_addHelp("exec", "show ingress lag", help_string_show_ingress_lag)


local function luaWrap_cpssPxIngressHashUdeEthertypeGet(command_data, devNum, hash_packet_type)
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxIngressHashUdeEthertypeGet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT", "hashPacketType", hash_packet_type},
            { "OUT", "GT_U16", "etherType" },
            { "OUT", "GT_BOOL", "valid" }
        })
    if isError then
        local error_string = "ERROR calling function cpssPxIngressHashUdeEthertypeGet"
        command_data:addErrorAndPrint(error_string)
        return nil
    end

    return values["etherType"], values["valid"]
end

local function ingress_hash_ude_show_dev(command_data, devNum, params)
    local packetTypes = {"CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E", "CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E"}
    local typeNames = {"UDE1", "UDE2"}
    local packetType
    local k

    for k=1, #packetTypes do
        packetType = packetTypes[k]
        local etherType, valid = luaWrap_cpssPxIngressHashUdeEthertypeGet(command_data,
            devNum, packetType)
        if (valid ~= nil) then
            if(valid == true) then
                command_data["result"] = string.format("%d\t\t%s\t\t0x%04x",
                    devNum, typeNames[k], etherType)
                command_data:addResultToResultArray()
            end
        end
    end
    command_data:updateEntries()
end

local function ingress_hash_ude_show_func(params)
    local head_line0 = "Device\t\tPacket Type\tEthertype\n"
    local head_line1 = "-----------------------------------------"
    params["header_string"] = head_line0..head_line1
    params["footer_string"] = "\n"

    generic_all_device_show_func(ingress_hash_ude_show_dev, params)
end

--------------------------------------------
-- command registration: show ingress hash ude
--------------------------------------------
CLI_addCommand("exec", "show ingress hash ude", {
    func   = ingress_hash_ude_show_func,
    help   = "Show hash packet types user-defined ethertypes",
    params = {
        { type = "named",
            "#all_device",
            mandatory = { "all_device" }
        }
    }
})

local help_ingress_hash_indexing_mode_set = "Set designated port table indexing mode"
CLI_addHelp("interface", "ingress lag", help_ingress_lag)
CLI_addHelp("interface", "ingress lag index-mode", help_ingress_hash_indexing_mode_set)

local hash_indexing_mode_c_enum = {
    [0]     = "CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E",
    [1]     = "CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E"
}

local function ingress_hash_indexing_mode_set_port(command_data, devNum, portNum, params)
    local idxmode = hash_indexing_mode_c_enum[params["indexing_mode"]]
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxIngressHashPortIndexModeSet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
            { "IN", "CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT", "indexMode", idxmode }
        })
    if isError then
        local error_string = "ERROR calling function cpssPxIngressHashPortIndexModeSet"
        command_data:addErrorAndPrint(error_string)
        return nil
    end
end

local function hash_index_mode_set_func(params)
    generic_port_range_func(ingress_hash_indexing_mode_set_port, params)
end

local help_hash_indexing_mode = "Designated port table indexing mode"
local hash_indexing_mode_enum = {
    ["hash"]     = { value = 0, help = "Hash" },
    ["prng"]     = { value = 1, help = "Pseudo-random number generator" }
}

CLI_addParamDictAndType_enum("indexing_mode", "Indexing mode", help_hash_indexing_mode, hash_indexing_mode_enum)

--------------------------------------------
-- command registration: ingress LAG index-mode
--------------------------------------------
CLI_addCommand("interface", "ingress lag index-mode", {
    func=hash_index_mode_set_func,
    help=help_ingress_hash_indexing_mode_set,
    params = {
        { type = "values", "%indexing_mode"}
    }
})

local help_ingress_hash_indexing_mode_show = "Show designated port table indexing modes"
CLI_addHelp("exec", "show ingress lag index-mode", help_ingress_hash_indexing_mode_show)

local function ingress_hash_index_mode_show_port(command_data, devNum, portNum, params)
    local mode
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxIngressHashPortIndexModeGet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
            { "OUT", "CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT", "indexModePtr" }
        })
    if isError then
        local error_string = "ERROR calling function cpssPxIngressHashPortIndexModeGet"
        command_data:addErrorAndPrint(error_string)
        return nil
    end
    if "CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E" == values["indexModePtr"] then
        mode = "hash"
    elseif "CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E" == values["indexModePtr"] then
        mode = "prng"
    end
    command_data["result"] = string.format("%d/%-8d %s", devNum, portNum, mode)
    command_data:addResultToResultArray()
end

local function ingress_hash_index_mode_show_func(params)
    local command_data = Command_Data()
    local head_line0 = "Dev/Port   Hash mode\n"
    local head_line1 = "----------------------"
    params["header_string"] = head_line0..head_line1
    params["footer_string"] = "\n"
    params["all_interfaces"] = "all"
    params["command_data"] = command_data
    if params["all_device"] ~= "all" then
        params["devID"] = params["all_device"]
    end
    generic_all_ports_show_func(ingress_hash_index_mode_show_port, params)
    command_data:updateEntries()
end

--------------------------------------------
-- command registration: show ingress LAG index mode
--------------------------------------------
CLI_addCommand("exec", "show ingress lag index-mode", {
    func=ingress_hash_index_mode_show_func,
    help=help_ingress_hash_indexing_mode_show,
    params = {
        { type = "named",
            "#all_device",
            mandatory = { "all_device" }
        },
    }
})

local help_ingress_hash_designated_set = "Set LAG designated ports table entries"
CLI_addHelp("config", "ingress lag designated", help_ingress_hash_designated_set)
local help_ingress_hash_designated_unset = "Set LAG designated ports table entries to default value (all port are members)"
CLI_addHelp("config", "no ingress lag designated", help_ingress_hash_designated_unset)

local function luaWrap_cpssPxIngressHashDesignatedPortsEntrySet(command_data, dev, entry, portBmp)
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxIngressHashDesignatedPortsEntrySet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", dev },
            { "IN", "GT_U32", "entryIndex", entry},
            { "IN", "CPSS_PX_PORTS_BMP", "portsBmp", portBmp }
        })
    if isError then
        local error_string = "ERROR calling function cpssPxIngressHashDesignatedPortsEntrySet"
        command_data:addErrorAndPrint(error_string)
        return nil
    end
    return isError
end

local function ingress_hash_designated_set_func(params)
    local command_data = Command_Data()
    local devices, devNum
    local portBmp
    local iterator, index
    local j

    if (params["all_device"]=="all") then
        devices = generic_getDeviceOfFamilyList(params["all_device"],
            {"CPSS_PX_FAMILY_PIPE_E"});
    else
        devices={params["all_device"]}
    end

    if params["flagNo"] == true then
        portBmp = 0x1FFFF
    else
        portBmp = params["entry"]
    end

    for j=1, #devices do
        devNum = devices[j]
        for iterator, index in number_range_iterator(params["indexes"]) do
            isError = luaWrap_cpssPxIngressHashDesignatedPortsEntrySet(
                command_data, devNum, index, portBmp)
            if isError then
                return
            end
        end
    end
end

CLI_type_dict["port_mask_hex"] = {
    checker = function(s, name, desc)
        local valid, value = CLI_check_param_hexnum(s, name, desc)
        if (valid == true) then
            if(value < 0x20000) then --TODO: get a constant
                return true, value
            else
                return false, "Port's bitmap is out of range"
            end
        else
            return false, value
        end
    end,
    max = 5 ,
    min = 0 ,
  help = "Hex bitmap of ports (with CPU)"
}

local function check_dpt_index_range(param)
    local err = "Index is out of designated port table range";
    local revision = wrlCpssDeviceRevisionGet(devEnv.dev)
    local max_index = 127
    if revision >= 1 then
        max_index = 255
    end
    for num in string.gmatch(param, "%d+") do
        if(tonumber(num) > max_index) then --127/255 is maximal index in designated port table
            return false, err
        end
    end
    return check_param_number_range(param)
end

CLI_type_dict["dpt_index_range"] = {
    checker = check_dpt_index_range,
    help = "The range of numbers. Example: 2-4,6 means 2,3,4,6 (0..127, 0..255 - revision A1)"
}

CLI_addCommand("config", "ingress lag designated", {
    func=ingress_hash_designated_set_func,
    help=help_ingress_hash_designated_set,
    params = {
        { type = "named",
            "#all_device",
            { format= "indexes %dpt_index_range", name = "indexes", help = "Designated port table index range"},
            { format= "entry %port_mask_hex", name = "entry", help = "Bitmap of ports in hex, 17 bits (with CPU port)"},
            mandatory = { "all_device", "indexes", "entry" }
        }
    }
})

CLI_addCommand("config", "no ingress lag designated", {
    func=function(params)
        params["flagNo"]= true
        return ingress_hash_designated_set_func(params)
    end,
    help=help_ingress_hash_designated_unset,
    params = {
        { type = "named",
            "#all_device",
            { format= "indexes %dpt_index_range", name = "indexes", help = "Designated port table index range"},
            mandatory = { "all_device", "indexes"}
        }
    }
})

help_ingress_hash_dpt_show = "Show LAG designated port table"
CLI_addHelp("exec", "show ingress lag designated-table", help_ingress_hash_dpt_show)

local function ingress_hash_dpt_show_dev(command_data, devNum, params)
    local iterator, index

    command_data["result"] =
        string.format("\nDevice %d\n", devNum)..
        "Idx/Port| 0| 1| 2| 3| 4| 5| 6| 7| 8| 9|10|11|12|13|14|15|16|\n"..
        "------------------------------------------------------------"
    command_data:addResultToResultArray()

    for iterator, index in number_range_iterator(params["indexes"]) do
        local isError, result, values =
            genericCpssApiWithErrorHandler(command_data,"cpssPxIngressHashDesignatedPortsEntryGet", {
                { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
                { "IN", "GT_U32", "portNum", index},
                { "OUT", "CPSS_PX_PORTS_BMP", "portsBmpPtr", portBmpPtr }
            })
        if isError then
            local error_string = "ERROR calling function cpssPxIngressHashDesignatedPortsEntryGet"
            command_data:addErrorAndPrint(error_string)
            return nil

        end
        local bitmap = values["portsBmpPtr"]

        command_data["result"] = string.format(
            "%-8d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|%-2d|",
            index,
            bit_and(bit_shr(bitmap, 0),1),  -- port0
            bit_and(bit_shr(bitmap, 1),1),  -- port1
            bit_and(bit_shr(bitmap, 2),1),  -- port2
            bit_and(bit_shr(bitmap, 3),1),  -- port3
            bit_and(bit_shr(bitmap, 4),1),  -- port4
            bit_and(bit_shr(bitmap, 5),1),  -- port5
            bit_and(bit_shr(bitmap, 6),1),  -- port6
            bit_and(bit_shr(bitmap, 7),1),  -- port7
            bit_and(bit_shr(bitmap, 8),1),  -- port8
            bit_and(bit_shr(bitmap, 9),1),  -- port9
            bit_and(bit_shr(bitmap, 10),1), -- port10
            bit_and(bit_shr(bitmap, 11),1), -- port11
            bit_and(bit_shr(bitmap, 12),1), -- port12
            bit_and(bit_shr(bitmap, 13),1), -- port13
            bit_and(bit_shr(bitmap, 14),1), -- port14
            bit_and(bit_shr(bitmap, 15),1), -- port15
            bit_and(bit_shr(bitmap, 16),1)  -- port16
        )
        command_data:addResultToResultArray()
    end
    command_data:updateEntries()
end

local function ingress_hash_dpt_show_func(params)
    if not params["indexes"] then
        local valid
        valid, params["indexes"] = check_param_number_range("0-127")
    end
    generic_all_device_show_func(ingress_hash_dpt_show_dev, params)
end

CLI_addCommand("exec", "show ingress lag designated-table", {
    func=ingress_hash_dpt_show_func,
    help=help_ingress_hash_dpt_show,
    params = {
        { type = "named",
            "#all_device",
            { format= "indexes %dpt_index_range", name = "indexes", help = "Designated port table index range"},
            mandatory = { "all_device" }
        }
    }
})

help_ingress_hash_mode_set = "Set hash calculation mode as CRC32 or as bit selection with defined offsets from the hash key for selected packet type"
CLI_addHelp("config", "ingress hash mode", help_ingress_hash_mode_set)
help_ingress_hash_mode_crc32_set = "CRC32 of hash key"
CLI_addHelp("config", "ingress hash mode crc32", help_ingress_hash_mode_crc32_set)
help_ingress_hash_mode_selected_set = "7 selected bits from hash key"
CLI_addHelp("config", "ingress hash mode selected-bits", help_ingress_hash_mode_selected_set)

local function ingress_hash_mode_func(params)
    local type_list
    local offsets
    local j, k
    local devices, devNum
    local iterator, index
    local status

    if params["packet_type"] then
        type_list = { params["packet_type"] }
    else
        type_list = { 0, 1, 2, 3, 4, 5, 6, 7, 8 } --All hash packet types
    end

    if params["mode"] == 1 then
        offsets = { params["bit0"], params["bit1"], params["bit2"],
                    params["bit3"], params["bit4"], params["bit5"],
                    params["bit6"] }
    else
        offsets = { 0, 0, 0, 0, 0, 0, 0 }
    end

    if (params["all_device"]=="all") then
        devices = generic_getDeviceOfFamilyList(params["all_device"],
            {"CPSS_PX_FAMILY_PIPE_E"});
    else
        devices={params["all_device"]}
    end

    for j=1, #devices do
        devNum = devices[j]
        for k=1, #type_list do
            local command_data = Command_Data()
            local status = wrlCpssPxIngressHashPacketTypeHashModeSet(devNum,
                type_list[k], params["mode"], offsets)
            if status ~= 0 then
                local error_string = "ERROR calling function cpssPxIngressHashPacketTypeHashModeSet"
                command_data:addErrorAndPrint(error_string)
                return
            end
        end
    end
end

CLI_addCommand("config", "ingress hash mode crc32", {
    func=function(params)
        params["mode"] = 0 -- CRC32
        return ingress_hash_mode_func(params)
    end,
    help=help_ingress_hash_mode_crc32_set,
    params = {
        { type = "named",
            "#all_device",
            { format= "hash-packet-type %hash_packet_type_enum", name = "hash-packet-type", help = "Hash packet type"},
            mandatory = { "all_device" }
        }
    }
})

CLI_type_dict["hk_bit_offset"] = {
    checker = function(value)
        val = tonumber(value)
        if (val > 343) or (val < 0) then -- 344 bits = 21 * sizeof(UDBP) + 8
            return false, "Bit offset is out of range"
        end
        return true, val
    end,
    help = "Hash key bit offset"
}

CLI_addCommand("config", "ingress hash mode selected-bits", {
    func=function(params)
        params["mode"] = 1 -- selected-bits
        return ingress_hash_mode_func(params)
    end,
    help=help_ingress_hash_mode_selected_set,
    params = {
        { type = "named",
            "#all_device",
            { format= "hash-packet-type %hash_packet_type_enum", name = "hash-packet-type", help = "Hash packet type"},
            { format= "bit0 %hk_bit_offset", name = "bit0", help = "Hash key bit 0 offset"},
            { format= "bit1 %hk_bit_offset", name = "bit1", help = "Hash key bit 1 offset"},
            { format= "bit2 %hk_bit_offset", name = "bit2", help = "Hash key bit 2 offset"},
            { format= "bit3 %hk_bit_offset", name = "bit3", help = "Hash key bit 3 offset"},
            { format= "bit4 %hk_bit_offset", name = "bit4", help = "Hash key bit 4 offset"},
            { format= "bit5 %hk_bit_offset", name = "bit5", help = "Hash key bit 5 offset"},
            { format= "bit6 %hk_bit_offset", name = "bit6", help = "Hash key bit 6 offset"},
            mandatory = { "all_device", "bit0", "bit1", "bit2", "bit3", "bit4", "bit5", "bit6" }
        }
    }
})

help_ingress_hash_mode_show = "Show hash calculation mode configuration"
CLI_addHelp("exec", "show ingress hash mode", help_ingress_hash_mode_show)

local function ingress_hash_mode_show_func(params)
    generic_all_device_show_func(ingress_hash_mode_show_dev, params)
end

function ingress_hash_mode_show_dev(command_data, devNum, params)
    local head_line0 = "Device "..to_string(devNum).."\n"
    local head_line1 = "--------------------------------------------------------\n"
    local head_line2 = "                                          Bits          \n"
    local head_line3 = "Packet type       Mode         0   1   2   3   4   5   6\n"
    local head_line4 = "--------------------------------------------------------"
    local status
    local hashMode
    local offsets
    local k, v
    local line
    local pt

    if params["packet_type"] then
        pt = params["packet_type"]
    else
        pt = -1
    end

    command_data["result"] = head_line0..head_line1..head_line2..head_line3..head_line4
    command_data:addResultToResultArray()

    for k,v in pairs(hptypes) do
        status = -1
        if pt > -1 then
            if k == pt then
                hash_packet_type = pt
                status, hashMode, offsets = wrlCpssPxIngressHashPacketTypeHashModeGet(devNum, k)
            end
        else
            status, hashMode, offsets = wrlCpssPxIngressHashPacketTypeHashModeGet(devNum, k)
        end

        if status == 0 then
            if hashMode == hash_indexing_mode_enum["hash"]["value"] then
                line = string.format("%-18s%-13s%-4s%-4s%-4s%-4s%-4s%-4s%-4s",
                    v, "crc32", "x", "x", "x", "x", "x", "x", "x")
            else
                line = string.format("%-18s%-13s%-4s%-4s%-4s%-4s%-4s%-4s%-4s",
                    v, "selected",
                    to_string(offsets[1]), to_string(offsets[2]),
                    to_string(offsets[3]), to_string(offsets[4]),
                    to_string(offsets[5]), to_string(offsets[6]),
                    to_string(offsets[7]))
            end

            command_data["result"] = line
            command_data:addResultToResultArray()
        else
            if status > 0 then
                local error_string = "ERROR calling function wrlCpssPxIngressHashPacketTypeHashModeGet"
                command_data:addErrorAndPrint(error_string)
                return
            end
        end
    end

    command_data:updateEntries()
end

CLI_addCommand("exec", "show ingress hash mode", {
    func=ingress_hash_mode_show_func,
    help=help_ingress_hash_mode_show,
    params = {
        { type = "named",
            "#all_device",
            { format= "hash-packet-type %hash_packet_type_enum", name = "hash-packet-type", help = "Hash packet type"},
            mandatory = { "all_device" }
        }
    }
})

help_ingress_hash_key_config_set = "Set packet hash key UDBP configuration for selected packet hash key"
CLI_addHelp("config", "ingress hash key-config", help_ingress_hash_key_config_set)
help_ingress_hash_key_config_unset = "Set packet hash key UDBP configuration for selected packet hash key"
CLI_addHelp("config", "no ingress hash key-config", help_ingress_hash_key_config_unset)

CLI_type_dict["hash_udbp_index"] = {
    checker = function(s, name, desc)
        local value = tonumber(s)
        if value then
            if value > desc["max"] then
                return false, "Out of range"
            end

            if value < desc["min"] then
                return false, "Out of range"
            end
            return true, value
        else
            return false, "Not a number"
        end
    end,
    max = 20, -- Maximal index of UDBP
    min = 0,
    help = "UDBP index"
}

CLI_type_dict["hash_packet_offset"] = {
    checker = function(s, name, desc)
        local value = tonumber(s)
        if value then
            if value > desc["max"] then
                return false, "Out of range"
            end

            if value < desc["min"] then
                return false, "Out of range"
            end
            return true, value
        else
            return false, "Not a number"
        end
    end,
    max = 63, -- Maximal offset
    min = 0,
    help = "UDBP offset"
}

CLI_type_dict["hash_udbp_mask"] = {
    checker = function(s, name, desc)
        local value
        local retval = {}
        if string.len(s) ~= 4 then
            return false, "It is only 4 digit mask"
        end
        for i=1,#s do
            value = tonumber(string.sub(s, i, i))
            if not value then
                return false, "The mask contains at least one not allowed symbol"
            end
            if (value ~= 1) and (value ~= 0) then
                return false, "The mask contains at least one not allowed symbol"
            end
            if value == 1 then
                retval[i] = true
            else
                retval[i] = false
            end
        end
        return true, retval
    end,
    help = "UDBP nibble mask"
}

local function ingress_hash_key_config_set_func(params)
    local devNum, devices
    local packet_types
    local j, k, v
    local udbp_array
    local src_port
    local status

    if (params["all_device"]=="all") then
        devices = generic_getDeviceOfFamilyList(params["all_device"],
            {"CPSS_PX_FAMILY_PIPE_E"});
    else
        devices={params["all_device"]}
    end

    if params["packet_type"] then
        packet_types = {[params["packet_type"]] = hash_packet_type_c_enum[params["packet_type"]]}
    else
        packet_types = hash_packet_type_c_enum
    end

    if params["source_port"] then
        src_port = true
    else
        src_port = false
    end

    if params["flagNo"]then
        for j=1, #devices do
            local command_data = Command_Data()
            devNum = devices[j]
            udbp_array = {}
            for k=1,21 do -- 21 UDBP
                udbp_array[k] = {}
                udbp_array[k]["anchor"] = 0
                udbp_array[k]["offset"] = 0
                udbp_array[k]["nibbleMaskArr"] = {[0]=false, [1]=false, [2]=false, [3]=false}
            end
            for k,v in pairs(packet_types) do
                status = wrlCpssPxIngressHashPacketTypeEntrySet(devNum, v, udbp_array, false)
                if status ~= 0 then
                    local error_string = "ERROR calling function wrlCpssPxIngressHashPacketTypeEntrySet"
                        command_data:addErrorAndPrint(error_string)
                end
            end
        end
        return
    end

    for j=1, #devices do
        devNum = devices[j]
        for k, v in pairs(packet_types) do
            local command_data = Command_Data()
            local src_prt_en
            status, udbp_array, src_prt_en = wrlCpssPxIngressHashPacketTypeEntryGet(devNum, v)
            if status ~= 0 then
                local error_string = "ERROR calling function wrlCpssPxIngressHashPacketTypeEntryGet"
                command_data:addErrorAndPrint(error_string)
                return
            end
            local idx = params["udbp_index"] + 1
            udbp_array[idx]["offset"] = params["offset"]
            if params["anchor"] == 0 then
                udbp_array[idx]["anchor"] = "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E"
            elseif params["anchor"] == 1 then
                udbp_array[idx]["anchor"] = "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E"
            elseif params["anchor"] == 2 then
                udbp_array[idx]["anchor"] = "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E"
            end
            udbp_array[idx]["nibbleMaskArr"] =
                { [0] = params["mask"][1], [1] = params["mask"][2],
                  [2] = params["mask"][3], [3] = params["mask"][4] }

            status = wrlCpssPxIngressHashPacketTypeEntrySet(devNum, v, udbp_array, src_port)
            if status ~= 0 then
                local error_string = "ERROR calling function wrlCpssPxIngressHashPacketTypeEntrySet"
                command_data:addErrorAndPrint(error_string)
                return
            end
        end
    end
end

CLI_addCommand("config", "ingress hash key-config", {
    func=ingress_hash_key_config_set_func,
    help=help_ingress_hash_key_config_set,
    params = {
        { type = "named",
            "#all_device",
            { format= "packet-type %hash_packet_type_enum", name = "packet_type", help = "Hash packet type"},
            { format= "udbp-index %hash_udbp_index", name = "udbp_index", help = "UDBP index (0..21)"},
            { format= "anchor %udb_anchor_type_enum", name = "anchor", help = "Anchor for the offset (L2, L3, L4)"},
            { format= "offset %hash_packet_offset", name = "offset", help = "Offset in bytes for the UDBP (0..63)"},
            { format= "mask %hash_udbp_mask", name = "mask", help = "Nibble mask written in binary manner, i.e. 1011"},
            { format= "source-port", name = "source_port", help = "Use source port in hash key"},
            mandatory = { "all_device", "udbp_index", "anchor", "offset", "mask" }
        },
    }
})

CLI_addCommand("config", "no ingress hash key-config", {
    func=function(params)
        params["flagNo"]= true
        return ingress_hash_key_config_set_func(params)
    end,
    help=help_ingress_hash_key_config_set,
    params = {
        { type = "named",
            "#all_device",
            { format= "packet-type %hash_packet_type_enum", name = "packet_type", help = "Hash packet type"},
            mandatory = { "all_device" }
        },
    }
})

help_ingress_hash_key_config_show = "Show packet hash key UDBP configuration"
CLI_addHelp("exec", "show ingress hash key-config", help_ingress_hash_key_config_show)

local function ingress_hash_key_config_show_func(params)
    generic_all_device_show_func(ingress_hash_key_config_show_dev, params)
end


function ingress_hash_key_config_show_dev(command_data, devNum, params)
    local j, k, v
    local head_line1 = "UDBP   Anchor   Offset   Mask\n"
    local head_line2 = "------------------------------"
    local packet_types

    if params["packet_type"] == nil then
        packet_types = hptypes
    else
        packet_types = { [params["packet_type"]] = hptypes[params["packet_type"]]}
    end

    for j,v in pairs(packet_types) do
        local head_line0 = string.format("Device %d - %s\n", devNum, v)
        command_data["result"] = head_line0..head_line1..head_line2
        command_data:addResultToResultArray()
        local status, udbp_array, src_port = wrlCpssPxIngressHashPacketTypeEntryGet(devNum, hash_packet_type_c_enum[j])
        if status ~= 0 then
            local error_string = "ERROR calling function wrlCpssPxIngressHashPacketTypeEntryGet"
            command_data:addErrorAndPrint(error_string)
            return
        end
        for k = 1,#udbp_array do
            local anchor
            if udbp_array[k]["anchor"] == "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E" then
                anchor = "L2"
            elseif udbp_array[k]["anchor"] == "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E" then
                anchor = "L3"
            elseif udbp_array[k]["anchor"] == "CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E" then
                anchor = "L4"
            end

            local bit0 = udbp_array[k]["nibbleMaskArr"][0] and 1 or 0
            local bit1 = udbp_array[k]["nibbleMaskArr"][1] and 1 or 0
            local bit2 = udbp_array[k]["nibbleMaskArr"][2] and 1 or 0
            local bit3 = udbp_array[k]["nibbleMaskArr"][3] and 1 or 0
            command_data["result"] = string.format("%-7d%-9s%-9d%d%d%d%d",
                k, anchor, udbp_array[k]["offset"], bit0, bit1, bit2, bit3)
            command_data:addResultToResultArray()
        end
        command_data["result"] = string.format("Usage of source port in hash key is %s\n\n",
            src_port and "enabled" or "disabled")
        command_data:addResultToResultArray()
    end
    command_data:updateEntries()
end

CLI_addCommand("exec", "show ingress hash key-config", {
    func=ingress_hash_key_config_show_func,
    help=help_ingress_hash_key_config_show,
    params = {
        { type = "named",
            "#all_device",
            { format= "packet-type %hash_packet_type_enum", name = "packet_type", help = "Hash packet type"},
            mandatory = { "all_device" }
        },
    }
})
