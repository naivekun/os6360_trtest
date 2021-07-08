--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bpe_802_1_br_neighbor_info.lua
--*
--* DESCRIPTION:
--*       manage IEEE 802.1BR configurations on eport/phyPort
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants
GT_TRUE = true
GT_FALSE = false

local FILLED_IN_RUN_TIME    = 0

local debug_on = false
local function _debug(...)
    if debug_on == true then
        printLog (...)
    end
end

-- the IEEE reserved value for TPID ETag
local ieee_tpid_ETag = 0x893f
-- reserved pclid for the lookup
local reserved_pclId = 0x123
-- do we currently in system of 6-bytes v-tag
-- Note : this indication is currently global and not per per or other indication.
--[[extern]] isVTag6BytesConfig = false
local long_tag = "long-tag"
--[[ 
    we not need to call cpssDxChBrgPePortPcidSet as it was needed for removing tag1 when packet egress on 
    downstream this (cascad) port, but we will get it because 'tti action' sets vid1=0. and the 'default' of the
    egress port is also PCID = 0 ... meaning that tag 1 will be removed ... without the need to call this API.
--]]        
local onPe_non_eArch_removed_cpssDxChBrgPePortPcidSet = true


-- first index that the 'appDemo' uses
-- valid range is 18k..(30k-1) --> 12k indexes for tti0 lookup
-- we only need up to 8k entries for 8k eports
local _3k  = 3*1024
local function numIndexForFloor(floorId)
    return floorId * _3k -- each floor 3k indexes
end
local tti_base_index = numIndexForFloor(6) --18k
local tti_last_index = tti_base_index + numIndexForFloor(2) - 1 -- 2 floors (6,7)
local tti_1_base_index = numIndexForFloor(10)
local tti_1_last_index = tti_1_base_index + numIndexForFloor(2) - 1 -- 2 floors (10,11)

function setTtiIndexRangeForPool(firstIndex,lastIndex,ttiLookupId)
    _debug("firstIndex = " , to_string(firstIndex))
    _debug("lastIndex = " , to_string(firstIndex))
    _debug("ttiLookupId = " , to_string(ttiLookupId))

    if ttiLookupId == nil or ttiLookupId == 0 then
        tti_base_index = firstIndex
        tti_last_index = lastIndex
    end
    if ttiLookupId == 1 then
        tti_1_base_index = firstIndex
        tti_1_last_index = lastIndex
    end
end

local _4k = 4 * 1024
local _8k = 8 * 1024
local tcam_max_num_rules_per_floor = 256 * 12

local tti_index_error_type_ok = 0
local tti_index_error_type_error_over_max = 1
local tti_index_error_type_error_not_numeric = 2

local ruleType = "CPSS_DXCH_TTI_RULE_UDB_10_E"
local keyType = "CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E"
local keySize = "CPSS_DXCH_TTI_KEY_SIZE_10_B_E"

local ingress_tpid_profile_for_empty_bmp_index = 6
-- dummy vid for 'physical port' - Don't care --> This is (only) to enable MC
local dummy_vid_0xfff = 0xfff

local function getAnyTtiIndex(eport,ruleSize)
    return tti_base_index + (eport * ruleSize)
end

-- return ok/error code and tti index to use
local function getTtiIndex(ttiIndex,eport,ruleSize)

    if ttiIndex == "any" then
        ttiIndex = getAnyTtiIndex(eport,ruleSize)
    else
        _debug(101)
        ttiIndex = tonumber(tti_rule_index)
        if ttiIndex ~= nil then
            if ttiIndex > _30k then
                return tti_index_error_type_error_over_max , ttiIndex
            end
        else
            return tti_index_error_type_error_not_numeric , ttiIndex
        end

    end

    return tti_index_error_type_ok , ttiIndex
end

-- Get free index in table.
-- NOTE: the function NOT reserve those indexes (you can use occupyIndexInTable or allocIndexInTable or set by yourself 'entry' into the index in the table)
--
-- inputs :
-- tableName - the table to query
-- firstIndex - the first valid index (when nil --> considered value 0)
-- lastIndex  - the last  valid index (when nil --> considered value 1)
-- alignment  - the alignment that new index must have (when nil --> considered value 1)
-- size       - the size of the entry (number of indexes to use) (when nil --> considered value 1)
-- fromEnd    - do we need allocation from end of table (in reverse) (when nil --> considered false)
--
-- reutrns
-- emptyIndex - empty index (the first index for the needed 'size')
--      may return nil on error or :
--          table is full .. no space for new size in needed alignment (the emptyIndex == nil in such case)
--
function getFreeIndexInTable(tableName,firstIndex,lastIndex,alignment,size,fromEnd,indexValidFunc)
    if firstIndex == nil then firstIndex = 0 end
    if lastIndex == nil  then lastIndex = 1  end -- ERROR
    if alignment == nil then alignment = 1   end
    if size == nil       then size = 1       end
    if lastIndex < firstIndex   then lastIndex = firstIndex end -- ERROR
    if alignment < size         then alignment = size       end  -- ERROR
    if fromEnd == nil then fromEnd = false end

    local lastIteratedIndex
    local emptyIndex = nil

    if fromEnd == true then
        -- swap values
        local tmp = lastIndex
        lastIndex = firstIndex
        firstIndex = tmp

        -- align the 'first index' according to alignment
        tmp = firstIndex - (alignment - 1)
        firstIndex = tmp + (tmp % alignment)

        -- reverse the iteration
        alignment = 0 - alignment

    else
        -- align the 'first index' according to alignment
        local tmp = firstIndex + (alignment - 1)
        firstIndex = tmp - (tmp % alignment)
    end

    _debug("firstIndex " .. firstIndex)
    _debug("lastIndex " .. lastIndex)
    _debug("fromEnd " .. tostring(fromEnd))

    for index = firstIndex,lastIndex,alignment do
        if tableName[index] == nil then
            if size > 1 then
                -- need to check that enough indexes for size
                for tmpIndex = (index + 1) , (index + size - 1) do
                    if tableName[tmpIndex] ~= nil then
                        break
                    end
                    lastIteratedIndex = tmpIndex
                end
            else
                lastIteratedIndex = index
            end

            if lastIteratedIndex == (index + size - 1) then
                local valid = true
                if indexValidFunc ~= nil then
                    valid = indexValidFunc(index)
                end
                if valid then
                    -- found space that match the needed size
                    emptyIndex = index
                    break
                end
            end
        end
    end

    -- NOTE: table can be full .. no spcae for new size in needed alignment
    -- the emptyIndex == nil in such case

    if emptyIndex == nil then
        print("tableName " .. tostring(tableName) .. " cant hold new entry of size =" .. size .. " and alignment = " .. alignment)
    end

    return emptyIndex
end

-- occupy index(s) in table
-- make table entries to be valid , by allocating empty entry
local function occupyIndexInTable(tableName,startIndex,size)
    if startIndex == nil then startIndex = 0 end
    if size == nil       then size = 1       end

    for index = startIndex,(startIndex + size - 1) do
        if tableName[index] == nil then
            tableName[index] = {}
        end
    end
end

-- allocate index in table.
--
-- inputs :
-- tableName - the table to query
-- firstIndex - the first valid index (when nil --> considered value 0)
-- lastIndex  - the last  valid index (when nil --> considered value 1)
-- alignment  - the alignment that new index must have (when nil --> considered value 1)
-- size       - the size of the entry (number of indexes to use) (when nil --> considered value 1)
--
--
-- reutrns
-- emptyIndex - empty index (the first index for the needed 'size')
--      may return nil on error or :
--          table is full .. no spcae for new size in needed alignment (the emptyIndex == nil in such case)
--
function allocIndexInTable(tableName,firstIndex,lastIndex,alignment,size)
    if firstIndex == nil then firstIndex = 0 end
    if lastIndex == nil  then lastIndex = 1  end -- ERROR
    if alignment == nil then alignment = 1   end
    if size == nil       then size = 1       end
    if lastIndex < firstIndex   then lastIndex = firstIndex end -- ERROR
    if alignment < size         then alignment = size       end  -- ERROR

    -- find free index
    local freeIndex = getFreeIndexInTable(tableName,firstIndex,lastIndex,alignment,size)

    if freeIndex == nil then
        return nil
    end

    -- set entries as used
    occupyIndexInTable(tableName,freeIndex,size)

    return freeIndex
end

-- free index in table.
--
-- inputs :
-- tableName - the table to query
-- allocatedIndex - the first index to free (when nil --> considered value 0)
-- size       - the size of the entry (number of indexes to use) (when nil --> considered value 1)
function freeIndexInTable(tableName,allocatedIndex,size)
    if allocatedIndex == nil then allocatedIndex = 0 end
    if size == nil           then size = 1           end

    for index = allocatedIndex,(allocatedIndex + size - 1) do
        tableName[index] = nil
    end

end


-- next 2 DBs :
-- index is 'tti index'
-- entry hold fields: callerIndexName - a string that 'unique' this entry from all other entries
local tti_0_db = {}
local tti_1_db = {}
-- next 2 DBs :
-- index is 'callerIndexName' (the entry from tti_0_db/tti_1_db)
-- entry hold fields: ttiRuleIndex , ttiRuleSize
local tti_0_users_names = {}
local tti_1_users_names = {}
--
-- get new TTI index or 'retrieve' from DB for this 'caller'
-- inputs:
-- myRuleSize  - the size of rule : 1/2/3 (number of indexes to occupy)
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
-- ttiLookupId - (optional) inticate "tti0" or "tti1" lookup (or nill --> considered "tti0")
--
-- returns:
-- the tti index
--
function allocTtiIndexFromPool(myRuleSize,callerIndexName,ttiLookupId,fromEnd)
    local minVal
    local maxVal
    local useTti0
    local newTtiIndex
    local isNew = true
    local nameOfTti

    if fromEnd == nil then fromEnd = false end

    if ttiLookupId == nil or ttiLookupId == "tti0" then
        minVal = tti_base_index
        maxVal = tti_last_index
        useTti0 = true
        nameOfTti = "index in Tti0"
    else
        minVal = tti_1_base_index
        maxVal = tti_1_last_index
        useTti0 = false
        nameOfTti = "index in Tti1"
    end

    _debug(nameOfTti)

    if useTti0 == true and tti_0_users_names[callerIndexName] == nil then
        -- need to allocate new index in tti0
        local func=nil
        if (NumberOfEntriesIn_TTI_TCAM_table / tcam_max_num_rules_per_floor) <= 3 then
            func=function(index)
                return (index%12) < 6
            end
        end
        newTtiIndex = getFreeIndexInTable(tti_0_db,minVal,maxVal,myRuleSize,myRuleSize,fromEnd,func)

        tti_0_db[newTtiIndex] = {
            callerName = callerIndexName
        }

        tti_0_users_names[callerIndexName] = {
            ttiRuleIndex = newTtiIndex,
            ttiRuleSize = myRuleSize
        }

    elseif useTti0 == false and tti_1_users_names[callerIndexName] == nil then
        -- need to allocate new index in tti1
        local func=nil
        if (NumberOfEntriesIn_TTI_TCAM_table / tcam_max_num_rules_per_floor) <= 3 then
            func=function(index)
                return (index%12) >= 6
            end
        end

        newTtiIndex = getFreeIndexInTable(tti_1_db,minVal,maxVal,myRuleSize,myRuleSize,fromEnd,func)
        tti_1_db[newTtiIndex] = {
            callerName = callerIndexName
        }

        tti_1_users_names[callerIndexName] = {
            ttiRuleIndex = newTtiIndex,
            ttiRuleSize = myRuleSize
        }

    elseif useTti0 == true then
        -- index was already allocated for this 'caller'
        newTtiIndex = tti_0_users_names[callerIndexName].ttiRuleIndex
        isNew = false
    else -- useTti1 == true
        -- index was already allocated for this 'caller'
        newTtiIndex = tti_1_users_names[callerIndexName].ttiRuleIndex
        isNew = false
    end

    if newTtiIndex == nil then
        -- error (table is full)
        return nil
    end

    if isNew == true then
        _debug(nameOfTti .. " allocated new ttiIndex = " .. newTtiIndex .. " for client " .. callerIndexName)
    else
        _debug(nameOfTti .. " retrieved ttiIndex = " .. newTtiIndex .. " for client " .. callerIndexName)
    end

    -- index is new or 'retrieved' from DB for this 'caller'
    return newTtiIndex
end


--
-- get new TTI index or 'retriev' from DB for this 'caller' - lookfor index staring from end of tcam
-- inputs:
-- myRuleSize  - the size of rule : 1/2/3 (number of indexes to occupy)
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
-- ttiLookupId - (optional) inticate "tti0" or "tti1" lookup (or nill --> considered "tti0")
--
-- returns:
-- the tti index
--
function allocTtiIndexFromPool_fromEnd(myRuleSize,callerIndexName,ttiLookupId)
    return allocTtiIndexFromPool(myRuleSize,callerIndexName,ttiLookupId,true)
end


-- delete entry(s) from tti_0_db/tti_1_db and from tti_0_users_names/tti_1_users_names
local function local_freeTtiIndexToPool(ttiIndex, tti_db , tti_users_names)
    local callerIndexName = tti_db[ttiIndex].callerName

    if tti_users_names[callerIndexName] == nil then
        _debug("ttiIndex = "..ttiIndex.." caller was not set for index in table")
    else
        -- start loop from second index .. because we anyway invalidate first one at the end
        for index = (ttiIndex+1),(ttiIndex+tti_users_names[callerIndexName].ttiRuleSize - 1) do
            tti_db[index] = nil
        end
    end

    tti_users_names[callerIndexName] = nil
    tti_db[ttiIndex] = nil

    _debug("local_freeTtiIndexToPool : free from DB index :" .. ttiIndex .. " for : " .. callerIndexName)
end


--
-- release the ttiIndex back to the DB (the pool)
-- inputs:
-- ttiIndex  - the tti index (as returned by allocTtiIndexFromPool(...))
--
-- returns:
-- nil
--
function freeTtiIndexToPool(ttiIndex)
    local useTti0
    local callerIndexName

    if ttiIndex <  tti_base_index or
       ttiIndex >  tti_1_last_index
    then
        _debug("ttiIndex = "..ttiIndex.." over max or below min")
        return nil
    end

    if ttiIndex < tti_1_base_index then
        useTti0 = true
    else
        useTti0 = false
    end


    if useTti0 == true and tti_0_db[ttiIndex] == nil then
        _debug("ttiIndex = "..ttiIndex.." index was not occupied in tti 0")
        -- index was not occupied
        return nil
    end
    if useTti0 == false and tti_1_db[ttiIndex] == nil then
        _debug("ttiIndex = "..ttiIndex.." index was not occupied in tti 1")
        -- index was not occupied
        return nil
    end

    if useTti0 == true then
        local_freeTtiIndexToPool(ttiIndex,tti_0_db,tti_0_users_names)
    else
        local_freeTtiIndexToPool(ttiIndex,tti_1_db,tti_1_users_names)
    end

    return nil
end


local function bpe_802_1_br_pcid_on_pe_eport_info_get_db(devNum, portNum)
    local db_vertex0, key;

    db_vertex0 = bpe_802_1_br_my_info_DB;
    if db_vertex0 == nil then
        return nil;
    end;
    db_vertex0 = db_vertex0["dev_info"];
    if db_vertex0 == nil then
        return nil;
    end;
    db_vertex0 = db_vertex0["pe_per_eport_info"];
    if db_vertex0 == nil then
        return nil;
    end;
    key = tostring(devNum) .. "/" .. tostring(portNum);
    return db_vertex0[key];
end

local function bpe_802_1_br_pcid_eport_info_save_in_db(params)
    local db_vertex0, key;
    local dummy, devNum, portNum;
    local command_data;

    command_data = Command_Data();
    command_data:initInterfaceDevPortRange();

    if bpe_802_1_br_my_info_DB == nil then
        bpe_802_1_br_my_info_DB = {};
    end
    db_vertex0 = bpe_802_1_br_my_info_DB;

    if db_vertex0["dev_info"] == nil then
        db_vertex0["dev_info"] = {};
    end;
    db_vertex0 = db_vertex0["dev_info"];

    if db_vertex0["pe_per_eport_info"] == nil then
        db_vertex0["pe_per_eport_info"] = {};
    end;
    db_vertex0 = db_vertex0["pe_per_eport_info"];

    -- loop over the eports with generic iterator
    for dummy, devNum, portNum in command_data:getPortIterator(true) do
        key = tostring(devNum) .. "/" .. tostring(portNum);
        if db_vertex0[key] == nil then
            db_vertex0[key] = {};
        end;

        if (params["flagNo"] == true) then
            db_vertex0[key] = nil;
        else
            db_vertex0                   = db_vertex0[key];
            db_vertex0["pe_name"]        = params["pe_name"];
            db_vertex0["pcid"]           = params["pcid"];
            db_vertex0["pvid0"]          = params["pvid0"];
            db_vertex0["tti_rule_index"] = params["tti_rule_index"];
        end
    end
end

local function bpe_802_1_br_pcid_on_pe_phy_port_info_save_in_db(params)
    local db_vertex0, key;
    local dummy, devNum, portNum;
    local command_data;

    command_data = Command_Data();
    command_data:initInterfaceDevPortRange();

    if bpe_802_1_br_my_info_DB == nil then
        bpe_802_1_br_my_info_DB = {};
    end
    db_vertex0 = bpe_802_1_br_my_info_DB;

    if db_vertex0["dev_info"] == nil then
        db_vertex0["dev_info"] = {};
    end;
    db_vertex0 = db_vertex0["dev_info"];

    if db_vertex0["pe_per_phy_port_info"] == nil then
        db_vertex0["pe_per_phy_port_info"] = {};
    end;
    db_vertex0 = db_vertex0["pe_per_phy_port_info"];

    -- loop over the eports with generic iterator
    for dummy, devNum, portNum in command_data:getPortIterator(true) do
        key = tostring(devNum) .. "/" .. tostring(portNum);
        if db_vertex0[key] == nil then
            db_vertex0[key] = {};
        end;

        if (params["flagNo"] == true) then
            db_vertex0[key] = nil;
        else
            db_vertex0                   = db_vertex0[key];
            db_vertex0["pcid"]           = params["pcid"];
        end
    end
end

local function bpe_802_1_br_pcid_on_pe_phy_port_info_get_db(devNum, portNum)
    local db_vertex0, key;

    db_vertex0 = bpe_802_1_br_my_info_DB;
    if db_vertex0 == nil then
        return nil;
    end;
    db_vertex0 = db_vertex0["dev_info"];
    if db_vertex0 == nil then
        return nil;
    end;
    db_vertex0 = db_vertex0["pe_per_phy_port_info"];
    if db_vertex0 == nil then
        return nil;
    end;
    key = tostring(devNum) .. "/" .. tostring(portNum);
    return db_vertex0[key];
end

function bpe_802_1_br_pcid_on_pe_eport_info_print_from_db()
    local db_vertex0, db_vertex1, key;
    local device, e_port, pe_name, p_cid, pvid0, tty_index;


    if bpe_802_1_br_my_info_DB == nil then
        return;
    end
    db_vertex0 = bpe_802_1_br_my_info_DB;

    if db_vertex0["dev_info"] == nil then
        return;
    end;
    db_vertex0 = db_vertex0["dev_info"];

    if db_vertex0["pe_per_eport_info"] == nil then
        return;
    end;

    db_vertex0 = db_vertex0["pe_per_eport_info"];

    print("PCIDs on downstream PEs (per ePort)");
    print("device\t e_port\t pe_name\t pcid  \t pvid0 \t tti_index");
    print("------\t ------\t -------\t ------\t ------\t ---------");

    for key, db_vertex1 in pairs(db_vertex0) do
        device, e_port = string.match(key, "(%d+)/(%d+)");
        p_cid     = 0;
        pvid0     = 0;
        tty_index = "";

        if db_vertex1 ~= nil then
            if db_vertex1["pe_name"] ~= nil then
                pe_name = db_vertex1["pe_name"];
            end
            if db_vertex1["pcid"] ~= nil then
                p_cid = db_vertex1["pcid"];
            end
            if db_vertex1["pvid0"] ~= nil then
                pvid0 = db_vertex1["pvid0"];
            end
            if db_vertex1["tti_rule_index"] ~= nil then
                tty_index = db_vertex1["tti_rule_index"];
            end
            print(string.format(
                "0x%4.4X\t 0x%4.4X\t %-7s\t 0x%4.4X\t 0x%4.4X\t %-9s",
                device, e_port, pe_name, p_cid, pvid0, tty_index));
        end;
    end;
end

--on PE : non eArch device :
-- save 'pe name' and 'pcid' for downstream PEs
local function bpe_802_1_br_non_eArch_on_pe_remote_pe_save_in_db(peName,pcid,unset)
    local db_vertex0, key;

    if bpe_802_1_br_my_info_DB == nil then
        return
    end
    db_vertex0 = bpe_802_1_br_my_info_DB;

    if db_vertex0["dev_info"] == nil then
        return
    end;
    db_vertex0 = db_vertex0["dev_info"];

    if db_vertex0["non_eArch_remote_pe_info"] == nil then
        db_vertex0["non_eArch_remote_pe_info"] = {};
    end;
    db_vertex0 = db_vertex0["non_eArch_remote_pe_info"];

    -- loop over the eports with generic iterator
    key = peName .. "/pcid_" .. tostring(pcid);
    if unset then
        db_vertex0[key] = nil;
    elseif db_vertex0[key] == nil then
        db_vertex0[key] = {peName=peName,pcid=pcid};
    end;
end

--on PE : non eArch device :
-- print 'pe name' and 'pcid' for downstream PEs
function bpe_802_1_br_non_eArch_on_pe_remote_pe_print_from_db()
    local db_vertex0;

    if bpe_802_1_br_my_info_DB == nil then
        return;
    end
    db_vertex0 = bpe_802_1_br_my_info_DB;

    if db_vertex0["dev_info"] == nil then
        return;
    end;
    db_vertex0 = db_vertex0["dev_info"];

    if db_vertex0["non_eArch_remote_pe_info"] == nil then
        return;
    end;

    db_vertex0 = db_vertex0["non_eArch_remote_pe_info"];

    print("PCIDs on downstream PEs");
    print("pe_name\t pcid  ");
    print("-------\t ------");

    for key, db_vertex1 in pairs(db_vertex0) do
        print(string.format(
            "%-7s\t 0x%4.4X",
            db_vertex1.peName, db_vertex1.pcid));
    end
end

-- define the UDBs that will be needed for the 802.1BR classification
--[[ we need next fields:
1. <Physical port>

    ***> metadata byte 26 bits 0..7 --> Local Device Source ePort/TrunkID[7:0]

2. tag1_exists = 1 --> the recognition of 'E-Tag'

    ***> metadata byte 16 bit 0 --> Tag1 exists

3. two UDBs of E-TAG from byte 4 of the 8 bytes :
    <E-CID_base>  - (12 bits) 'PCID' of the 'source vm'
    <GRP>             - (2 bits)  must be 0 because src is not multicast.
    <Re->              - (2 bits)  must be 0 reserved

    ***> 'l2 offset' byte 16 (12+4) : 8 LSB of <E-CID_base>
    ***> 'l2 offset' byte 17        : bits 4..7 are 0 and bits 0..3 are the 4 MSB of <E-CID_base>

4. pclid = 0x123 --> see reserved_pclId

    ***> metadata byte 22 bit 0..4 --> 5 LSB bits of pclid
    ***> metadata byte 23 bit 0..4 --> 5 MSB bits of pclid

--]]
local udbArr =
{ --byte#   offset type                         offset
    -- metadata section
-- bits  0 .. 7 in the key
    { udbIndex = 0,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 22      }, -- bit 0..4 , 6 --> 5 LSB bits of pclid , bit 6 - <DSA Tag Source Is Trunk>
-- bits  8..15 in the key
    { udbIndex = 1,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 23      }, -- bit 0..4 --> 5 MSB bits of pclid
-- bits 16..23 in the key
    { udbIndex = 2,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 16      }, -- bit 0 --> Tag1 exists
-- bits 24..31 in the key
    { udbIndex = 3,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 26      }, -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
    -- offsets in the packet section
-- bits 32..39 in the key
    { udbIndex = 4,    offsetType = "CPSS_DXCH_TTI_OFFSET_L2_E",        offset = 17      }, -- 8 LSB of <E-CID_base>
-- bits 40..47 in the key
    { udbIndex = 5,    offsetType = "CPSS_DXCH_TTI_OFFSET_L2_E",        offset = 16      }, -- bits 4..7 are 0 and bits 0..3 are the 4 MSB of <E-CID_base>
-- bits 48..55 in the key
    { udbIndex = 6,    offsetType = "CPSS_DXCH_TTI_OFFSET_L2_E",        offset = 15      }, -- 8 LSB of <ingress_E-CID_base>
-- bits 56..63 in the key
    { udbIndex = 7,    offsetType = "CPSS_DXCH_TTI_OFFSET_L2_E",        offset = 14      }, -- bits 0..3 are the 4 MSB of <ingress_E-CID_base> , bits 4..7 should be ignored (E-PCP,E-DEI)

-- metadata section - continue
-- bits 64..71 in the key
    { udbIndex = 8,    offsetType = "CPSS_DXCH_TTI_OFFSET_METADATA_E",  offset = 27      }  -- bits 0..4 --> Local Device Source Port/TrunkID[12:8]

}

local mask_pclId = 0x3ff -- 10 bits mask
local mask_full_byte = 0xff
local _6_bits_mask = 0x3f
local _5_bits_mask = 0x1f
local _4_bits_mask = 0x0f
local bit0 = 1
local bit6 = bit_shl(1,6)

-- pattern_udbArray
local pattern_udbArray = {
    [0] = bit_and(reserved_pclId,_5_bits_mask),             -- bit 0..4 , 6 --> 5 LSB bits of pclid , bit 6 - <DSA Tag Source Is Trunk>
    [1] = bit_and(bit_shr(reserved_pclId,5) , _5_bits_mask),-- bit 0..4 --> 5 MSB bits of pclid
    [2] = bit0,                 -- bit 0 --> Tag1 exists
    [3] = FILLED_IN_RUN_TIME,   -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
    [4] = FILLED_IN_RUN_TIME,   -- 8 LSB of <E-CID_base>
    [5] = FILLED_IN_RUN_TIME,   -- bits 4..7 are 0 and bits 0..3 are the 4 MSB of <E-CID_base>
    [6] = 0,
    [7] = 0,
    [8] = FILLED_IN_RUN_TIME,-- bits 0..4 --> Local Device Source Port/TrunkID[12:8]
    [9] = 0
}

-- mask_udbArray
local mask_udbArray = {
    [0] = bit_and(mask_pclId,_5_bits_mask) + bit6,             -- bit 0..4 --> 5 LSB bits of pclid , bit 6 - <DSA Tag Source Is Trunk>
    [1] = bit_and(bit_shr(mask_pclId,5) , _5_bits_mask),-- bit 0..4 --> 5 MSB bits of pclid
    [2] = bit0,                 -- bit 0 --> Tag1 exists
    [3] = mask_full_byte,   -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
    [4] = mask_full_byte,   -- 8 LSB of <E-CID_base>
    [5] = mask_full_byte,   -- bits 4..7 are 0 and bits 0..3 are the 4 MSB of <E-CID_base>
    [6] = 0,
    [7] = 0,
    [8] = _5_bits_mask,-- bits 0..4 --> Local Device Source Port/TrunkID[12:8]
    [9] = 0
}

-- define the table that will hold the 'devices involved in the BPE 802.1BR system'
-- each line in the table hold info about deferent device.
-- each device hold next info :
--                  1. role - br/pe
--                  2. connection_type - My connection to this neighbor <direct/indirect>
--                          if 'nil' meaning that this is 'me' ! (my device)
--                  3. etag_ether_type  - The tpid (ethertype) that identify the ETag (8 bytes tag)
--                  4. interface_info   - The interface info that used to reach the device from 'my device'
local entry_format_example = 0
local max_ether_type = 0xFFFF
table_bpe_802_1_br_system_info_devices = {
    [entry_format_example] = {
        role = "control_bridge / port_extender" ,
        connection_type = "direct/indirect" ,
        etag_ether_type = (max_ether_type + 1) ,
        command_data = nil ,--"command_data"

        etag_ether_type_Hex_string = "0xHHHH"--HEX foramt of the ethertype
    }
}
local num_neighbors = 0
-- on PE : the 'name' of the neighbor CB device
local onPe_control_bridge_name = nil
-- on PE : the 'upstream device,port'
local onPe_upstream_device = nil
onPe_upstream_port = nil
-- working on PE requires the 'uplink port' to be implemented as 'trunkId' (with single member) .. and also all the cascade ports
-- should be associated with this trunkId !!! (see CPSS API cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet)
-- so this is 'reserved' trunkId for that purpose.
-- NOTE: if the 'uplink' is 'port-channel' (by application) then the trunkId by application will be used and not this one .
--
-- SIP5 : In a PE that is connected to downstream PEs :
-- 1.	the Upstream port need to be member of trunk
-- 2.	the downstream port(s) (to other PE(s)) need to be set with < TRUNK_ID > in ‘HA_PHYSICAL_PORT_2’  (same trunk !!!)
-- So why do we make such 'bind' between upstream port and the downstream port(s) ?
-- Due to HA logic of building the E-TAG for the downstream port(s) :
-- Build of : egress  E-Tag : <ingress_E-CID_base> is based on ‘SrcTrgPhysInterfacesAreEqual’ (see logic in FS)
-- So if (in the PE) packet from upstream port (with E-TAG) need to egress downstream port with ETAG must have SrcTrgPhysInterfacesAreEqual = TRUE
-- Because if SrcTrgPhysInterfacesAreEqual = FALSE --> egress  E-Tag : <ingress_E-CID_base> is 0 ('default')
--
local onPe_reserved_trunkid_for_uplink_port = 450 -- 450 is number like any other number ... nothing special about it (1..512 trunks)
local on_Pe_need_reserved_trunkid_for_uplink_port = true
--(this value not used by physical ports (0..255) and not used by eports (256 .. 8k-1))
-- eport for physical port of upsteam port , for ingress packets with ETag (from 'cascade' ports)
local onPe_reserved_eport_of_upstream_ingress_with_etag = 170
-- (this value not used by physical ports (0..255) and not used by eports (256 .. 8k-1))
-- eport for physical port of upsteam port, for ingress packets without ETag (from 'cascade' ports or from 'port extender' ports)
local onPe_reserved_eport_of_upstream_ingress_with_no_etag_base = 128
local onPe_reserved_eport_of_upstream_ingress_with_no_etag_max_num = onPe_reserved_eport_of_upstream_ingress_with_etag - onPe_reserved_eport_of_upstream_ingress_with_no_etag_base
local onPe_reserved_eport_of_upstream_ingress_with_no_etag_arr = {}
local onPe_reserved_eport_of_upstream_ingress_with_no_etag_curr_num = 0

--(this value not used by physical ports (0..255) and not used by eports (256 .. 8k-1))
-- eport for physical port of upsteam port , for ingress packets with ETag (from 'cascade' ports)
local onPe_reserved_eport_of_pcid_on_cscd_port_base = onPe_reserved_eport_of_upstream_ingress_with_etag + 1 --[[171]]

local on_pe_num_eports_used_for_pcid_on_cscd_port = 0
local onPe_EportOfPcidOnCscdPortsArr = {}

local on_pe_max_eports_for_cascade = 10

-- on PE : eports for local physical port that is not cascade
-- NOTE: needed for 'downstream' traffic that need flooding using the L2MLL nodes
-- eports after 'onPe_reserved_eport_of_pcid_on_cscd_port_base'
local onPe_reserved_eport_of_pcid_on_non_cscd_port_base = onPe_reserved_eport_of_pcid_on_cscd_port_base + on_pe_max_eports_for_cascade --[[171+10=181]]
-- eports after 'onPe_reserved_eport_of_pcid_on_non_cscd_port_base'
local on_pe_max_eports_for_non_cascade = 255 - onPe_reserved_eport_of_pcid_on_non_cscd_port_base --[[255-181=74]]
local on_pe_num_eports_used_for_pcid_on_non_cscd_port = 0
local onPe_EportOfPcidOnNonCscdPortsArr = {}

-- working on PE requires the 'uplink port' to be implemented as 'trunkId' (with single member) .. and also all the cascade ports
-- should be associated with this trunkId !!! (see CPSS API cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet)
-- so this is 'reserved' trunkId for that purpose.
-- NOTE: if the 'uplink' is 'port-channel' (by application) then the trunkId by application will be used and not this one .
function bpe_802_1_br_onPe_setTrunkId(devNum)
    onPe_reserved_trunkid_for_uplink_port = 450 % NumberOfEntriesIn_TRUNK_table
end

-- on PE : get new/existing eport for the cascade port
-- NOTE: needed for 'downstream' traffic that need to get out the cascade port without ETag
local function onPe_getEportFromReservedRange(portNum,tableName,offset,currentNum,maxAllowed)
    --_debug("get eport for cascade on port " .. portNum)

    local index = to_string(portNum)
    if tableName[index] == nil then
        if currentNum < maxAllowed then
            currentNum = currentNum + 1
            -- the cascade port did not allocated eport yet
            tableName[index] = offset + currentNum
        else
            print ("ERROR : can not add new eport for portNum " .. portNum ..  " no more free eports , from range [" ..
                    to_string(offset).. 
                    ".." .. 
                    to_string(offset+maxAllowed).. "]")
            print ("already occupied with next physical ports : " , to_string(tableName))
        end
    end

    if tableName == onPe_reserved_eport_of_upstream_ingress_with_no_etag_arr then
        onPe_reserved_eport_of_upstream_ingress_with_no_etag_curr_num = currentNum
    elseif tableName == onPe_EportOfPcidOnCscdPortsArr then
        on_pe_num_eports_used_for_pcid_on_cscd_port = currentNum
    elseif  tableName == onPe_EportOfPcidOnNonCscdPortsArr then
        on_pe_num_eports_used_for_pcid_on_non_cscd_port = currentNum
    end

    return tableName[index]
end

-- on PE : get new/existing eport for local physical port that is not cascade
-- NOTE: needed for 'downstream' traffic that need flooding using the L2MLL nodes
function onPe_getEportOfPcidOnNonCascadePort(portNum)
    local eport =  onPe_getEportFromReservedRange(portNum,
                onPe_EportOfPcidOnNonCscdPortsArr,
                onPe_reserved_eport_of_pcid_on_non_cscd_port_base,
                on_pe_num_eports_used_for_pcid_on_non_cscd_port,
                on_pe_max_eports_for_non_cascade)

    return eport

end

-- on PE : get new/existing eport for the non-cascade prot
-- NOTE: needed for 'downstream' traffic that need to get out the non-cascade port without ETag
local function onPe_freeEportOfPcidOnNonCscdPort(portNum)
    local index = to_string(portNum)
    onPe_EportOfPcidOnNonCscdPortsArr[index] = nil
end

-- on PE : return true/false if there is existing eport for the non-cascade prot
-- NOTE: needed for 'downstream' traffic that need to get out the non-cascade port without ETag
local function onPe_isExistsEportOfPcidOnNonCscdPort(portNum)
    local index = to_string(portNum)
    if onPe_EportOfPcidOnNonCscdPortsArr[index] == nil then
        return false
    else
        return true
    end
end

-- on PE : get new/existing eport for the cascade port
-- NOTE: needed for 'downstream' traffic that need to get out the cascade port without ETag
local function onPe_getEportOfPcidOnCscdPort(portNum)
  local  eport = onPe_getEportFromReservedRange(portNum,
                onPe_EportOfPcidOnCscdPortsArr,
                onPe_reserved_eport_of_pcid_on_cscd_port_base,
                on_pe_num_eports_used_for_pcid_on_cscd_port,
                on_pe_max_eports_for_cascade)

    return eport
end

-- on PE : get new/existing eport for the cascade port
-- NOTE: needed for 'downstream' traffic that need to get out the cascade port without ETag
local function onPe_freeEportOfPcidOnCscdPort(portNum)
    local index = to_string(portNum)
    onPe_EportOfPcidOnCscdPortsArr[index] = nil
end

-- on PE : return true/false if there is existing eport for the cascade port
-- NOTE: needed for 'downstream' traffic that need to get out the cascade port without ETag
local function onPe_isExistsEportOfPcidOnCscdPort(portNum)
    local index = to_string(portNum)
    if onPe_EportOfPcidOnCscdPortsArr[index] == nil then
        return false
    else
        return true
    end
end

-- on PE : 
--  function to get eport from range : 
--  onPe_reserved_eport_of_upstream_ingress_with_no_etag_base .. onPe_reserved_eport_of_upstream_ingress_with_etag
local function onPe_getEportOf_reserved_eport_of_upstream_ingress_with_no_etag(portNum)
    local eport =  onPe_getEportFromReservedRange(portNum,
                onPe_reserved_eport_of_upstream_ingress_with_no_etag_arr,
                onPe_reserved_eport_of_upstream_ingress_with_no_etag_base,
                onPe_reserved_eport_of_upstream_ingress_with_no_etag_curr_num,
                onPe_reserved_eport_of_upstream_ingress_with_no_etag_max_num)

    return eport

end

-- on PE : 
--  function to free eport from range : 
--  onPe_reserved_eport_of_upstream_ingress_with_no_etag_base .. onPe_reserved_eport_of_upstream_ingress_with_etag
local function onPe_freeEportOf_reserved_eport_of_upstream_ingress_with_no_etag(portNum)
    local index = to_string(portNum)
    onPe_reserved_eport_of_upstream_ingress_with_no_etag_arr[index] = nil
end


-- function to build format of tti rule (pattern,mask) for 802.1_BR common use
-- inputs:
-- physicalPortOrTrunk - the physical port/trunk (exact match)
-- ETag_exists - true/false - is ETag exists (exact match) , if nil then 'mask' is 0 (don't care)
-- grp       - the grp (can be nil --> 'ignored')
-- ecid_base - the ecid (can be nil --> 'ignored')
-- isTrunk - indication that the 'physicalPort' is actually 'trunk' (can be nil --> 'ignored' --> considered 'false')
--
-- function return 2 ETH rule formats:
-- ETH pattern  (CPSS_DXCH_TTI_ETH_RULE_STC)
-- ETH mask     (CPSS_DXCH_TTI_ETH_RULE_STC)
local function bpe_802_1_br_non_eArch_buildTtiPatternMask(physicalPortOrTrunk,ETag_exists,grp,ecid_base,isTrunk)
    local eth_pattern = {} 
    local eth_mask = {}
    local common_pattern = {} 
    local common_mask = {}
    
    common_pattern.srcIsTrunk = isTrunk
    common_pattern.srcPortTrunk = physicalPortOrTrunk
    common_mask.srcIsTrunk = true
    common_mask.srcPortTrunk = 0x7f
    
    if ETag_exists == true then
        eth_pattern.isVlan1Exists = true
        eth_mask.isVlan1Exists = true
    elseif ETag_exists == false then
        eth_pattern.isVlan1Exists = false
        eth_mask.isVlan1Exists = true
    else -- ETag_exists == nil
    end
    
    if grp then
        eth_pattern.eTagGrp = grp
        eth_mask.eTagGrp    = 3
    end
    
    if ecid_base then
        eth_pattern.vid1 = ecid_base
        eth_mask.vid1 = 0xfff
    end

    eth_pattern.common = common_pattern
    eth_mask.common    = common_mask
    
    return eth_pattern , eth_mask
end

-- vTag_6_bytes_pattern_udbArray
local vTag_6_bytes_pattern_udbArray = {
    [0] = bit_and(reserved_pclId,_5_bits_mask),             -- bit 0..4 , 6 --> 5 LSB bits of pclid , bit 6 - <DSA Tag Source Is Trunk>
    [1] = bit_and(bit_shr(reserved_pclId,5) , _5_bits_mask),-- bit 0..4 --> 5 MSB bits of pclid
    [2] = bit0,                 -- bit 0 --> Tag1 exists
    [3] = FILLED_IN_RUN_TIME,   -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
--[[byte17]]    [4] = FILLED_IN_RUN_TIME, -- 8 LSB of <srcInterface>
--[[byte16]]    [5] = FILLED_IN_RUN_TIME, -- bit 7 <isMcFilter> , bits 4-6 are set to 0 ,  bits 0..3 are the 4 MSB of <srcInterface>
--[[byte15]]    [6] = FILLED_IN_RUN_TIME, -- 8 LSB of <trgInterface> 
--[[byte14]]    [7] = FILLED_IN_RUN_TIME, -- bit 7 <direction> , bit 6 <isMulticast> , bits 0..5 are the 6 MSB of <trgInterface>
    [8] = FILLED_IN_RUN_TIME,-- bits 0..4 --> Local Device Source Port/TrunkID[12:8]
    [9] = 0
}

-- vTag_6_bytes_mask_udbArray
local vTag_6_bytes_mask_udbArray = {
    [0] = bit_and(mask_pclId,_5_bits_mask) + bit6,             -- bit 0..4 --> 5 LSB bits of pclid , bit 6 - <DSA Tag Source Is Trunk>
    [1] = bit_and(bit_shr(mask_pclId,5) , _5_bits_mask),-- bit 0..4 --> 5 MSB bits of pclid
    [2] = bit0,                 -- bit 0 --> Tag1 exists
    [3] = mask_full_byte, -- bits 0..7 --> Local Device Source ePort/TrunkID[7:0]
--[[byte17]]    [4] = mask_full_byte, -- 8 LSB of <srcInterface>
--[[byte16]]    [5] = mask_full_byte, -- bit 7 <isMcFilter> , bits 4-6 are set to 0 ,  bits 0..3 are the 4 MSB of <srcInterface>
--[[byte15]]    [6] = mask_full_byte, -- 8 LSB of <trgInterface> 
--[[byte14]]    [7] = mask_full_byte, -- bit 7 <direction> , bit 6 <isMulticast> , bits 0..5 are the 6 MSB of <trgInterface>
    [8] = _5_bits_mask,-- bits 0..4 --> Local Device Source Port/TrunkID[12:8]
    [9] = 0
}

-- options for parameter : direction
local direction_upstream   = 0
local direction_downstream = 1
-- -- -- -- -- -- 6-bytes v-tag -- -- -- -- -- -- 
-- function relevant for e-Arch device sip_5_10
-- function to build format of tti rule (pattern,mask) for 6-bytes v-tag
-- inputs:
-- physicalPortOrTrunk - the physical port/trunk (exact match)
-- isTrunk - (bool) indication that the 'physicalPortOrTrunk' is actually 'trunk' (can be nil --> 'ignored' --> considered 'false')
-- tag_exists - (bool) is v-tag exists (exact match)
-- direction - direction_upstream/direction_downstream (exact match)
-- isMulticast  - (bool) is trgInterface multicast ? (can be nil --> 'ignored')
-- trgInterface - the trg-EPort or the trg-vidx (can be nil --> 'ignored')
-- srcInterface - the src interface . for 'learning' and for 'src-mc-filter'(can be nil --> 'ignored')
-- isMcFilter - (bool) is multicast filter needed (can be nil --> 'ignored')
--                  should be used when isMulticast = true.
local function vTag_6_bytes_buildTtiPatternMask(physicalPortOrTrunk,isTrunk,tag_exists,direction,isMulticast,trgInterface,srcInterface,isMcFilter)
    printLog("vTag_6_bytes_buildTtiPatternMask")
    printLog("physicalPortOrTrunk",tostring(physicalPortOrTrunk))
    printLog("isTrunk",tostring(isTrunk))
    printLog("tag_exists",tostring(tag_exists))
    printLog("direction",tostring(direction))
    printLog("isMulticast",tostring(isMulticast))
    printLog("trgInterface",tostring(trgInterface))
    printLog("srcInterface",tostring(srcInterface))
    printLog("isMcFilter",tostring(isMcFilter))
    
    if (bpe_802_1_br_my_info_DB["dev_info"]["role"] ~= "control_bridge" and
            on_Pe_need_reserved_trunkid_for_uplink_port == true) and
       physicalPortOrTrunk == onPe_upstream_port and
       isTrunk ~= true
    then
        physicalPortOrTrunk = onPe_reserved_trunkid_for_uplink_port--physical trunk
        isTrunk = true
    end

    local currentPattern_udbArray =  {}
    local currentMask_udbArray = {}

    for index = 0,3 do
        currentPattern_udbArray[index] = vTag_6_bytes_pattern_udbArray[index]
        currentMask_udbArray[index] = vTag_6_bytes_mask_udbArray[index]
    end

    for index = 4,9 do
        currentPattern_udbArray[index] = 0
        currentMask_udbArray[index]    = 0
    end


    local index = 0
    if isTrunk == true then
        currentPattern_udbArray[index] = currentPattern_udbArray[index] + bit6
    end


    index = 2
    if tag_exists == false then
        currentPattern_udbArray[index] = 0
    else
        currentPattern_udbArray[index] = 1
    end
    
    index = 3
    if physicalPortOrTrunk ~= nil then
        currentPattern_udbArray[index] = bit_and(physicalPortOrTrunk       ,mask_full_byte)
    else
        currentMask_udbArray[index] = 0
    end

    index = 4
    if srcInterface ~= nil then
        -- index 4
        currentPattern_udbArray[index] = bit_and(srcInterface               , mask_full_byte)
        -- index 5
        currentPattern_udbArray[index + 1] = bit_and(bit_shr(srcInterface,8)    , _4_bits_mask)

        -- index 4
        currentMask_udbArray[index]   = mask_full_byte
        -- index 5
        currentMask_udbArray[index+1] = _4_bits_mask

    end
    
    index = 5
    if isMcFilter ~= nil then
        if isMcFilter == true then 
            isMcFilter = 1 
        else
            isMcFilter = 0
        end
        -- index 5
        currentPattern_udbArray[index] = currentPattern_udbArray[index] + bit_shl(isMcFilter    , 7)
        -- index 5
        -- add bit_shl(1    , 7)
        currentMask_udbArray[index] = currentMask_udbArray[index] + 0x80
    end

    index = 6
    if trgInterface ~= nil then
        -- index 6
        currentPattern_udbArray[index] = bit_and(trgInterface                   , mask_full_byte)
        currentPattern_udbArray[index + 1] = bit_and(bit_shr(trgInterface,8)    , _6_bits_mask)
        -- index 7
        currentMask_udbArray[index]   = mask_full_byte
        currentMask_udbArray[index+1] = _6_bits_mask
    end

    index = 7
    if direction ~= nil then
        if direction ~= direction_downstream then
            direction = direction_upstream
        end
    
        currentPattern_udbArray[index] = currentPattern_udbArray[index] + bit_shl(direction    , 7)
        
        currentMask_udbArray[index] = currentMask_udbArray[index] + 0x80
    end
    
    if isMulticast ~= nil then
        if isMulticast == true then 
            isMulticast = 1 
        else
            isMulticast = 0
        end

        currentPattern_udbArray[index] = currentPattern_udbArray[index] + bit_shl(isMulticast    , 6)
        
        currentMask_udbArray[index] = currentMask_udbArray[index] + 0x40
        
    end
    
    
    index = 8
    if physicalPortOrTrunk ~= nil then
        currentMask_udbArray[index] = mask_udbArray[index]
        currentPattern_udbArray[index] = bit_and(bit_shr(physicalPortOrTrunk,8)    , _5_bits_mask)
    end

    printLog("currentPattern_udbArray",to_string(currentPattern_udbArray))
    printLog("currentMask_udbArray",to_string(currentMask_udbArray))
    
    return currentPattern_udbArray , currentMask_udbArray
    
end

-- function to convert from bpe_802_1_br_buildTtiPatternMask() to vTag_6_bytes_buildTtiPatternMask()
local function convert_ttiRule_BPE_to_vTag_6_bytes(physicalPortOrTrunk,ETag_exists,grp,ecid_base,ingress_ecid_base,isTrunk)
    local tag_exists = ETag_exists
    local isMulticast
    local direction
    local trgInterface
    local srcInterface
    local isMcFilter
    
    if grp then
        if grp == 0 then 
            isMulticast = false
        else
            isMulticast = true
        end
    end
    
    if bpe_802_1_br_is_control_bridge() then -- on CB -- all ingress as upstream
        direction = direction_upstream
    elseif (physicalPortOrTrunk == onPe_upstream_port) then -- on pe : src port is the upstream port  -- all ingress as downstream
        direction = direction_downstream
    else
        direction = direction_upstream
    end
    
    if direction == direction_downstream then
        trgInterface = ecid_base
        if grp and grp ~= 0 then
            trgInterface = trgInterface + (grp * 0x1000)
        end

        srcInterface = ingress_ecid_base
        
        if ingress_ecid_base then
            if ingress_ecid_base == 0 then
                isMcFilter = false
            else
                isMcFilter = true
            end
        end
    else -- upstream
        srcInterface = ecid_base
    end

    if ETag_exists == false then
        isMulticast = nil
        trgInterface = nil
        direction = nil
        srcInterface = nil
        isMcFilter = nil
    end

    
    if isMulticast ~= true then
        -- field relevant only we expect multicast
        isMcFilter = nil
    end
    
    return vTag_6_bytes_buildTtiPatternMask(physicalPortOrTrunk,isTrunk,tag_exists,direction,isMulticast,trgInterface,srcInterface,isMcFilter)
end

-- function to build format of tti rule (pattern,mask) for 802.1_BR common use
-- inputs:
-- physicalPortOrTrunk - the physical port/trunk (exact match)
-- ETag_exists - is ETag exists (exact match)
-- grp       - the grp (can be nil --> 'ignored')
-- ecid_base - the ecid (can be nil --> 'ignored')
-- ingress_ecid_base - the ingress ecid for 'flood' (can be nil --> 'ignored')
-- isTrunk - indication that the 'physicalPort' is actually 'trunk' (can be nil --> 'ignored' --> considered 'false')
--
-- function return 2 arrays:
-- UDBs pattern array
-- UDBs mask array
function bpe_802_1_br_buildTtiPatternMask(physicalPortOrTrunk,ETag_exists,grp,ecid_base,ingress_ecid_base,isTrunk)
    local index

    if not bpe_802_1_br_is_eArch_Device() then
        -- use function bpe_802_1_br_non_eArch_buildTtiPatternMask() instead
        -- note : parameters are different in bpe_802_1_br_non_eArch_buildTtiPatternMask()
        return nil,nil
    end

    if isVTag6BytesConfig then
        return convert_ttiRule_BPE_to_vTag_6_bytes(physicalPortOrTrunk,ETag_exists,grp,ecid_base,ingress_ecid_base,isTrunk)
    end

    
    
    if (bpe_802_1_br_my_info_DB["dev_info"]["role"] ~= "control_bridge" and
            on_Pe_need_reserved_trunkid_for_uplink_port == true) and
       physicalPortOrTrunk == onPe_upstream_port and
       isTrunk ~= true
    then
        physicalPortOrTrunk = onPe_reserved_trunkid_for_uplink_port--physical trunk
        isTrunk = true
    end

    local currentPattern_udbArray =  {}
    local currentMask_udbArray = {}

    for index = 0,3 do
        currentPattern_udbArray[index] = pattern_udbArray[index]
        currentMask_udbArray[index] = mask_udbArray[index]
    end

    for index = 4,9 do
        currentPattern_udbArray[index] = 0
        currentMask_udbArray[index]    = 0
    end


    index = 0
    if isTrunk == true then
        currentPattern_udbArray[index] = currentPattern_udbArray[index] + bit6
    end


    index = 2
    if ETag_exists == false then
        currentPattern_udbArray[index] = 0
    else
        currentPattern_udbArray[index] = 1
    end

    index = 3
    currentPattern_udbArray[index] = bit_and(physicalPortOrTrunk       ,mask_full_byte)

    index = 4
    if ecid_base ~= nil then
        -- index 4
        currentPattern_udbArray[index] = bit_and(ecid_base               , mask_full_byte)
        -- index 5
        currentPattern_udbArray[index + 1] = bit_and(bit_shr(ecid_base,8)    , _4_bits_mask)

        -- index 4
        currentMask_udbArray[index]   = mask_full_byte
        -- index 5
        currentMask_udbArray[index+1] = _4_bits_mask

    end

    index = 5
    if grp ~= nil then
        if grp > 3 then
            return nil,nil
        end
        -- index 5
        currentPattern_udbArray[index] = currentPattern_udbArray[index] + bit_shl(grp    , 4)
        -- index 5
        -- add bit_shl(3    , 4)
        currentMask_udbArray[index] = currentMask_udbArray[index] + 0x30
    end

    index = 6
    if ingress_ecid_base ~= nil then
        -- index 6
        currentPattern_udbArray[index] = bit_and(ingress_ecid_base                   , mask_full_byte)
        currentPattern_udbArray[index + 1] = bit_and(bit_shr(ingress_ecid_base,8)    , _4_bits_mask)
        -- index 7
        currentMask_udbArray[index]   = mask_full_byte
        currentMask_udbArray[index+1] = _4_bits_mask
    end

    index = 8
    currentMask_udbArray[index] = mask_udbArray[index]
    currentPattern_udbArray[index] = bit_and(bit_shr(physicalPortOrTrunk,8)    , _5_bits_mask)

    return currentPattern_udbArray , currentMask_udbArray
end

local keyTypeArr = {
     "CPSS_DXCH_TTI_KEY_UDB_IPV4_TCP_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_MPLS_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_IPV4_FRAGMENT_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_IPV6_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_IPV6_TCP_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_UDE_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_UDE1_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_UDE2_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_UDE3_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_UDE4_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_UDE5_E"
    ,"CPSS_DXCH_TTI_KEY_UDB_UDE6_E"
}
local keyTypeArr_non_eArch = {
    "CPSS_DXCH_TTI_KEY_IPV4_E"
    ,"CPSS_DXCH_TTI_KEY_MPLS_E"
    ,"CPSS_DXCH_TTI_KEY_ETH_E"
    ,"CPSS_DXCH_TTI_KEY_MIM_E"
}

--- function call cpss with multiple key types
function tti_multiple_pciId_and_Udb_config_set(command_data,devNum,pclId,udbArray)
    local keyTypeArr_ptr

    if not bpe_802_1_br_is_eArch_Device() then
        keyTypeArr_ptr = keyTypeArr_non_eArch
    else    
        keyTypeArr_ptr = keyTypeArr
    end
    
    for dummy,currentKeyType in pairs(keyTypeArr_ptr) do

        _debug(currentKeyType)

        local apiName = "cpssDxChTtiPclIdSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",                         "devNum",     devNum },
            { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    currentKeyType},
            { "IN",     "GT_U32",                        "pclId",      pclId  },
        })

        if bpe_802_1_br_is_eArch_Device() then
            -- configure the UDBs needed for the match
            apiName = "cpssDxChTtiUserDefinedByteSet"
            for dummy,udbEntry in pairs(udbArray) do
                genericCpssApiWithErrorHandler(command_data,
                    apiName, {
                    { "IN",     "GT_U8",                         "devNum",     devNum },
                    { "IN",     "CPSS_DXCH_TTI_KEY_TYPE_ENT",    "keyType",    currentKeyType},
                    { "IN",     "GT_U32",                        "udbIndex",   udbEntry.udbIndex },
                    { "IN",     "CPSS_DXCH_TTI_OFFSET_TYPE_ENT", "offsetType", udbEntry.offsetType },
                    { "IN",     "GT_U8",                         "offset",     udbEntry.offset }
                })

            end
        end
    end

end
--- function call cpss with multiple key types
function tti_multiple_cpssDxChTtiPortLookupEnableSet(command_data,devNum,portNum,enableLookup)
    local keyTypeArr_ptr

    if not bpe_802_1_br_is_eArch_Device() then
        keyTypeArr_ptr = keyTypeArr_non_eArch
    else    
        keyTypeArr_ptr = keyTypeArr
    end

    for dummy,currentKeyType in pairs(keyTypeArr_ptr) do
        --***************
        --***************
        local apiName = "cpssDxChTtiPortLookupEnableSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", currentKeyType},
            { "IN", "GT_BOOL", "enable", enableLookup}
        })
        
        
    end
end

-- -- -- -- -- -- 6-bytes v-tag -- -- -- -- -- -- 
-- function relevant for e-Arch device sip_5_10
-- function on CB :
-- don't allow the CB to get downstream indication of 'from BC to PE'.
-- needed due to the testing environment as the LUA tests uses 'loopback ports'
-- NOTE: rule must be first before any other rule
--
local function func_onCb_vtag_6bytes_ttiRule_deny_upstreamTraffic(command_data,devNum,portNum,addPort)
    local myRuleSize = 1
    --***************
    --***************
    local ttiAction = {}
    --hard drop
    ttiAction['command']       = "CPSS_PACKET_CMD_DROP_HARD_E"
    ttiAction['tag1VlanCmd']   = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

    local finalTtiAction = {type2 = ttiAction}
    local finalTtiPattern = {}
    local finalTtiMask = {}

    finalTtiPattern.udbArray , finalTtiMask.udbArray =
        vTag_6_bytes_buildTtiPatternMask(portNum,--physicalPortOrTrunk,
            nil,--isTrunk,
            true,--tag_exists,
            direction_downstream,--direction,
            nil,--isMulticast,
            nil,--trgInterface,
            nil,--srcInterface,
            nil)--isMcFilter)

    local myIndexName = "BPE: don't allow the CB to get downstream indication of 'from BC to PE' (hard drop), device " .. devNum .. " and port ".. portNum
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    local ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti0")

    if addPort == false then
        local apiName = "cpssDxChTtiRuleValidStatusSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U32", "index", ttiIndex},
            { "IN", "GT_BOOL", "valid", GT_FALSE}
        })

        freeTtiIndexToPool(ttiIndex)

    else
        local apiName = "cpssDxChTtiRuleSet"
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

-- @param command_data
-- @param addPort
-- @param devNum
-- @param portNum
--
-- function on CB :
-- set tti config for the physical port that connected to the PE devices
local function func_onCb_phyPortConnectedToPe_ttiConfigSet(command_data,addPort,devNum,portNum)
    local apiName
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local ttiAction = {}
    local ttiIndex
    local enableLookup

    -- configure the physical port
    if addPort == true then
        enableLookup = GT_TRUE
    else
        enableLookup = GT_FALSE
    end

    --***************
    --***************
    apiName = "cpssDxChTtiPacketTypeKeySizeSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "CPSS_DXCH_TTI_KEY_TYPE_ENT", "keyType", keyType},
        { "IN", "CPSS_DXCH_TTI_KEY_SIZE_ENT", "keySize", keySize}
    })
    
    

    --***************
    --***************
    tti_multiple_cpssDxChTtiPortLookupEnableSet(command_data,devNum,portNum,enableLookup)

    --***************
    --***************
    if isVTag6BytesConfig then
        -- don't allow the CB to get downstream indication of 'from BC to PE'.
        -- needed due to the testing environment as the LUA tests uses 'loopback ports'
        func_onCb_vtag_6bytes_ttiRule_deny_upstreamTraffic(command_data,devNum,portNum,addPort)
    end
    
    local myRuleSize = 1
    -- remove the associated tti entry of this physical port
    -- index within the 'action 0' range
    local myIndexName = "BPE: no ETag assign vid0=0 , device" .. devNum .. " and port ".. portNum
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    ttiIndex = allocTtiIndexFromPool_fromEnd(myRuleSize,myIndexName,"tti0")

    ---- force all packets on this physical port that did not matched the 'ETag' to assign vid0=0 --> cause drop !
    ttiAction['tag0VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"
    ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"
    ttiAction['tag0VlanId']            = 0
    ttiAction['tag1VlanId']            = 0

    finalTtiAction.type2 = ttiAction

    -- match all the traffic on the physical port (that was not matched by ETag)
    finalTtiPattern.udbArray = {}
    finalTtiPattern.udbArray[3] = bit_and(portNum                   , mask_full_byte)
    finalTtiMask.udbArray    = {}
    finalTtiMask.udbArray[3] = mask_full_byte

    if addPort == false then
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

-- function to set e2phy : eport to physical port mapping
-- command_data - 
-- devNum - the 'sw devNum'
-- eport - the eport as index in e2phy table
-- phyPort - the physical port number
-- unset - when nil or false , e2phy entry for eport set with phyPort info
--			when true , e2phy entry for eport set with '0' as info
function e2phySet(command_data,devNum,eport,phyPort,unset)

    local physicalInfo = {}
    physicalInfo.type = "CPSS_INTERFACE_PORT_E"
    physicalInfo.devPort = {}
    local hwDevNum , dummyPortNum =
        Command_Data:getHWDevicePort(devNum,phyPort)

    if unset == true then
        physicalInfo.devPort.devNum = 0
        physicalInfo.devPort.portNum = 0
    else
        physicalInfo.devPort.devNum  = hwDevNum
        physicalInfo.devPort.portNum = phyPort
    end

    --***************
    --***************
    -- map the EPort to the physical port 
    local apiName = "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", eport},
        { "IN", "CPSS_INTERFACE_INFO_STC", "physicalInfoPtr", physicalInfo }
    })
end

-- function to set/unset the 'push command' on the eport
local function pushETagCommandOnEPort(command_data,devNum,eport,vlanCmd,pcid)
    --***************
    --***************
    local apiName = "cpssDxChBrgVlanPortPushVlanCommandSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", eport},
        { "IN", "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT", "vlanCmd", vlanCmd }
    })
    
    

    --***************
    --***************
    apiName = "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", eport},
        { "IN", "GT_U32", "tpidEntryIndex", bpe_802_1_br_egress_pushed_tag_tpid_reserved_index }
    })
    
    

    --***************
    --***************
    apiName = "cpssDxChBrgVlanPortPushedTagValueSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", eport},
        { "IN", "GT_U16", "tagValue",  pcid}
    })
    
    
end

-- function on CB,PE :
-- set config for cascade port (connected to the PE device)
-- set 'push ETag' --> needed for 'flood in vlan' (when target is 'useVidx' --> flood to vlan/vidx)
local function func_cascadePortPushETag(command_data,addPort,devNum,portNum)
  local vlanCmd
    -- configure the physical port
    if addPort == true then
        vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E"
    else
        vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E"
    end
    pushETagCommandOnEPort(command_data,devNum,portNum,vlanCmd,dummy_vid_0xfff)
end


-- function on CB :
-- set config for physical port that connected to the PE devices
local function func_onCb_phyPortConnectedToPe(command_data,addPort,devNum,portNum)
    local apiName

    -- configure the physical port
    if addPort == true then
        _debug ("Add direct PE on port =" .. portNum)
    else
        _debug ("Remove direct PE from port =" .. portNum)
    end

    func_onCb_phyPortConnectedToPe_ttiConfigSet(command_data,addPort,devNum,portNum)

    -- set 'push ETag' --> needed for 'flood in vlan' (when target is 'useVidx' --> flood to vlan/vidx)
    func_cascadePortPushETag(command_data,addPort,devNum,portNum)

end

-- function on CB :
-- set configuration that needed by the CB that connected to any PE
local function func_onCb_deviceConnectedToPe(command_data,addFirstPort,devNum)
    local apiName
    local dummy,udbEntry
    local pclId = 0
    local tpidBmp

    if addFirstPort == true then
        _debug ("Add first PE device")
        pclId = reserved_pclId
        tpidBmp = 0
    else
        _debug ("Remove last PE device")
        pclId = 0
        tpidBmp = 0xFF
    end

    --***************
    --***************
    _debug("use profile 6 as reserved for 'force no tag 1 recognition' on the device (for srcEPorts after 'pop')")
    apiName = "cpssDxChBrgVlanIngressTpidProfileSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "profile", ingress_tpid_profile_for_empty_bmp_index},
        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
        { "IN", "GT_U32", "tpidBmp", tpidBmp }
    })
    
    tti_multiple_pciId_and_Udb_config_set(command_data,devNum,pclId,udbArr)
end


-- function on PE :
-- set tti config for the physical port that connected to the PE devices
-- set 'cascade port'
local function func_onPe_phyPortConnectedToPe_ttiConfigSet(command_data,addPort,devNum,portNum)
    local apiName
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local ttiAction = {}
    local ttiIndex
    local enableLookup

    -- set the TTI rule + action
    local ttiIndex
    local ttiAction = {}
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local egressInterface = {}

    local myRuleSize = 1

    -- cinfigure the physical port
    if addPort == true then
        enableLookup = GT_TRUE
    else
        enableLookup = GT_FALSE
    end

    --***************
    --***************
    tti_multiple_cpssDxChTtiPortLookupEnableSet(command_data,devNum,portNum,enableLookup)

    --***************
    --***************
    ttiAction = {}
    ttiAction['nestedVlanEnable']       = GT_TRUE
    ttiAction['bridgeBypass']           = GT_TRUE
    ttiAction['ingressPipeBypass']      = GT_TRUE

    ttiAction['redirectCommand']           = "CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E"

    egressInterface.type = "CPSS_INTERFACE_PORT_E"

    local physical_hwDevNum , dummy_portNum =
        Command_Data:getHWDevicePort(devNum,onPe_reserved_eport_of_upstream_ingress_with_etag)

    egressInterface.devPort = {}
    egressInterface.devPort.devNum = physical_hwDevNum
    egressInterface.devPort.portNum = onPe_reserved_eport_of_upstream_ingress_with_etag

    ttiAction['egressInterface'] = egressInterface

    ttiAction['tag0VlanCmd']           = "CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E"
    ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

    finalTtiAction.type2 = ttiAction

    finalTtiPattern.udbArray , finalTtiMask.udbArray =
        bpe_802_1_br_buildTtiPatternMask(portNum,--physicalPort
            true,-- ETag_exists
            nil,-- grp
            nil,-- ecid_base
            nil) -- ingress_ecid_base

    local myIndexName = "BPE: PVE to upstream port from cascade port with ETag , device " .. devNum .. " and port ".. portNum
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti0")

    if addPort == false then
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

-- function on PE :
-- set the cascade port associated with the trunkId of the uplink port/trunk.
-- Set 'cascade port'
local function func_onPe_cascadePort_associated_to_uplink_trunk_set(command_data,addFirstPort,devNum,portNum)
    local trunkId = onPe_reserved_trunkid_for_uplink_port
    local enable = addFirstPort

    --***************
    --***************
    _debug("use trunkId [" .. trunkId .. "] for upstream port [" .. portNum .. "]")


    local apiName = "cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PHYSICAL_PORT_NUM", "cascadePortNum", portNum},
        { "IN", "GT_BOOL", "enable", enable },
        { "IN", "GT_TRUNK_ID", "trunkId", trunkId }
    })
end

-- use index 7 -- assumed to be set by the 'configuration'
local tpidBmpForTag1 = bit_shl(1,bpe_802_1_br_egress_pushed_tag_tpid_reserved_index)

-- function on PE , non eArch device
-- set config for physical port that connected to the PE devices
local function func_onPe_non_eArch_phyPortWithETag(command_data,addPort,devNum,portNum)
    local enable,tpidBmp,isMember,taggingCmd,enableLookup,tpidEntryIndex
    

    if addPort == true then
        enable = true
        enableLookup = true
        -- use index 7 -- assumed to be set by the 'configuration'
        tpidBmp = tpidBmpForTag1
        tpidEntryIndex = bpe_802_1_br_egress_pushed_tag_tpid_reserved_index
        isMember = true
        taggingCmd = "CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E"
    else
        enable = false
        enableLookup = false
        --[[
            Non-BPE ports (if they exist), must be configured not to classify any TPID as Tag1, i.e.,
            non-BPE ports may only have Tag0 classification performed        
        --]]
        tpidBmp = 0 -- same value that set by func_onPe_non_eArch_bpe_802_1_br__my_info !!!
                    -- the 'restore' to 0xFF is done by CLI command : "no bpe-802-1-br my-info"
        tpidEntryIndex = 0
        isMember = false
        taggingCmd = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E"
    end

    local apiName = "cpssDxChBrgPePortEnableSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "portNum", portNum},
        { "IN", "GT_BOOL", "enable", enable }
    })
    
    -- The E-Tag is to be classified as Tag1 by configuring the E-Tag TPID (0x893F) in the TPID table, and
    -- configure the Port Extender ports to classify the E-Tag TPID as Tag1
    apiName = "cpssDxChBrgVlanPortIngressTpidSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "portNum", portNum},
        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
        { "IN", "GT_U32", "tpidBmp", tpidBmp }
    })
    
    
    apiName = "cpssDxChBrgVlanPortEgressTpidSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "portNum", portNum},
        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
        { "IN", "GT_U32", "tpidEntryIndex", tpidEntryIndex }
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
        { "IN",     "GT_BOOL",    "CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT",  taggingCmd}
    })
    
    -- enable tti lookup on the physical port
    tti_multiple_cpssDxChTtiPortLookupEnableSet(command_data,devNum,portNum,enableLookup)
end
-- function on PE , non eArch device
-- set config for physical port that connected to the PE devices
-- set 'cascade port'
local function func_onPe_non_eArch_phyPortConnectedToPe(command_data,addPort,devNum,portNum)
    if addPort == true then
        _debug ("Add direct PE on port =" .. portNum)
    else
        _debug ("Remove direct PE from port =" .. portNum)
    end
    func_onPe_non_eArch_phyPortWithETag(command_data,addPort,devNum,portNum)
end
-- function on PE :
-- set config for physical port that connected to the PE devices
-- set 'cascade port'
local function func_onPe_phyPortConnectedToPe(command_data,addPort,devNum,portNum)
    if not bpe_802_1_br_is_eArch_Device() then
        -- no eArch device
        return func_onPe_non_eArch_phyPortConnectedToPe(command_data,addPort,devNum,portNum)
    end

    local apiName
    local trgCscdEPort

    -- configure the physical port
    if addPort == true then
        _debug ("Add direct PE on port =" .. portNum)
    else
        _debug ("Remove direct PE from port =" .. portNum)
    end

    if (on_Pe_need_reserved_trunkid_for_uplink_port == true) then
        -- set the cascade port associated with the trunkId of the uplink port/trunk.
        func_onPe_cascadePort_associated_to_uplink_trunk_set(command_data,addPort,devNum,portNum)
    end


    -- allocate/get eport for the cscd port to use
    trgCscdEPort = onPe_getEportOfPcidOnCscdPort(portNum)

    func_onPe_phyPortConnectedToPe_ttiConfigSet(command_data,addPort,devNum,portNum)

    -- set 'push ETag' --> needed for 'flood in vlan' (when target is 'useVidx' --> flood to vlan/vidx)
    func_cascadePortPushETag(command_data,addPort,devNum,portNum)

    --***************
    --***************
    local tpidBmp

    if addPort == true then
        tpidBmp = 0
    else
        tpidBmp = 0xFF
    end
    _debug("use profile 6 as reserved for 'force no tag 1 recognition' on the device (for serEPorts after 'pop')")
    apiName = "cpssDxChBrgVlanIngressTpidProfileSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "profile", ingress_tpid_profile_for_empty_bmp_index},
        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
        { "IN", "GT_U32", "tpidBmp", tpidBmp }
    })
    
    

    --***************
    --***************
    if addPort == true then
        -- add the port to vidx of cascade ports
        apiName = "cpssDxChBrgMcMemberAdd"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U16", "vidx", bpe_reservedVidxForCascadePortsGet()},
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum}
        })
    else
        -- remove the port from vidx of cascade ports
        apiName = "cpssDxChBrgMcMemberDelete"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U16", "vidx", bpe_reservedVidxForCascadePortsGet()},
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum}
        })
    end

    -- map the EPort to the physical port (cscd port)
    if addPort == true then
        e2phySet(command_data,devNum,trgCscdEPort,portNum)
    else
        e2phySet(command_data,devNum,trgCscdEPort,portNum,true)
    end

    -- configure the physical port
    if addPort ~= true then
        -- free the eport of the cscd port to use
        onPe_freeEportOfPcidOnCscdPort(portNum)
    end

end

-- function on PE :
-- convert physical port number to it's srcId --> 
-- used for src port filtering on downstream
local function srcIdFromPhysicalPortNum(portNum)
    -- avoid the value 0, to be 'unattached'
    return (portNum + 1)
end

-- -- -- -- -- -- 6-bytes v-tag -- -- -- -- -- -- 
-- function relevant for e-Arch device sip_5_10
-- function on PE :
-- calc final srcId for source filtering
local function srcIdCalcForVTag6BytesGet(srcId , isMcFilter)
    local value = srcId + bit_shl(1,bpe_802_1_br_6_bytes_tag_dBitInSrcId)

    if isMcFilter then
        return value + bit_shl(1,bpe_802_1_br_6_bytes_tag_lBitInSrcId)
    else
        return value
    end
end

-- function on PE :
-- set tti rule for the physical port that connected to the CB device
-- rule_for_ingr_ecid0 - false is default rule, true is rule mathing Ingress_ECid==0
-- Set 'upstream port'
local function func_onPe_phyPortConnectedToCb_ttiConfigSet_rule(
    command_data, addFirstPort, devNum, portNum, rule_for_ingr_ecid0)
    local apiName,result
    local myIndexName
    local ingress_ecid_base_pattern;

    if (rule_for_ingr_ecid0 == true) then
        ingress_ecid_base_pattern = 0;
    else
        ingress_ecid_base_pattern = nil;
    end

    --[[
    Set the source ePort of packet's with E-TAG<Ingress_E-CID> = 0 to a reserved ePort
    that represent non-existing Extended Port. This is to disable Multicast source filtering for that packet ==
    tti rule :
        'my_current_port'
        E-TAG<Ingress_E-CID> = <0>
    tti action:
        reassign srcEport = 1
        srcEport = reserved_eport_of_upstream_ingress_with_etag
    ]]--

    -- set the TTI rule + action
    local ttiIndex
    local ttiAction = {}
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}

    local myRuleSize = 1

    ttiAction['sourceEPortAssignmentEnable'] = GT_TRUE
    ttiAction['sourceEPort']           = onPe_reserved_eport_of_upstream_ingress_with_etag
    ttiAction['tag0VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

    -- assign srcId for the src filtering that needed for 'multi-destination'
    -- this is much more scalable mechanism then the L2MLL that described in the FS.
    ttiAction.sourceIdSetEnable = GT_TRUE

    -- use srcId that should not be part of MC-filtering
    local srcId = 0
    
    if isVTag6BytesConfig then
        -- the filter need to be for '<direction> = direction_downstream' and '<isMcFilter> = true' indications
        if (rule_for_ingr_ecid0 == true) then
            ttiAction.sourceId = srcIdCalcForVTag6BytesGet(srcId , false--[[<isMcFilter> = false]])
        else
            ttiAction.sourceId = srcIdCalcForVTag6BytesGet(srcId , true--[[<isMcFilter> = true]])
        end
    else
        ttiAction.sourceId = srcId 
    end
    
    
    -- enable ability for looup target interface
    ttiAction['continueToNextTtiLookup'] = GT_TRUE

    finalTtiAction.type2 = ttiAction

    finalTtiPattern.udbArray , finalTtiMask.udbArray =
        bpe_802_1_br_buildTtiPatternMask(portNum, --physicalPort
            true,-- ETag_exists
            nil,-- grp
            nil,-- ecid_base
            ingress_ecid_base_pattern, -- ingress_ecid_base
            isTrunk)-- isTrunk

    if (rule_for_ingr_ecid0 == true) then
        myIndexName = "BPE: E-TAG<Ingress_E-CID_base> = 0 , device " .. devNum .. " and port " .. portNum
    else
        myIndexName = "BPE: E-TAG<Ingress_E-CID_base> dont_care, device " .. devNum .. " and port " .. portNum
    end

    if (on_Pe_need_reserved_trunkid_for_uplink_port == true) then
        myIndexName = myIndexName .. " use physical trunkId " .. onPe_reserved_trunkid_for_uplink_port
    end

    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    if (rule_for_ingr_ecid0 == true) then
        ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti0")
    else
        ttiIndex = allocTtiIndexFromPool_fromEnd(myRuleSize,myIndexName,"tti0")
    end

    if addFirstPort == false then
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

-- function on PE :
-- set tti config for the physical port that connected to the CB device
-- Set 'upstream port'
local function func_onPe_phyPortConnectedToCb_ttiConfigSet(command_data,addFirstPort,devNum,portNum)
    local pclId = 0;
    if addFirstPort == true then
        pclId = reserved_pclId
    else
        pclId = 0
    end

    tti_multiple_pciId_and_Udb_config_set(command_data,devNum,pclId,udbArr)

    -- default rule for any Ingress ECID (Ingress ECID - "don't care")
    func_onPe_phyPortConnectedToCb_ttiConfigSet_rule(
        command_data,addFirstPort,devNum,portNum,false);

    -- default rule for any Ingress ECID==0
    func_onPe_phyPortConnectedToCb_ttiConfigSet_rule(
        command_data,addFirstPort,devNum,portNum,true);
end

-- bind the port with profile that resereved for 'bmp = 0' to cause no recognition of tag1 (after pop of ETag)
-- this is bc2_A0 limitation
local function setPortToNotRecognizeTag1AfterEtagPopped(command_data,devNum,portNum,isDefaultProfile,addFirstPort)
  local profile
  
    if addFirstPort == true then
        profile = ingress_tpid_profile_for_empty_bmp_index
    else
        profile = 0
    end

    local apiName = "cpssDxChBrgVlanPortIngressTpidProfileSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PORT_NUM", "portNum", portNum},
        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
        { "IN", "GT_BOOL", "isDefaultProfile", isDefaultProfile },
        { "IN", "GT_U32", "profile", profile }
    })
    
    

end

-- function on PE :
-- set the cascade port associated with the trunkId of the uplink port/trunk.
-- Set 'upstream port'
local function func_onPe_upstreamPort_trunkId_set(command_data,addFirstPort,devNum,portNum)
    local trunkId = onPe_reserved_trunkid_for_uplink_port
    local numOfEnabledMembers
    local startIndex = 1
    local enabledMembersArray  = {[startIndex] = {device= 5 ,port = 33}}
--    local disabledMembersArray = {[startIndex] = {device= 9 ,port = 55}}

    local apiName
    
    local hwDevNum , dummy_portNum =
        Command_Data:getHWDevicePort(devNum,portNum)

    enabledMembersArray[startIndex].device = hwDevNum
    enabledMembersArray[startIndex].port = portNum

    --[[ it seems that the LUA currentyl not support parsing correctly the  enabledMembersArray !
    --  so instead use 'add'/'remove' member
    --
    apiName = "cpssDxChTrunkMembersSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_TRUNK_ID", "trunkId", trunkId},
        { "IN", "GT_U32", "numOfEnabledMembers", numOfEnabledMembers },
        { "IN", "CPSS_TRUNK_MEMBER_STC", "enabledMembersArray", enabledMembersArray },
        { "IN", "GT_U32", "numOfDisabledMembers", 0 },
        { "IN", "CPSS_TRUNK_MEMBER_STC", "disabledMembersArray", disabledMembersArray }
    })
    --]]
    if addFirstPort == true then
        apiName = "cpssDxChTrunkMemberAdd"
    else
        apiName = "cpssDxChTrunkMemberRemove"
    end

--    print (apiName , to_string(enabledMembersArray))

    genericCpssApiWithErrorHandler(command_data,
        apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_TRUNK_ID", "trunkId", trunkId},
            { "IN", "CPSS_TRUNK_MEMBER_STC", "memberPtr", enabledMembersArray[startIndex] }
        })



end

local function func_onPe_non_eArch_bpe_802_1_tti_Rule_update(command_data,myIndexName,addRule,devNum,finalTtiPattern,finalTtiMask,finalTtiAction,from_end_tcam)
    local myRuleSize = 3
    local ruleType = "CPSS_DXCH_TTI_RULE_ETH_E"
    local apiName
    
    local ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti0",from_end_tcam)

    if addRule == false then
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
            { "IN",     "CPSS_DXCH_TTI_RULE_UNT_eth", "patternPtr", finalTtiPattern},
            { "IN",     "CPSS_DXCH_TTI_RULE_UNT_eth", "maskPtr",    finalTtiMask},
            { "IN",     "CPSS_DXCH_TTI_ACTION_TYPE_ENT",   "actionType", "CPSS_DXCH_TTI_ACTION_TYPE2_ENT"},
            { "IN",     "CPSS_DXCH_TTI_ACTION_UNT_type2",  "actionPtr",  finalTtiAction},
        })
    end
end

-- function on PE , non eArch device
-- set configuration that needed by the PE that connected to any CB
-- Set 'upstream port'
local function func_onPe_non_eArch_deviceConnectedToCb(command_data,addFirstPort)
    local devNum  = onPe_upstream_device
    local portNum = onPe_upstream_port

    if addFirstPort == true then
        _debug("Add the one and only CB device")
    else
        _debug("Remove the one and only CB device")
    end

    func_onPe_non_eArch_phyPortWithETag(command_data,addFirstPort,devNum,portNum)

    local pclId
    if addFirstPort == true then
        pclId = reserved_pclId
    else
        pclId = 0
    end

    tti_multiple_pciId_and_Udb_config_set(command_data,devNum,pclId,nil)
    
    --[[
        multicast packets also bypass the ingress pipe, but unlike Unicast packets, the output port/s are
        determined in the egress pipe.
        Two TTI rules (assigned to the upstream port) are allocated to bypass ingress pipe processing.
        The first TTI rule have the following key:
            - <Tag1 Exist>=Exists - This ensures that the packet is E-Tagged.
            - <E-Tag GRP>=1
        The second TTI rule have the following key:
            - <Tag1 Exist>=Exists - This ensures that the packet is E-Tagged.
            - <E-Tag GRP>=2 or 3
        The TTI rules apply the following action:
            <Bypass Ingress Pipe>= Enabled
    --]]
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local ttiAction = {}
    
    ttiAction['bridgeBypass']           = true
    ttiAction['ingressPipeBypass']      = true
    ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

    finalTtiAction.type2 = ttiAction
    
    finalTtiPattern.eth , finalTtiMask.eth =
        bpe_802_1_br_non_eArch_buildTtiPatternMask(portNum,--physicalPort
            true,-- ETag_exists
            1,-- grp
            nil,-- ecid_base
            false) -- isTrunk

    local myIndexName = "BPE: downstream multicast packets set bypass the ingress pipe (grp = 0), device " .. devNum .. " and port ".. portNum
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    func_onPe_non_eArch_bpe_802_1_tti_Rule_update(command_data,myIndexName,addFirstPort,devNum,finalTtiPattern,finalTtiMask,finalTtiAction)

    finalTtiPattern.eth , finalTtiMask.eth =
        bpe_802_1_br_non_eArch_buildTtiPatternMask(portNum,--physicalPort
            true,-- ETag_exists
            3,-- grp
            nil,-- ecid_base
            false) -- isTrunk
            
    finalTtiPattern.eth.eTagGrp = 2 
    finalTtiMask.eth.eTagGrp    = 2 -- ignore LSBit to cover values 2,3
    local myIndexName = "BPE: downstream multicast packets set bypass the ingress pipe (grp = 2,3), device " .. devNum .. " and port ".. portNum
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    func_onPe_non_eArch_bpe_802_1_tti_Rule_update(command_data,myIndexName,addFirstPort,devNum,finalTtiPattern,finalTtiMask,finalTtiAction)
    
end

-- function to set eport with 'pop-x-bytes of tag'
local function numOfTagBytesToPopSet(devNum, portNum, numBytesToPop)
    if numBytesToPop == long_tag then
        if isVTag6BytesConfig then
            numBytesToPop = 6
        else
            numBytesToPop = 8
        end
    end
--[[
    if numBytesToPop == 0 or numBytesToPop == nil then 
        numBytesToPop = "GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E"
    elseif numBytesToPop == 4 then
        numBytesToPop = "GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_4_E"
    elseif numBytesToPop == 8 then
        numBytesToPop = "GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_8_E"
    elseif numBytesToPop == 6 then
        numBytesToPop = "GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_6_E"
    end
  
    local apiName = "cpssDxChBrgVlanPortNumOfTagWordsToPopSet"
    return cpssPerPortParamSet(
        apiName,
        devNum, portNum, numBytesToPop, "numBytesToPop",
        "GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT")
--]]

    if numBytesToPop == 0 or numBytesToPop == nil then 
        numBytesToPop = 0
    elseif numBytesToPop == 4 then
        numBytesToPop = 1
    elseif numBytesToPop == 8 then
        numBytesToPop = 2
    elseif numBytesToPop == 6 then
        numBytesToPop = 3
    end

    local apiName = "cpssDxChBrgVlanPortNumOfTagWordsToPopSet"
    return cpssPerPortParamSet(
        apiName,
        devNum, portNum, numBytesToPop, "numBytesToPop",
        "GT_U32")


end


-- function on PE :
-- set configuration that needed by the PE that connected to any CB
-- Set 'upstream port'
--[[
physical_port_upstream = my_current_port (save to DB for others to use too)
Set e2phy to map reserved_eport_of_upstream_ingress_with_etag to physical port (my_current_port)
HA <Target ePort Tag State> = untagged
HA <ePort Tag State Mode> = 'eport'
For all port_in_loop = 128 to 253 :
Set e2phy to map (reserved_eport_of_upstream_ingress_with_no_etag_base +  port_in_loop)  to physical port (my_current_port)
== Set the source ePort of packet's with E-TAG<Ingress_E-CID> = 0 to a reserved ePort that represent non-existing Extended Port. This is to disable Multicast source filtering for that packet ==
tti rule :
'my_current_port'
E-TAG<Ingress_E-CID> = <0>
        tti action:
reassign srcEport = 1
srcEport = reserved_eport_of_upstream_ingress_with_etag
add/remove default downstream TTI1 rule that drops packets with unknown <Group, PCid_base>
]]--
local function func_onPe_deviceConnectedToCb(command_data,addFirstPort)
    if not bpe_802_1_br_is_eArch_Device() then
        -- no eArch device
        return func_onPe_non_eArch_deviceConnectedToCb(command_data,addFirstPort)
    end


    local cb_physical_devNum , cb_physical_portNum , cb_physical_hwDevNum
    local apiName
    local dummy,udbEntry
    local tagStateMode , tagCommand , numberOfTagWords , vlanCmd , enableLookup
    local devNum,portNum
    local tpidBmp
    local loopEPortNum

    tagCommand = "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E"

    if addFirstPort == true then
        _debug("Add the one and only CB device")
        tagStateMode = "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E"
        enableLookup = GT_TRUE
        numberOfTagWords = 2
        vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E"
        tpidBmp = 0
    else
        _debug("Remove the one and only CB device")
        tagStateMode = "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E"
        enableLookup = GT_FALSE
        numberOfTagWords = 0
        vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E"
        tpidBmp = 0xFF
    end

    devNum  = onPe_upstream_device
    portNum = onPe_upstream_port

    if (on_Pe_need_reserved_trunkid_for_uplink_port == true) then
        -- set the upstream port as trunk member in the onPe_reserved_trunkid_for_uplink_port.
        func_onPe_upstreamPort_trunkId_set(command_data,addFirstPort,onPe_upstream_device,onPe_upstream_port)
    end

    --***************
    --***************
    _debug("use profile 6 as reserved for 'force no tag 1 recognition' on the device (for serEPorts after 'pop')")
    apiName = "cpssDxChBrgVlanIngressTpidProfileSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "profile", ingress_tpid_profile_for_empty_bmp_index},
        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
        { "IN", "GT_U32", "tpidBmp", tpidBmp }
    })
    
    

    --***************
    --***************
    -- for onPe_reserved_eport_of_upstream_ingress_with_etag need to do 'pop 2 tags'
    local result = numOfTagBytesToPopSet(
        devNum, onPe_reserved_eport_of_upstream_ingress_with_etag, long_tag)

    --***************
    --***************
    setPortToNotRecognizeTag1AfterEtagPopped(command_data,devNum,onPe_reserved_eport_of_upstream_ingress_with_etag,false,addFirstPort)

    --***************
    --***************
    tti_multiple_cpssDxChTtiPortLookupEnableSet(command_data,devNum,portNum,enableLookup)

    cb_physical_hwDevNum , cb_physical_portNum =
        Command_Data:getHWDevicePort(devNum,portNum)

    --***************
    --***************
    -- eport for physical port of upsteam port , for ingress packets with ETag (from 'cascade' ports)
    if addFirstPort == false then
        e2phySet(command_data,devNum,onPe_reserved_eport_of_upstream_ingress_with_etag,portNum,true)
    else
        e2phySet(command_data,devNum,onPe_reserved_eport_of_upstream_ingress_with_etag,portNum)
    end

    --***************
    --***************
    -- loop on all ports and assign 'eports' for the physical ports
    -- eport for physical port of upsteam port, for ingress packets without ETag (from 'cascade' ports or from 'port extender' ports)
    for index = onPe_reserved_eport_of_upstream_ingress_with_no_etag_base,
                (onPe_reserved_eport_of_upstream_ingress_with_etag - 1)
    do
        loopEPortNum = index

        if addFirstPort == false then
            e2phySet(command_data,devNum,loopEPortNum,portNum,true)
        else
            e2phySet(command_data,devNum,loopEPortNum,portNum)
        end

        --***************
        --***************
        -- the upstream port must add the ETag (as ingress came without ETag)
        apiName = "cpssDxChBrgVlanPortPushVlanCommandSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", loopEPortNum},
            { "IN", "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT", "vlanCmd", vlanCmd }
        })
        
        
    end

    -- loop on all those 'eports' that represents the 'upstream port' and state to egress untagged according to eport.
    -- because the upstream direction is doing 'PVE to upstream port'
    for index = onPe_reserved_eport_of_upstream_ingress_with_no_etag_base,
                onPe_reserved_eport_of_upstream_ingress_with_etag
    do
        loopEPortNum = index
        --***************
        --***************
        apiName = "cpssDxChBrgVlanEgressPortTagStateModeSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",                         "devNum",     devNum },
            { "IN",     "GT_PORT_NUM",                   "portNum",    loopEPortNum},
            { "IN",     "CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT","stateMode",      tagStateMode  },
        })
        
        --***************
        --***************
        apiName = "cpssDxChBrgVlanEgressPortTagStateSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",                         "devNum",     devNum },
            { "IN",     "GT_PORT_NUM",                   "portNum",    loopEPortNum},
            { "IN",     "CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT","tagCmd",      tagCommand  },
        })
        
        
    end


    --***************
    --***************
    -- default rule in Tti1 that drops packets with unknown <Group, PCid_base> target
    on_pe_evidx_flood_tti_default_rule(devNum, portNum, addFirstPort);
    func_onPe_phyPortConnectedToCb_ttiConfigSet(command_data,addFirstPort,devNum,portNum)

end

-- the 6-bytes-tag system need to explicit state the role type to the device.
-- on 8-bytes system it is not needed
local function func_onCbAndOnPe_bpe_802_1_br_6_bytes_SetMyRole(command_data , devNum , role_is_control_bridge)
    local role_is_pe = not role_is_control_bridge
    local apiName = "cpssDxChBrgVlan6BytesTagConfigSet"
    
    isVTag6BytesConfig = true
    
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_BOOL", "isPortExtender", role_is_pe},
        { "IN", "GT_U32", "lBitInSrcId", bpe_802_1_br_6_bytes_tag_lBitInSrcId},
        { "IN", "GT_U32", "dBitInSrcId", bpe_802_1_br_6_bytes_tag_dBitInSrcId }
    })
end

-- ************************************************************************
---
--  bpe_802_1_br_neighbor_port_neighbor_info
--        @description  Info about the neighbor to 'my device' in BPE system (bridge port externder) according to IEEE-802.1BR standard
--
--        @param params             - params["bpe_802_1_br___role"]: the role <cb/pe>
--                                  - params["role_name"]: the role name (string)
--                                  - params["connection_type"]: My connection to this neighbor <direct/indirect>
--                                    and port interface
--                                  - params["tag_type"]: (optional) the tag-type of the connection :
--                                  	'8-bytes-tag' (default) : the tag is 8-bytes (standard 802.1br)
--                                  	'6-bytes-tag'  : the tag is 6-bytes
--
--        @return       true on success, otherwise false and error message
--
local function bpe_802_1_br_neighbor_port_neighbor_info(params)
    -- Common variables declaration
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration

    params["etag_ether_type"] = ieee_tpid_ETag

    local etherType     = tonumber(params["etag_ether_type"])
    local neighbor_name = params["neighbor_name"]
    local role          = params["role"]
    local connection_type = params["connection_type"]
    local connection_type = params["connection_type"]
    local tag_type = params["tag_type"] 
    local remove_neighbor = false
    local apiName
    local my_role_is_control_bridge

    if not bpe_802_1_br_my_info_DB then
        print(bpe_802_1_br_error_system_is_not_802_1br_string)
        return nil
    end

    if tag_type == nil then
        -- the default for this optional parameter
        tag_type = "8-bytes-tag"
    end
    
    if params.flagNo == true then
        -- indication that we do 'remove' neighbor
        remove_neighbor = true

        local neighbor_entry = table_bpe_802_1_br_system_info_devices[neighbor_name]    
        
        if neighbor_entry == nil then
            print("ERROR : trying to remove  unknown neighbor " .. neighbor_name)
            return nil
        end
        -- get info from the DB 
        role = neighbor_entry.role
        connection_type = neighbor_entry.connection_type
        etherType = neighbor_entry.etag_ether_type
    end

    
    
    if bpe_802_1_br_my_info_DB["dev_info"]["role"] == "control_bridge" then
        my_role_is_control_bridge = true
    else
        my_role_is_control_bridge = false
    end

    if role == "control_bridge" and my_role_is_control_bridge == true then
        print("ERROR : 'control bridge' can't have other 'control bridge' as neighbor")
        return nil
    end

    -- check that PE must have neighbor CB before any neighbor PE
    if my_role_is_control_bridge == false   and
       role ~= "control_bridge"      and
       onPe_control_bridge_name == nil  and
       remove_neighbor == false then

        print("ERROR : you must define 'control bridge' neighbor before any 'port extender' neighbor")
        return nil

    end

    -- on PE only single neighbor CB
    if my_role_is_control_bridge == false       and
            role == "control_bridge"     and
            onPe_control_bridge_name ~= nil     and
            onPe_control_bridge_name ~= neighbor_name  then

        if remove_neighbor == true then
            print("ERROR : tring to remove 'control bridge' with wrong name " .. neighbor_name .. "should be " .. onPe_control_bridge_name)
        else
            print("ERROR : Only single 'control bridge' neighbor allowed")
        end

        return nil

    end


    if etherType == nil then
        print("ERROR : The format of the 'etag-tpid' must be 'numeric' (decimal/HEX)")
        return nil
    end

    if(etherType > max_ether_type) then
        print("ERROR : bad value for 'etag-tpid' must be equal or less then " .. string.format("0x%x",tostring(max_ether_type)) )
        return nil
    end

    local numOfmatch = 0
    local numOfIterations = 0
    local current_index , current_entry
    local current_command_data
    local current_is_interface_match
    local current_dummy,current_portNum,current_devNum
    local save_current_portNum
    local save_current_portNum_1,save_current_devNum_1

    local entryInfo = {}

    --print("1")

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()
    -- check that only single interface will be used
    for dummy, devNum, portNum in command_data:getPortIterator() do
        numOfIterations = numOfIterations + 1
        if numOfIterations > 1 then
            print("ERROR : The feature supports single port or trunk but not multi-ports")
            return nil
        end

        save_current_devNum_1 = devNum
        save_current_portNum_1 = portNum

        --print("devNum = " .. devNum)
        --print("portNum = " .. portNum)

        --print("3")

        --  get the first element in the table
        current_index , current_entry = next(table_bpe_802_1_br_system_info_devices,nil)
        while current_entry do

            --print("4")
            --print("current_index = " .. current_index)

            current_command_data = current_entry["command_data"]
            current_is_interface_match = false
            save_current_portNum = 0
            if current_command_data ~= nil then
                --print("5")
                -- Common variables initialization.
                --current_command_data:initInterfaceRangeIterator()
                --current_command_data:initInterfaceDeviceRange()
                for current_dummy, current_devNum, current_portNum in current_command_data:getPortIterator() do
                    --print("6")
                    save_current_portNum = current_portNum

                    --print ("current_devNum = " .. current_devNum)
                    --print ("current_portNum = " .. current_portNum)
                    if current_devNum == devNum and current_portNum == portNum then
                        numOfmatch = numOfmatch + 1
                        current_is_interface_match = true

                        --print("match found")

                        break
                    end
                end
            end

            --print("7")
            --print("current_index = " .. current_index)
            --print("neighbor_name = " .. neighbor_name)

            -- check element is under 'local physical port'
            if current_is_interface_match == true then

                --print("9")

                if remove_neighbor == true then

                    --print("10")
                    if current_index == neighbor_name then
                        --print("11")

                        if current_entry["connection_type"] == "direct" or
                           (onPe_control_bridge_name ~= nil and onPe_control_bridge_name == neighbor_name)
                        then
                            -- remove configurations
                            if my_role_is_control_bridge == true then
                                func_onCb_phyPortConnectedToPe(command_data,false,devNum,portNum)
                            else
                                if onPe_control_bridge_name ~= nil and onPe_control_bridge_name == neighbor_name then
                                    -- remove the CB
                                    func_onPe_deviceConnectedToCb(command_data,false)
                                else
                                    -- remove 'direct' PE
                                    func_onPe_phyPortConnectedToPe(command_data,false,devNum,portNum)
                                end

                            end

                        end

                        -- remove the entry from the DB
                        table_bpe_802_1_br_system_info_devices[current_index] = nil
                        -- decrement number of neighbors
                        num_neighbors = num_neighbors - 1

                        -- get next entry
                        local next_index , dummy_entry = next(table_bpe_802_1_br_system_info_devices,current_index)
                        -- we remove last PE
                        if next_index == nil and current_index == 1 then
                            if my_role_is_control_bridge == true then
                                func_onCb_deviceConnectedToPe(command_data,false,devNum)
                            end
                        end

                        -- on PE remove the name of the CB
                        -- onPe_control_bridge_name ~= nil only on PE device
                        if onPe_control_bridge_name == neighbor_name  then

                            onPe_control_bridge_name = nil
                            onPe_upstream_device = nil
                            onPe_upstream_port = nil

                        end

                        print ("SUCCESS")
                        return nil
                    end

                    -- check if not our element
                elseif current_index ~= neighbor_name then

                    --print("12")
                    -- check that the port not have 2 requests for 'directly' connected
                    if current_entry["connection_type"] == "direct" and connection_type == "direct" then
                        print("ERROR : The port already hold 'directly' connected device to it named: " .. current_index)
                        return nil
                    end

                    if current_entry["etag_ether_type"] ~= etherType and current_entry["etag_ether_type"] <= max_ether_type then

                        -- the ether must be the same for all devices (PEs) under this physical port
                        print("ERROR : The etag ether type must be like all other previous PEs under this interface : " ..
                                current_entry["etag_ether_type_Hex_string"])
                        return nil

                    end

                end

                -- check that the device not connected via other port
            elseif current_index == neighbor_name then

                --print("13")
                if remove_neighbor == true then
                    print("ERROR : The neighbor : " .. neighbor_name ..      " is connected via other port: " .. save_current_portNum)
                else
                    print("ERROR : The neighbor : " .. neighbor_name .. " already connected via other port: " .. save_current_portNum)
                end

                return nil
            end

            --print("14")
            --  get the next element in the table
            current_index , current_entry = next(table_bpe_802_1_br_system_info_devices,current_index)
        end -- while ...

    end -- getPortIterator(...)

    --print("15")

    if remove_neighbor == true then
        print("WARNING : the neighbor : ".. neighbor_name ..  " that needed to be removed was not found !")
        return nil
    end

    -- on PE save the name of the CB
    if my_role_is_control_bridge == false   and
            role == "control_bridge" and
            onPe_control_bridge_name == nil  then

        if num_neighbors > 0 then
            print("ERROR : before adding a new CB , must remove all downstream PEs too")
            return nil
        end

        onPe_control_bridge_name = neighbor_name
        onPe_upstream_device = save_current_devNum_1
        onPe_upstream_port = save_current_portNum_1

    end

    -- do not allow define 'upstream' PE devices , because we not use it . (the port connected to the 'CB' direction is 'upstream port')
    if my_role_is_control_bridge == false   and
            role == "port_extender" and
            save_current_portNum_1 ~= nil and
            onPe_upstream_port ~= nil and
            save_current_portNum_1 == onPe_upstream_port
    then
        print("WARNING : No need to define upstream PE neighbor : ".. neighbor_name)
        return nil
    end


    if is_sip_5_15(save_current_devNum_1) and 
       tag_type == "6-bytes-tag" then

        -- the 6-bytes-tag system need to explicit state the role type to the device.
        -- on 8-bytes system it is not needed
       func_onCbAndOnPe_bpe_802_1_br_6_bytes_SetMyRole(command_data,save_current_devNum_1 , my_role_is_control_bridge)
       
    end

    if num_neighbors == 0 then -- we add first neighbor
   
        if my_role_is_control_bridge == true then
            -- we add first PE
            func_onCb_deviceConnectedToPe(command_data,true,save_current_devNum_1)
        else
            -- we add the one and only CB
            func_onPe_deviceConnectedToCb(command_data,true)
        end

    end

    -- we add 'direct' neighbor
    if table_bpe_802_1_br_system_info_devices[neighbor_name] == nil and connection_type == "direct" then
        if my_role_is_control_bridge == true then
            -- add the port configurations
            func_onCb_phyPortConnectedToPe(command_data,true,save_current_devNum_1,save_current_portNum_1)
        else
            if role == "port_extender" then
                -- add the port configurations
                func_onPe_phyPortConnectedToPe(command_data,true,save_current_devNum_1,save_current_portNum_1)
            else
                -- already added the 'CB'
            end

        end

    end

    --print("16")

    entryInfo["role"] = role
    entryInfo["command_data"] = command_data
    entryInfo["connection_type"] = connection_type
    entryInfo["etag_ether_type"] = etherType
    entryInfo["neighbor_name"] = neighbor_name
    entryInfo["etag_ether_type_Hex_string"] = string.format("0x%x",etherType)
    entryInfo["tag_type"] = tag_type

    if not table_bpe_802_1_br_system_info_devices[neighbor_name] then
        num_neighbors = num_neighbors + 1
    end

    -- save the info into DB
    table_bpe_802_1_br_system_info_devices[neighbor_name] = entryInfo

    print ("SUCCESS")

end

local help_role_name = "Unique string that identify the device (in eDsa stacking identify the 'stack') For example \"PE1\" , \"port extender 33\" , \"CB\" , \"control bridge 7\""
local generic_help_bpe_802_1_br = "BPE system (bridge port externder) according to IEEE-802.1BR standard"
CLI_addHelp("interface", "bpe-802-1-br" , generic_help_bpe_802_1_br)

-- the command looks like :
--Console(config)# interface ethernet 0/1
--Console(config-if)# bpe-802-1-br neighbor-info role <PE> name <"PE1"> my-connection <direct / indirect> [tag-type  <8-bytes-tag / 6-bytes-tag >]
CLI_addCommand("interface", "bpe-802-1-br neighbor-info", {
    func   = bpe_802_1_br_neighbor_port_neighbor_info,
    help   = "Info about the neighbor to 'my device' in BPE system",
    params = {
        { type = "named",
            { format = "role %bpe_802_1_br___role",                         name = "role",              help = "The role of the neighbor (CB - Control Bridge , PE - port extender)" },
            { format = "name %neighbor_name",                               name = "neighbor_name",     help = help_role_name},
            { format = "my-connection %bpe_802_1_br___connection_type",     name = "connection_type",   help = "My connection to this neighbor (direct/indirect)"},
            { format= "tag-type %6_or_8_bytes_tag", name = "tag_type",    help = "(optional) the tag-type of the connection"},--optional
            mandatory = { "role", "neighbor_name", "connection_type"}
        }
    }
})

-- the command looks like :
--Console(config)# interface ethernet 0/1
--Console(config-if)# no bpe-802-1-br neighbor-info name <"PE1">
CLI_addCommand("interface", "no bpe-802-1-br neighbor-info", {
    func=function(params)
        params.flagNo=true
        return bpe_802_1_br_neighbor_port_neighbor_info(params)
    end,
    help   = "remove a neighbor of 'my device' from the BPE system",
    params = {
        { type = "named",
            { format = "name %neighbor_name",                               name = "neighbor_name",     help = help_role_name},
            mandatory = { "neighbor_name"}
        }
    }
})

--print all 'my' neighbors
function bpe_802_1_br_neighbor_port_neighbor_info_show(params)
    --  get the first element in the table
    local current_index , current_entry
    local current_command_data
    local iterator = 1

    current_index , current_entry = next(table_bpe_802_1_br_system_info_devices,nil)

    print("index# ".. " | " .. "neighbor_name " .. " | " .. "role ".. " | " .."connection_type " .. " | " .. "etag_ether_type")
    print("------------------------------------------------------------------------------")

    while current_entry do

        current_command_data = current_entry["command_data"]

        if current_command_data ~= nil then
            local tag_type_string
            if(current_entry["tag_type"] == "6-bytes-tag")then
                tag_type_string = "(".."6-bytes-tag"..")"
            else
                tag_type_string = "" -- no indication
            end
        
            print("    " .. iterator  .. " | " ..
                  current_entry["neighbor_name"]  .. " | " ..
                  current_entry["role"] .. " | " ..
                  current_entry["connection_type"]  .. " | " ..
                  current_entry["etag_ether_type_Hex_string"] .. tag_type_string)

            for current_dummy, current_devNum, current_portNum in current_command_data:getPortIterator() do
                print ("        devNum = " .. current_devNum .. " PortNum = " .. current_portNum)
            end
            -- empty line
            print ("")
            iterator = iterator + 1
        end

        current_index , current_entry = next(table_bpe_802_1_br_system_info_devices,current_index)

    end

    print ("End of " .. (iterator - 1) .. " neighbors")

end


-- the command looks like :
--Console# interface ethernet 0/1
--Console# show bpe-802-1-br neighbor-info
CLI_addCommand("exec", "show bpe-802-1-br neighbor-info", {
    func   = bpe_802_1_br_neighbor_port_neighbor_info_show,
    help   = "Show Info about all neighbors of 'my device' in BPE system",
    params = {}
})

-- get the PE name and return 2 parameters of {dev,physical port} that the PE is bound on
local function get_physical_info_of_pe(pe_name)
    local current_entry = table_bpe_802_1_br_system_info_devices[pe_name]
    
    if current_entry == nil then
        return nil, nil
    end

    local current_command_data = current_entry["command_data"]

    if current_command_data == nil then
        return nil, nil
    end

    -- iterator but only single interface
    for current_dummy, current_devNum, current_portNum in current_command_data:getPortIterator() do
        return current_devNum, current_portNum
    end
end

-- get into 'eport' interface in order to unique the pcid on pe and generate attributes
-- eport that represents the remote 'Extended Port'
-- implementation on CB
--[[
This command will do:
implicit :
(tti)
a. <Number of tag words to pop>=2 (since vm use 'BPE' and not 'EVB')
b. enable TTI ETH lookup on physical port
c. set TTI rule :
1. <Physical port>
2. tag1_exists = 1 --> the recognition of 'E-Tag'
3. two UDBs of E-TAG from byte 4 of the 8 bytes :
<E-CID_base>  - (12 bits) 'PCID' of the 'source vm'
<GRP>             - (2 bits)  must be 0 because src is not multicast.
<Re->              - (2 bits)  must be 0 reserved
d. set TTI action :
1. assign src eport = vm_unique_id
2. evlan command = 'modify untagged' (use the below "vlan_id for untagged")

(L2i)
e. controlled learn + NA to cpu

(EQ)
e. e2phy - bind trgEport vm_unique_id with it's <Physical port>
(HA)
f. <Push Tag Enable> = 1
g. <Push Tag VLAN-ID value> = the PCID of the eport ('source vm')

set for the 'physical port' !!! :
(HA)
a. <Push Tag Enable> = 1
b. <Push Tag VLAN-ID value> = 0xfff -->  Don't care --> This is to enable MC

explicit:
(tti)
b. vlan_id for untagged (refer to tag after the E-Tag that is poped)
c. (optional - default 'any') tti_action_index = 'any' or '0..33k'
--]]
-- implementation on CB
local function local_onCb_bpe_802_1_br_pcid_on_pe_eport_info(params,command_data)
    local pe_physical_devNum , pe_physical_portNum , pe_physical_hwDevNum
    local apiName,numberOfTagWords,result,enableLookup
    local result
    local PCID_of_eport = params.pcid
    local saveMyEport
    local addPort
    local db_vertex, pe_name;
    local dummy, devNum, portNum, numOfPorts;
    local dummy_portNum

    local enable, learn_status, vlanCmd

    local learn_cmd
    
    -- handle default values
    if params.pvid0 == nil          then params.pvid0 = 1 end
    if params.tti_rule_index == nil then params.tti_rule_index = "any" end

    numOfPorts = 0;
    for dummy, devNum, portNum in command_data:getPortIterator(true) do
        numOfPorts = numOfPorts + 1;
    end

    if (numOfPorts ~= 1) then
        print("Command supported only fo one ePort Context");
        return
    end

    if params.flagNo == true then -- 'unset'
        -- loop over the eports with generic iterator
        for dummy, devNum, portNum in command_data:getPortIterator(true) do
            db_vertex = bpe_802_1_br_pcid_on_pe_eport_info_get_db(devNum, portNum);
            if (db_vertex == nil) then
                -- Not configured ePort
                print("Configuration for ePort not found in DB"
                    .. " dev = " .. tostring(devNum)
                    .. " ePort " .. tostring(portNum));
                return;
            end
            pe_name = db_vertex["pe_name"];
            if (pe_name == nil) then
                print("PE nane for ePort not found in DB");
                return;
            end
            break;
        end;
    else
        pe_name = params.pe_name;
    end

    -- check that the pe_name exists on a physical port
    pe_physical_devNum , pe_physical_portNum = get_physical_info_of_pe(pe_name)

    if params.flagNo == true then -- 'unset'
        addPort = false
    else
        addPort = true
    end

    if pe_physical_portNum == nil or pe_physical_devNum == nil then

        print("ERROR : the PE " .. params.pe_name .. "was not defined")
        return nil
    end

    pe_physical_hwDevNum , dummy_portNum =
        Command_Data:getHWDevicePort(pe_physical_devNum,pe_physical_portNum)

    -- configure the eport with generic iterator
    for dummy, devNum, portNum in command_data:getPortIterator() do

        saveMyEport = portNum
        learn_cmd = "CPSS_LOCK_FRWRD_E"

        if params.flagNo == true then -- 'unset'
            numberOfTagWords = 0
            enable = GT_FALSE
            learn_status = GT_TRUE
            vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E"

        else -- 'set'
            -- the BPE need to remove 8 bytes ETag (2 time 4 bytes)
            numberOfTagWords = long_tag
            enable = GT_TRUE
            learn_status = GT_FALSE
            vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E"
        end

        _debug("enable =" .. tostring(enable) .. " learn_status = " .. tostring(learn_status) .. " vlanCmd = " .. vlanCmd)
        --***************
        --***************
        apiName = "cpssDxChBrgFdbNaToCpuPerPortSet"
        result = cpssPerPortParamSet(
            apiName,
            devNum, portNum, enable, "enable",
            "GT_BOOL")
        
        

        --***************
        --***************
        apiName = "cpssDxChBrgFdbPortLearnStatusSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "IN", "GT_BOOL", "status", learn_status},
            { "IN", "CPSS_PORT_LOCK_CMD_ENT", "cmd", learn_cmd }
        })
        
        

        --***************
        --***************
        apiName = "cpssDxChBrgMcPhysicalSourceFilteringEnableSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_PORT_NUM", "portNum", portNum},
            { "IN", "GT_BOOL", "enable", GT_FALSE }
        })
        
        

        --***************
        --***************
        result = numOfTagBytesToPopSet(
            devNum, portNum, numberOfTagWords)
        
        --***************
        --***************
        if params.flagNo == true then
            e2phySet(command_data,devNum,portNum,pe_physical_portNum,true)
        else
            e2phySet(command_data,devNum,portNum,pe_physical_portNum)
        end

        -- state the port to have ETag
        pushETagCommandOnEPort(command_data,devNum,portNum,vlanCmd,PCID_of_eport)

        --***************
        --***************
        setPortToNotRecognizeTag1AfterEtagPopped(command_data,devNum,portNum,false,addPort)
        --***************
        --***************

    end

    devNum = pe_physical_devNum

    -- set the TTI rule + action
    local ttiIndex
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local ttiAction = {}

    local myRuleSize = 1

    if params.tti_rule_index == "any" then
        local myIndexName = "BPE: <E-CID_base> = <pcid> , device " .. devNum .. " and eport ".. saveMyEport
        -- if myIndexName not exists in DB it will give 'new index' and saved into DB
        -- if myIndexName exists in DB it will retrieved from DB
        ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti0")
    else
        local ttiErrorCode,ttiIndex = getTtiIndex(params.tti_rule_index,saveMyEport,myRuleSize)
        _debug("ttiIndex = " .. tostring(ttiIndex) .. "   ttiErrorCode = " .. tostring(ttiErrorCode))

        if ttiErrorCode ~= tti_index_error_type_ok or ttiIndex == nil then
            command_data:setFailStatus()
            if ttiErrorCode == tti_index_error_type_error_over_max then
                command_data:addError("tti index over max value: %d", _30k-1)
            elseif ttiErrorCode == tti_index_error_type_error_not_numeric then
                command_data:addError("tti index is not 'any' and is not numeric value: %s", params.tti_rule_index)
            else
                command_data:addError("bad format or value for tti index")
            end

            return nil
        end
    end

    -- tag0 can keep it's tagging from before the 'pop'
    ttiAction.tag0VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    ttiAction.tag0VlanId = params.pvid0
    ttiAction.tag1VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    ttiAction.tag1UpCommand = "CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E"

    -- need to reassign eport
    ttiAction.sourceEPortAssignmentEnable = GT_TRUE
    ttiAction.sourceEPort = saveMyEport

    finalTtiAction.type2 = ttiAction

    finalTtiPattern.udbArray , finalTtiMask.udbArray =
        bpe_802_1_br_buildTtiPatternMask(pe_physical_portNum,--physicalPort
                    true,--ETag_exists
                    nil,-- grp
                    params.pcid,-- ecid_base
                    nil) -- ingress_ecid_base

    if addPort == false then
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

    --success
    return true;

end

-- get into 'eport' interface in order to unique the pcid on pe and generate attributes
-- eport that represents the remote 'Extended Port'
-- implementation on PE
--[[
Implementation:

(the physical port is cascade port , so already got cascade port upstream setting and was added to reserved_vidx_for_cascade_ports)



my_pcid_eport = eport

For downstream

Lookup 0:

tti rule :
physical_port_upstream
E-TAG<Ingress_E-CID> = < pcid >  use offset in the packet with UDBs
tti action:
reassign srcEport = 1
srcEport = my_pcid_eport
enable lookup 1


lookup 1:

tti rule :
вЂ�physical_port_upstreamвЂ™
E-TAG<GRP, E-CID> use offset in the packet with UDBs
    i.      GRP  = 0                meaning known UC
    ii.      E-CID = <pcid>

tti action:
Redirect to target eport = my_pcid_eport
Bypass ingress  = 1
Bypass bridge = 1

Also:
(TTI)
<Number of tag words to pop>=2
(EQ)
e2phy - bind my_pcid_eport with it's <Physical port>
(HA)
<Push Tag Enable> = 1
 <Push Tag VLAN-ID value> = the pcid of the eport

--]]
-- implementation on PE
local function local_onPe_bpe_802_1_br_pcid_on_pe_eport_info(params,command_data)
    local pe_physical_devNum , pe_physical_portNum , pe_physical_hwDevNum
    local apiName,numberOfTagWords,result,enableLookup
    local result
    local PCID_of_eport = params.pcid
    local currentEport
    local addPort
    local dummy, devNum, portNum, numOfPorts;
    local db_vertex
    local dummy_portNum, pe_name

    local vlanCmd
    
    -- handle default values
    if params.pvid0 == nil          then params.pvid0 = 1 end
    if params.tti_rule_index == nil then params.tti_rule_index = "any" end

    if params.tti_rule_index ~= "any" then
        print("ERROR: due to multi tti-indexes (for lookup 0 and 1), you must state 'tti-rule-index any' and not specific index")
        return nil
    end

    numOfPorts = 0;
    for dummy, devNum, portNum in command_data:getPortIterator(true) do
        numOfPorts = numOfPorts + 1;
    end

    if (numOfPorts ~= 1) then
        print("Command supported only fo one ePort Context");
        return
    end

    if params.flagNo == true then -- 'unset'
        -- loop over the eports with generic iterator
        for dummy, devNum, portNum in command_data:getPortIterator(true) do
            db_vertex = bpe_802_1_br_pcid_on_pe_eport_info_get_db(devNum, portNum);
            if (db_vertex == nil) then
                -- Not configured ePort
                print("Configuration for ePort not found in DB"
                    .. " dev = " .. tostring(devNum)
                    .. " ePort " .. tostring(portNum));
                return;
            end
            pe_name = db_vertex["pe_name"];
            if (pe_name == nil) then
                print("PE name for ePort not found in DB");
                return;
            end
            break;
        end;
    else
        pe_name = params.pe_name;
    end

    -- check that the pe_name exists on a physical port
    pe_physical_devNum , pe_physical_portNum = get_physical_info_of_pe(pe_name);

    if params.flagNo == true then -- 'unset'
        addPort = false
    else
        addPort = true
    end

    if pe_physical_portNum == nil or pe_physical_devNum == nil then

        print("ERROR : the PE " .. params.pe_name .. "was not defined")
        return nil
    end

    pe_physical_hwDevNum , dummy_portNum =
        Command_Data:getHWDevicePort(pe_physical_devNum,pe_physical_portNum)

    -- configure the eport with generic iterator
    for dummy, devNum, portNum in command_data:getPortIterator() do

        currentEport = portNum

        if params.flagNo == true then -- 'unset'
            numberOfTagWords = 0
            vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E"

        else -- 'set'
            numberOfTagWords = long_tag
            vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E"
        end

        --***************
        --***************
        result = numOfTagBytesToPopSet(
            devNum, portNum, numberOfTagWords)
        
        --***************
        --***************
        -- set the e2phy
        if params.flagNo == true then
            e2phySet(command_data,devNum,portNum,pe_physical_portNum,true)
        else
            e2phySet(command_data,devNum,portNum,pe_physical_portNum)
        end

        --***************
        --***************
        -- state the port to have ETag
        pushETagCommandOnEPort(command_data,devNum,portNum,vlanCmd,PCID_of_eport)

        --***************
        --***************
        setPortToNotRecognizeTag1AfterEtagPopped(command_data,devNum,portNum,false,addPort)
    end

    local devNum,portNum

    devNum  = pe_physical_devNum
    portNum = currentEport

    -- set the TTI rule + action
    local ttiIndex
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local ttiAction = {}
    local egressInterface = {}

    local myRuleSize = 1

    --***************
    --***************
    -- rule for 'downstream' Lookup 0:
    --

    local myIndexName = "BPE: downstream E-TAG<Ingress_E-CID> = < pcid > , assign srcEPort for source filtering , tti0, device " .. devNum .. " and eport ".. portNum
    if (on_Pe_need_reserved_trunkid_for_uplink_port == true) then
        myIndexName = myIndexName .. " use physical trunkId " .. onPe_reserved_trunkid_for_uplink_port
    end
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti0")

    ttiAction = {}
    -- tag0 can keep it's tagging from before the 'pop'
    ttiAction.tag0VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    ttiAction.tag1VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

    if isVTag6BytesConfig then
        -- we need to assign srcId for the packets that continue to next PE (Port Extender)
        -- need to set direction ('downstream') and filter bit('do filter') as this is multi-destination
        -- use as basic srcId = 0
        local srcId = 0
        
        ttiAction.sourceIdSetEnable = GT_TRUE
        ttiAction.sourceId = srcIdCalcForVTag6BytesGet(srcId , true--[[<isMcFilter> = true]])
    end
    
    -- need to reassign eport
    ttiAction.sourceEPortAssignmentEnable = GT_TRUE
    ttiAction.sourceEPort = currentEport
    -- enable lookup 1
    ttiAction.continueToNextTtiLookup = GT_TRUE

    finalTtiAction.type2 = ttiAction

    finalTtiPattern.udbArray , finalTtiMask.udbArray =
        bpe_802_1_br_buildTtiPatternMask(onPe_upstream_port,--physicalPort
            true,--ETag_exists
            nil,-- grp
            nil,-- ecid_base
            PCID_of_eport) -- ingress_ecid_base

    --print ("index " ..ttiIndex.. " pattern : " , to_string(finalTtiPattern.udbArray) , "\n mask:", to_string(finalTtiMask.udbArray))


    if addPort == false then
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

    
    

    --***************
    --***************
    -- rule for 'downstream' Lookup 1:
    --
    local myIndexName = "BPE: downstream E-TAG<Ingress_E-CID> = < pcid > , assign trgEPort , tti1, device " .. devNum .. " and eport ".. portNum
    if (on_Pe_need_reserved_trunkid_for_uplink_port == true) then
        myIndexName = myIndexName .. " use physical trunkId " .. onPe_reserved_trunkid_for_uplink_port
    end
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti1")

    ttiAction = {}

    -- tag0 can keep it's tagging from before the 'pop'
    ttiAction.tag0VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
    ttiAction.tag1VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

    ttiAction['bridgeBypass']           = GT_TRUE
    ttiAction['ingressPipeBypass']      = GT_TRUE

    ttiAction['redirectCommand']           = "CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E"

    egressInterface.type = "CPSS_INTERFACE_PORT_E"

    local physical_hwDevNum , dummy_portNum =
        Command_Data:getHWDevicePort(devNum,portNum)

    egressInterface.devPort = {}
    egressInterface.devPort.devNum = physical_hwDevNum
    egressInterface.devPort.portNum = currentEport

    ttiAction['egressInterface'] = egressInterface

    if isVTag6BytesConfig then
        -- we need to assign srcId for the packets that continue to next PE (Port Extender)
        -- need to set direction ('downstream')and filter bit('no filter') as this is not multi-destination
        -- use as basic srcId = 0
        local srcId = 0
        
        ttiAction.sourceIdSetEnable = GT_TRUE
        ttiAction.sourceId = srcIdCalcForVTag6BytesGet(srcId , false--[[<isMcFilter> = false]])
    end
    
    finalTtiAction.type2 = ttiAction


    finalTtiPattern.udbArray , finalTtiMask.udbArray =
        bpe_802_1_br_buildTtiPatternMask(onPe_upstream_port,--physicalPort
            true,--ETag_exists
            0,-- grp
            PCID_of_eport,-- ecid_base
            nil) -- ingress_ecid_base

    if addPort == false then
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

    --success
    return true;

end


function bpe_802_1_br_pcid_on_pe_eport_info(params)
    local command_data = Command_Data()
    local ret, my_role_is_control_bridge

    if bpe_802_1_br_my_info_DB["dev_info"]["role"] == "control_bridge" then
        my_role_is_control_bridge = true
    else
        my_role_is_control_bridge = false
    end


    -- prepare dev,port iterators
    command_data:initInterfaceDevPortRange()

    if my_role_is_control_bridge == true then
        ret = local_onCb_bpe_802_1_br_pcid_on_pe_eport_info(params,command_data)
    else
        ret = local_onPe_bpe_802_1_br_pcid_on_pe_eport_info(params,command_data)
    end

    if (ret == true) then
        -- DB should be updated after configuration, no-command uses old DB state
        bpe_802_1_br_pcid_eport_info_save_in_db(params);
    end

    -- handle all the errors that may come from calling the function
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end


CLI_type_dict["value_12_bits"] = {
    checker = CLI_check_param_number,
    min=0,
    max=_4k-1,
    complete = CLI_complete_param_number,
    help = "Value in range 0..(4k-1)"
}

CLI_type_dict["value_13_bits"] = {
    checker = CLI_check_param_number,
    min=0,
    max=_8k-1,
    complete = CLI_complete_param_number,
    help = "Value in range 0..(8k-1)"
}


CLI_addHelp({"interface", "interface_eport"}, "bpe-802-1-br" ,generic_help_bpe_802_1_br)

-- the command looks like :
--Console(config)# interface eport 0/4101
--Console(config-if-eport)# bpe-802-1-br pcid-on-pe <pe-name> <pcid> pvid0 <pvid0> tti_rule_index <any/index>
CLI_addCommand({"interface", "interface_eport"}, "bpe-802-1-br pcid-on-pe", {
    func   = bpe_802_1_br_pcid_on_pe_eport_info,
    help   = "Bind PCID of PE to remote 'Extended Port' (the eport) in BPE system",
    params = {
        { type = "named",
            { format = "pe-name %s", name = "pe_name", help = "The name of the PE , for example \"PE1\" or \"port extender 33\" "},
            { format = "pcid %value_12_bits",    name = "pcid",    help = "The PCID (<E-Cid_base> in the ETag) that represents the 'Extended Port'"},
            { format = "pvid0 %value_13_bits",   name = "pvid0",   help = "The 'default vid0' for untagged/priority tagged packets. (when tag 0 not recognized by tpid) Refers to tag0 after the E-Tag that is popped . range : 0..(8K-1) " ..
                                                             "optional parameter (default is '1') "  },
            { format = "tti-rule-index %s", name = "tti_rule_index", help = "Bind this PCID that represents the remote 'Extended port' on the 'PE' (recognized on the ETag as <E-Cid_base>)" ..
                                                            "optional parameter (default is 'any') "},
            mandatory = { "pe_name", "pcid"}
        }
    }
})

-- the command looks like :
--Console(config)# interface eport 0/4101
--Console(config-if-eport)#no bpe-802-1-br pcid-on-pe <pe-name> <pcid> pvid0 <pvid0> tti_rule_index <any/index>
CLI_addCommand({"interface", "interface_eport"}, "no bpe-802-1-br pcid-on-pe", {
    func   = function(params)
        params["flagNo"] = true;
        bpe_802_1_br_pcid_on_pe_eport_info(params);
    end,
    help   = "Bind PCID of PE to remote 'Extended Port' (the eport) in BPE system",
    params = {}
})

-- update 'srcId' entry to add/remove a port from entry
local function updateSrcIdEntry(command_data,devNum,srcId,portNum,addPort)
    local apiName
    
    if addPort == true then
        apiName = "cpssDxChBrgSrcIdGroupPortAdd"
    else
        apiName = "cpssDxChBrgSrcIdGroupPortDelete"
    end
    
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "sourceId", srcId},
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum}
    })
end

-- set tti part on 'PE' for 'pcid on me'
local function func_onPe_bpe_802_1_pcid_on_me_port_info_tti(params,command_data)
    local apiName,result
    local PCID_of_port = params.pcid
    local currentEport
    local addPort
    local enableLookup
    local myIndexName

    -- set the TTI rule + action
    local ttiIndex
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local ttiAction = {}
    local egressInterface = {}

    local myRuleSize = 1

    if params.flagNo == true then
        addPort = false
        enableLookup = GT_FALSE
    else
        addPort = true
        enableLookup = GT_TRUE
    end


    -- configure the port with generic iterator
    for dummy, devNum, portNum in command_data:getPortIterator() do

        --***************
        --***************
        -- enable lookup on the physical port
        tti_multiple_cpssDxChTtiPortLookupEnableSet(command_data,devNum,portNum,enableLookup)


        currentEport = onPe_getEportOf_reserved_eport_of_upstream_ingress_with_no_etag(portNum)

        --***************
        --***************
        -- rule for 'upstream' :
        --
        -- we need trg eport with 'push tag'
        ttiAction = {}
        ttiAction['nestedVlanEnable']       = GT_TRUE
        ttiAction['bridgeBypass']           = GT_TRUE
        ttiAction['ingressPipeBypass']      = GT_TRUE

        ttiAction['redirectCommand']           = "CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E"

        egressInterface.type = "CPSS_INTERFACE_PORT_E"

        local physical_hwDevNum , dummy_portNum =
            Command_Data:getHWDevicePort(devNum,currentEport)

        egressInterface.devPort = {}
        egressInterface.devPort.devNum = physical_hwDevNum
        egressInterface.devPort.portNum = currentEport

        ttiAction['egressInterface'] = egressInterface

        ttiAction['tag0VlanCmd']           = "CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E"
        ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

        finalTtiAction.type2 = ttiAction

        finalTtiPattern.udbArray , finalTtiMask.udbArray =
            bpe_802_1_br_buildTtiPatternMask(portNum,--physicalPort
                false,-- ETag_exists
                nil,-- grp
                nil,-- ecid_base
                nil) -- ingress_ecid_base

        myIndexName = "BPE: PVE to upstream port from local Extended port (WITHOUT ETag) , device " .. devNum .. " and port ".. portNum
        -- if myIndexName not exists in DB it will give 'new index' and saved into DB
        -- if myIndexName exists in DB it will retrieved from DB
        ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti0")

        if addPort == false then
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

        --***************
        --***************
        -- need to reassign eport , because:
        -- 1. 'pop 2 tags' (the ETag)
        -- 2. src filterring in the L2MLL
        -- 3. this eport must not recognize 'tag1'
        local eportForDownstream
        if true == onPe_isExistsEportOfPcidOnCscdPort(portNum) then
            -- note this 'eport' will not filter any member of the L2MLL , as the mll not hold it.
            -- the L2MLL holds vidx which has the 'cascade physical port' in it.
            -- but we need the 'eport reassignment' , so the new srcEport will have 'pop 2 tags' (the ETag)
            eportForDownstream = onPe_getEportOfPcidOnCscdPort(portNum)
        else
            eportForDownstream = onPe_getEportOfPcidOnNonCascadePort(portNum)
        end

        setPortToNotRecognizeTag1AfterEtagPopped(command_data,devNum,eportForDownstream,false,addPort)

        --***************
        --***************
        -- rule for 'downstream' Lookup 0:
        --

        myIndexName = "BPE: downstream E-TAG<Ingress_E-CID> = < pcid > , assign srcEPort for source filtering  , tti0, device " .. devNum .. " and port ".. portNum
        if (on_Pe_need_reserved_trunkid_for_uplink_port == true) then
            myIndexName = myIndexName .. " use physical trunkId " .. onPe_reserved_trunkid_for_uplink_port
        end
        -- if myIndexName not exists in DB it will give 'new index' and saved into DB
        -- if myIndexName exists in DB it will retrieved from DB
        ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti0")

        ttiAction = {}
        -- tag0 can keep it's tagging from before the 'pop'
        ttiAction.tag0VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
        ttiAction.tag1VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

        -- assign srcId for the src filtering that needed for 'multi-destination'
        -- this is much more scalable mechanism then the L2MLL that described in the FS.
        ttiAction.sourceIdSetEnable = GT_TRUE

        -- convert physical port number to it's srcId --> 
        -- used for src port filtering on downstream
        local srcId = srcIdFromPhysicalPortNum(portNum)
        
        if isVTag6BytesConfig then
            -- the filter need to be for '<direction> = direction_downstream' and '<isMcFilter> = true' indications
            ttiAction.sourceId = srcIdCalcForVTag6BytesGet(srcId , true--[[<isMcFilter> = true]])
        else
            ttiAction.sourceId = srcId 
        end

        -- need to reassign eport
        ttiAction.sourceEPortAssignmentEnable = GT_TRUE
        ttiAction.sourceEPort = eportForDownstream
        -- enable lookup 1
        ttiAction.continueToNextTtiLookup = GT_TRUE


        finalTtiAction.type2 = ttiAction

        finalTtiPattern.udbArray , finalTtiMask.udbArray =
            bpe_802_1_br_buildTtiPatternMask(onPe_upstream_port,--physicalPort
                true,--ETag_exists
                nil,-- grp
                nil,-- ecid_base
                PCID_of_port) -- ingress_ecid_base

        if addPort == false then
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

        
        

        --***************
        --***************
        -- rule for 'downstream' Lookup 1:
        --
        myIndexName = "BPE: downstream E-TAG<Ingress_E-CID> = < pcid > , assign trgEPort , tti1, device " .. devNum .. " and port ".. portNum
        if (on_Pe_need_reserved_trunkid_for_uplink_port == true) then
            myIndexName = myIndexName .. " use physical trunkId " .. onPe_reserved_trunkid_for_uplink_port
        end
        -- if myIndexName not exists in DB it will give 'new index' and saved into DB
        -- if myIndexName exists in DB it will retrieved from DB
        ttiIndex = allocTtiIndexFromPool(myRuleSize,myIndexName,"tti1")

        ttiAction = {}

        -- tag0 keep it's tagging from before the 'pop'
        ttiAction.tag0VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
        ttiAction.tag1VlanCmd = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"

        ttiAction['bridgeBypass']           = GT_TRUE
        ttiAction['ingressPipeBypass']      = GT_TRUE

        ttiAction['redirectCommand']           = "CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E"

        egressInterface.type = "CPSS_INTERFACE_PORT_E"

        physical_hwDevNum , dummy_portNum =
            Command_Data:getHWDevicePort(devNum,portNum)

        local trgCscdEPort

        if true == onPe_isExistsEportOfPcidOnCscdPort(portNum) then
            trgCscdEPort = onPe_getEportOfPcidOnCscdPort(portNum)
        else
            trgCscdEPort = portNum --dummy not really used
        end

        egressInterface.devPort = {}
        egressInterface.devPort.devNum = physical_hwDevNum
        egressInterface.devPort.portNum = trgCscdEPort

        ttiAction['egressInterface'] = egressInterface

        finalTtiAction.type2 = ttiAction


        finalTtiPattern.udbArray , finalTtiMask.udbArray =
            bpe_802_1_br_buildTtiPatternMask(onPe_upstream_port,--physicalPort
                true,--ETag_exists
                0,-- grp
                PCID_of_port,-- ecid_base
                nil) -- ingress_ecid_base

        if addPort == false then
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

end

-- check if the port was defined as CSCD port to neighbor device (downstream)
local function func_onPe_non_eArch_bpe_802_1_is_cscd_port(devNum,portNum)
    --  get the first element in the table
    local current_index , current_entry = next(table_bpe_802_1_br_system_info_devices,nil)
    while current_entry do
        local current_command_data = current_entry["command_data"]
        if current_command_data ~= nil then
            for current_dummy, current_devNum, current_portNum in current_command_data:getPortIterator() do
                if devNum == current_devNum and portNum == current_portNum then
                    -- found the port in the DB
                    return true
                end
            end
        end
        
        current_index , current_entry = next(table_bpe_802_1_br_system_info_devices,current_index)
    end
    
    return false
end
       
-- function on PE , non eArch device
-- set tti related config
local function func_onPe_non_eArch_bpe_802_1_pcid_on_me_port_info_tti(params,command_data)
    local apiName,result
    local PCID_of_port = params.pcid
    local addPort
    local enableLookup
    local myIndexName

    -- set the TTI rule + action
    local ttiIndex
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local ttiAction = {}
    local egressInterface = {}

    if params.flagNo == true then
        addPort = false
        enableLookup = false
    else
        addPort = true
        enableLookup = true
    end

    -- configure the port with generic iterator
    for dummy, devNum, portNum in command_data:getPortIterator() do
        --***************
        --***************
        local physical_hwDevNum , dummy_portNum =
            Command_Data:getHWDevicePort(devNum,portNum)
       
        local is_cscd_port = func_onPe_non_eArch_bpe_802_1_is_cscd_port(devNum,portNum)
       
        if false == is_cscd_port then
            -- enable lookup on the physical port
            tti_multiple_cpssDxChTtiPortLookupEnableSet(command_data,devNum,portNum,enableLookup)
            
            --[[
                If the host behind an Extended Port is untrusted, add a rule to drop E-Tagged packets prior to the
                rule listed above:
                    Key: <Tag1 Exist>=Exists
                    Action: <Packet Command>= DROP
            --]]
            finalTtiAction = {}
            finalTtiPattern = {}
            finalTtiMask = {}
            ttiAction = {}
            egressInterface = {}

            ttiAction['command']            = "CPSS_PACKET_CMD_DROP_HARD_E"
            ttiAction['tag1VlanCmd']        = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
            
            finalTtiAction.type2 = ttiAction

            finalTtiPattern.eth , finalTtiMask.eth =
                bpe_802_1_br_non_eArch_buildTtiPatternMask(portNum,--physicalPort
                    true,-- ETag_exists
                    nil,-- grp
                    nil,-- ecid_base
                    false) -- isTrunk
            
            myIndexName = "BPE: untrusted Extended Port DROP packets ingress with ETag , device " .. devNum .. " and port ".. portNum
            -- if myIndexName not exists in DB it will give 'new index' and saved into DB
            -- if myIndexName exists in DB it will retrieved from DB
            func_onPe_non_eArch_bpe_802_1_tti_Rule_update(command_data,myIndexName,addPort,devNum,finalTtiPattern,finalTtiMask,finalTtiAction)
            
        end

        --
        -- set the upstream from the port represent the pcid to the 'upstream port'
        --
        finalTtiAction = {}
        finalTtiPattern = {}
        finalTtiMask = {}
        ttiAction = {}
        egressInterface = {}
        
        -- we need trg port
        ttiAction['bridgeBypass']           = true
        ttiAction['ingressPipeBypass']      = true

        ttiAction['redirectCommand']           = "CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E"

        egressInterface.type = "CPSS_INTERFACE_PORT_E"

        egressInterface.devPort = {}
        egressInterface.devPort.devNum = physical_hwDevNum
        egressInterface.devPort.portNum = onPe_upstream_port

        ttiAction['egressInterface'] = egressInterface
        
        --[[
            <Tag1 VLAN Command>= "Assign <Tag1 VLAN-ID> if the packet does not have Tag1"
            <Tag1 VLAN-ID>= Port PCID
        --]]
        ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
        ttiAction['tag1VlanId']            = PCID_of_port

        finalTtiAction.type2 = ttiAction
        
        finalTtiPattern.eth , finalTtiMask.eth =
            bpe_802_1_br_non_eArch_buildTtiPatternMask(portNum,--physicalPort
                nil,-- ETag_exists
                nil,-- grp
                nil,-- ecid_base
                false) -- isTrunk

        myIndexName = "BPE: Redirect to upstream port from local Extended port (WITH/WITHOUT ETag) , device " .. devNum .. " and port ".. portNum
        -- if myIndexName not exists in DB it will give 'new index' and saved into DB
        -- if myIndexName exists in DB it will retrieved from DB
        func_onPe_non_eArch_bpe_802_1_tti_Rule_update(command_data,myIndexName,addPort,devNum,finalTtiPattern,finalTtiMask,finalTtiAction)

        --
        -- set the downstream UC to the port represent the pcid
        --
        finalTtiAction = {}
        finalTtiPattern = {}
        finalTtiMask = {}
        ttiAction = {}
        egressInterface = {}

        -- we need trg port
        ttiAction['bridgeBypass']           = true
        ttiAction['ingressPipeBypass']      = true

        ttiAction['redirectCommand']           = "CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E"

        egressInterface.type = "CPSS_INTERFACE_PORT_E"
        egressInterface.devPort = {}
        egressInterface.devPort.devNum = physical_hwDevNum
        egressInterface.devPort.portNum = portNum

        ttiAction['egressInterface'] = egressInterface
        
        local extra_string_info
        
        if false == is_cscd_port or onPe_non_eArch_removed_cpssDxChBrgPePortPcidSet == true then
            --The FS or the FS erratum will specify that for downstream UC traffic 
            --to extended port or to the PCID of cascade port, the TTI rule should set vid1=0.
            ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E"
            ttiAction['tag1VlanId']           = 0
            
            extra_string_info = " (must set vid1=0)"
        else
            -- assume the cascade port that hold the PCID never need to be filtered from the MC group.
            -- so the UC erratum will not damage it
            ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
            extra_string_info = ""
        end
        
        finalTtiAction.type2 = ttiAction

        finalTtiPattern.eth , finalTtiMask.eth =
            bpe_802_1_br_non_eArch_buildTtiPatternMask(onPe_upstream_port,--physicalPort
                true,-- ETag_exists
                0,-- grp
                PCID_of_port,-- ecid_base
                false) -- isTrunk

        myIndexName = "BPE: downstream Unicast Forwarding E-TAG<ecid_base> = < pcid > ," .. extra_string_info .. " assign trgEPort , device " .. devNum .. " and port ".. portNum
        -- if myIndexName not exists in DB it will give 'new index' and saved into DB
        -- if myIndexName exists in DB it will retrieved from DB
        func_onPe_non_eArch_bpe_802_1_tti_Rule_update(command_data,myIndexName,addPort,devNum,finalTtiPattern,finalTtiMask,finalTtiAction)
        
    end
end

-- function on PE , non eArch device
-- get into 'local physical port' interface in order to unique the pcid (on 'me')  and generate attributes
-- local port is the 'Extended Port'
local function func_onPe_non_eArch_bpe_802_1_pcid_on_me_port_info(params,command_data)
    local PCID_of_port
    local mcFilterEnable
    local phy_port_info
    local orig_pcid
    local apiName
    
    if params.flagNo == true then -- 'unset'
        PCID_of_port = 0
        mcFilterEnable = false
    else
        PCID_of_port = params.pcid
        mcFilterEnable = true
    end

    for dummy, devNum, portNum in command_data:getPortIterator() do

        if params.flagNo == true then
            phy_port_info = bpe_802_1_br_pcid_on_pe_phy_port_info_get_db(devNum,portNum)
            if phy_port_info == nil then
                print("ERROR : current port was not set with PCID")
                return
            end
            
            orig_pcid = phy_port_info.pcid
        else
            orig_pcid = PCID_of_port
        end
        
        local is_cscd_port = func_onPe_non_eArch_bpe_802_1_is_cscd_port(devNum,portNum)

        if onPe_non_eArch_removed_cpssDxChBrgPePortPcidSet == false then
            --[[ 
                    we not need to call this function as it was needed for removing tag1 when packet egress on 
                    downstream this (cascad) port, but we will get it because 'tti action' sets vid1=0. and the 'default' of the
                    egress port is also PCID = 0 ... meaning that tag 1 will be removed ... without the need to call this API.
            --]]        
            
            apiName = "cpssDxChBrgPePortPcidSet"
            genericCpssApiWithErrorHandler(command_data,
                apiName ,{
                { "IN",     "GT_U8",                    "devNum",     devNum },
                { "IN",     "GT_PHYSICAL_PORT_NUM",     "portNum",    portNum },
                { "IN",     "GT_U32",                   "pcid",   PCID_of_port}
            })
        end
        
        if is_cscd_port == false then
            -- update the downsteam multicast source filtering
            apiName = "cpssDxChBrgPePortPcidMcFilterEnableSet"
            genericCpssApiWithErrorHandler(command_data,
                apiName ,{
                { "IN",     "GT_U8",                    "devNum",     devNum },
                { "IN",     "GT_PHYSICAL_PORT_NUM",     "portNum",    portNum },
                { "IN",     "GT_U32",                   "pcid",       orig_pcid},
                { "IN",     "GT_BOOL",                  "mcFilterEnable",   mcFilterEnable}
            })
        end
    end
    
    --  set tti related
    func_onPe_non_eArch_bpe_802_1_pcid_on_me_port_info_tti(params,command_data)    
    
end

-- get into 'local physical port' interface in order to unique the pcid (on 'me')  and generate attributes
-- local port is the 'Extended Port'
--[[
Defenitions:
my_pcid_eport = my_current_port

Implementation:
for upstream:
tti rule :
my_current_port
tag1 NOT exists (not have ETag)
tti action:
Redirect to target eport = reserved_eport_of_upstream_ingress_with_no_etag_base +  my_current_port
Bypass ingress  = 1
Bypass bridge = 1
evlan command = 'modify untagged' (use the below "vlan_id for untagged")
evlan = <pvid0>

for eport = (reserved_eport_of_upstream_ingress_with_no_etag_base +  my_current_port)

HA <Push Tag Enable> = 1
HA <Push Tag VLAN-ID value> = < pcid >

For downstream

Lookup 0:

tti rule :
a.       'physical_port_upstream'

E-TAG<Ingress_E-CID> = < pcid >  --> use offset in the packet with UDBs
tti action:
reassign srcEport = 1
srcEport = my_pcid_eport
enable lookup 1

lookup 1:

tti rule :
'physical_port_upstream'
E-TAG<GRP, E-CID> --> use offset in the packet with UDBs
               i.      GRP  = 0                --> meaning 'known UC'
             ii.      E-CID = <pcid>

tti action:
<Enable Nested VLAN>=1
Redirect to target eport = my_pcid_eport
Bypass ingress  = 1
Bypass bridge = 1

for eport = (my_pcid_eport)

TTI unit : <Number of tag words to pop>=2
--]]
local function func_onPe_bpe_802_1_pcid_on_me_port_info(params,command_data)
    if not bpe_802_1_br_is_eArch_Device() then
        -- no eArch device
        return func_onPe_non_eArch_bpe_802_1_pcid_on_me_port_info(params,command_data)
    end

    local apiName,result,enableLookup
    local result
    local PCID_of_port = params.pcid
    local eportForUpstream,eportForDownstream
    local vlanCmd,numberOfTagWords
    local db_vertex, db_str;

    -- handle default values
    if params.pvid0 == nil          then params.pvid0 = 1 end

    -- set tti related config
    func_onPe_bpe_802_1_pcid_on_me_port_info_tti(params,command_data)

    -- configure the port with generic iterator
    for dummy, devNum, portNum in command_data:getPortIterator() do

        eportForUpstream = onPe_getEportOf_reserved_eport_of_upstream_ingress_with_no_etag(portNum)

        if params.flagNo == true then -- 'unset'
            numberOfTagWords = 0
            vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E"

        else -- 'set'
            -- the BPE need to remove 8 bytes ETag (2 time 4 bytes)
            numberOfTagWords = long_tag
            vlanCmd = "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E"
        end

        --[ save info to DB --]
        db_vertex = bpe_802_1_br_my_info_DB["dev_info"];
        if db_vertex["pe_per_phy_port_info"] == nil then db_vertex["pe_per_phy_port_info"] = {}; end;
        db_vertex = db_vertex["pe_per_phy_port_info"];
        db_str = tostring(devNum).. "/" .. tostring(portNum);
        if params.flagNo == true then -- 'unset'
            if (db_vertex[db_str] ~= nil) then
                db_vertex[db_str]["pcid"]  = nil;
                db_vertex[db_str]["pvid0"] = nil;
                if (next(db_vertex[db_str], nil) == nil) then
                    db_vertex[db_str] = nil;
                end
            end
        else
            if db_vertex[db_str] == nil then db_vertex[db_str] = {}; end;
            db_vertex = db_vertex[db_str];
            db_vertex["pcid"]  = params["pcid"];
            db_vertex["pvid0"] = params["pvid0"];
        end

        -- set the eport to have ETag
        pushETagCommandOnEPort(command_data,devNum,eportForUpstream,vlanCmd,PCID_of_port)

        if true == onPe_isExistsEportOfPcidOnCscdPort(portNum) then
            -- this is indication that it is 'cscd port' that need to set it's eport with the 'PCID_of_port'
            local cscdPortEPort = onPe_getEportOfPcidOnCscdPort(portNum)

            eportForDownstream = cscdPortEPort

            -- set the eport to have ETag (for known UC)
            -- set the eport associated with 'PCID_of_port' but without the 'push' because 
            -- unicast to this PCID_of_port should be without the tag
            -- the PCID needed for <ingress_E-CID_base> setting in 'flooding' 
            -- that ingress the system from this PCID_of_port
            pushETagCommandOnEPort(command_data,devNum,cscdPortEPort,
                "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E",
                PCID_of_port)

            if params.flagNo == true then -- 'unset'
                -- 'restore' the 'push ETag' on the cascade port to special 'flood in vlan' (when target is 'useVidx' --> flood to vlan/vidx)
                func_cascadePortPushETag(command_data,false,devNum,portNum)
            else
                -- set the physical port to have ETag (for 'flood')
                pushETagCommandOnEPort(command_data,devNum,portNum,vlanCmd,PCID_of_port)
            end
        else
            if isVTag6BytesConfig then
                -- in this mode we also need to set the PCID_of_port to 'physical port',
                -- but without 'push tag' ... only setting the 'PCID_of_port'
                -- this is needed for upstream to properly set the <srcInterface> in the tag
                pushETagCommandOnEPort(command_data,devNum,portNum,
                    "CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E",PCID_of_port)
                    
                -- more than that ... the <trgEPort> not need to use the <PCID_of_port>
                -- so let's keep it as 0 , so the tag on upstream traffic will get <trgInterface> = 0
                -- we use the 'push' command here ! so tag will be added.
                pushETagCommandOnEPort(command_data,devNum,eportForUpstream,
                    vlanCmd,0)
            end


            eportForDownstream = onPe_getEportOfPcidOnNonCascadePort(portNum)

            --***************
            --***************
            -- set the e2phy
            if params.flagNo == true then
                e2phySet(command_data,devNum,eportForDownstream,portNum,true)
            else
                e2phySet(command_data,devNum,eportForDownstream,portNum)
            end

            if params.flagNo == true then -- 'unset'
                -- the physical port not need the eport any more
                onPe_freeEportOfPcidOnNonCscdPort(portNum)
            end
            
            -- convert physical port number to it's srcId --> 
            -- used for src port filtering on downstream
            local srcId = srcIdFromPhysicalPortNum(portNum)
            local final_srcId
            
            if isVTag6BytesConfig then
                -- the filter need to be for '<direction> = direction_downstream' and '<isMcFilter> = true' indications
                final_srcId = srcIdCalcForVTag6BytesGet(srcId , true--[[<isMcFilter> = true]])
            else
                final_srcId = srcId 
            end
            
            local addPort
            
            if params.flagNo == true then -- 'unset'
                -- we restore the port to the group
                addPort = true
            else
                -- we remove the port from the group --> to cause filtering
                addPort = false
            end
            
            -- the port need to support 'MC src-filtering'
            updateSrcIdEntry(command_data,devNum,final_srcId,portNum,addPort)
        end

        --***************
        --***************
        result = numOfTagBytesToPopSet(
            devNum, eportForDownstream, numberOfTagWords)

    end

end

local function internal_func_onPe_bpe_802_1_pcid_on_me_port_info(params)
    local command_data = Command_Data()
    local my_role_is_control_bridge

    if not bpe_802_1_br_my_info_DB then
        print(bpe_802_1_br_error_system_is_not_802_1br_string)
        return nil
    end

    if bpe_802_1_br_my_info_DB["dev_info"]["role"] == "control_bridge" then
        my_role_is_control_bridge = true
    else
        my_role_is_control_bridge = false
    end

    -- prepare dev,port iterators
    command_data:initInterfaceDevPortRange()

    if my_role_is_control_bridge == true then
        print("ERROR : 'Extended port' was not implemented yet for 'Control Bridge' (implemented only for 'Port Extender')")
        return nil
    else
        func_onPe_bpe_802_1_pcid_on_me_port_info(params,command_data)
        -- save the info
        bpe_802_1_br_pcid_on_pe_phy_port_info_save_in_db(params)
    end

    -- handle all the errors that may come from calling the function
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

function bpe_802_1_pcid_on_me_port_info_show()
    local db_vertex, db_str, db_vertex1;
    local devNum, portNum, pcid, pvid0;

    --[ get info to DB --]
    db_vertex = bpe_802_1_br_my_info_DB;
    if db_vertex == nil then  return; end;
    db_vertex = db_vertex["dev_info"];
    if db_vertex == nil then  return; end;
    db_vertex = db_vertex["pe_per_phy_port_info"];
    if db_vertex == nil then  return; end;
    print("PCIDs on me (local PE)");
    print("device\t port\t pcid  \t pvid0 ");
    print("------\t ----\t ------\t ------");
    db_str = next(db_vertex, nil);
    while (db_str ~= nil) do
        db_vertex1 = db_vertex[db_str];
        devNum, portNum = string.match(db_str, "(%d+)/(%d+)");
        pcid = db_vertex1["pcid"];
        if (pcid == nil) then pcid = 0; end
        pvid0 = db_vertex1["pvid0"];
        if (pvid0 == nil) then pvid0 = 0; end
        if (portNum ~= nil) then
            print(string.format(
                "0x%4.4X\t 0x%2.2X\t 0x%4.4X\t 0x%4.4X",
                devNum, portNum, pcid, pvid0));
            end
        db_str = next(db_vertex, db_str);
    end
end

CLI_addHelp({"interface", "ethernet"}, "bpe-802-1-br" ,generic_help_bpe_802_1_br)

-- the command looks like :
--Console(config)# interface ethernet 0/18
--Console(config-if)# bpe-802-1-br pcid-on-me pcid <pcid> [pvid0 <pvid0>]
CLI_addCommand({"interface", "ethernet"}, "bpe-802-1-br pcid-on-me", {
    func   = internal_func_onPe_bpe_802_1_pcid_on_me_port_info,
    help   = "Bind PCID to 'Extended port' (current local interface) in BPE system",
    params = {
        { type = "named",
            { format = "pcid %value_12_bits",    name = "pcid",    help = "The PCID (<E-Cid_base> in the ETag) that represents the 'Extended Port'"},
            { format = "pvid0 %value_13_bits",   name = "pvid0",   help = "The 'default vid0' for untagged/priority tagged packets. (when tag 0 not recognized by tpid) Refers to tag0 after the E-Tag that is popped . range : 0..(8K-1) " ..
                    "optional parameter (default is '1') "  },
            mandatory = { "pcid"}
        }
    }
})

-- the command looks like :
--Console(config)# interface ethernet 0/18
--Console(config-if)# no bpe-802-1-br pcid-on-me
CLI_addCommand({"interface", "ethernet"}, "no bpe-802-1-br pcid-on-me", {
    func = function(params)
        params["flagNo"] = true;
        internal_func_onPe_bpe_802_1_pcid_on_me_port_info(params);
    end,
    help   = "Unbind physical port from in BPE system",
    params = {}
})

--[[
    debug 'show' copmmands to see the internal DB of the TTI tcam manager
--]]
CLI_addHelp("debug", "show tti", "show TTI related info")
CLI_addHelp("debug", "show tti internal-db", "show TTI internal DB managed by the LUA commands")

CLI_addCommand("debug", "show tti internal-db tcam", {
    func = function(params)

        print ("\n start entries for TCAM-TTI 0")
        print ("index   , purpose ")
        debug_table_print(tti_0_db)
        print ("end TTI 0 \n")

        print ("\n start entries for TCAM-TTI 1")
        print ("index   , purpose ")
        debug_table_print(tti_1_db)
        print ("end TTI 1 \n")

    end,
    help="Dump internal DB of TTI tcam"
})

--[[
on PE : non eArch device :
set ECID used by other downstream PE
======
Downstream Unicast Forwarding
For Unicast forwarding, a TTI rule should be allocated per E-CID and assigned to the
Upstream port.
TTI rule is used, the key is:
    <Tag1 Exist>=Exists - This ensures that the packet is E-Tagged
    <E-Tag GRP>=0 - This ensures that the packet is unicast
    <Tag1 VLAN-ID>= E-CID - For Unicast packets the attribute <Tag1 VLAN-ID> holds the
incoming packet E-Tag<E-CID> (see Section 30.2.2.1, Parsing E-Tags).

The TTI rule action is configured as follows:
    <Redirect Command>= "Redirect packet to egress interface"
    <Egress Interface>= Cascade port or Extended port
    <Bypass Ingress Pipe>= Enabled
--]]
local function local_onPe_bpe_802_1_br_non_eArch_pcid_on_pe_config_info(params,command_data)
    local unset = (params.flagNo)
    
    
    local apiName,result
    local PCID_of_port = params.pcid
    local addPort
    local myIndexName
    local portNum
    local devNum  

    -- set the TTI rule + action
    local finalTtiAction = {}
    local finalTtiPattern = {}
    local finalTtiMask = {}
    local ttiAction = {}
    local egressInterface = {}

    if unset == true then
        addPort = false
    else
        addPort = true
    end

    -- configure the port with generic iterator
    -- check that the pe_name exists on a physical port
    local pe_physical_devNum , pe_physical_portNum = get_physical_info_of_pe(params.pe_name)

    if pe_physical_portNum == nil or pe_physical_devNum == nil then

        print("ERROR : the PE " .. params.pe_name .. "was not defined")
        return nil
    end

    local pe_physical_hwDevNum , dummy_portNum =
        Command_Data:getHWDevicePort(pe_physical_devNum,pe_physical_portNum)
    
    portNum = pe_physical_portNum
    devNum = pe_physical_devNum
    
    --
    -- set the downstream UC to the port represent the pcid
    --
    finalTtiAction = {}
    finalTtiPattern = {}
    finalTtiMask = {}
    ttiAction = {}
    egressInterface = {}

    -- we need trg port
    ttiAction['bridgeBypass']           = true
    ttiAction['ingressPipeBypass']      = true

    ttiAction['redirectCommand']           = "CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E"

    egressInterface.type = "CPSS_INTERFACE_PORT_E"
    egressInterface.devPort = {}
    egressInterface.devPort.devNum = pe_physical_hwDevNum
    egressInterface.devPort.portNum = portNum

    ttiAction['egressInterface'] = egressInterface
    
    ttiAction['tag1VlanCmd']           = "CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E"
        
    finalTtiAction.type2 = ttiAction

    finalTtiPattern.eth , finalTtiMask.eth =
        bpe_802_1_br_non_eArch_buildTtiPatternMask(onPe_upstream_port,--physicalPort
            true,-- ETag_exists
            0,-- grp
            PCID_of_port,-- ecid_base
            false) -- isTrunk

    myIndexName = "BPE: downstream Unicast Forwarding E-TAG<ecid_base> = < " .. PCID_of_port .. " > , assign trgEPort , device " .. devNum .. " and port ".. portNum
    -- if myIndexName not exists in DB it will give 'new index' and saved into DB
    -- if myIndexName exists in DB it will retrieved from DB
    func_onPe_non_eArch_bpe_802_1_tti_Rule_update(command_data,myIndexName,addPort,devNum,finalTtiPattern,finalTtiMask,finalTtiAction)
    
    if not unset then
        bpe_802_1_br_non_eArch_on_pe_remote_pe_save_in_db(params.pe_name,params.pcid,unset)
    end
    
end

local function bpe_802_1_br_pcid_on_pe_config_info(params)
    local command_data = Command_Data()
    local unset = (params.flagNo)
    
    if not bpe_802_1_br_my_info_DB then
        print(bpe_802_1_br_error_system_is_not_802_1br_string)
        return nil
    end

    if bpe_802_1_br_is_eArch_Device() then
        -- for those devices use same command in the eport context
        print("ERROR : command not relevant for current device : use same command in the 'interface eport' context")
        return nil
    end

    -- prepare dev,port iterators
    command_data:initInterfaceDevPortRange()

    local_onPe_bpe_802_1_br_non_eArch_pcid_on_pe_config_info(params,command_data)
    
    if unset then
        bpe_802_1_br_non_eArch_on_pe_remote_pe_save_in_db(params.pe_name,params.pcid,unset)
    end

    -- handle all the errors that may come from calling the function
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- the command looks like :
--Console(config)# bpe-802-1-br pcid-on-pe <pe-name> <pcid>
-- NOTE: same command under 'eport' context for BC2 devices
CLI_addCommand("config", "bpe-802-1-br pcid-on-pe", {
    func   = bpe_802_1_br_pcid_on_pe_config_info,
    help   = "state that PCID exists on another PE in BPE system",
    params = {
        { type = "named",
            { format = "pe-name %s", name = "pe_name", help = "The name of the PE , for example \"PE1\" or \"port extender 33\" "},
            { format = "pcid %value_12_bits",    name = "pcid",    help = "The PCID (<E-Cid_base> in the ETag) that represents the 'Extended Port'"},
            mandatory = { "pe_name", "pcid"}
        }
    }
})
CLI_addCommand("config", "no bpe-802-1-br pcid-on-pe", {
    func   = function(params) 
                params.flagNo = true
                return bpe_802_1_br_pcid_on_pe_config_info(params)
             end,
    help   = "state that PCID exists on another PE in BPE system",
    params = {
        { type = "named",
            { format = "pe-name %s", name = "pe_name", help = "The name of the PE , for example \"PE1\" or \"port extender 33\" "},
            { format = "pcid %value_12_bits",    name = "pcid",    help = "The PCID (<E-Cid_base> in the ETag) that represents the 'Extended Port'"},
            mandatory = { "pe_name", "pcid"}
        }
    }
})

-- Reset DBs after disable PBE on device.
--
-- inputs :
--      none
--
-- reutrns
--      none
--
function bpe_802_1_br_neighbor_db_reset()
    on_pe_num_eports_used_for_pcid_on_non_cscd_port = 0
    onPe_EportOfPcidOnNonCscdPortsArr = {}
    on_pe_num_eports_used_for_pcid_on_cscd_port = 0
    onPe_EportOfPcidOnCscdPortsArr = {}
    onPe_reserved_eport_of_upstream_ingress_with_no_etag_curr_num = 0
    onPe_reserved_eport_of_upstream_ingress_with_no_etag_arr = {}
    
    isVTag6BytesConfig = false
end