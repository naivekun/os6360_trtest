--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pipe_ingress.lua
--*
--* DESCRIPTION:
--*       Pipe device: Ingress Configuration
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[
    ********* Global commands
    CLI_addCommand("config", "ingress tpid", {
    CLI_addCommand("config", "no ingress tpid", {

    ********* Per-port commands
    CLI_addCommand("interface", "ingress port-packet-type-key", {

    ********* Per packet-type commands
    CLI_addCommand("packet-type-key", "key-entry", {
    CLI_addCommand("packet-type-key", "format-entry", {

    ********* Show commands
    CLI_addCommand("exec", "show ingress tpid", {
    CLI_addCommand("exec", "show ingress port-packet-type-key", {
    CLI_addCommand("exec", "show ingress packet-type-key", {
    CLI_addCommand("exec", "show ingress packet-type-format", {
    CLI_addCommand("exec", "show ingress packet-type-error", {
    CLI_addCommand("exec", "show ingress port-map-entry", {
]]--

-- get Port Packet Type key
function luaWrap_cpssPxIngressPortPacketTypeKeyGet(command_data, devNum, portNum)
    local apiName = "cpssPxIngressPortPacketTypeKeyGet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum", portNum},
        {"OUT","CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC","portKey"}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressPortPacketTypeKeyGet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError, values.portKey
end

-- get Packet Type Key entry
function luaWrap_cpssPxIngressPacketTypeKeyEntryGet(command_data, devNum, packetType)
    local apiName = "cpssPxIngressPacketTypeKeyEntryGet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_U32","packetType", packetType},
        {"OUT","CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC","keyData"},
        {"OUT","CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC","keyMask"}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressPacketTypeKeyEntryGet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError, values.keyData, values.keyMask
end

-- set Packet Type Key entry
function luaWrap_cpssPxIngressPacketTypeKeyEntrySet(command_data, devNum, packetType, keyData, keyMask)
    local apiName = "cpssPxIngressPacketTypeKeyEntrySet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_U32","packetType", packetType},
        {"IN","CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC","keyData", keyData},
        {"IN","CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC","keyMask", keyMask}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressPacketTypeKeyEntrySet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError
end

-- get TPID entry
local function luaWrap_cpssPxIngressTpidEntryGet(command_data, devNum, entryIndex)
    local apiName = "cpssPxIngressTpidEntryGet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_U32","entryIndex", entryIndex},
        {"OUT","CPSS_PX_INGRESS_TPID_ENTRY_STC","tpidEntry"}
    })
    
    if isError then
        local error_string = "ERROR calling function cpssPxIngressTpidEntryGet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError, values.tpidEntry
end

-- get Packet Type Error
local function luaWrap_cpssPxIngressPacketTypeErrorGet(command_data, devNum)
    local apiName = "cpssPxIngressPacketTypeErrorGet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"OUT","GT_U32","errorCounter"},
        {"OUT","CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC","errorKeyPtr"}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressPacketTypeErrorGet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError, values.errorCounter, values.errorKeyPtr
end

-- set TPID entry
local function luaWrap_cpssPxIngressTpidEntrySet(command_data, devNum, entryIndex, tpidEntry)
    local apiName = "cpssPxIngressTpidEntrySet"
    local isError, result = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_U32","entryIndex", entryIndex},
        {"IN","CPSS_PX_INGRESS_TPID_ENTRY_STC","tpidEntry", tpidEntry}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressTpidEntrySet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError
end

-- set Port Packet Type key
function luaWrap_cpssPxIngressPortPacketTypeKeySet(command_data, devNum, portNum, portKey)
    local apiName = "cpssPxIngressPortPacketTypeKeySet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum", portNum},
        {"IN","CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC","portKey", portKey}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressPortPacketTypeKeySet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError
end


-- get Port Map Packet Type Format Entry
function luaWrap_cpssPxIngressPortMapPacketTypeFormatEntryGet(command_data, devNum, tableType, packetType)
    local apiName = "cpssPxIngressPortMapPacketTypeFormatEntryGet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT","tableType", tableType},
        {"IN","GT_U32","packetType", packetType},
        {"OUT","CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC","packetTypeFormat"}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressPortMapPacketTypeFormatEntryGet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError, values.packetTypeFormat
end

-- set Port Map Packet Type Format Entry
function luaWrap_cpssPxIngressPortMapPacketTypeFormatEntrySet(command_data, devNum, tableType, packetType, packetTypeFormat)
    local apiName = "cpssPxIngressPortMapPacketTypeFormatEntrySet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT","tableType", tableType},
        {"IN","GT_U32","packetType", packetType},
        {"IN","CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC","packetTypeFormat", packetTypeFormat}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressPortMapPacketTypeFormatEntryGet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError
end

-- get Get Port Map Table Entry
function luaWrap_cpssPxIngressPortMapEntryGet(command_data, devNum, tableType, entryIndex)
    local apiName = "cpssPxIngressPortMapEntryGet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT","tableType", tableType},
        {"IN","GT_U32","entryIndex", entryIndex},
        {"OUT","CPSS_PX_PORTS_BMP","portsBmp"},
        {"OUT","GT_BOOL","bypassLagDesignatedBitmapPtr"}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxIngressPortMapEntryGet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError, values.portsBmp
end

local help_string_ingress = "Ingress processing commands"
local help_string_ingress_tpid = "Configure TPID table entries"
local help_string_ingress_port_packet_type = "Set port packet type key generation info"
local help_string_ingress_packet_type = "Set packet type key generation info"
local help_string_ingress_packet_type_format = "Set packet type format info"
local help_string_ingress_packet_type_error = "Show packet type error"

CLI_addHelp("config", "no ingress", help_string_ingress)
CLI_addHelp("config", "ingress", help_string_ingress)
CLI_addHelp("config", "ingress tpid", help_string_ingress_tpid)

CLI_addHelp("interface", "ingress", help_string_ingress .. " per port configuration")
CLI_addHelp("interface", "ingress port-packet-type-key", help_string_ingress_packet_type_error)

CLI_addHelp("exec", "show ingress packet-type-error", help_string_ingress_packet_type_error)

local function tpid_func(params)
    local tpidEntry
    local command_data = Command_Data()
    local devlist, entryIndex, etherType, tagSize, enable

    entryIndex = tonumber(params.entry_index)

    if params.devID ~= "all" then
        devlist = {tonumber(params.devID)}
    else
        devlist = wrLogWrapper("wrlDevList")
    end

    for i, devNum in pairs(devlist) do
        local isError, tpidEntry = luaWrap_cpssPxIngressTpidEntryGet(command_data, devNum, entryIndex)
        if isError then
            return
        end
        
        if params.flagNo == nil then
            tpidEntry.valid = true
        else
            tpidEntry.valid = false
        end

        if params.tag_size ~= nil then
            tpidEntry.size = tonumber(params.tag_size)
        end 
        if params.ether_type ~= nil then
            tpidEntry.val = params.ether_type
        end

        luaWrap_cpssPxIngressTpidEntrySet(command_data, devNum, entryIndex, tpidEntry)
    end
end

--------------------------------------------
-- command registration: ingress tpid
--------------------------------------------
CLI_addCommand("config", "ingress tpid", {
  func   = tpid_func,
  help   = help_string_ingress_tpid,
  params = {
      { type = "named",
          { format= "device %devID_all", name="devID", help="The device number" },
          { format= "index %GT_U32",     name = "entry_index",   help = "Entry index for TPID table"},
          { format= "ethertype %ethertype", name = "ether_type",    help = "ETAG: TPID (etherType) 16 bits field"},
          { format= "tag-size  %GT_U32", name = "tag_size",    help = "TPID size in bytes"},
        mandatory = { "devID", "entry_index", "ether_type"}
      }
  }
})

--------------------------------------------
-- command registration: no ingress tpid
--------------------------------------------
CLI_addCommand("config", "no ingress tpid", {
func=function(params)
    params.flagNo=true
    return tpid_func(params)
  end,
  help="Invalidate TPID table entry",
  params = {
      { type = "named",
          { format= "device %devID_all", name="devID", help="The device number" },
          { format= "index %GT_U32", name = "entry_index",   help = "Entry index for TPID table"},
        mandatory = { "devID"}
      }
  }
}) 


udb_anchor_type_help = "UDB anchor type"
local udb_anchor_type_enum = {
    ["L2"]  = { value = 0, help = "L2 Anchor - start of packet" },
    ["L3"]  = { value = 1, help = "L3 Anchor - start of L3 header" },
    ["L4"]  = { value = 2, help = "L4 Anchor - start of L4 header" }
}
CLI_addParamDictAndType_enum("udb_anchor_type_enum","UDBP key", udb_anchor_type_help, udb_anchor_type_enum)

local function port_packet_type_func_body(command_data, devNum, portNum, params) 

    local isError, portKey = luaWrap_cpssPxIngressPortPacketTypeKeyGet(command_data, devNum, portNum)
    if isError then
        return
    end

    if params["profile_index"] ~= nil then
        portKey.srcPortProfile = params["profile_index"]
    end

    if params["udb_index"] ~= nil then
        local i = params["udb_index"]
        portKey.portUdbPairArr[i].udbAnchorType = params["anchor_type"]
        portKey.portUdbPairArr[i].udbByteOffset = params["byte_offset"]
    end

    luaWrap_cpssPxIngressPortPacketTypeKeySet(command_data, devNum, portNum, portKey)
end

local function port_packet_type_func(params)
    return generic_port_range_func(port_packet_type_func_body, params)
end

--------------------------------------------
-- command registration: port-packet-type-key
--------------------------------------------
CLI_addCommand("interface", "ingress port-packet-type-key", {
  func   = port_packet_type_func,
  help   = help_string_ingress_port_packet_type,
  params = {
      { type = "named",
          { format= "profile  %GT_U32",     name = "profile_index",   help = "Source port profile"},
          { format= "udbp %GT_U32", name = "udb_index",    help = "UDBP index"},
          { format= "anchor %udb_anchor_type_enum", name = "anchor_type",    help = "The UDB anchor type in the per-port packet type key"},
          { format= "byte-offset %GT_U32", name = "byte_offset",    help = "Byte offset relative to Anchor Type"},
          requirements = {
                  anchor_type = {"udb_index"},
                  byte_offset = {"udb_index"}
              }
      }
  }
})

local function addPacketTypeKey(params)
    local devNum, packetTypeList, keyPart
    local entry = {udbp = duplicate(udbPairsEmptyArr)}
    local devices, j
    local command_data = Command_Data()
    local packetTypeKey, keyData, keyMask
    local udbIdx, udbVal

    devNum = getGlobal("g_pipeIDDev")
    -- (devNum=="all") or passed device number
    devices = generic_getDeviceOfFamilyList(devNum, {"CPSS_PX_FAMILY_PIPE_E"});
    if not next(devices) then
        -- no devices to treat
        return true, "no devices to treat"
    end

    local direction = getGlobal("g_pipeDirection")
    if direction ~= "ingress" then
        return false,"ingress direction only supported"
    end
    packetTypeList = getGlobal("g_pipePacketType")

    keyPart = params["packet_type_entry_part_enum"]

    for index, packetType in pairs(packetTypeList) do
        for j=1, #devices do
            devNum = devices[j]
            -- get key data and mask
            local isError, keyData, keyMask = luaWrap_cpssPxIngressPacketTypeKeyEntryGet(command_data, devNum, packetType)
            if isError then
                return
            end

            local packetKeyPart
            if keyPart == 0 then
                packetTypeKey = keyData
            else
                packetTypeKey = keyMask
            end

            -- set part of key
            if params["macDa"] ~= nil then
                packetTypeKey.macDa=params["macDa"]
            end
            if params["ethertype"] ~= nil then
                packetTypeKey.etherType=params["ethertype"]
            end
            if params["llc_non_snap"] ~= nil then
                packetTypeKey.isLLCNonSnap=params["llc_non_snap"]
            end
            if params["profile_index"] then
                packetTypeKey.profileIndex=params["profile_index"]
            end
            if params["ip2me_index"] then
                packetTypeKey.ip2meIndex=params["ip2me_index"]
            end

            for i = 0, 3 do
                udbIdx = string.format("udb_%s", i) 
                if params[udbIdx] ~= nil then
                    udbVal = params[udbIdx]
                    packetTypeKey.udbPairsArr[i].udb[0]=udbVal/256
                    packetTypeKey.udbPairsArr[i].udb[1]=udbVal%256
                end
            end

            luaWrap_cpssPxIngressPacketTypeKeyEntrySet(command_data, devNum, packetType, keyData, keyMask)
        end 
    end
end

packet_type_entry_help = "Packet type key data/mask"
local packet_type_entry_part_enum = {
    ["data"]  = { value = 0, help = "17 bytes packet type key data" },
    ["mask"]  = { value = 1, help = "17 bytes packet type key mask" }
}
CLI_addParamDictAndType_enum("packet_type_entry_part_enum","Packet type key", packet_type_entry_part_help, packet_type_entry_part_enum)

udb_bytes_pair_help = "The 2 consecutive UDB bytes. Format : 'HEX' number(0x021A/0x0014)"

CLI_type_dict["udb_bytes_pair"] = {
    checker = CLI_check_param_hexnum,
    min=4,
    max=4,
    help=udb_bytes_pair_help
} 

CLI_type_dict["ethertype"] = {
    checker = CLI_check_param_hexnum,
    min=4,
    max=4,
    help="The ethertype of the CN messages , mandatory two bytes in hex (i.e. 0x8100, etc)"
}

CLI_type_dict["ip2me_matched_index"] = {
    checker = CLI_check_param_number,
    min=0,
    max=7,
    help="Matched IP2ME index 0..7"
}

--------------------------------------------
-- command registration: key-entry
--------------------------------------------
CLI_addCommand("packet-type-key", "key-entry", {
  func=addPacketTypeKey,
  help = "Create an packet type key entry",
  params={
    {type="values", "%packet_type_entry_part_enum", name="key_part"},
    {type="named",
        { format= "mac-destination %mac-address", name="macDa", help= "The destination mac" },
        { format= "ethertype %ethertype", name = "ethertype", help = "Ethernet type"},
        { format= "llc-non-snap  %enable_disable", name = "llc_non_snap", help = "Packet is LLC non-SNAP"},
        { format= "source-port-profile %GT_U32", name = "profile_index", help = "Source port profile index"},
        { format= "udbp-0 %udb_bytes_pair", name = "udb_0",    help = "2 UDB bytes"},
        { format= "udbp-1 %udb_bytes_pair", name = "udb_1",    help = "2 UDB bytes"},
        { format= "udbp-2 %udb_bytes_pair", name = "udb_2",    help = "2 UDB bytes"},
        { format= "udbp-3 %udb_bytes_pair", name = "udb_3",    help = "2 UDB bytes"},
        { format= "ip2me-index %ip2me_matched_index", name = "ip2me_index",    help = "Matched IP2ME index"}
    }
  }
})

--------------------------------------------
-- command registration: no key-entry
--------------------------------------------
CLI_addCommand("packet-type-key", "no key-entry", {
    func=function(params)
        params["udb_0"] = 0
        params["udb_1"] = 0
        params["udb_2"] = 0
        params["udb_3"] = 0
        params["ip2me_index"]   = 0
        params["llc_non_snap"]  = false
        params["profile_index"] = 0
        params["ethertype"]     = 0
        params["macDa"] = {
            "00", "00", "00", "00", "00", "00",
            isMulticast = false,
            string      = "00:00:00:00:00:00",
            isBroadcast = false,
            isUnicast   = true
        }
        params["packet_type_entry_part_enum"] = 0
        addPacketTypeKey(params)
        params["packet_type_entry_part_enum"] = 1
        addPacketTypeKey(params)
    end,
    help = "Remove an packet type key entry",
})

format_entry_help = "Packet type key format table entry"
local format_entry_enum= {
    ["source"]  = { value = 0, help = "Source port map table related info" },
    ["destination"]  = { value = 1, help = "Destination port map table related info" }
}
CLI_addParamDictAndType_enum("format_entry_enum","Format table entry", format_entry_help, format_entry_enum)

local function addPacketTypeFormat(params)
    local devNum, packetTypeList, tableType, i
    local entry, entryTable
    local command_data = Command_Data()
    local devices, j

    devNum = getGlobal("g_pipeIDDev")
    -- (devNum=="all") or passed device number
    devices = generic_getDeviceOfFamilyList(devNum, {"CPSS_PX_FAMILY_PIPE_E"});
    if not next(devices) then
        -- no devices to treat
        return true;
    end

    local direction = getGlobal("g_pipeDirection")
    if direction ~= "ingress" then
        return false,"ingress direction only supported"
    end

    packetTypeList = getGlobal("g_pipePacketType")

    tableType = params["format_entry_enum"]

     
    for index, packetType in pairs(packetTypeList) do
        for j=1, #devices do
            devNum = devices[j]
            local isError, entryTable = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntryGet(command_data, devNum, tableType, packetType)
            if isError then
                return
            end

            if params["const_idx"] ~= nil then
                entryTable.indexConst = params["const_idx"]
            end
            if params["max_idx"] ~= nil then
                entryTable.indexMax = params["max_idx"]
            end
            if params["bit_field_idx"] ~= nil then
                i=params["bit_field_idx"]
                entryTable.bitFieldArr[i].byteOffset = params["byte_offset"]
                entryTable.bitFieldArr[i].startBit = params["start_bit"]
                entryTable.bitFieldArr[i].numBits = params["num_bits"]
            end

            luaWrap_cpssPxIngressPortMapPacketTypeFormatEntrySet(command_data, devNum, tableType, packetType, entryTable)
        end 
    end
end

--------------------------------------------
-- command registration: format-entry
--------------------------------------------
CLI_addCommand("packet-type-key", "format-entry", {
  func=addPacketTypeFormat,
  help = "Create an packet type format entry",
  params={
    {type="values", "%format_entry_enum"},
    {type="named",
        { format= "constant-index %number", name="const_idx", help="Signed constant used to compute the Source/Destination index" },
        { format= "max-index %GT_U32", name="max_idx", help="The maximum acceptable value for the resulting Source/Destination index" },
        { format= "bit-field %GT_U32", name="bit_field_idx", help="Index of bit offset structure" },
        { format= "byte-offset %GT_U32", name="byte_offset", help="The Byte Offset relative to the start of the packet" },
        { format= "start-bit %GT_U32", name="start_bit", help="The start bit in the byte specified by the Byte Offset" },
        { format= "num-bits %GT_U32", name="num_bits", help="The number of bits to take starting from the start bit" },
        requirements = {
                byte_offset = {"bit_field_idx"},
                start_bit = {"bit_field_idx"},
                num_bits = {"bit_field_idx"}
            }
    }
  }
})


-------------------------------------------------
-- show ingress pipe commands
-------------------------------------------------
CLI_addHelp("exec", "show ingress", "Show the ingress info")
CLI_addHelp("exec", "show ingress tpid", "Show the ingress TPID")
CLI_addHelp("exec", "show ingress port-packet-type-key", "Show the ingress port packet type")


local function show_all_tpid(command_data,devNum,params)
    local values, entryIndex

    for entryIndex = 0, 3 do
        local isError, tpidEntry = luaWrap_cpssPxIngressTpidEntryGet(command_data, devNum, entryIndex)
        if isError then
            return
        end
        command_data["result"] =
                        string.format("%d\t\t%d\t\t0x%04x\t\t%d\t\t%s", devNum, entryIndex, tpidEntry.val, tpidEntry.size, tostring(tpidEntry.valid))

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end
end

local function show_tpid(params)
    local header_string, footer_string

    params.header_string = 
    "Dev             Index           Ethertype       Tag size        Enable\n" .. 
    "------          --------        ----------      ---------       ------"
    params.footer_string = "\n"

    generic_all_device_show_func(show_all_tpid, params)
end

--------------------------------------------
-- command registration: show ingress tpid
--------------------------------------------
CLI_addCommand("exec", "show ingress tpid", {
  func   = show_tpid,
  help   = "Show tpid device",
  params = {
      { type = "named",
          "#all_device",
        mandatory = { "all_device" }
      }
  }
})

local function show_port_packet_type_key_body(command_data, devNum, portNum, params)
    local devPortStr = tostring(devNum).."/"..tostring(portNum)
    
    local isError, portKey = luaWrap_cpssPxIngressPortPacketTypeKeyGet(command_data, devNum, portNum)
    if isError then
        return
    end

    local udbAnchorType = {["CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E"] = "L2", ["CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E"] = "L3"}
    udbAnchorType0 = udbAnchorType[portKey.portUdbPairArr[0].udbAnchorType]
    udbAnchorType1 = udbAnchorType[portKey.portUdbPairArr[1].udbAnchorType]
    udbAnchorType2 = udbAnchorType[portKey.portUdbPairArr[2].udbAnchorType]
    udbAnchorType3 = udbAnchorType[portKey.portUdbPairArr[3].udbAnchorType]

    command_data["result"] =
        string.format("%-12s %-2d \t %-20s %-20s %-20s %-20s", devPortStr, portKey.srcPortProfile,
                      udbAnchorType0.."/".. tostring(portKey.portUdbPairArr[0].udbByteOffset),
                      udbAnchorType1.."/".. tostring(portKey.portUdbPairArr[1].udbByteOffset),
                      udbAnchorType2.."/".. tostring(portKey.portUdbPairArr[2].udbByteOffset),
                      udbAnchorType3.."/".. tostring(portKey.portUdbPairArr[3].udbByteOffset))

    command_data:addResultToResultArray()
end

local function show_port_packet_type_key(params)
    local header_string, footer_string

    header_string =
        "\n" ..
        "Dev/Port   Profile    UDBP0 Achr/Offset   UDBP1 Achr/Offset   UDBP2 Achr/Offset   UDBP3 Achr/Offset\n" ..
        "---------  -------    -----------------   ------------------- ------------------- -------------------"
    footer_string = "\n"    

    params.header_string = header_string
    params.footer_string = footer_string

    return generic_all_ports_show_func(show_port_packet_type_key_body, params)
end

----------------------------------------------------------
-- command registration: show ingress port-packet-type-key
----------------------------------------------------------
CLI_addCommand("exec", "show ingress port-packet-type-key", {
  func   = show_port_packet_type_key,
  help   = "Show port packet type key",
  params = {
        { type = "named",
            "#all_interfaces_ver1",
            mandatory = { "all_interfaces" }
        }
    }
})

local function mac_addr_2_str(macDa)
    if (type(macDa)=="string") then
        return macDa
    else
        return macDa.string
    end
end

local function show_packet_type_key_body(command_data,devNum,params)
    local values
    local macDa, macDaMask

    local packetTypeList = params.packet_type -- list of range/single or {"all"} 
    -- the packetType is 0..31
    local max_packet_type = 31

    if(packetTypeList == nil) then 
        packetTypeList = "all" 
    end

    if packetTypeList == "all" then         
        packetTypeList = {}
        for index = 0, max_packet_type do
            table.insert(packetTypeList, index)
        end
    end

    for index, packetType in pairs(packetTypeList) do
        local isError, keyData, keyMask = luaWrap_cpssPxIngressPacketTypeKeyEntryGet(command_data, tonumber(devNum), packetType)
        if isError then
            return
        end

        macDa = mac_addr_2_str(keyData.macDa)
        macDaMask = mac_addr_2_str(keyMask.macDa)

        udbp0 = keyData.udbPairsArr[0].udb[0] * 256 + keyData.udbPairsArr[0].udb[1]
        udbp1 = keyData.udbPairsArr[1].udb[0] * 256 + keyData.udbPairsArr[1].udb[1]
        udbp2 = keyData.udbPairsArr[2].udb[0] * 256 + keyData.udbPairsArr[2].udb[1]
        udbp3 = keyData.udbPairsArr[3].udb[0] * 256 + keyData.udbPairsArr[3].udb[1]

        -- MAC DA, Ether Type, LLC-Non-Snap,  Profile,  UDBP0,  UDBP1,   UDBP2,   UDBP3, IP@ME idx
        formatStr1 = "%s \t 0x%04x \t %s \t\t %02d \t 0x%04x  0x%04x  0x%04x  0x%04x %-11d"
        -- [D]ata, Device, Packet Type,
        formatStr = "D\t%d \t %d \t\t".. formatStr1
        command_data["result"] = string.format(formatStr, devNum, packetType,
                                               macDa,
                                               keyData.etherType,
                                               tostring(keyData.isLLCNonSnap),
                                               keyData.profileIndex, 
                                               udbp0, udbp1, udbp2, udbp3,
                                               keyData.ip2meIndex)
        command_data:addResultToResultArray()

        udbp0 = keyMask.udbPairsArr[0].udb[0] * 256 + keyMask.udbPairsArr[0].udb[1]
        udbp1 = keyMask.udbPairsArr[1].udb[0] * 256 + keyMask.udbPairsArr[1].udb[1]
        udbp2 = keyMask.udbPairsArr[2].udb[0] * 256 + keyMask.udbPairsArr[2].udb[1]
        udbp3 = keyMask.udbPairsArr[3].udb[0] * 256 + keyMask.udbPairsArr[3].udb[1]
        -- [M]ask
        formatStr = "M\t\t\t\t".. formatStr1
        command_data["result"] = string.format(formatStr,
                                               macDaMask, 
                                               keyMask.etherType,
                                               tostring(keyMask.isLLCNonSnap),
                                               keyMask.profileIndex, 
                                               udbp0, udbp1, udbp2, udbp3,
                                               keyMask.ip2meIndex)
        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end 

end


local function show_packet_type_key(params)
    local header_string, footer_string

    local head_line0 = "Data/   Device   Packet Type    Mac DA                  Ethertype       LLC-Non-Snap    Profile  UDBP0   UDBP1   UDBP2   UDBP3  IP2ME index\n"
    local head_line1 = "Mask    -----------------------------------------------------------------------------------------------------------------------------------"

    params.header_string = head_line0..head_line1
    params.footer_string = "\n"

    generic_all_device_show_func(show_packet_type_key_body, params)
end

----------------------------------------------------------
-- command registration: show ingress packet-type-key
----------------------------------------------------------
CLI_addCommand("exec", "show ingress packet-type-key", {
  func   = show_packet_type_key,
  help   = "Show packet type key",
  params = {
        { type = "named",
            "#all_device",
            { format = "packet-type %show_packetType",  name="packet_type", help = "Use single/range/'all' for packetType(s)"},
            mandatory = { "all_device", "packet_type" }
        }
    }
})

local function show_packet_type_format_body(command_data,devNum,params)
    local packetTypeList = params["packet_type"] -- list of range/single or {"all"} 
    local tableType = params["format_entry_enum"]
    -- the packetType is 0..31
    local max_packet_type = 31

    if(packetTypeList == nil) then 
        packetTypeList = "all" 
    end

    if packetTypeList == "all" then         
        packetTypeList = {}
        for index = 0, max_packet_type do
            table.insert(packetTypeList, index)
        end
    end

    for index, packetType in pairs(packetTypeList) do
        local isError, entryTable = luaWrap_cpssPxIngressPortMapPacketTypeFormatEntryGet(command_data, tonumber(devNum), tableType, packetType)
        if isError then
            return
        end

        -- Resulting string formatting and adding.
        local byteBitLen = string.format("%d/%d/%d\t\t %d/%d/%d\t\t %d/%d/%d\t\t %d/%d/%d",
                                         entryTable.bitFieldArr[0].byteOffset, 
                                         entryTable.bitFieldArr[0].startBit,
                                         entryTable.bitFieldArr[0].numBits, 
                                         entryTable.bitFieldArr[1].byteOffset, 
                                         entryTable.bitFieldArr[1].startBit,
                                         entryTable.bitFieldArr[1].numBits, 
                                         entryTable.bitFieldArr[2].byteOffset, 
                                         entryTable.bitFieldArr[2].startBit,
                                         entryTable.bitFieldArr[2].numBits, 
                                         entryTable.bitFieldArr[3].byteOffset, 
                                         entryTable.bitFieldArr[3].startBit,
                                         entryTable.bitFieldArr[3].numBits)

        command_data["result"] =
                        string.format("%d\t%d\t%d\t%d\t\t%s", devNum, packetType, entryTable.indexConst, entryTable.indexMax, byteBitLen)

        command_data:addResultToResultArray()
        command_data:updateEntries() 
    end

end

local function show_packet_type_format(params)
    local header_string, footer_string
    local tableName
    local tableType = params["format_entry_enum"]
    if tableType == 0 then
        tableName = "Source Packet Type Format Table\n"
    else
        tableName = "Destination Packet Type Format Table\n"
    end

    local head_line0 = "Dev PcktType ConstIdx MaxIdx  Fld0(Byte/Bit/Len)  Fld1(Byte/Bit/Len)  Fld2(Byte/Bit/Len)  Fld3(Byte/Bit/Len)\n"
    local head_line1 = "------------------------------------------------------------------------------------------------------------"
    params.header_string = tableName .. head_line0 .. head_line1
    params.footer_string = "\n"

    generic_all_device_show_func(show_packet_type_format_body, params)
end

--------------------------------------------------------
-- command registration: show ingress packet-type-format
---------------------------------------------------------
CLI_addCommand("exec", "show ingress packet-type-format", {
    func   = show_packet_type_format,
    help   = "Show packet type format",
    params = {
          { type="values", "%format_entry_enum"},
          { type = "named",
              "#all_device",
              { format = "packet-type %show_packetType",  name="packet_type", help = "Use single/range/'all' for packetType(s)"},
              requirements = { packet_type = { "all_device" } },
              mandatory = { "all_device"}
          }
      }
})

local function show_packet_type_error_body(command_data,devNum,params)
    local caption_str =   
    "Device             :   %d\n" .. 
    "Error Counter      :   %d\n" ..
    "MAC DA             :   %s\n" ..
    "Ethertype          :   0x%04X\n" ..
    "LLC-Non-Snap       :   %s\n" ..
    "Profile index      :   %d\n" ..
    "UDBP0              :   0x%04X\n" ..
    "UDBP1              :   0x%04X\n" ..
    "UDBP2              :   0x%04X\n" ..
    "UDBP3              :   0x%04X"


    local isError, errorCounter, errorKey = luaWrap_cpssPxIngressPacketTypeErrorGet(command_data, tonumber(devNum))
    if isError then
        return
    end

    udbp0 = errorKey.udbPairsArr[0].udb[0] * 256 + errorKey.udbPairsArr[0].udb[1]
    udbp1 = errorKey.udbPairsArr[1].udb[0] * 256 + errorKey.udbPairsArr[1].udb[1]
    udbp2 = errorKey.udbPairsArr[2].udb[0] * 256 + errorKey.udbPairsArr[2].udb[1]
    udbp3 = errorKey.udbPairsArr[3].udb[0] * 256 + errorKey.udbPairsArr[3].udb[1]

    command_data["result"] =
                    string.format(caption_str, 
                                  devNum, 
                                  errorCounter, 
                                  mac_addr_2_str(errorKey.macDa),
                                  errorKey.etherType, 
                                  tostring(errorKey.isLLCNonSnap), 
                                  errorKey.profileIndex, 
                                  udbp0, udbp1, udbp2, udbp3)

    command_data:addResultToResultArray()
    command_data:updateEntries() 
end

local function show_packet_type_error(params)
    local header_string, footer_string

    params.header_string = "\n"
    params.footer_string = "\n"

    generic_all_device_show_func(show_packet_type_error_body, params)
end

-----------------------------------------------
-- command registration: port-packet-type-error
-----------------------------------------------
CLI_addCommand("exec", "show ingress packet-type-error", {
func   = show_packet_type_error,
help   = "Show packet type error",
params = {
          { type = "named",
              "#all_device",
              mandatory = { "all_device"}
          }
      }
})

local function show_port_map_entry_body(command_data,devNum,params)
    local tableType = params["format_entry_enum"]
    local fromIndex = params["from_index"]
    local toIndex

    -- the port map table entry maximal index
    local max_port_map_entry = 0x2000
    if tableType == 0 then
        max_port_map_entry = 0x1000
    end

    if params["to_index"] == 0 then
        toIndex = max_port_map_entry
    else
        toIndex = params["to_index"]
    end

    if fromIndex > toIndex then
        fromIndex = toIndex
    end

    for entryIndex = fromIndex, toIndex do
        local isError, entryTable = luaWrap_cpssPxIngressPortMapEntryGet(command_data, tonumber(devNum), tableType, entryIndex)
        if isError then
            return
        end

        command_data["result"] =
                        string.format("%d\t\t%4d\t\t0x%05x", devNum, entryIndex, entryTable)

        command_data:addResultToResultArray()
        command_data:updateEntries()
    end

end

local function show_port_map_entry(params)
    local header_string, footer_string
    local tableName
    local tableType = params["format_entry_enum"]
    if tableType == 0 then
        tableName = "Source Port Map\n"
    else
        tableName = "Destination Port Map\n"
    end

    local head_line0 = "Device      Entry Index         Port Map\n"
    local head_line1 = "-----------------------------------------"
    params.header_string = tableName .. head_line0 .. head_line1
    params.footer_string = "\n"

    generic_all_device_show_func(show_port_map_entry_body, params)
end

-----------------------------------------------
-- command registration: port-map-entry
-----------------------------------------------
CLI_addCommand("exec", "show ingress port-map-entry", {
    func   = show_port_map_entry,
    help   = "Show port map table entry",
    params = {
          { type="values", "%format_entry_enum"},
          { type = "named",
              "#all_device",
              { format = "from %GT_U32",  name="from_index", help = "Range start index"},
              { format = "to %GT_U32",  name="to_index", help = "Range end index"},
              requirements = { from_index = { "all_device" },  to_index = {"from_index"}},
              mandatory = { "all_device", "from_index"}
          }
      }
})

function luaWrap_cpssPxIngressPortMapEntrySet(command_data, devNum, direction, index, portBmp)
    local tableType

    if direction == 1 then
        tableType = "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E"
    else
        tableType = "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E"
    end

    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxIngressPortMapEntrySet", {
            { "IN", "GT_SW_DEV_NUM", "devNum", devNum },
            { "IN", "CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT", "tableType", tableType },
            { "IN", "GT_U32", "entryIndex", index },
            { "IN", "CPSS_PX_PORTS_BMP", "portsBmp", portBmp }
        })
    if isError then
        local error_string = "ERROR calling function cpssPxIngressHashDesignatedPortsEntrySet"
        command_data:addErrorAndPrint(error_string)
        return nil
    end

    return isError
end

local function set_port_map_entry(params)
    local command_data = Command_Data()
    local devices
    local devNum
    local j
    local index
    local iterator

    if (params["all_device"]=="all") then
        devices = generic_getDeviceOfFamilyList(params["all_device"],
            {"CPSS_PX_FAMILY_PIPE_E"});
    else
        devices={params["all_device"]}
    end

    if params["flagNo"] == true then
        portBmp = 0x00000
    else
        portBmp = params["entry"]
    end

    for j=1, #devices do
        devNum = devices[j]
        for iterator, index in number_range_iterator(params["index_range"]) do
            isError = luaWrap_cpssPxIngressPortMapEntrySet(
                command_data, devNum, params["destination"], index, portBmp)
            if isError then
                return
            end
        end
    end
end

CLI_type_dict["ingress_portmap_src_index_range"] = {
    checker = function(param)
    local err = "Index is out of source port map table range";
    for num in string.gmatch(param, "%d+") do
        if(tonumber(num) > 8191) then --8191 is maximal index in designated port table
            return false, err
        end
    end
    return check_param_number_range(param)
end,
    help = "The range of numbers. Example: 2-4,6 means 2,3,4,6 (0..8191)"
}

CLI_type_dict["ingress_portmap_dst_index_range"] = {
    checker = function(param)
    local err = "Index is out of destination port map table range";
    for num in string.gmatch(param, "%d+") do
        if(tonumber(num) > 4095) then --4095 is maximal index in designated port table
            return false, err
        end
    end
    return check_param_number_range(param)
end,
    help = "The range of numbers. Example: 2-4,6 means 2,3,4,6 (0..4095)"
}

-----------------------------------------------
-- command registration: port-map-entry
-----------------------------------------------
CLI_addHelp("config", "ingress port-map-entry", "Set port map table entries")

-----------------------------------------------
-- command registration: ingress port-map-entry source
-----------------------------------------------
CLI_addCommand("config", "ingress port-map-entry source", {
    func   = function(params)
                params["destination"] = 0
                params["flagNo"] = false
                set_port_map_entry(params)
             end,
    help   = "Set port map source table entries",
    params = {
          { type = "named",
              "#all_device",
              { format = "indexes %ingress_portmap_src_index_range",  name="index_range", help = "Index range"},
              { format= "entry %port_mask_hex", name = "entry", help = "Bitmap of ports in hex, 17 bits (with CPU port)"}
          }
      }
})

-----------------------------------------------
-- command registration: ingress port-map-entry destination
-----------------------------------------------
CLI_addCommand("config", "ingress port-map-entry destination", {
    func   = function(params)
                params["destination"] = 1
                params["flagNo"] = false
                set_port_map_entry(params)
             end,
    help   = "Set port map destination table entries",
    params = {
          { type = "named",
              "#all_device",
              { format = "indexes %ingress_portmap_dst_index_range",  name="index_range", help = "Index range"},
              { format= "entry %port_mask_hex", name = "entry", help = "Bitmap of ports in hex, 17 bits (with CPU port)"}
          }
      }
})

-----------------------------------------------
-- command registration: no ingress port-map-entry source
-----------------------------------------------
CLI_addCommand("config", "no ingress port-map-entry source", {
    func   = function(params)
                params["destination"] = 0
                params["flagNo"] = true
                set_port_map_entry(params)
             end,
    help   = "Unset port map source table entries",
    params = {
          { type = "named",
              "#all_device",
              { format = "indexes %ingress_portmap_src_index_range",  name="index_range", help = "Index range"}
          }
      }
})

-----------------------------------------------
-- command registration: no ingress port-map-entry destination
-----------------------------------------------
CLI_addCommand("config", "no ingress port-map-entry destination", {
    func   = function(params)
                params["destination"] = 1
                params["flagNo"] = true
                set_port_map_entry(params)
             end,
    help   = "Unset port map destination table entries",
    params = {
          { type = "named",
              "#all_device",
              { format = "indexes %ingress_portmap_dst_index_range",  name="index_range", help = "Index range"}
          }
      }
})


-----------------------------------------------
-- command body: ingress port-redirect
-----------------------------------------------

local function ingress_port_redirect_func_body(command_data, devNum, portNum, params) 
--[[
    print("devNum   " ..  tostring(devNum))
    print("portNum  " ..  tostring(portNum))
    print("params .." ..  to_string(params))
    print("params['bitmap'] ..  " ..  tostring(params["bitmap"]))
]]--
    
    if params["bitmap"] == nil then
      local error_string = "ERROR calling function cpssPxIngressPortRedirectSet"
      command_data:addErrorAndPrint(error_string)
      return true
    end
    
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxIngressPortRedirectSet", {
            { "IN", "GT_SW_DEV_NUM",        "devNum",         devNum },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",        portNum},
            { "IN", "CPSS_PX_PORTS_BMP",    "targetPortsBmp", params["bitmap"] }
        })
    if isError then
        local error_string = "ERROR calling function cpssPxIngressPortRedirectSet"
        command_data:addErrorAndPrint(error_string)
    end
    return isError
end

local function ingress_port_redirect_func(params)
    return generic_port_range_func(ingress_port_redirect_func_body, params)
end

--------------------------------------------
-- command registration: ingress port-redirect
--------------------------------------------
CLI_addCommand("interface", "ingress port-redirect", {
  func   = ingress_port_redirect_func,
  help   = "Set redirecting of the all packets from ingress port to list of egress ports",
  params = {
      { type = "named",
          { format= "to egress bitmap %port_mask_hex",  
             name = "bitmap",  
             help = "bitmap in hex with list of egress ports"}
      }
  }
})

local function port_filtering_func(params, direction)
    local devices
    local enable
    local devNum
    local iterator
    local index
    local result
    local isError
    local values
    local command_data = Command_Data()

    if (params["all_device"]=="all") then
        devices = generic_getDeviceOfFamilyList(params["all_device"],
            {"CPSS_PX_FAMILY_PIPE_E"});
    else
        devices={params["all_device"]}
    end

    if params["flagNo"] == true then
        enable = false
    else
        enable = true
    end

    for j=1, #devices do
        devNum = devices[j]
        for iterator, index in number_range_iterator(params["packet_type_range"]) do
            local isError, result, values =
            genericCpssApiWithErrorHandler(command_data,"cpssPxIngressFilteringEnableSet", {
                { "IN", "GT_SW_DEV_NUM",         "devNum",     devNum },
                { "IN", "CPSS_DIRECTION_ENT", "direction",  params["direction"]},
                { "IN", "CPSS_PX_PACKET_TYPE",   "packetType", index },
                { "IN", "GT_BOOL",               "enable",     enable }
            })
            if isError then
                local error_string = "ERROR calling function cpssPxIngressFilteringEnableSet"
                command_data:addErrorAndPrint(error_string)
                return isError
            end
        end
    end
end

-------------------------------------------------------
-- type registration: packet type range
-------------------------------------------------------
CLI_type_dict["packetTypeRange"] = {
    checker = CLI_check_param_number_range,
    min=0,
    max=31,
    complete = CLI_complete_param_number_range,
    help = "Packet types. Example: 1-3,5 means packet types 1,2,3,5"
}

--------------------------------------------
-- command registration: ingress port filtering
--------------------------------------------
help_ingress_port_filtering = "Enable ingress port filtering for packet selected type"
CLI_addHelp("config", "ingress port filtering", help_ingress_port_filtering)
CLI_addCommand("config", "ingress port filtering", {
  func   = function(params)
                params["direction"] = "CPSS_DIRECTION_INGRESS_E"
                port_filtering_func(params)
           end,
  help   = help_ingress_port_filtering,
  params = {
      { type = "named",
          "#all_device",
          { format= "packet-type %packetTypeRange",
             name = "packet_type_range",
             help = "Packet type range"},
          mandatory = { "all_device", "packet_type_range"}
      }
  }
})

--------------------------------------------
-- command registration: egress port filtering
--------------------------------------------
help_egress_port_filtering = "Enable egress port filtering for packet selected type"
CLI_addHelp("config", "egress port filtering", help_egress_port_filtering)
CLI_addCommand("config", "egress port filtering", {
  func   = function(params)
                params["direction"] = "CPSS_DIRECTION_EGRESS_E"
                port_filtering_func(params)
           end,
  help   = help_egress_port_filtering,
  params = {
      { type = "named",
          "#all_device",
          { format= "packet-type %packetTypeRange",
             name = "packet_type_range",
             help = "Packet type range"},
          mandatory = { "all_device", "packet_type_range"}
      }
  }
})

--------------------------------------------
-- command registration: no ingress port filtering
--------------------------------------------
help_no_ingress_port_filtering = "Disable ingress port filtering for packet selected type"
CLI_addHelp("config", "no ingress port filtering", help_no_ingress_port_filtering)
CLI_addCommand("config", "no ingress port filtering", {
  func   = function(params)
                params["direction"] = "CPSS_DIRECTION_INGRESS_E"
                params["flagNo"] = true
                port_filtering_func(params)
           end,
  help   = help_no_ingress_port_filtering,
  params = {
      { type = "named",
          "#all_device",
          { format= "packet-type %packetTypeRange",
             name = "packet_type_range",
             help = "Packet type range"},
          mandatory = { "all_device", "packet_type_range"}
      }
  }
})

--------------------------------------------
-- command registration: no egress port filtering
--------------------------------------------
help_no_egress_port_filtering = "Disable egress port filtering for packet selected type"
CLI_addHelp("config", "no ingress port filtering", help_no_egress_port_filtering)
CLI_addCommand("config", "no egress port filtering", {
  func   = function(params)
                params["direction"] = "CPSS_DIRECTION_EGRESS_E"
                params["flagNo"] = true
                port_filtering_func(params)
           end,
  help   = help_no_egress_port_filtering,
  params = {
      { type = "named",
          "#all_device",
          { format= "packet-type %packetTypeRange",
             name = "packet_type_range",
             help = "Packet type range"},
          mandatory = { "all_device", "packet_type_range"}
      }
  }
})

local function show_port_filtering_func(params)
    local devices
    local enable
    local devNum
    local iterator
    local index
    local result
    local isError
    local values
    local enabled
    local outstring = ""

    outstring = outstring .. "Device | Direction | Packet type | Enabled \n"
    outstring = outstring .. "-------------------------------------------\n"
    local command_data = Command_Data()
    command_data:clearResultArray()

    if (params["all_device"]=="all") then
        devices = generic_getDeviceOfFamilyList(params["all_device"],
            {"CPSS_PX_FAMILY_PIPE_E"});
    else
        devices={params["all_device"]}
    end

    for j=1, #devices do
        devNum = devices[j]
        for iterator, index in number_range_iterator(params["packet_type_range"]) do
            local isError, result, values =
            genericCpssApiWithErrorHandler(command_data,"cpssPxIngressFilteringEnableGet", {
                { "IN", "GT_SW_DEV_NUM",         "devNum",     devNum },
                { "IN", "CPSS_DIRECTION_ENT", "direction",  "CPSS_DIRECTION_INGRESS_E"},
                { "IN", "CPSS_PX_PACKET_TYPE",   "packetType", index },
                { "OUT", "GT_BOOL",               "enablePtr"}
            })
            if isError then
                local error_string = "ERROR calling function cpssPxIngressFilteringEnableGet"
                command_data:addErrorAndPrint(error_string)
                return isError
            end
            if values["enablePtr"] == true then
                enabled = "enabled"
            else
                enabled = "disabled"
            end
            outstring = outstring .. string.format("% 6d   %09s   % 11d   %07s\n", devNum, "ingress", index, enabled)

            local isError, result, values =
            genericCpssApiWithErrorHandler(command_data,"cpssPxIngressFilteringEnableGet", {
                { "IN", "GT_SW_DEV_NUM",         "devNum",     devNum },
                { "IN", "CPSS_DIRECTION_ENT", "direction",  "CPSS_DIRECTION_EGRESS_E"},
                { "IN", "CPSS_PX_PACKET_TYPE",   "packetType", index },
                { "OUT", "GT_BOOL",               "enablePtr"}
            })
            if isError then
                local error_string = "ERROR calling function cpssPxIngressFilteringEnableGet"
                command_data:addErrorAndPrint(error_string)
                return isError
            end
            if values["enablePtr"] == true then
                enabled = "enabled"
            else
                enabled = "disabled"
            end
            outstring = outstring .. string.format("% 6d   %09s   % 11d   %07s\n", devNum, "egress", index, enabled)
        end
    end
    command_data["result"] = outstring

    command_data:addResultToResultArray()
    command_data:updateEntries()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
end

--------------------------------------------
-- command registration: show port filtering
--------------------------------------------
help_show_port_filtering = "Show port filtering configuration"
CLI_addHelp("exec", "show port filtering", help_ingress_port_filtering)
CLI_addCommand("exec", "show port filtering", {
  func   = show_port_filtering_func,
  help   = help_show_port_filtering,
  params = {
      { type = "named",
          "#all_device",
          { format= "packet-type %packetTypeRange",
             name = "packet_type_range",
             help = "Packet type range"},
          mandatory = { "all_device", "packet_type_range"}
      }
  }
})

-- sets VLAN tag state entry for target ports
local function luaWrap_cpssPxEgressVlanTagStateEntrySet(command_data, devNum, vlanId, portsTagging)
    local apiName = "cpssPxEgressVlanTagStateEntrySet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_U32","vlanId", vlanId},
        {"IN","CPSS_PX_PORTS_BMP","portsTagging", portsTagging}
    })

    if isError then
        local error_string = "ERROR calling function cpssPxEgressVlanTagStateEntrySet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError
end

-- gets VLAN tag state entry for target ports
local function luaWrap_cpssPxEgressVlanTagStateEntryGet(command_data, devNum, vlanId)
    local apiName = "cpssPxEgressVlanTagStateEntryGet"
    local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_SW_DEV_NUM","devNum",devNum},
        {"IN","GT_U32","vlanId", vlanId},
        {"OUT","CPSS_PX_PORTS_BMP","portsTaggingPtr"},
    })

    if isError then
        local error_string = "ERROR calling function cpssPxEgressVlanTagStateEntryGet"
        -- use command_data:addErrorAndPrint to be able to see also errors that came from 
        command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
    end

    return isError, values.portsTaggingPtr
end

local function set_egress_vlan_tag_state_entry(params)
    local command_data = Command_Data()
    local devices
    local devNum
    local j
    local vlan
    local iterator
    local portBmp

    if (params["all_device"]=="all") then
        devices = generic_getDeviceOfFamilyList(params["all_device"],
            {"CPSS_PX_FAMILY_PIPE_E"});
    else
        devices={params["all_device"]}
    end

    if params["flagNo"] == true then
        portBmp = 0x0000
    else
        portBmp = params["entry_bmp"]
    end


    for j=1, #devices do
        devNum = devices[j]
        for iterator, vlan in number_range_iterator(params["vlan_range"]) do
            isError = luaWrap_cpssPxEgressVlanTagStateEntrySet(
                command_data, devNum, vlan, portBmp)
            if isError then
                return
            end
        end
    end
end


CLI_type_dict["egress_vlan_tag_state_range"] = {
    checker = function(param)
    local err = "Index is out of VLAN range";
    for num in string.gmatch(param, "%d+") do
        if(tonumber(num) > 4095) then --4095 is maximal Vlan ID in designated port table
            return false, err
        end
    end
    return check_param_number_range(param)
end,
    help = "The range of VLANs. Example: 2-4,6 means 2,3,4,6 (0..4095)"
}

-----------------------------------------------
-- command registration: vlan-tag-state
-----------------------------------------------
CLI_addHelp("config", "egress vlan-tag-state", "Sets VLAN tag state entry for target ports")

-----------------------------------------------
-- command registration: egress vlan-tag-state
-----------------------------------------------
CLI_addCommand("config", "egress vlan-tag-state", {
    func   = function(params)
                params["flagNo"] = false
                set_egress_vlan_tag_state_entry(params)
             end,
    help   = "Sets VLAN tag state entry for target ports",
    params = {
          { type = "named",
              "#all_device",
              { format = "vlanID %egress_vlan_tag_state_range", name = "vlan_range", help = "VLAN range"},
              { format = "entryBmp %port_mask_hex", name = "entry_bmp", help = "Bitmap of physical ports tagged in the VLAN in hex, 16 bits (without CPU port)"},
              requirements = { vlan_range = { "all_device" },  entry_bmp = {"vlan_range"}},
              mandatory = { "all_device", "vlan_range", "entry_bmp"}
          }
      }
})

-----------------------------------------------
-- command registration: no egress vlan-tag-state
-----------------------------------------------
CLI_addHelp("config", "no egress vlan-tag-state", "Resets VLAN tag state entry for target ports")

-----------------------------------------------
-- command registration: egress no vlan-tag-state
-----------------------------------------------
CLI_addCommand("config", "no egress vlan-tag-state", {
    func   = function(params)
                params["flagNo"] = true
                set_egress_vlan_tag_state_entry(params)
             end,
    help   = "Resets VLAN tag state entry for target ports",
    params = {
          { type = "named",
              "#all_device",
              { format = "vlanID %egress_vlan_tag_state_range", name = "vlan_range", help = "VLAN range"},
              mandatory = { "all_device", "vlan_range"}
          }
      }
})


local function show_vlan_tag_state_body(command_data, devNum, params)
    local fromIndex = params["from_index"]
    local toIndex

    if params["to_index"] == nil then
        toIndex = fromIndex
    else
        toIndex = params["to_index"]
    end

    if fromIndex > toIndex then
        fromIndex = toIndex
    end

    for vlan = fromIndex,  toIndex do
        local isError, entryTable = luaWrap_cpssPxEgressVlanTagStateEntryGet(command_data, tonumber(devNum), vlan)
        if isError then
            return
        end

        command_data["result"] =
                        string.format("%d\t\t%4d\t\t0x%04x", devNum, vlan, entryTable)

        command_data:addResultToResultArray()
        command_data:updateEntries()
    end
end

local function show_vlan_tag_state_entry(params)
    local header_string, footer_string

    local head_line0 = "Device         Vlan ID      VLAN tag state\n"
    local head_line1 = "------------------------------------------"
    params.header_string = head_line0 .. head_line1
    params.footer_string = "\n"

    generic_all_device_show_func(show_vlan_tag_state_body, params)
end

-----------------------------------------------
-- command registration: vlan-tag-state
-----------------------------------------------
CLI_addCommand("exec", "show egress vlan-tag-state", {
    func   = show_vlan_tag_state_entry,
    help   = "Show VLAN tag state entry for target ports",
    params = {
          { type = "named",
              "#all_device",
              { format = "from %GT_U32",  name="from_index", help = "VLAN range start index"},
              { format = "to %GT_U32",  name="to_index", help = "VLAN range end index"},
              requirements = { from_index = { "all_device" },  to_index = {"from_index"}},
              mandatory = { "all_device", "from_index"}
          }
      }
})

CLI_type_dict["ip2me_index"] = {
    checker = CLI_check_param_number,
    min=1,
    max=7,
    help="Entry index 1..7"
}

CLI_type_dict["ip2me_prefix"] = {
    checker = CLI_check_param_number,
    min=1,
    max=128,
    help="Prefix length 1..128"
}

local function set_ip2me_entry(params)
    local entry
    local devices
    local devNum

    local command_data = Command_Data()

    if (params["all_device"]=="all") then
        devices = generic_getDeviceOfFamilyList(params["all_device"],
            {"CPSS_PX_FAMILY_PIPE_E"});
    else
        devices={params["all_device"]}
    end

    entry = {}
    if params["flagNo"] == true then
        entry["valid"] = false
        entry["prefixLength"] = 32
        entry["isIpv6"] = 0
        entry["ipAddr"] = "0.0.0.0"
    else
        entry["valid"] = true
        entry["prefixLength"] = params["prefix"]
        if params["ip"]["compound"] == "ipv6" then
            entry["isIpv6"] = true
        else
            entry["isIpv6"] = false
        end
        entry["ipAddr"] = params["ip"]["string"]
    end

    for j=1, #devices do
        devNum = devices[j]
        local isError, result, values = genericCpssApiWithErrorHandler(command_data,
        "cpssPxIngressIp2MeEntrySet", {
            {"IN","GT_SW_DEV_NUM","devNum",devNum},
            {"IN","GT_U32","entryIndex", params["index"]},
            {"IN","CPSS_PX_INGRESS_IP2ME_ENTRY_STC","ip2meEntryPtr", entry}
        })

        if isError then
            local error_string = "ERROR calling function cpssPxIngressIp2MeEntrySet"
            command_data:addErrorAndPrint(error_string)
            return
        end
    end

end

-----------------------------------------------
-- command registration: ingress ip2me-entry
-----------------------------------------------
CLI_addHelp("config", "ingress ip2me-entry", "Configure IP2ME entry")
CLI_addCommand("config", "ingress ip2me-entry", {
    func   = set_ip2me_entry,
    help   = "Configure IP2ME entry",
    params = {
          { type = "named",
              "#all_device",
              { format = "index %ip2me_index",  name="index", help = "Entry index 1..7"},
              { format = "ip %ipv4_ipv6",  name="ip", help = "IP address"},
              { format = "prefix %ip2me_prefix",  name="prefix", help = "Prefix length. For IPv4 1..32, for IPv6 1..128"},
              mandatory = { "all_device", "index", "prefix", "ip"}
          }
      }
})

-----------------------------------------------
-- command registration: no ingress ip2me-entry
-----------------------------------------------
CLI_addHelp("config", "no ingress ip2me-entry", "Disable IP2ME entry")
CLI_addCommand("config", "no ingress ip2me-entry", {
    func   = function(params)
                params["flagNo"] = true
                set_ip2me_entry(params)
             end,
    help   = "Disable IP2ME entry",
    params = {
          { type = "named",
              "#all_device",
              { format = "index %ip2me_index",  name="index", help = "Entry index 1..7"},
              mandatory = { "all_device", "index"}
          }
      }
})

local function show_ip2me_entry_perdevice(command_data, devNum, params)
    local index
    local val
    for index = 1,7 do
        local isError, result, values = genericCpssApiWithErrorHandler(command_data,
            "cpssPxIngressIp2MeEntryGet", {
                {"IN","GT_SW_DEV_NUM","devNum",devNum},
                {"IN","GT_U32","entryIndex", index},
                {"OUT","CPSS_PX_INGRESS_IP2ME_ENTRY_STC","ip2meEntryPtr"}
            })
        if isError then
            local error_string = "ERROR calling function cpssPxIngressIp2MeEntrySet"
            command_data:addErrorAndPrint(error_string)
            return
        end
        val = values["ip2meEntryPtr"]
        if val["valid"] == true then
            command_data["result"] = string.format("%-6d  %-4d    %-39s %-4d",
                devNum, index, val["ipAddr"], val["prefixLength"])
            command_data:addResultToResultArray()
            command_data:updateEntries()
        end
    end
end

local function show_ip2me_entry(params)
    local header_string, footer_string

    local head_line0 = "Device  Index   IP address                              Prefix\n"
    local head_line1 = "-----------------------------------------------------------------"
    params.header_string = head_line0 .. head_line1
    params.footer_string = "\n"

    generic_all_device_show_func(show_ip2me_entry_perdevice, params)
end

-----------------------------------------------
-- command registration: show ip2me-entry
-----------------------------------------------
CLI_addHelp("exec", "show ingress ip2me-entry", "Show IP2ME entries")
CLI_addCommand("exec", "show ingress ip2me-entry", {
    func   = show_ip2me_entry,
    help   = "Configure IP2ME entry",
    params = {
          { type = "named",
              "#all_device",
              mandatory = { "all_device" }
          }
      }
})