--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bpe_802_1_br_my_info.lua
--*
--* DESCRIPTION:
--*       manage IEEE 802.1BR general configurations
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

-- global DB - initialized as empty table on registration
-- bpe_802_1_br_my_info_DB;
cmdLuaCLI_registerCfunction("wrlCpssDxChTcamTtiHitInfoGet")

-- used extenal data and functions:
function setTtiLookups0And1AccordingToAppDemo(devNum)
    local result, hit0_base , hit0_size , hit1_base , hit1_size ,
          hit2_base , hit2_size , hit3_base , hit3_size =
            wrLogWrapper("wrlCpssDxChTcamTtiHitInfoGet", "(devNum)", devNum)

    --[[
    _debug_to_string("wrlCpssDxChTcamTtiHitInfoGet: result",result)
    _debug_to_string("wrlCpssDxChTcamTtiHitInfoGet: hit0_base",hit0_base)
    _debug_to_string("wrlCpssDxChTcamTtiHitInfoGet: hit0_size",hit0_size)
    _debug_to_string("wrlCpssDxChTcamTtiHitInfoGet: hit1_base",hit1_base)
    _debug_to_string("wrlCpssDxChTcamTtiHitInfoGet: hit1_size",hit1_size)
    _debug_to_string("wrlCpssDxChTcamTtiHitInfoGet: hit2_base",hit2_base)
    _debug_to_string("wrlCpssDxChTcamTtiHitInfoGet: hit2_size",hit2_size)
    _debug_to_string("wrlCpssDxChTcamTtiHitInfoGet: hit3_base",hit3_base)
    _debug_to_string("wrlCpssDxChTcamTtiHitInfoGet: hit3_size",hit3_size)
    --]]

    -- state the ranges used by TTI lookup 0
    setTtiIndexRangeForPool(hit0_base, hit0_base + hit0_size - 1 , 0)
    -- state the ranges used by TTI lookup 1
    setTtiIndexRangeForPool(hit1_base, hit1_base + hit1_size - 1 , 1)
end

-- returns list of {} keyed by ((0x1000 * devNum) + portNum) values
-- supported types: "downstream", "upstream", "all"
function bpe_802_1_br_get_cascade_ports_list(type)
    local pe_ports_list = {};
    local cb_ports_list = {};
    local index, entry;
    local command_data;
    local dummy, devNum, portNum, devPortKey;
    local result_ports_list = {};

    index = nil;
    while (true) do
        index, entry =
            next(table_bpe_802_1_br_system_info_devices, index);
        if ((index == nil) or (entry == nil)) then break; end;
        command_data = entry["command_data"];
        if (command_data ~= nil) then
            for dummy, devNum, portNum in command_data:getPortIterator() do
                devPortKey = ((0x1000 * devNum) + portNum);
                if (entry["role"] == "control_bridge") then
                    cb_ports_list[devPortKey] = {};
                else
                    pe_ports_list[devPortKey] = {};
                end
            end
        end;
    end;

    if (type == "downstream") then
        result_ports_list = pe_ports_list;
        for devPortKey, dummy in pairs(cb_ports_list) do
            result_ports_list[devPortKey] = nil;
        end
    elseif (type == "upstream") then
        result_ports_list = cb_ports_list;
    elseif (type == "all") then
        result_ports_list = pe_ports_list;
        for devPortKey, dummy in pairs(cb_ports_list) do
            result_ports_list[devPortKey] = {};
        end
    else
        print("bpe_802_1_br_get_cascade_ports_list unsupported port type");
    end

    return result_ports_list;
end

-- converts list of {} keyed by ((0x1000 * devNum) + portNum) values
-- to bitmap of ports of given device
local function convert_ports_list_to_dev_ports_bitmap(portList, devNum)
    local dummy, currDevNum, portNum, devPortKey;
    local wordIndex, bitIndex;
    local result_bitmap = {0,0,0,0,0,0,0,0};
    for devPortKey, dummy in pairs(portList) do
        currDevNum = math.floor(devPortKey / 0x1000);
        if (currDevNum == devNum) then
            portNum    = (devPortKey % 0x1000);
            wordIndex  = (math.floor(portNum / 32) + 1);
            bitIndex   = (portNum % 32);
            result_bitmap[wordIndex] = result_bitmap[wordIndex] + (2 ^ bitIndex);
        end;
    end
    return result_bitmap;
end

local function bpe_802_1_br_create_downstream_ports_vidx(vidxId)
    local port_list, port_bitmap, port_bmp_stc, ret, val;
    local devices, j, devNum;

    devices = wrLogWrapper("wrlDevList");
    port_list = bpe_802_1_br_get_cascade_ports_list("downstream");

    for j=1, #devices do
        devNum = devices[j];
        port_bitmap = convert_ports_list_to_dev_ports_bitmap(port_list, devNum);
        port_bmp_stc = {};
        port_bmp_stc["ports"] = port_bitmap;

        ret, val = myGenWrapper("cpssDxChBrgMcEntryWrite",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U16","vidx",vidxId},
                {"IN","CPSS_PORTS_BMP_STC","portBitmapPtr", port_bmp_stc}
        });
        if (ret ~= 0) then
            print("cpssDxChBrgMcEntryWrite failed, rc = " .. ret);
        end
    end
end

-- check if the primary device supports the role
function bpe_802_1_br_check_device_for_role(role)
    local devNum = devEnv.dev

    if role == "port_extender" and 
       is_supported_feature(devNum , "BPE_802_1_BR_PORT_EXTENDER") 
    then
        return true
    end
    
    if role == "control_bridge" and 
       is_supported_feature(devNum , "BPE_802_1_BR_CONTROL_BRIDGE") 
    then
        return true
    end
    
   -- the device not supports the role
   return false
end

-- set the tti range of indexes according to device info
-- this function good for tcam that uses the full tti 0 lookup
local function setTtiIndexRangeForPoolForDevice(command_data,devNum)
    local apiName = "cpssDxChCfgTableNumEntriesGet"
    local isError , result, values = genericCpssApiWithErrorHandler(command_data,
        apiName, {
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_DXCH_CFG_TABLES_ENT","table","CPSS_DXCH_CFG_TABLE_TTI_TCAM_E"},
        {"OUT","GT_U32","numEntries"}
    })

    if isError then
        return
    end

    -- set the tti range of indexes for tti
    setTtiIndexRangeForPool(0,(values.numEntries - 1))

end
-- function on PE , non eArch device
-- Info about the 'my device' in BPE system (bridge port externder) according to IEEE-802.1BR standard
local function func_onPe_non_eArch_bpe_802_1_br__my_info(params,command_data,dev_range)
    local portNum
    local enable
    local tpidBmp_eth0,tpidBmp_eth1
    local pvid
    local isMember,isTagged
    local last_valid_vidx_index
    -- PATCH: vlanId set as PVID, but the varable was not defined
    local vlanId = 1;
    local apiName

    if not params.flagNo then
        enable = true
        tpidBmp_eth1 = 0 -- not allow recognize tag 1
        -- remove index 7 from the recognition of tag 0
        tpidBmp_eth0 = 0xff - bit_shl(1,bpe_802_1_br_egress_pushed_tag_tpid_reserved_index)
        pvid = bpe_802_1_br_not_valid_vid
        isMember = true
        last_valid_vidx_index = 0x3fff -- allow the device to work with 12K vidx.
    else
        enable = false
        tpidBmp_eth1 = 0xff -- default of all ports
        tpidBmp_eth0 = 0xff -- default of all ports
        pvid = 1 -- default pvid
        isMember = false
        last_valid_vidx_index = 0xFFF - 1 -- default
    end


    -- allow the device to work with 12K vidx.
    bridge_multicast_set_last_valid_vidx_index(last_valid_vidx_index)


    for index, devNum in pairs(dev_range) do
        local result, numOfPorts = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)

        setTtiIndexRangeForPoolForDevice(command_data,devNum)

        apiName = "cpssDxChBrgPeEnableSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",      "devNum",     devNum},
            { "IN",     "GT_BOOL",    "enable",     enable}
        })

        for i = 1, numOfPorts do
            portNum = i - 1

            --[[
                Non-BPE ports (if they exist), must be configured not to classify any TPID as Tag1, i.e.,
                non-BPE ports may only have Tag0 classification performed
            --]]
            -- tag 0
            apiName = "cpssDxChBrgVlanPortIngressTpidSet"
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum},
                { "IN",     "GT_PORT_NUM",    "portNum",     portNum},
                { "IN",     "CPSS_ETHER_MODE_ENT",    "ethMode",     "CPSS_VLAN_ETHERTYPE0_E"},
                { "IN",     "GT_U32",    "tpidBmp",     tpidBmp_eth0}
            })

            -- tag 1
            apiName = "cpssDxChBrgVlanPortIngressTpidSet"
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum},
                { "IN",     "GT_PORT_NUM",    "portNum",     portNum},
                { "IN",     "CPSS_ETHER_MODE_ENT",    "ethMode",     "CPSS_VLAN_ETHERTYPE1_E"},
                { "IN",     "GT_U32",    "tpidBmp",     tpidBmp_eth1}
            })

            -- pvid
            apiName = "cpssDxChBrgVlanPortVidSet"
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum},
                { "IN",     "GT_PORT_NUM",    "portNum",     portNum},
                { "IN",     "CPSS_DIRECTION_ENT",    "direction",     "CPSS_DIRECTION_BOTH_E"},
                { "IN",     "GT_U16",    "vlanId",     vlanId}
            })

            -- in the vlan table set the Egress VLAN Tag State of VLAN 4095 to the {Tag1} state for
            -- Upstream ports and Cascade ports, and to the {Untagged} state for Extended ports.
            apiName = "cpssDxChBrgVlanMemberSet"
            genericCpssApiWithErrorHandler(command_data,
                apiName, {
                { "IN",     "GT_U8",      "devNum",     devNum},
                { "IN",     "GT_U16",    "vlanId",     bpe_802_1_br_not_valid_vid},
                { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",     portNum},
                { "IN",     "GT_BOOL",    "isMember",     isMember},
                { "IN",     "GT_BOOL",    "isTagged",     false},
                { "IN",     "GT_BOOL",    "CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT",  "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E"}
            })
        end
    end

end

-- ************************************************************************
---
--  bpe_802_1_br__my_info
--        @description  Info about the 'my device' in BPE system (bridge port extender) according to IEEE-802.1BR standard
--
--        @param params             - params["bpe_802_1_br___role"]: the role <cb/pe>
--                                    and port interface
--
--        @return       true on success, otherwise false and error message
--
local function bpe_802_1_br__my_info(params)
    local command_data = Command_Data()
    local role = params["role"]
    local unset = (params.flagNo)
    local apiName

    if not unset then
        if (bpe_802_1_br_my_info_DB == nil) then
                bpe_802_1_br_my_info_DB = {};
        end
        if (bpe_802_1_br_my_info_DB["dev_info"] == nil) then
                bpe_802_1_br_my_info_DB["dev_info"] = {};
        end

        if false == bpe_802_1_br_check_device_for_role(role) then
            print("Role " .. role .. " not supported for device " .. to_string(DeviceFamily))
            return
        end

        bpe_802_1_br_my_info_DB["dev_info"]["role"] = role;
        -- bind callback for registered MAC MC groups
        bridge_multicast_address_group_flood_callback_bind(
            bridge_multicast_address_group_bpe_802_1_br_mc_callback);
    else -- unset
        -- unbind callback for registered MAC MC groups
        bridge_multicast_address_group_flood_callback_bind(nil);
        -- need to remove configurations
        if not bpe_802_1_br_my_info_DB then
            print(bpe_802_1_br_error_system_is_not_802_1br_string)
            return nil
        end
    end
    local devices = wrLogWrapper("wrlDevList");

    if (bpe_802_1_br_my_info_DB["dev_info"]["role"] == "control_bridge") then
        local j;
        local mllPairEntry;

        local tag1Mode
        if not unset then
            tag1Mode = "CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_AND_INGRESS_WITHOUT_TAG1_E"
            print("configuring control bridge");
        else
            tag1Mode = "CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E"
            print("remove configuration of control bridge");
        end

        mllPairEntry = {};
        if not unset then
            mllPairEntry["firstMllNode"] = {};
            mllPairEntry["secondMllNode"] = {};
            mllPairEntry["secondMllNode"]["last"] = true;
            mllPairEntry["secondMllNode"]["egressInterface"] = {};
            mllPairEntry["secondMllNode"]["egressInterface"]["type"] =
                    "CPSS_INTERFACE_VIDX_E";
            mllPairEntry["secondMllNode"]["egressInterface"]["vidx"] = 0xFFF;
        end

        for j=1, #devices do
            apiName = "cpssDxChCfgTableNumEntriesGet"
            local isError , result, values = genericCpssApiWithErrorHandler(command_data,
                apiName, {
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","CPSS_DXCH_CFG_TABLES_ENT","table","CPSS_DXCH_CFG_TABLE_MLL_PAIR_E"},
                {"OUT","GT_U32","numEntries"}
            })

            if isError then
                return
            end

            local lastMllPairIndex = values.numEntries - 1

            local ret, val = myGenWrapper("cpssDxChL2MllPairWrite",{
                    {"IN","GT_U8","devNum",devices[j]},
                    {"IN","GT_U32","mllPairEntryIndex",lastMllPairIndex},
                    {"IN",TYPE["ENUM"],"mllPairWriteForm",
                        1 --[[CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E--]]},
                    {"IN","CPSS_DXCH_L2_MLL_PAIR_STC","mllPairEntryPtr", mllPairEntry}
            });
            if (ret ~= 0) then
                print("cpssDxChL2MllPairWrite returned " .. ret .. " for devNum = " .. devices[j]);
            end

            setTtiLookups0And1AccordingToAppDemo(devices[j])

            ret, val = myGenWrapper("cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT","mode",tag1Mode}
            });
            if (ret ~= 0) then
                print("cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet returned " .. ret .. " for devNum = " .. devices[j]);
            end
        end
    elseif (bpe_802_1_br_my_info_DB["dev_info"]["role"] == "port_extender") then
        if not unset then
            print("configuring port extender");
        else
            print("remove configuration of port extender");
        end

        --[[ function must not be called here
        bpe_802_1_br_create_downstream_ports_vidx(
                reserved_vidx_for_cascade_ports);
        --]]

        if not bpe_802_1_br_is_eArch_Device() then
            func_onPe_non_eArch_bpe_802_1_br__my_info(params,command_data,devices)
        else
            for j=1, #devices do
                setTtiLookups0And1AccordingToAppDemo(devices[j])

                bpe_802_1_br_onPe_setTrunkId(devices[j])
            end
        end
    else
        print("wrong role");
    end

    if unset then
        bpe_802_1_br_my_info_DB = nil
        bpe_802_1_br_neighbor_db_reset()
    end

end

CLI_addCommand("config", "bpe-802-1-br my-info",
    { func = bpe_802_1_br__my_info,
      help = "bpe-802-1-br my-info manipulation",
      params =
      {
            { type="named",
               { format = "role %bpe_802_1_br___role", name = "role",
               help = "The role of the device (CB - Control Bridge , PE - port extender)" },
               mandatory = {"role"}
            }
      }
    });

CLI_addCommand("config", "no bpe-802-1-br my-info",
    { func = function(params)
            params.flagNo = true
            return bpe_802_1_br__my_info(params)
        end,
      help = "remove bpe-802-1-br my-info",
      params = {} -- no parameters
    });

---------------------------------------------------------------------------------
-- UTILS
---------------------------------------------------------------------------------

-- L2MLL table management

-- used
-- function allocIndexInTable(tableName,firstIndex,lastIndex,alignment,size)
-- function freeIndexInTable(tableName,allocatedIndex,size)

-- L2MLL entries management table
-- separated to subtables using tostring(devId) as primary key.
local L2MllEntriesUsingTable            = {};
local L2MllEntriesUsingTable_firstIndex = 0;
local L2MllEntriesUsingTable_lastIndex  = 0x7FFD; -- 32K-2

-- returns list of indexes
function allocL2MllTableEntriesChain(devNum, numOfElements, keyString)
    local result_array = {};
    local entriesKey = "l2mll_entries_dev_" .. tostring(devNum);
    local dictionnaryKey = "dictionnary_dev_" .. tostring(devNum);
    local first_index, index, elmNum, num;
    local first_allocation, alignment, size, j, k;
    local oldChainHead;

    if (numOfElements <= 0) then
        return {};
    end

    if (L2MllEntriesUsingTable[entriesKey] == nil) then
        L2MllEntriesUsingTable[entriesKey] = {};
    end
    if (L2MllEntriesUsingTable[dictionnaryKey] == nil) then
        L2MllEntriesUsingTable[dictionnaryKey] = {};
    end

    first_allocation = true;
    elmNum           = numOfElements;

    while (elmNum > 0) do
        if (elmNum >= 2) then
            alignment = 2;
            size      = 2;
        else
            alignment = 1;
            size      = 1;
        end;

        index = allocIndexInTable(
            L2MllEntriesUsingTable[entriesKey],
            L2MllEntriesUsingTable_firstIndex,
            L2MllEntriesUsingTable_lastIndex,
            alignment, size);
        if (index == nil) then
            if (first_allocation == false) then
                freeL2MllTableEntriesChain(devNum, first_index);
            end;
            return nil;
        end

        if (first_allocation == true) then
            first_allocation = false;
            first_index      = index;
        end

        for j = 1, size do
            k = index + j - 1;
            result_array[#result_array + 1] = k;
            if (k ~= first_index) then
                num = #(L2MllEntriesUsingTable[entriesKey][first_index]);
                L2MllEntriesUsingTable[entriesKey][first_index][num + 1] = k;
            end
        end

        elmNum = elmNum - size;
    end

    if (keyString ~= nil) then
        oldChainHead = L2MllEntriesUsingTable[dictionnaryKey][keyString];
        L2MllEntriesUsingTable[dictionnaryKey][keyString] = first_index;
        if (oldChainHead ~= nil) then
            freeL2MllTableEntriesChain(devNum, oldChainHead);
        end
    end

    return result_array;
end

-- returns array of all chain members excluding the first
local function getL2MllTableEntriesChainTail(devNum, allocatedBaseIndex)
    local key = "l2mll_entries_dev_" .. tostring(devNum);
    if (L2MllEntriesUsingTable[key] == nil) then
        return nil;
    end
    return L2MllEntriesUsingTable[key][allocatedBaseIndex];
end

function freeL2MllTableEntriesChain(devNum, allocatedIndex)
    local key = "l2mll_entries_dev_" .. tostring(devNum);
    local chain_array, index, alcIndex, size;

    if (L2MllEntriesUsingTable[key] == nil) then
        return;
    end

    chain_array = L2MllEntriesUsingTable[key][allocatedIndex];

    index = 0;
    while index <= #chain_array do
        if (index == 0) then
            alcIndex = allocatedIndex;
        else
            alcIndex = chain_array[index];
        end

        if ((index < #chain_array) and ((alcIndex + 1) == chain_array[index + 1])) then
            size = 2;
        else
            size = 1;
        end
        freeIndexInTable(
            L2MllEntriesUsingTable[key],
            alcIndex, size);
        index = index + size;
    end
end

function freeL2MllTableEntriesChainByString(devNum, keyString)
    local dictionnaryKey = "dictionnary_dev_" .. tostring(devNum);
    local dictTab;
    local chainHead;

    dictTab = L2MllEntriesUsingTable[dictionnaryKey]
    if dictTab == nil then
        return;
    end
    chainHead = dictTab[keyString];
    if (chainHead ~= nil) then
        freeL2MllTableEntriesChain(devNum, chainHead);
        L2MllEntriesUsingTable[dictionnaryKey][keyString] = nil;
    end
end

--------------------------------------------------------------------------------------------------
function convert_key_list_to_array(key_list)
    local result_array = {};
    local key, index;

    index = 1;
    key = next(key_list, nil);
    while (key ~= nil) do
        result_array[index] = key;
        index = index + 1;
        key = next(key_list, key);
    end
    return result_array;
end

--------------------------------------------------------------------------------------------------

local function string_dump(val)
    if (type(val) == "table") then
        return "TABLE";
    elseif (type(val) == "number") then
        return string.format("0x%08X", val);
    elseif (type(val) == "string") then
        return ("\"" .. val .. "\"");
    else
        return tostring(val);
    end
end

local function leveled_table_dump(level, tab)
    local key, leaf;

    if (type(tab) ~= "table") then
        print("dumped data is not a table: " .. tab);
        return;
    end

    key = next(tab, nil);
    while (key ~= nil) do
        leaf = tab[key];
        if (type(leaf) == "table") then
            print(string.format("0x%04X  ", level) .. string_dump(key) .. " --> " .. "<<TABLE BELOW>>");
            leveled_table_dump((level + 1), leaf);
        else
            print(string.format("0x%04X  ", level) .. string_dump(key) .. " --> " .. string_dump(leaf));
        end
        key = next(tab, key);
    end
end

local function debug_table_dump(tab)
    leveled_table_dump(0, tab);
end

local function pathed_table_print(path, tab)
    local key, leaf, tmp_path;

    if (type(tab) ~= "table") then
        print("dumped data is not a table: " .. tab);
        return;
    end

    key = next(tab, nil);
    while (key ~= nil) do
        leaf = tab[key];
        tmp_path = path .. "/" .. string_dump(key)
        if (type(leaf) == "table") then
            if (nil == next(leaf, nil)) then
                print(tmp_path .. " = {}");
            else
                pathed_table_print(tmp_path, leaf);
            end
        else
            print(tmp_path .. " = " .. string_dump(leaf));
        end
        key = next(tab, key);
    end
end

function debug_table_print(tab)
    pathed_table_print("", tab);
end

-- print table :
-- parameters are :
-- title - string to print at start and end
-- dbName - the db name --> the actual table
local function print_table(title,dbName)
    title = title .. " table"

    print ("\n start entries of " .. title)
    debug_table_print(dbName)
    print (" end " .. title .. "\n")
end

--[[
    debug 'show' commands to see the internal DB of the MLL manager
--]]
CLI_addHelp("debug", "show bridge internal-db l2mll", "show bridge internal DB related to L2MLL managed by the LUA commands")

CLI_addCommand("debug", "show bridge internal-db l2mll", {
    func = function(params)
        --***********************
        --***********************
        local title = "l2mll"
        local dbName = L2MllEntriesUsingTable

        print_table(title,dbName)

    end,
    help="Dump internal DB of bridge mac multicast"
})
