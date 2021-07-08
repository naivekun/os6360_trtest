--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_multicast_mac_address.lua
--*
--* DESCRIPTION:
--*       registering/deregistering of MAC-layer multicast address to the bridge
--*       table and assign interfaces to this multicast address
--*
--* FILE REVISION NUMBER:
--*       $Revision: 10 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssDxChBrgIsEntryExists")

-- Package dependent callback for bridge_multicast_address_group
-- "set" and "no" commands can
-- 1. create/delete FDB Mac entry <db_key_str> => <vidx or eVidx>
-- 2. allocate/free vidx and configure Vidx Entry to set of ports
-- 3. add ports to vlan ("set" only)
-- The callback function can do (1) or (2) or both instead of main code
-- callback function format
--
-- bridge_multicast_address_group_flood_callback(
--    command_data, devNum, db_key_str, port_range, vidx, eVidx, unset)
--
-- parameters:
-- command_data, devNum, db_key_str are command and context data
-- port_range is command data, format - {portNum1, portNum2, ...}
-- vidx, eVidx retrieved from local DB of local bridge_multicast_address_group DB
-- each of vidx, eVidx may be nil
-- unset - false for "set", true for "no"
--
-- function should allocate or find in DB and configure or update vidx or/and eVidx
-- to flood packets to specified physical ports and ePorts
-- callback returns result = {
-- [vidx = vidx], [dontUpdateVidxEntry = true],
-- [dontUpdateFdbEntry = true]
-- [eVidx = eVidx]}
-- each member is optional
-- result == nil means - needed regular support
-- main code should not update VidxEntry if result contains[dontUpdateVidxEntry = true]
-- main code should not update FDB_Entry if result contains[dontUpdateFdbEntry = true]
--
-- the variable is reference to function to be bound by client package
local bridge_multicast_address_group_flood_callback = nil;

-- BIND API
function bridge_multicast_address_group_flood_callback_bind(callback)
    bridge_multicast_address_group_flood_callback = callback;
end

-- allow the eVidx manipulations on no BPE ... for easier debug and configuration
local debug_checkOnlyOnBpeSystem = false

-- debug
local debug_on = true
local function _debug(...)
    if debug_on == true then
        printLog (...)
    end
end
-- function that operate 'to_string' on each parameter before call to '_debug()'
-- meaning this function can print also tables as parameters .
function _debug_to_string(...)
    local table = {...}

    for index,entry in pairs(table) do
        _debug(to_string(entry))
    end
end

local function print_error(...)
    print ("ERROR : " , ...)
end
-- print table :
-- parameters are :
-- title - string to print at start and end
-- dbName - the db name --> the actual table
local function print_table(title,dbName)
    title = title .. " table"

    print ("\n start entries of " .. title)
    --print ("_debug_to_string");
    --_debug_to_string(dbName)
    --print ("debug_table_print");
    debug_table_print(dbName)
    print (" end " .. title .. "\n")
end

-- *********************************************************************
-- *********************************************************************
-- *******************  DB declarations                  ***************
-- *********************************************************************
-- *********************************************************************
-- IMPORTANT :
-- the format of mcMacAddr (in all functions/DB) is :
--      {"00","01","02","03","04","05",
--        isMulticast=false,
--        string="00:01:02:03:04:05",
--        isBroadcast=false,
--        isUnicast=true
--      }

-- internal DB
-- the 'group' represented by 'key' of : {vlanId ,mcMacAddr.string}.
-- each 'entry' hold next info :
-- fdbIndex - index for entry if exists already
-- vidx - the vidx for the entry
-- eVidx - the eVidx for the entry
-- bpeInfo.grp - the bpe 802.1br inf - grp
-- bpeInfo.ecid - the bpe 802.1br inf - ecid
-- members - list of ports members

local db_mcGroupInfo = {}
local string_for_dummy_mac_for_vlan_indication = " vlan indication "

-- names of fields of each entry in the DB : db_mcGroupInfo
local db_field_fdbIndex = "fdbIndex"
local db_field_vidx = "vidx"
local db_field_eVidx = "eVidx"
local db_field_bpeInfo = "bpeInfo"
local db_field_members = "members"
local db_field_ttiIndexName = "ttiIndexName"

-- internal DB
-- the 'key' is the 'vidx' (usually 0..4K-1)
-- the entry hold next info :
-- 1. vlanId : the vlanId
-- 2. mcMacAddr : the multicast mac address
local db_vidxUsedArr = {}

-- names of fields of each entry in the DB : db_vidxUsedArr
local db_field_vidx_db_key_str = "db_key_str"

-- *********************************************************************
-- *********************************************************************
-- *******************  start manager of db_mcGroupInfo  ***************
-- *********************************************************************
-- *********************************************************************


-- return updated entry
local function db_updateEntryIfFieldNotNil(entry,fieldName,value)
    --_debug_to_string ("db_updateEntryIfFieldNotNil(entry,fieldName,value)" , entry,fieldName,value)

    -- set/override fields only if valid,
    -- if value == nil field unchanged
    if value == false then
        -- remove from db
        entry[fieldName] = nil
    elseif value then
        entry[fieldName] = value
    end
end

-- set entry in DB:
-- vlanId - the vlan Id
-- mcMacAddr - in format of :
--      {"00","01","02","03","04","05",
--        isMulticast=false,
--        string="00:01:02:03:04:05",
--        isBroadcast=false,
--        isUnicast=true
--      }
--
--      NOTE: but actual implementation use ONLY 'mcMacAddr.string'
--
-- if groupInfo == false --> remove entry
-- if groupInfo == nil --> ERROR --> runtime crash !!!!

-- if groupInfo.fdbIndex == false --> remove indication of fdbIndex
-- if groupInfo.fdbIndex == nil   --> db not change this parameter

-- if groupInfo.vidx == false --> remove indication of vidx
-- if groupInfo.vidx == nil   --> db not change this parameter

-- if groupInfo.eVidx == false --> remove indication of eVidx
-- if groupInfo.eVidx == nil   --> db not change this parameter

-- if groupInfo.bpeInfo == false --> remove indication of bpeInfo
-- if groupInfo.bpeInfo == nil   --> db not change this parameter
local function db_setMcGroupInfo(db_key_str,groupInfo)
    --_debug_to_string ("db_setMcGroupInfo(vlanId,mcMacAddr,groupInfo)" , vlanId,mcMacAddr,groupInfo)

    local index = db_key_str;

    if not groupInfo then
        -- remove from db
        db_mcGroupInfo[index] = nil
        -- nothing more to do
        return
    end

    if db_mcGroupInfo[index] == nil then
        db_mcGroupInfo[index] = {}
    end

  db_updateEntryIfFieldNotNil(db_mcGroupInfo[index],   db_field_fdbIndex,  groupInfo.fdbIndex)
  db_updateEntryIfFieldNotNil(db_mcGroupInfo[index],   db_field_vidx,      groupInfo.vidx)
  db_updateEntryIfFieldNotNil(db_mcGroupInfo[index],   db_field_eVidx,     groupInfo.eVidx)
  db_updateEntryIfFieldNotNil(db_mcGroupInfo[index],   db_field_bpeInfo,   groupInfo.bpeInfo)

    --_debug_to_string("db_setMcGroupInfo : db_mcGroupInfo[index]", index , db_mcGroupInfo[index])

    return
end

-- check if entry can be deleted
-- return true if 'empty' entry and can be removed
-- else return false
local function db_isEmptyMcGroupInfo(db_key_str)
    --_debug_to_string ("db_isEmptyMcGroupInfo(db_key_str)" , db_key_str)
    local index = db_key_str;

    if db_mcGroupInfo[index] == nil then
        return true
    end

    local entry = db_mcGroupInfo[index]

    if #entry == 0 then
        return true
    end

    return false

end

-- remove entry from the DB
local function db_deleteMcGroupInfo(db_key_str)
    db_setMcGroupInfo(db_key_str,false)
end

-- set a field in mc group entry
local function db_setMcGroupField(db_key_str,fieldName,value)
    --_debug_to_string ("db_setMcGroupField(db_key_str,fieldName,value)" , db_key_str,fieldName,value)

    local groupInfo = {}
    if value == nil then
        groupInfo[fieldName] = false
    else
        groupInfo[fieldName] = value
    end

    --_debug_to_string("db_setMcGroupField(db_key_str,fieldName,value)", db_key_str,fieldName,value)
    db_setMcGroupInfo(db_key_str,groupInfo)

    --_debug_to_string("db_setMcGroupField : db_mcGroupInfo", db_mcGroupInfo)
end

-- return the mc group field/full entry
local function db_getMcGroupField(db_key_str,fieldName)
    --_debug_to_string ("db_getMcGroupField(db_key_str,fieldName)" , db_key_str,fieldName)
    local index = db_key_str;

    --_debug_to_string ("db_getMcGroupField(db_key_str,fieldName)" , db_key_str,fieldName)

    local groupInfo = db_mcGroupInfo[index]
    if groupInfo == nil then
        return nil
  elseif fieldName == nil then
    return groupInfo
  end

    --_debug_to_string("db_getMcGroupField : db_mcGroupInfo[index]", index , db_mcGroupInfo[index])

    return groupInfo[fieldName] -- this still can be nil !!!
end

-- remove multi fields from the DB
-- fieldsArr{} is array of 'names of fields' (strings)
local function db_removeFieldsFromMcGroupInfo(db_key_str,fieldsArr)
    for index,entry in pairs(fieldsArr) do
        --entry is 'string' name of the field
        db_setMcGroupField(db_key_str,entry,nil)
    end
end

-- *********************************************************************
-- *********************************************************************
-- *******************  end manager of db_mcGroupInfo    ***************
-- *********************************************************************
-- *********************************************************************

-- *********************************************************************
-- *********************************************************************
-- *******************  start manager of db_vidxUsedArr  ***************
-- *********************************************************************
-- *********************************************************************
local lastVidxIndex = 0xFFF - 1 --(do not allow to allocate the 0xFFF index)
local firstVidxIndex = 5 --(just not value 0 ... bad for 'debug')

function bridge_multicast_set_last_valid_vidx_index(index)
    lastVidxIndex = index
    sharedResourceNamedVidxMemoReset({{base = firstVidxIndex, size = index}});
end

function bridge_multicast_get_last_valid_vidx_index()
    return lastVidxIndex
end

-- function to get the vidx associated with the db_key_str
-- the function check if DB already 'know' this db_key_str
-- if new vidx needed it is 'allocated' in the DB of db_vidxUsedArr[] and
-- associated into db_mcGroupInfo[]
-- otherwise new vidx 'allocated'
local function db_vidxGet(db_key_str)
    -- check if DB already allocated vidx for this db_key_str
    local vidx  = db_getMcGroupField(db_key_str, db_field_vidx)
    local found = true
    local vlanMacName = "FDB_GROUP," .. db_key_str;

    if vidx == nil then
        -- we need new vidx
        found = false
        vidx = sharedResourceNamedVidxMemoAlloc(vlanMacName, 1 --[[size--]])
    end

    if vidx == nil then
        -- ERROR
        print_error(" no more available VIDX ?! in range ["
           .. firstVidxIndex .. " .. " .. lastVidxIndex .. "]")
        return nil -- will cause crash at the caller
    end

    -- update the entry in the DB db_vidxUsedArr[]
    if db_vidxUsedArr[vidx] == nil then
        db_vidxUsedArr[vidx] = {}
    end

    -- update entry if not found in the DB : db_mcGroupInfo
    if found == false then
        --_debug_to_string ("db_vidxGet(db_key_str)" , db_key_str)
        db_vidxUsedArr[vidx][db_field_vidx_db_key_str]    = db_key_str
        -- associate this new vidx into db_mcGroupInfo[]
        db_setMcGroupField(db_key_str, db_field_vidx,vidx)
    end

    return vidx
end

-- free the vidx from the DB of db_vidxUsedArr[]
local function db_vidxFree(vidx)
    if db_vidxUsedArr[vidx] == nil then
        -- already not used
        return
    end

    local vlanMacName =  "FDB_GROUP," .. db_vidxUsedArr[vidx][db_field_vidx_db_key_str];

    local removedFields = {db_field_vidx}
    -- remove the field from db_mcGroupInfo[]
    db_removeFieldsFromMcGroupInfo(
        db_vidxUsedArr[vidx][db_field_vidx_db_key_str],
        removedFields)

    -- free the entry
    db_vidxUsedArr[vidx] = nil
    sharedResourceNamedVidxMemoFree(vlanMacName);
    return
end

-- global function to get key string from db of db_vidxUsedArr[vidx]
function bridge_multicast_db_key_str_from_db_get(vidx)
    local entry, vlanIdEntry, vlanId, vid1;
    if db_vidxUsedArr[vidx] == nil then
        -- not used
        return nil
    end

    return db_vidxUsedArr[vidx][db_field_vidx_db_key_str];
end

-- update the vidx to new vidx in the DB of db_vidxUsedArr[]
local function db_vidxUpdate(origVidx,newVidx)
    db_vidxUsedArr[newVidx] = db_vidxUsedArr[origVidx]
    db_vidxUsedArr[origVidx] = nil

    local db_key_str = db_vidxUsedArr[newVidx][db_field_vidx_db_key_str];

    db_setMcGroupField(db_key_str, db_field_vidx, newVidx);
end

-- update the vidx to new vidx in the DB of db_vidxUsedArr[]
local function db_vidxSet(db_key_str, vidx)
    --_debug_to_string ("db_vidxSet(vlanId,mcMacAddr,vidx)" , vlanId,mcMacAddr,vidx)
    local vlanMacName = db_key_str;
--    _debug_to_string("vlanId",vlanId)
--    _debug_to_string("mcMacAddr",mcMacAddr)
--    _debug_to_string("vidx",vidx)
--    _debug_to_string("db_vidxUsedArr",db_vidxUsedArr)
    if db_vidxUsedArr[vidx] == nil then
        db_vidxUsedArr[vidx] = {}
    end
    db_vidxUsedArr[vidx][db_field_vidx_db_key_str]    = db_key_str;
    -- associate this new vidx into db_mcGroupInfo[]
    db_setMcGroupField(db_key_str, db_field_vidx, vidx);
end


-- get vidx from the DB for the specific db_key_str
-- it should be used when the FDB entry may hold eVidx that represents vidx
-- return vidx from DB --> can be nil
function bridge_multicast_mac_address_get_vidx_from_db(db_key_str)
    -- do not use db_vidxGet() .. so in case 'group' not in DB it will not allocate new vidx
    local vidx   = db_getMcGroupField(db_key_str, db_field_vidx)

    return vidx
end

--'move' the entry from 'origIndex' to 'newIndex'
local function move_vidx_entry(command_data,devNum,origIndex,newIndex)
    -- read the orig entry
    local apiName = "cpssDxChBrgMcEntryRead"
    local isError , result, values = genericCpssApiWithErrorHandler(command_data,
        apiName ,{
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U16",     "vidx",    origIndex },
        { "OUT",     "CPSS_PORTS_BMP_STC",  "portBitmap"}
    })
    if isError == true then
        return
    end

    local portBitmap = values.portBitmap

    -- copy to the new entry
    apiName = "cpssDxChBrgMcEntryWrite"
    genericCpssApiWithErrorHandler(command_data,
        apiName ,{
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U16",     "vidx",    newIndex },
        { "IN",     "CPSS_PORTS_BMP_STC",  "portBitmap" , portBitmap}
    })

    -- delete the orig entry
    apiName = "cpssDxChBrgMcGroupDelete"
    genericCpssApiWithErrorHandler(command_data,
        apiName ,{
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U16",     "vidx",    origIndex }
    })

end

-- we need to 'copy the vlan' entry members into the 'vidx entry'
local function copy_vidx_entry_from_vlan_entry(command_data,devNum,vlanId,vidx)
    -- get members from the vlan entry
    local apiName = "cpssDxChBrgVlanEntryRead"
    local isError , result, values = genericCpssApiWithErrorHandler(command_data,
        apiName ,{
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U16",     "vlanId",    vlanId },
        { "OUT",     "CPSS_PORTS_BMP_STC",  "portsMembers"},
        { "OUT",     "CPSS_PORTS_BMP_STC",  "portsTagging"},
        { "OUT",     "CPSS_DXCH_BRG_VLAN_INFO_STC",  "vlanInfo"},
        { "OUT",     "GT_BOOL",  "isValid"},
        { "OUT",     "CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC",  "portsTaggingCmd"}
    })
    if isError == true then
        return
    end

    local portBitmap = values.portsMembers

    -- copy to the vidx entry
    apiName = "cpssDxChBrgMcEntryWrite"
    genericCpssApiWithErrorHandler(command_data,
        apiName ,{
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_U16",     "vidx",    vidx },
        { "IN",     "CPSS_PORTS_BMP_STC",  "portBitmap" , portBitmap}
    })

end

-- function on PE , non eArch device
-- bind the grp,ecid to the mc group
function func_onPe_non_eArch_bpe_802_1_br_mc_etag_info_bind_grp_ecid(
    command_data,vlanId,mcMacAddr,devNum,grp,ecid,unset)
    local unicast_mac = {string = string_for_dummy_mac_for_vlan_indication}
    local is_per_vlan
    local apiName

    local macString = nil;
    if mcMacAddr then
        macString = mcMacAddr.string;
    end
    local db_key_str = command_data_getVidMacKeyString(vlanId, macString);

    if mcMacAddr == nil then
        mcMacAddr = unicast_mac
        is_per_vlan = true
    else
        is_per_vlan = false
    end

    local current_vidx  = db_getMcGroupField(db_key_str, db_field_vidx)

    local new_vidx
    if grp and ecid then
        new_vidx = bpe_802_1_br_mc_eVidx_get(grp,ecid)
    else
        new_vidx = current_vidx
    end

    if unset == true then
        if current_vidx then
            db_vidxFree(current_vidx)
        end
        -- delete group from DB
        db_deleteMcGroupInfo(db_key_str)
        if current_vidx then
            -- delete the entry
            apiName = "cpssDxChBrgMcGroupDelete"
            genericCpssApiWithErrorHandler(command_data,
                apiName ,{
                { "IN",     "GT_U8",      "devNum",     devNum },
                { "IN",     "GT_U16",     "vidx",    current_vidx }
            })
        end
    elseif current_vidx == nil then
        -- new vidx into db about the new vidx
        db_vidxSet(db_key_str,new_vidx)
    elseif new_vidx ~= current_vidx then
        if is_per_vlan == false then
            -- we need to 'move' the entry from 'current_vidx' to 'new_vidx'
            move_vidx_entry(command_data,devNum,current_vidx,new_vidx)
        end

        -- update db about the new vidx
        db_vidxUpdate(current_vidx,new_vidx)
    end

    if is_per_vlan == true and unset ~= true then
        -- we need to 'copy the vlan' entry members into the 'vidx entry'
        copy_vidx_entry_from_vlan_entry(command_data,devNum,vlanId,new_vidx)
    end

    if unset ~= true then
        -- bind the pcid,grp to the entry
        local new_bpeInfo = {["grp"] = grp,["ecid"] = ecid}
        db_setMcGroupField(db_key_str, db_field_bpeInfo, new_bpeInfo)
    end

end


--constants

-- cpssDxChBrgMcIpv6BytesSelectSet not supported by automaict XML generator
-- and yet has no Manual wrappers, but the needed configuratin is default:
-- bytes 12,13,14,15
-- "\014\015\016\017" also supposes octal numbers, but LUA treats them as decimal
local ipv6BytesSelectDone = {}
-- Sets last 4 bytes from the IPv6 SIP and DIP are used upon IPv6 MC bridging.
local function check_ipv6BytesSelect(devNum)
    -- bypass function body
    ipv6BytesSelectDone[devNum] = true
    return true
    --[[ original code
    if ipv6BytesSelectDone[devNum] == true then
        return true
    end
    local ret = cpssGenWrapper("cpssDxChBrgMcIpv6BytesSelectSet", {
        {"IN","GT_U8","devId", devNum},
        {"IN","string","dipmask","\014\015\016\017"},
        {"IN","string","sipmask","\014\015\016\017"}})
    if ret == 0 then
        ipv6BytesSelectDone[devNum] = true
        return true
    end
    return false
    --]]
end

-- ************************************************************************
---
--  bridge_multicast_address_group_func
--        @description  registes MAC-layer multicast address to the bridge
--                      table and assign interfaces to this multicast
--                      address
--
--        @param params             - params[%mac-address]: MAC multicast
--                                    address;
--                                    params["ipv4"]: ip v4, could be
--                                    irrelevant;
--                                    params["ipv4_mc"]: multicast ip v4
--                                    address, could be irrelevant (related
--                                    to params["sip"] or params["dip"]);
--                                    params["dip"]: multicast dip
--                                    property, could be irrelevant
--                                    (related to params["ipv4_mc"]);
--                                    params["sip"]: multicast sip
--                                    property, could be irrelevant
--                                    (related to params["ipv4_mc"]);
--                                    params["ipv6"]: ip v6, could be
--                                    irrelevant;
--                                    params["ipv6_mc"]: multicast ip v6
--                                    address, could be irrelevant (related
--                                    to params["sip"] or params["dip"]);
--                                    params["dip"]: multicast dip
--                                    property, could be irrelevant
--                                    (related to params["ipv6_mc"]);
--                                    params["sip"]: multicast sip
--                                    property, could be irrelevant
--                                    (related to params["ipv6_mc"]);
--                                    params["add"]: adding property of
--                                    ethernet port range or trunk, could be
--                                    irrelevant ;
--                                    params["add"]: adding property of
--                                    ethernet port range or trunk, could be
--                                    irrelevant ;
--                                    params["ethernet"]: entry ethernet
--                                    interface dev/port range, could be
--                                    irrelevant;
--                                    params["port-channel"]: entry trunk,
--                                    could be irrelevant;
--
--        @usage common_global      - common_global[ifRange]: dev/vlan
--                                    range
--
--        @return       true on success, otherwise false and error message
--
function bridge_multicast_address_group_func(params)
    -- Common variables declaration
    local result, values
    local devNum, db_key_str, key_err;
    local command_data = Command_Data()
    -- Command specific variables declaration
    local dev_vlan_ports, does_ports_added
    local vidx, empty_vidx, eVidx, dummyVidx;
    local mac_entry_index, vlan_multicast_entries_existing
    local fdb_vidx -- vidx to be used by the FDB entry (can be 'eVidx')
    local callback_ret, dontUpdateVidxEntry, dontUpdateFdbEntry;
    local full_port_list;

    local vid1 = params["vid1"];
    -- print("params " .. to_string(params));

    -- Common variables initialization
    command_data:initInterfaceDevVlanRange()
    command_data:initInterfaceDeviceRange("dev_vlan_range")
    command_data:initDevPortRange(params, true, true)
    command_data:initDevicesVidxRange()

    -- filter ports >= 256 (as eports) from ports for vlan
    full_port_list              = command_data:getDevicePortRange()
    --print("full_port_list",to_string(full_port_list))
    dev_vlan_ports              = {}
    dev_vlan_ports["added"]     = port_list_limited_copy(full_port_list, 0, 511)--511 to support Bobcat3

    -- Command specific variables initialization
    empty_vidx                  = -1
    does_ports_added            = getTrueIfNotNil(dev_vlan_ports["added"])
    -- Device vlans ports configuring.
    if (true == command_data["status"])             and
       (true == does_ports_added)                   then
        command_data:configureDeviceVlanRange(dev_vlan_ports)
        command_data:addWarningIfNoSuccessVlans(
            "Can not configure ports of all processed vlans.")
    end

    -- Main vlans handling cycle
    local iterator;
    for iterator, devNum, vlanId in
                                command_data:getInterfaceVlanIterator() do
        command_data:clearVlanStatus()
        command_data:clearLocalStatus()
        command_data:initAllDeviceMacEntryIterator(
            params, { ["key"] = {
                        ["key"] = {
                          ["macVlan"] = {
                            ["vlanId"]  = vlanId 
                          },
                        },
                      },
                      ["dstInterface"]  = { ["type"]  = "CPSS_INTERFACE_VIDX_E" },
                      ["isStatic"]      = true })
       db_key_str, key_err = command_data:getMacEntryFilterKeyString();

       -- Create(add) vlanId for IPv4 and IPv6 (ipMcast) cases of CPSS_MAC_ENTRY_EXT_KEY_STC
       if ((command_data["mac_filter"]["key"] or {})["key"] or {})["ipMcast"] ~= nil and
          (((command_data["mac_filter"]["key"] or {})["key"] or {})["macVlan"] or {})["vlanId"] ~= nil
       then
             command_data["mac_filter"]["key"]["key"]["ipMcast"]["vlanId"] = 
                      command_data["mac_filter"]["key"]["key"]["macVlan"]["vlanId"]
       end
       
       -- printMsg("DBG:bridge_MC KeyString: " .. to_string(db_key_str) .. "-- " .. to_string(key_err));
       -- printMsg("DBG:bridge_MC Filter: ".. to_string(command_data["mac_filter"]));

        callback_ret        = nil;
        vidx                = nil;
        eVidx               = nil;
        dontUpdateVidxEntry = nil;
        dontUpdateFdbEntry  = nil;
        if bridge_multicast_address_group_flood_callback then
            callback_ret = bridge_multicast_address_group_flood_callback(
               command_data, devNum, db_key_str,
               full_port_list[devNum] --[[port_range--]],
               db_getMcGroupField(db_key_str, db_field_vidx)--[[vidx--]],
               db_getMcGroupField(db_key_str, db_field_eVidx)--[[eVidx--]],
               false --[[unset--]]);
        end
        if callback_ret then
            vidx                = callback_ret.vidx;
            eVidx               = callback_ret.eVidx;
            dontUpdateVidxEntry = callback_ret.dontUpdateVidxEntry;
            dontUpdateFdbEntry  = callback_ret.dontUpdateFdbEntry;
        end

        -- attempt to retrieve vidx/eVidx by DB
        if not vidx then
            vidx   = db_getMcGroupField(db_key_str, db_field_vidx)
        end
        if not eVidx then
            -- check if DB hold eVidx to represent 'L2MLL' for this {vlan,mac}
            -- if 'yes' then the FDB should use it instead of the 'vidx'
            -- but NOTE: the vidx is still allocated , as it may be pointed by the
            -- L2MLL for physical ports (such as DSA tag cascade ports and for 'bpe 802.1 br cascade ports')
            eVidx  = db_getMcGroupField(db_key_str, db_field_eVidx)
        end
        if (not eVidx) and (not vidx) then
            -- get from DB existing/new vidx for this {vlan,mac}
            vidx = db_vidxGet(db_key_str)
            if vidx == nil then
                command_data:setFailVlanAndLocalStatus()
                command_data:addWarning(
                    " no more available VIDX ?! in range [0..%d]", lastVidxIndex)
            end
        end
        if eVidx == nil then
            fdb_vidx = vidx
        else
            fdb_vidx = eVidx
        end

        vlan_multicast_entries_existing = false;
        -- Checking of vlan multicast mac-entries existence.
        if (not dontUpdateFdbEntry) and command_data["local_status"] then
           local mac_filter_ = command_data["mac_filter"]
           
           -- printMsg("DBG:bridge_MC wrlCpssDxChBrgIsEntryExists mac_filter=".. to_string(mac_filter_));
           
           result, values, mac_entry_index, dummyVidx =
                wrLogWrapper("wrlCpssDxChBrgIsEntryExists" , 
                              "(devNum, 0, mac_filter_)", 
                              devNum, 0, mac_filter_)
            if        0 == result   then
                vlan_multicast_entries_existing = values
                if vlan_multicast_entries_existing
                    and fdb_vidx ~= dummyVidx
                then
                    command_data:addWarning(
                        "DB wrong state:" ..
                        " vidx in DB: " .. tostring(fdb_vidx) ..
                        " vidx in HW: " .. tostring(dummyVidx) ..
                        " mac_entry_index: " .. tostring(mac_entry_index));
                end

            elseif 0x10 == result   then
                command_data:setFailVlanStatus()
                command_data:addWarning("It is not allowed to get multicast " ..
                                        "mac-entries of vlan %d on device %d.",
                                        vlanId, devNum)
            elseif    0 ~= result   then
                command_data:setFailVlanAndLocalStatus()
                command_data:addError("Error at multicast mac-entries " ..
                                      "getting of vlan %d on device %d: %s.",
                                      vlanId, devNum, values)
            end
        end

        -- Adding of new vlan multicast mac-entry.
        if ((not dontUpdateFdbEntry) and command_data["local_status"]) and
            (false == vlan_multicast_entries_existing)    then

            -- Forming of mac entry.
            if true == command_data["local_status"] then
                -- Vlan mac-entries iteration.
                command_data:initAllDeviceMacEntryIterator(
                    params, { ["key"]= { 
                                ["key"] = {
                                  ["macVlan"] = {
                                    ["vlanId"]  = vlanId 
                                  },
                                },
                              },
                              ["dstInterface"]  = { ["type"]    =
                                                    "CPSS_INTERFACE_VIDX_E" },
                              ["isStatic"]      = true })
                db_key_str, key_err = command_data:getMacEntryFilterKeyString();
                command_data["mac_entry"] = command_data["mac_filter"]
                mergeTablesRightJoin(command_data["mac_entry"],
                                     { ["dstInterface"] =
                                         { ["vidx"] = fdb_vidx }})
            end

            -- Writing of mac-address with vidx.
            if command_data["local_status"] then
                check_ipv6BytesSelect(devNum)
                
                -- printMsg("DBG:bridge_MC mac_entry_set mac_entry=".. to_string(command_data["mac_entry"]));

                -- Create(add) vlanId for IPv4 and IPv6 (ipMcast) cases of CPSS_MAC_ENTRY_EXT_KEY_STC
                if ((command_data["mac_entry"]["key"] or {})["key"] or {})["ipMcast"] ~= nil and
                   (((command_data["mac_entry"]["key"] or {})["key"] or {})["macVlan"] or {})["vlanId"] ~= nil
                then
                    command_data["mac_entry"]["key"]["key"]["ipMcast"]["vlanId"] = 
                          command_data["mac_entry"]["key"]["key"]["macVlan"]["vlanId"]
                end
                
                result, values = mac_entry_set(devNum,
                                               command_data["mac_entry"])
                if     0x10 == result   then
                    command_data:setFailVlanStatus()
                    command_data:addWarning("It is not allowed to write " ..
                                            "mac-entry with vidx %d on " ..
                                            "device %d.", empty_vidx,
                                            devNum)
                elseif    0 ~= result   then
                    command_data:setFailVlanAndLocalStatus()
                    command_data:addError("Error at mac-entry writing with " ..
                                          "vidx %d on device %d: %s.",
                                          empty_vidx, devNum, values)
                end
            end
        end

        command_data:initAllDeviceMacEntryIterator({}, command_data["mac_entry"])
        -- db_key_str not recalculated here becase DB entry registred by
        -- fields from command line, but the relevant field ommited in
        -- command line and retieved from FDB entry must not present
        -- in DB key string.

        -- Create(add) vlanId for IPv4 and IPv6 (ipMcast) cases of CPSS_MAC_ENTRY_EXT_KEY_STC
        if ((command_data["mac_filter"]["key"] or {})["key"] or {})["ipMcast"] ~= nil and
           (((command_data["mac_filter"]["key"] or {})["key"] or {})["macVlan"] or {})["vlanId"] ~= nil
        then
            command_data["mac_filter"]["key"]["key"]["ipMcast"]["vlanId"] = 
                  command_data["mac_filter"]["key"]["key"]["macVlan"]["vlanId"]
        end

        -- update the 'groups' DB about the FDB entry  index
        if ((not dontUpdateFdbEntry) and command_data["local_status"]) and
           vlan_multicast_entries_existing == false -- indication that the fdb entry added (by message) during this action
        then
           result, values, mac_entry_index, dummyVidx =
                wrlCpssDxChBrgIsEntryExists(devNum, 0,
                                            command_data["mac_filter"])
            if 0 == result   then
                -- update the DB about the used fdbIndex
                db_setMcGroupField(db_key_str, db_field_fdbIndex ,mac_entry_index)
                -- printMsg("DBG:bridge_MC db_setMcGroupField  db_key_str=".. to_string(db_key_str));
                -- printMsg("DBG:bridge_MC db_setMcGroupField  db_field_fdbIndex=".. to_string(db_field_fdbIndex));
                -- printMsg("DBG:bridge_MC db_setMcGroupField  mac_entry_index=".. to_string(mac_entry_index));
            end
        end

        -- write VIDX table entry
        if (not dontUpdateVidxEntry) and vidx and command_data["local_status"] then
            command_data["dev_vidx_range"][devNum] = {vidx};

            -- this function will write the HW with the vidx entry
            command_data:configureDeviceVidxRange(dev_vlan_ports)
            command_data:addWarningIfNoSuccessVidixes(
                "Can not configure port members of processed vidx.")
        end
        -- print("End -- dev_vlan_ports" .. to_string(dev_vlan_ports))

        command_data:updateStatus()

        command_data:updateVlans()
    end


    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_bridge_multicast_mac_address_group_func
--        @description  unregistes MAC-layer multicast address to the bridge
--                      table and assign interfaces to this multicast
--                      address
--
--        @param params             - params[%mac-address]: MAC multicast
--                                    address;
--                                    params["ipv4_mc"]: multicast ip v4
--                                    address, could be irrelevant (related
--                                    to params["sip"] or params["dip"]);
--                                    params["dip"]: multicast dip
--                                    property, could be irrelevant
--                                    (related to params["ipv4_mc"]);
--                                    params["sip"]: multicast sip
--                                    property, could be irrelevant
--                                    (related to params["ipv4_mc"]);
--                                    params["ipv6_mc"]: multicast ip v6
--                                    address, could be irrelevant (related
--                                    to params["sip"] or params["dip"]);
--                                    params["dip"]: multicast dip
--                                    property, could be irrelevant
--                                    (related to params["ipv6_mc"]);
--                                    params["sip"]: multicast sip
--                                    property, could be irrelevant
--                                    (related to params["ipv6_mc"]);
--
--        @usage common_global      - common_global[ifRange]: dev/vlan
--                                    range
--
--        @return       true on success, otherwise false and error message
--
function no_bridge_multicast_address_group_func(params)
    -- Common variables declaration
    local result, values
    local iterator, devNum, vlanId, loopDevNum;
    local command_data = Command_Data()
    -- Command specific variables declaration
    local vidx, eVidx, mac_address, sip, dip
    local contextVlanId
    local callback_ret, dontUpdateVidxEntry, dontUpdateFdbEntry;
    local db_key_str, key_err;

    -- print("params = " .. to_string(params))
    -- Common variables initialization
    command_data:initInterfaceDevVlanRange()
    command_data:initInterfaceDeviceRange("dev_vlan_range")

    -- this needed so we will not remove groups from other vlans
    for iterator, loopDevNum, vlanId in
        command_data:getInterfaceVlanIterator() do
        -- get the vlanId of our context
        contextVlanId = vlanId;
        devNum = loopDevNum;
        break
    end

    command_data:initAllDeviceMacEntryIterator(
        params, {["key"]= { 
                  ["key"] = {
                    ["macVlan"] = {
                      ["vlanId"]  = contextVlanId
                    },
                  },
                 } ,
                 ["dstInterface"]  = { ["type"] = "CPSS_INTERFACE_VIDX_E" }})

    db_key_str, key_err = command_data:getMacEntryFilterKeyString();
    --print("KeyString: " .. to_string(db_key_str) .. "-- " .. to_string(key_err));
    --print("Filter: ".. to_string(command_data["mac_filter"]));
    -- check if the DB is aware to this entry ...
    -- as the implementation may not use the FDB as 'forwarding mechanism' .. .
    -- but may use the TTI/IPCL redirect to vidx !
    local current_fdbIndex  = db_getMcGroupField(db_key_str, db_field_fdbIndex)
    local current_vidx  = db_getMcGroupField(db_key_str, db_field_vidx)
    local done_processing = false

    callback_ret        = nil;
    vidx                = nil;
    eVidx               = nil;
    dontUpdateVidxEntry = nil;
    dontUpdateFdbEntry  = nil;
    if bridge_multicast_address_group_flood_callback then
        callback_ret = bridge_multicast_address_group_flood_callback(
           command_data, devNum, db_key_str,
           nil --[[port_range--]],
           db_getMcGroupField(db_key_str, db_field_vidx)--[[vidx--]],
           db_getMcGroupField(db_key_str, db_field_eVidx)--[[eVidx--]],
           true --[[unset--]]);
    end
    if callback_ret then
        vidx                = callback_ret.vidx;
        eVidx               = callback_ret.eVidx;
        dontUpdateVidxEntry = callback_ret.dontUpdateVidxEntry;
        dontUpdateFdbEntry  = callback_ret.dontUpdateFdbEntry;
    end

    if current_vidx and not current_fdbIndex then
        -- the DB knows about the {mac+vlan} as vidx without FDB entry !
        -- all needed already done in callback

        done_processing = true
    end

    if done_processing == false then

        -- make sure to iterate only on mac addresses in 'our vlan'


        -- Main entries handling cycle
        if true == command_data["status"] then
            local iterator
            local entryArr = {}
            
            for iterator, devNum, entry_index in
                            command_data:getValidMacEntryIterator() do
                
                entryArr[iterator] = {devNum=devNum, entry_index=entry_index}
            end
            
            for iterator, entry in pairs(entryArr) do
                local devNum = entry.devNum
                local entry_index = entry.entry_index

                command_data:clearEntryStatus()

                -- Mac address entry getting.
                command_data:clearLocalStatus()

                if not eVidx then
                    -- when we have eVidx for this group , it will be in the FDB enty and not the vidx
                    -- so in such case take info from the DB
                    eVidx  = db_getMcGroupField(db_key_str, db_field_eVidx)
                end
                if not vidx then
                    -- do not use db_vidxGet() .. so in case 'group' not in DB it will not allocate new vidx
                    vidx   = db_getMcGroupField(db_key_str, db_field_vidx)
                    if command_data["mac_entry"] ~= nil and 
                       (eVidx == nil or vidx == nil) then
                        vidx        = command_data["mac_entry"]["dstInterface"]["vidx"]
                    end
                end

                -- Deleting of vidx.
                if (not dontUpdateVidxEntry) and vidx and command_data["local_status"] then
                    result, values =
                        cpssPerDeviceParamSet("cpssDxChBrgMcGroupDelete",
                                              devNum, vidx)
                    if     0x10 == result   then
                        command_data:setFailEntryStatus()
                        command_data:addWarning("It is not allowed to delete " ..
                                                "vidx %d on device %d.", vidx,
                                                devNum)
                    elseif    0 ~= result   then
                        command_data:setFailEntryAndLocalStatus()
                        command_data:addError("Error at %d vidx deleting on " ..
                                              "device %d: %s.", vidx, devNum,
                                              returnCodes[result])
                    end
                end

                -- Invalidating of mac-entry.
                if (not dontUpdateFdbEntry) and command_data["local_status"] then
                    result, values = mac_entry_invalidate(devNum, entry_index)

                    if     0x10 == result   then
                        command_data:setFailEntryStatus()
                        command_data:addWarning("It is not allowed to invalidate " ..
                                                "mac-entry %s on device %d.",
                                                db_key_str,
                                                devNum)
                    elseif    0 ~= result   then
                        command_data:setFailEntryAndLocalStatus()
                        command_data:addError("Error at %s mac-entry invalidating " ..
                                              "on device %d: %s.",
                                              db_key_str,
                                              devNum, returnCodes[result])
                    end
                end

                if true == command_data["local_status"] then
                    if (not dontUpdateVidxEntry) and vidx then
                        -- remove the vidx from the DB
                        db_vidxFree(vidx)
                    end

                    if (not dontUpdateFdbEntry) then
                        -- delete group from DB
                        db_deleteMcGroupInfo(db_key_str)
                    end
                end

                command_data:updateStatus()

                command_data:updateEntries()
            end
            
            command_data:addErrorIfNoSuccessEntries(
                "Error : Can not de-register all processed mac-entries.")
        end
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


----------------------------------------------------------------
-- command registration: bridge multicast mac address group
----------------------------------------------------------------
CLI_addHelp("vlan_configuration", "bridge",
                                    "Bridge configuration subcommand")
CLI_addHelp("vlan_configuration", "bridge multicast",
                                    "Multicast Bridging configuration commands")
CLI_addHelp("vlan_configuration", "bridge multicast mac",
                                    "Multicast Bridging configuration mac-address commands")
CLI_addHelp("vlan_configuration", "bridge multicast mac address",
                                    "Multicast Bridging configuration mac-address commandss")
CLI_addCommand("vlan_configuration", "bridge multicast mac address group", {
    func   = bridge_multicast_address_group_func,
    help   = "Forward a particular Ethernet multicast address",
    params = {
        { type = "values",
          "%mac-address-bc-mc"
        },
        { type = "named",
          "#interface",
            {format = "vid1 %unchecked-vlan"},
          requirements = {["vid1"] = {"interface"}},
          mandatory = { "interface" }
        }
    }
})

----------------------------------------------------------------
-- command registration: no bridge multicast mac address group
----------------------------------------------------------------
CLI_addHelp("vlan_configuration", "no bridge",
                                    "Bridge configuration subcommand")
CLI_addHelp("vlan_configuration", "no bridge multicast",
                                    "Multicast Bridging configuration commands")
CLI_addHelp("vlan_configuration", "no bridge multicast mac",
                                    "Multicast Bridging configuration mac-address commands")
CLI_addHelp("vlan_configuration", "no bridge multicast mac address",
                                    "Multicast Bridging configuration mac-address commands")
CLI_addCommand("vlan_configuration", "no bridge multicast mac address group", {
  func   = no_bridge_multicast_address_group_func,
  help   = "Forward a particular Ethernet multicast address",
  params = {
      { type = "values",
          "%mac-address-bc-mc"
      },
      {
          type = "named",
          {format = "vid1 %unchecked-vlan"}
      }
  }
})

-- update fdb entry by index
-- this function needed for update exist entry and not for 'new entry'
local function updateFdbEntryByIndex(command_data,devNum,fdbIndex,fieldsToUpdate)

    local apiName = "cpssDxChBrgFdbMacEntryRead"
    local isError, result, values

    --_debug_to_string(apiName , devNum,fdbIndex,fieldsToUpdate)

    local isError , result, values = genericCpssApiWithErrorHandler(command_data,
        apiName,{
            { "IN", "GT_U8", "dev", devNum}, -- devNum
            { "IN", "GT_U32", "index", fdbIndex },
            { "OUT","GT_BOOL", "valid" }, -- *validPtr,
            { "OUT","GT_BOOL", "skip" }, -- *skipPtr,
            { "OUT","GT_BOOL", "aged" }, -- *agedPtr,
            { "OUT","GT_U8", "devNum" },   -- *associatedDevNumPtr,
            { "OUT","CPSS_MAC_ENTRY_EXT_STC", "entry" } --  *entryPtr
        })

    --_debug_to_string(values)
    local macEntry, skip
    if isError == false then
        macEntry = values.entry
        -- combine the exist entry with fields to be updated
        mergeTablesRightJoin(macEntry,fieldsToUpdate)
        skip = false
        -- call to write the updated entry
        result, values = mac_entry_write(devNum, fdbIndex, skip, macEntry)
    end

end

-- callback for bpe_802_1_br (for bridge_multicast_address_group_flood_callback(...))
function bridge_multicast_address_group_bpe_802_1_br_mc_callback(
    command_data, devNum, db_key_str, port_range, vidx, eVidx, unset)
    local vidx;
    if not bpe_802_1_br_is_system() then
        return nil;
    end
    if bpe_802_1_br_is_system_with_non_eArch_Device() then
        if not unset then
            -- non eArch device in br802.1 br , not need the FDB to hold the vidx ,
            -- because get redirect explicit logic from TTI unit
            -- get the associated vidx of the entry
            vidx = db_vidxGet(db_key_str)
            if vidx == nil then
                command_data:setFailVlanAndLocalStatus()
                command_data:addWarning(" no more available VIDX ?! in range [0..%d]", lastVidxIndex)
            end
            return {vidx = vidx, dontUpdateFdbEntry = true};
        else
            vidx  = db_getMcGroupField(db_key_str, db_field_vidx)
            -- PE : non-eArch device
            if vidx then
                local vlanId       = command_data["mac_filter"]["key"]["key"]["macVlan"]["vlanId"];
                local mc_group_mac = command_data["mac_filter"]["key"]["key"]["macVlan"]["mac_entry"];
                func_onPe_non_eArch_bpe_802_1_br_mc_etag_info_bind_grp_ecid(
                    command_data,vlanId,mc_group_mac,devNum,nil,nil,true)

                -- delete the entry
                local apiName = "cpssDxChBrgMcGroupDelete"
                genericCpssApiWithErrorHandler(command_data,
                    apiName ,{
                    { "IN",     "GT_U8",      "devNum",     devNum },
                    { "IN",     "GT_U16",     "vidx",    vidx }
                })

                return {dontUpdateFdbEntry = true};
            end
            -- default processing (maybe using predefined eVidx)
            return nil;
        end
    else -- earch device
        if not unset then
            if bpe_802_1_br_is_control_bridge() == false then -- port extender
                -- FDB not needed as we are using the TTI to assign eVidx for the packet.
                -- the eVidx will hold 'vidx' member.
                -- the vidx will hold the physical ports.
                return {dontUpdateFdbEntry = true}
            end
        
            -- nothing needed, eVidx added to DB by another commands
        else
            -- remove settings from the L2Mll and the L2Mll-LTT
            -- extern function evidx_flood_via_l2mll_chain_remove(devNum, vlanId,vidx)
            vidx  = db_getMcGroupField(db_key_str, db_field_vidx)
            if vidx == nil then
                command_data:setFailVlanAndLocalStatus()
                command_data:addWarning(" no more available VIDX ?! in range [0..%d]", lastVidxIndex)
            end
            evidx_flood_via_l2mll_chain_remove(devNum, nil, vidx)
            
            if bpe_802_1_br_is_control_bridge() == false then -- port extender
                -- FDB not needed as we used the TTI to assign eVidx for the packet.

                -- remove the TTI rule
                func_onPe_bpe_802_1_br_mc_etag_info_bind_grp_ecid__tti_set(
                    command_data,db_key_str,devNum,
                    nil--[[vlanId]],nil--[[grp]],nil--[[ecid]],nil--[[eVidx]],
                    true--[[isMc]],
                    true--[[unset entry]])
                    
                if vidx then
                    db_vidxFree(vidx)
                end
                local vlanId       = command_data["mac_filter"]["key"]["key"]["macVlan"]["vlanId"];
                local mc_group_mac = command_data["mac_filter"]["key"]["key"]["macVlan"]["mac_entry"];
                -- delete group from DB
                db_deleteMcGroupInfo(db_key_str)
                if vidx then
                    -- delete the entry
                    local apiName = "cpssDxChBrgMcGroupDelete"
                    genericCpssApiWithErrorHandler(command_data,
                        apiName ,{
                        { "IN",     "GT_U8",      "devNum",     devNum },
                        { "IN",     "GT_U16",     "vidx",    vidx }
                    })
                end

                return {dontUpdateFdbEntry = true}

            end
            
        end
    end
    -- default processing (maybe using predefined eVidx)
    return nil;
end

-- ON PE (eArch): function to set TTI rule for downstream of 'multi-destination' (by <GRP>,<ecid>)
-- to redirect to eVidx
function func_onPe_bpe_802_1_br_mc_etag_info_bind_grp_ecid__tti_set(
    command_data,db_key_str,devNum,vlanId,grp,ecid,eVidx,isMc,unset)

    -- set the TTI rule + action
    local ttiIndex
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local ttiAction = {}
    local egressInterface = {}
    local apiName

    local myRuleSize = 1
    local ruleType = "CPSS_DXCH_TTI_RULE_UDB_10_E"

    -- check if fdb entry already exists
    local myIndexName  = db_getMcGroupField(db_key_str, db_field_ttiIndexName)

    if myIndexName ~= nil then
        -- we have TTI entry to update : the rule (new <GRP>,<ecid>) and the action (new eVidx)
    else
        -- we need to create new TTI rule for this <GRP>,<ecid>
        if isMc == true then
            myIndexName = "BPE: Downstream Registered MC (redirect to eVidx), device " .. devNum .. " " .. db_key_str
        else
            myIndexName = "BPE: Downstream Vlan Flooding (redirect to eVidx), device " .. devNum .. " " .. db_key_str
        end
    end

    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti1")
    
    ttiAction = {}

    -- tag0 can keep it's tagging from before the 'pop'
    ttiAction.tag0VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    -- assign 'my vid' for untagged with those <grp>,<ecid>
    ttiAction.tag0VlanId = vlanId -- used for tr101 tagging of egress port
    ttiAction.tag1VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

    ttiAction['bridgeBypass']           = GT_TRUE
    ttiAction['ingressPipeBypass']      = GT_TRUE

    ttiAction['redirectCommand']           = "CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E"

    egressInterface.type = "CPSS_INTERFACE_VIDX_E"
    egressInterface.vidx = eVidx

    ttiAction['egressInterface'] = egressInterface

    finalTtiAction.type2 = ttiAction


    finalTtiPattern.udbArray , finalTtiMask.udbArray =
        bpe_802_1_br_buildTtiPatternMask(onPe_upstream_port,--physicalPort
            true,--ETag_exists
            grp,-- grp
            ecid,-- ecid_base
            nil) -- ingress_ecid_base

    if unset == true then
        apiName = "cpssDxChTtiRuleValidStatusSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U32", "index", ttiIndex},
            { "IN", "GT_BOOL", "valid", GT_FALSE}
        })

        freeTtiIndexToPool(ttiIndex)
    else
        apiName = "cpssDxChTtiRuleSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName ,{
            { "IN",     "GT_U8",                           "devNum",     devNum },
            { "IN",     "GT_U32",                          "index",      ttiIndex },
            { "IN",     "CPSS_DXCH_TTI_RULE_TYPE_ENT",     "ruleType",   ruleType},
            { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "patternPtr", finalTtiPattern},
            { "IN",     "CPSS_DXCH_TTI_RULE_UNT_udbArray", "maskPtr",    finalTtiMask},
            { "IN",     "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"},
            { "IN",     "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr",  finalTtiAction},
        })
    end
end

-- bind the grp,ecid to the mc group
local function func_bpe_802_1_br_mc_etag_info_bind_grp_ecid(
    command_data,vlanId,mcMacAddr,devNum,grp,ecid)
    if not bpe_802_1_br_is_eArch_Device() then
        func_onPe_non_eArch_bpe_802_1_br_mc_etag_info_bind_grp_ecid(
            command_data,vlanId,mcMacAddr,devNum,grp,ecid)
        return
    end

    -- get the eVidx that need to be used for this {grp,ecid}
    local new_eVidx = bpe_802_1_br_mc_eVidx_get(grp,ecid)
    local needTtiUpdate = false -- indication that the TTI need update (PE : TTI used instead of FDB)
    local needFdbUpdate = true -- indication that the FDB need update
    local needL2MllUpdate = true -- indication that the L2Mll need update
    local macString = nil;
    if mcMacAddr then
        macString = mcMacAddr.string;
    end
    local db_key_str = command_data_getVidMacKeyString(vlanId, macString);

    -- get the eVidx that is already associated with the group
    local current_eVidx  = db_getMcGroupField(db_key_str, db_field_eVidx)

    if bpe_802_1_br_is_control_bridge() == false then -- port extender
        -- FDB not needed as we are using the TTI to assign eVidx for the packet.
        -- the eVidx will hold 'vidx' member.
        -- the vidx will hold the physical ports.
        needFdbUpdate = false
        needTtiUpdate = true -- (PE : TTI used instead of FDB)
    end
    
    
    if current_eVidx ~= nil then
        if current_eVidx ~= new_eVidx then
            --_debug_to_string("eVidx changed from : " , current_eVidx , " to : " , new_eVidx)
        else
            -- the FDB already hold the correct eVidx in the entry
            needFdbUpdate = false
            needL2MllUpdate = false
            needTtiUpdate = false -- (PE : TTI used instead of FDB)
        end
    end

    -- update db about new evidx
    db_setMcGroupField(db_key_str, db_field_eVidx,new_eVidx)

    -- check if group already have fdbIndex:
    -- if yes:
        -- need to update the FDB entry with proper eVidx that match <grp,ecid>
    if needFdbUpdate == true then
        -- check if fdb entry already exists
        local fdbIndex  = db_getMcGroupField(db_key_str, db_field_fdbIndex)

        if fdbIndex ~= nil then
            -- we have fdb entry to update
            local fieldsToUpdate = { ["dstInterface"] =
                                    { ["type"] = "CPSS_INTERFACE_VIDX_E",
                                      ["vidx"] = new_eVidx }
                             }

            updateFdbEntryByIndex(command_data,devNum,fdbIndex,fieldsToUpdate)
        end
    end
    
    if needTtiUpdate == true then
        -- relevant only to PE
        -- create/update the rule to catch proper <GRP>,<ecid> and redirect to eVidx
        func_onPe_bpe_802_1_br_mc_etag_info_bind_grp_ecid__tti_set(
            command_data,db_key_str,devNum,vlanId,grp,ecid,new_eVidx,
            true--[[isMc]],
            false--[[set entry (not remove it)]])
    end
        
    -- get vidx for this group
    local vidx = db_vidxGet(db_key_str)

    local new_bpeInfo = {["grp"] = grp,["ecid"] = ecid}

    -- get the bpeInfo that is already associated with the group
    local current_bpeInfo  = db_getMcGroupField(db_key_str, db_field_bpeInfo)
    if current_bpeInfo ~= nil then
        -- info exists
    end

    -- check if need to set/update L2Mll table
    if needL2MllUpdate == true then
        if current_eVidx ~= nil then
            -- remove previous settings from the L2Mll and the L2Mll-LTT
            -- extern function evidx_flood_via_l2mll_chain_remove(devNum, vlanId,vidx)
            evidx_flood_via_l2mll_chain_remove(devNum, nil,vidx)
        end

        -- set the L2Mll and the L2Mll-LTT
        -- the the M2MLL with single member... the 'vidx'
        -- extern function evidx_flood_via_l2mll_chain_config(devNum, vlanId, eVidx, downstreamVidx)
        evidx_flood_via_l2mll_chain_config(devNum, nil, new_eVidx, vidx)
    end

    -- update db about new bpeInfo
    db_setMcGroupField(vlanId,mcMacAddr, db_field_bpeInfo, new_bpeInfo)

end


-- global DB - initialized as empty table on registration
-- bpe_802_1_br_my_info_DB = {};

-- bind ETag into to the 'mc group' represented by vidx.
--
--  params["e-cid-base"] = "0x123",
--  params.grp = "2",
--  params.mcMacAddr={"00","01","02","03","04","05",
--        isMulticast=false,
--        string="00:01:02:03:04:05",
--        isBroadcast=false,
--        isUnicast=true
--      }
--  NOTE: the type "%mac-address-bc-mc" already check that mac address in indead 'multicast'
local function func_bpe_802_1_br_mc_etag_info_bind(params)
    local role

    if debug_checkOnlyOnBpeSystem then
        if bpe_802_1_br_my_info_DB ~= nil and
           bpe_802_1_br_my_info_DB.dev_info ~= nil
        then
            role = bpe_802_1_br_my_info_DB.dev_info.role
        else
            role = nil
        end

        if role == nil then
            print_error ("The BPE 802.1BR role of board not defined\n")
            return
        end

        if role == "port_extender" then
            print_error ("Command not implemented yet for port extender \n")
            return
        end
    end

    --print (to_string(params))

    local mcMacAddr = params["mcMacAddr"]
    local grp  = params["grp"]
    local ecid = params["e-cid-base"]

    local command_data = Command_Data()

    local max_grp = bpe_802_1_br_max_grp_get() 
    if grp > max_grp then
        command_data:setFailStatus()
        command_data:addError("ERROR : the device not support <GRP> = " .. tostring(grp) .. "only 1.. " .. tostring(max_grp))
        return
    end
    
    -- Common variables initialization
    command_data:initInterfaceDevVlanRange()
    command_data:initInterfaceDeviceRange("dev_vlan_range")

    if true == command_data["status"] then
        for iterator, devNum, vlanId in command_data:getInterfaceVlanIterator() do
            command_data:clearVlanStatus()
            command_data:clearLocalStatus()

            -- let the function be called per device per vlan
            func_bpe_802_1_br_mc_etag_info_bind_grp_ecid(command_data,vlanId,mcMacAddr,devNum,grp,ecid)

            command_data:updateStatus()
            command_data:updateVlans()
        end
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()

end
-- function for show 'bpe' where mc groups bound to <GRP>,<ecid>
function show_bpe_802_1_br_func_print_mc_groups()
    -- Common variables declaration
    local grp, ecid, role

    if debug_checkOnlyOnBpeSystem then
        if bpe_802_1_br_my_info_DB ~= nil and
           bpe_802_1_br_my_info_DB.dev_info ~= nil
        then
            role = bpe_802_1_br_my_info_DB.dev_info.role
        else
            role = nil
        end

        if role == nil then
            print_error ("The BPE 802.1BR role of board not defined\n")
            return
        end
    end

    print("Registered multicast groups Info.")
    print("index \t VlanId and macAddr                    \t GRP   \t ecid  ")
    print("----- \t ------------------------------------- \t ----- \t ----  ")
    local ii = 1
    local grp_format, ecid_format, vlanId_and_mcMacAddrString
    for index, entry in pairs(db_mcGroupInfo) do
        vlanId_and_mcMacAddrString = index

        if entry[db_field_bpeInfo] ~= nil then
            grp = entry[db_field_bpeInfo].grp
            ecid = entry[db_field_bpeInfo].ecid
            grp_format = "%5d \t"
            ecid_format = "0x%4.4X \t"
        else
            grp  = "N/A"
            ecid = grp
            grp_format  = "    %s \t"
            ecid_format = "%s \t"
        end

        print(string.format(
            "0x%4.4X \t " .. vlanId_and_mcMacAddrString .. grp_format.. ecid_format,
            ii, grp, ecid));

        ii = ii + 1
    end
end

----------------------------------------------------------------
-- command registration: bpe-802-1-br mc-etag-info  grp <grp> e-cid-base <ecid> mac <address>
----------------------------------------------------------------
CLI_addCommand("vlan_configuration", "bpe-802-1-br mc-etag-info", {
  func   = func_bpe_802_1_br_mc_etag_info_bind,
  help   = "Attach 'mc group' to IEEE 802.1 ETag <GRP> and <ecid> fields",
  params =
  {
    { type="named",
       { format = "grp %bpe_802_1_br__ETag_GRP", name = "grp",
           help = "the 2 bits <GRP> in the ETag format (values 1,2,3)" },
       { format = "e-cid-base %bpe_802_1_br__ETag_ecid", name = "e-cid-base",
           help = "the 12 bits <E-CID_base> in the ETag format (values 0..4K-1)" },
       { format = "mac %mac-address-bc-mc", name = "mcMacAddr",
           help = "The multicast mac-address represent the MC group" },
       mandatory = {"grp", "e-cid-base","mcMacAddr"}
    }
  }
})

--[[
    debug 'show' commands to see the internal DB of the TTI tcam manager
--]]
CLI_addHelp("debug", "show bridge", "show bridge related info")
CLI_addHelp("debug", "show bridge internal-db", "show bridge internal DB managed by the LUA commands")
CLI_addHelp("debug", "show bridge internal-db mac", "show bridge internal DB related to mac-addresses managed by the LUA commands")

CLI_addCommand("debug", "show bridge internal-db mac multicast", {
    func = function(params)
        --***********************
        --***********************
        local title = "bridge mac multicast"
        local dbName = db_mcGroupInfo

        print_table(title,dbName)

        --***********************
        --***********************
        title = "vidx"
        dbName = db_vidxUsedArr

        print_table(title,dbName)

    end,
    help="Dump internal DB of bridge mac multicast"
})

