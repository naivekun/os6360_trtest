--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* indexManagementUtil.lua
--*
--* DESCRIPTION:
--*       Indexes management implementation
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
    end
end

---------------------------------------------
------------- PCL Index management table -----
---------------------------------------------

-- first index that the 'appDemo' uses
-- valid range is 0..(18k-1)
local _18k = 18*1024
local pcl_base_index = 0
local pcl_last_index = _18k - 1

-- the defaults above are for 'bobcat2' ... we need support for xcat3...
function pcl_manager_range_set(baseIndex,lastIndex)
    pcl_base_index = baseIndex
    pcl_last_index = lastIndex
end

-----------  pcl table index manipulation   -------------------------
-- hold fields: callerIndexName
local pcl_db = {}
-- hold fields: pclIndex , pclSize
local pcl_users_names = {}

-- get new PCL index or 'retrieve' from DB for this 'caller'
-- inputs:
-- myPclSize  - the size of entry : number of indexes to occupy
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the pcl index
--
function allocPclIndexFromPool(myPclSize,callerIndexName,fromEnd)
    local minVal
    local maxVal
    local newPclIndex
    local isNew = true

    if fromEnd == nil then fromEnd = false end

    minVal = pcl_base_index
    maxVal = pcl_last_index

    if pcl_users_names[callerIndexName] == nil then
        -- need to allocate new index in pcl_db

        newPclIndex = getFreeIndexInTable(pcl_db,minVal,maxVal,myPclSize,myPclSize,fromEnd)

        pcl_db[newPclIndex] = {}
        pcl_db[newPclIndex].callerName = callerIndexName

        pcl_users_names[callerIndexName] = {}
        pcl_users_names[callerIndexName].pclIndex = newPclIndex
        pcl_users_names[callerIndexName].pclSize = myPclSize
    else
        -- index was already allocated for this 'caller'
        newPclIndex = pcl_users_names[callerIndexName].pclIndex
        isNew = false
    end

    if newPclIndex == nil then
        -- error (table is full)
        return nil
    end

    if isNew == true then
        _debug("allocated new pclIndex = " .. newPclIndex .. " for client " .. callerIndexName)
    else
        _debug("retrieved pclIndex = " .. newPclIndex .. " for client " .. callerIndexName)
    end

    -- index is new or 'retrieved' from DB for this 'caller'
    return newPclIndex
end

--
-- get new PCL index or 'retrieve' from DB for this 'caller' - lookfor index staring from end of table
-- inputs:
-- myPclSize  - the size of entry :number of indexes to occupy:
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the pcl index
--
function allocPclIndexFromPool_fromEnd(myPclSize,callerIndexName)
    return allocPclIndexFromPool(myPclSize,callerIndexName,true)
end

-- delete entry(s) from pcl_db and from pcl_users_names
local function local_freePclIndexToPool(pclIndex)
    local callerIndexName = pcl_db[pclIndex].callerName

    if pcl_users_names[callerIndexName] == nil then
        _debug("pclIndex = "..pclIndex.." caller was not set for index in table")
    else
        -- start loop from second index .. because we anyway invalidate first one at the end
        for index = (pclIndex+1),(pclIndex+pcl_users_names[callerIndexName].pclSize - 1) do
            pcl_db[index] = nil
        end
    end

    pcl_users_names[callerIndexName] = nil
    pcl_db[pclIndex] = nil

    _debug("local_freePclIndexToPool : free from DB index :" .. pclIndex .. " for : " .. callerIndexName)
end

--
-- release the pclIndex back to the DB (the pool)
-- inputs:
-- pclIndex  - the pcl index (as returned by allocPclIndexFromPool(...))
--
-- returns:
-- nil
--
function freePclIndexToPool(pclIndex)
    local callerIndexName

    if pclIndex <  pcl_base_index or
            pclIndex >  pcl_last_index
    then
        _debug("pclIndex = "..pclIndex.." over max or below min")
        return nil
    end

    if pcl_db[pclIndex] == nil then
        _debug("pclIndex = "..pclIndex.." index was not occupied in pcl_db")
        -- index was not occupied
        return nil
    end
    local_freePclIndexToPool(pclIndex)
    return nil
end


---------------------------------------------
------------- TS Index management table -----
---------------------------------------------
-- min and  max values of TS table
local _16k = 16*1024
local ts_base_index = 0
local ts_last_index = _16k-1

-- hold fields: callerIndexName
local ts_db = {}
-- hold fields: tunnelStartIndex , tunnelStartSize
local ts_users_names = {}

-- get new TS index or 'retrieve' from DB for this 'caller'
-- inputs:
-- myTsSize  - the size of entry : 1/2 (number of indexes to occupy: 1 for ipv4, 2 for ipv6)
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the ts index
--
function allocTsIndexFromPool(myTsSize,callerIndexName,fromEnd)
    local minVal
    local maxVal
    local newTsIndex
    local isNew = true

    if fromEnd == nil then fromEnd = false end

    minVal = ts_base_index
    maxVal = ts_last_index

    if ts_users_names[callerIndexName] == nil then
        -- need to allocate new index in ts_db

        newTsIndex = getFreeIndexInTable(ts_db,minVal,maxVal,myTsSize,myTsSize,fromEnd)

        ts_db[newTsIndex] = {}
        ts_db[newTsIndex].callerName = callerIndexName

        ts_users_names[callerIndexName] = {}
        ts_users_names[callerIndexName].tunnelStartIndex = newTsIndex
        ts_users_names[callerIndexName].tunnelStartSize = myTsSize
    else
        -- index was already allocated for this 'caller'
        newTsIndex = ts_users_names[callerIndexName].tunnelStartIndex
        isNew = false
    end

    if newTsIndex == nil then
        -- error (table is full)
        return nil
    end

    if isNew == true then
        _debug("allocated new tsIndex = " .. newTsIndex .. " for client " .. callerIndexName)
    else
        _debug("retrieved tsIndex = " .. newTsIndex .. " for client " .. callerIndexName)
    end

    -- index is new or 'retrieved' from DB for this 'caller'
    return newTsIndex
end

--
-- get new TS index or 'retrieve' from DB for this 'caller' - lookfor index staring from end of table
-- inputs:
-- myTsSize  - the size of entry : 1/2 (number of indexes to occupy: 1 for ipv4, 2 for ipv6)
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the ts index
--
local function allocTsIndexFromPool_fromEnd(myTsSize,callerIndexName)
    return allocTsIndexFromPool(myTsSize,callerIndexName,true)
end

-- delete entry(s) from ts_db and from ts_users_names
local function local_freeTsIndexToPool(tsIndex)
    local callerIndexName = ts_db[tsIndex].callerName

    if ts_users_names[callerIndexName] == nil then
        _debug("tsIndex = "..tsIndex.." caller was not set for index in table")
    else
        -- start loop from second index .. because we anyway invalidate first one at the end
        for index = (tsIndex+1),(tsIndex+ts_users_names[callerIndexName].tunnelStartSize - 1) do
            ts_db[index] = nil
        end
    end

    ts_users_names[callerIndexName] = nil
    ts_db[tsIndex] = nil

    _debug("local_freeTsIndexToPool : free from DB index :" .. tsIndex .. " for : " .. callerIndexName)
end

--
-- release the tsIndex back to the DB (the pool)
-- inputs:
-- tsIndex  - the ts index (as returned by allocTsIndexFromPool(...))
--
-- returns:
-- nil
--
function freeTsIndexToPool(tsIndex)
    local callerIndexName

    if tsIndex <  ts_base_index or
            tsIndex >  ts_last_index
    then
        _debug("tsIndex = "..tsIndex.." over max or below min")
        return nil
    end

    if ts_db[tsIndex] == nil then
        _debug("tsIndex = "..tsIndex.." index was not occupied in ts_db")
        -- index was not occupied
        return nil
    end
    local_freeTsIndexToPool(tsIndex)
    return nil
end

-------------------------------------------------
------------- MAC2ME Index management table -----
-------------------------------------------------

-- min and  max values of mac2me table
local mac2me_base_index = 0
local mac2me_last_index = 255

-- hold fields: callerIndexName
local mac2me_db = {}
-- hold fields: mac2meIndex
local mac2me_users_names = {}

-- get new mac2me index or 'retrieve' from DB for this 'caller'
-- inputs:
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the ts index
--

function allocMac2MeIndexFromPool(callerIndexName,fromEnd)
    local minVal
    local maxVal
    local newMac2MeIndex
    local isNew = true
    if fromEnd == nil then fromEnd = false end
    minVal = mac2me_base_index
    maxVal = mac2me_last_index

    if mac2me_users_names[callerIndexName] == nil then
        -- need to allocate new index in mac2me_db
        newMac2MeIndex = getFreeIndexInTable(mac2me_db,minVal,maxVal,1,1,fromEnd)
        mac2me_db[newMac2MeIndex] = {}
        mac2me_db[newMac2MeIndex].callerName = callerIndexName
        mac2me_users_names[callerIndexName] = {}
        mac2me_users_names[callerIndexName].mac2meIndex = newMac2MeIndex
    else
        -- index was already allocated for this 'caller'
        newMac2MeIndex = mac2me_users_names[callerIndexName].mac2meIndex
        isNew = false
    end

    if newMac2MeIndex == nil then
        -- error (table is full)
        return nil
    end

    if isNew == true then
        _debug("allocated new mac2meIndex = " .. newMac2MeIndex .. " for client " .. callerIndexName)
    else
        _debug("retrieved mac2meIndex = " .. newMac2MeIndex .. " for client " .. callerIndexName)
    end

    -- index is new or 'retrieved' from DB for this 'caller'
    return newMac2MeIndex
end

--
-- get new mac2me index or 'retrieve' from DB for this 'caller' - lookfor index staring from end of table
-- inputs:
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the mac2me index
--
local function allocMac2MeIndexFromPool_fromEnd(callerIndexName)
    return allocMac2MeIndexFromPool(callerIndexName,true)
end

-- delete entry(s) from mac2me_db and from mac2me_users_names
local function local_freeMac2MeIndexToPool(mac2meIndex)
    local callerIndexName = mac2me_db[mac2meIndex].callerName
    if mac2me_users_names[callerIndexName] == nil then
        _debug("mac2meIndex = "..mac2meIndex.." caller was not set for index in table")
    else
        mac2me_db[mac2meIndex] = nil
    end
    mac2me_users_names[callerIndexName] = nil
    mac2me_db[mac2meIndex] = nil
    _debug("local_freeMac2MeIndexToPool : free from DB index :" .. mac2meIndex .. " for : " .. callerIndexName)
end
--
-- release the mac2meIndex back to the DB (the pool)
-- inputs:
-- mac2meIndex  - the mac2me index (as returned by allocMac2MeIndexFromPool(...))
--
-- returns:
-- nil
--
function freeMac2MeIndexToPool(mac2meIndex)
    local callerIndexName
    if mac2meIndex <  mac2me_base_index or
            mac2meIndex >  mac2me_last_index
    then
        _debug("mac2meIndex = "..mac2meIndex.." over max or below min")
        return nil
    end

    if mac2me_db[mac2meIndex] == nil then
        _debug("mac2meIndex = "..mac2meIndex.." index was not occupied in mac2me_db")
        -- index was not occupied
        return nil
    end

    local_freeMac2MeIndexToPool(mac2meIndex)
    return nil
end

-------------------------------------------------
------------- Control Word Index management table -----
-------------------------------------------------

-- min and  max values of cw table
local cw_base_index = 0
local cw_last_index = 14

-- hold fields: callerIndexName
local cw_db = {}
-- hold fields: cwIndex
local cw_users_names = {}

-- get new cw index or 'retrieve' from DB for this 'caller'
-- inputs:
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the ts index
--
function allocCwIndexFromPool(callerIndexName,fromEnd)
    local minVal
    local maxVal
    local newCwIndex
    local isNew = true

    if fromEnd == nil then fromEnd = false end

    minVal = cw_base_index
    maxVal = cw_last_index

    if cw_users_names[callerIndexName] == nil then
        -- need to allocate new index in cw_db

        newCwIndex = getFreeIndexInTable(cw_db,minVal,maxVal,1,1,fromEnd)

        cw_db[newCwIndex] = {}
        cw_db[newCwIndex].callerName = callerIndexName

        cw_users_names[callerIndexName] = {}
        cw_users_names[callerIndexName].cwIndex = newCwIndex
    else
        -- index was already allocated for this 'caller'
        newCwIndex = cw_users_names[callerIndexName].cwIndex
        isNew = false
    end

    if newCwIndex == nil then
        -- error (table is full)
        return nil
    end

    if isNew == true then
        _debug("allocated new cwIndex = " .. newCwIndex .. " for client " .. callerIndexName)
    else
        _debug("retrieved cwIndex = " .. newCwIndex .. " for client " .. callerIndexName)
    end

    -- index is new or 'retrieved' from DB for this 'caller'
    return newCwIndex
end

--
-- get new cw index or 'retrieve' from DB for this 'caller' - lookfor index staring from end of table
-- inputs:
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the cw index
--
local function allocCwIndexFromPool_fromEnd(callerIndexName)
    return allocCwIndexFromPool(callerIndexName,true)
end

-- delete entry(s) from cw_db and from cw_users_names
local function local_freeCwIndexToPool(cwIndex)
    local callerIndexName = cw_db[cwIndex].callerName

    if cw_users_names[callerIndexName] == nil then
        _debug("cwIndex = "..cwIndex.." caller was not set for index in table")
    else
        cw_db[cwIndex] = nil
    end

    cw_users_names[callerIndexName] = nil
    cw_db[cwIndex] = nil

    _debug("local_freecwIndexToPool : free from DB index :" .. cwIndex .. " for : " .. callerIndexName)
end

--
-- release the cwIndex back to the DB (the pool)
-- inputs:
-- cwIndex  - the cw index (as returned by allocCwIndexFromPool(...))
--
-- returns:
-- nil
--
function freeCwIndexToPool(cwIndex)
    local callerIndexName

    if cwIndex <  cw_base_index or
            cwIndex >  cw_last_index
    then
        _debug("cwIndex = "..cwIndex.." over max or below min")
        return nil
    end

    if cw_db[cwIndex] == nil then
        _debug("cwIndex = "..cwIndex.." index was not occupied in cw_db")
        -- index was not occupied
        return nil
    end
    local_freeCwIndexToPool(cwIndex)
    return nil
end

-------------------------------------------------
------------- IP NextHop Index management table -----
-------------------------------------------------
-- first index that the 'appDemo' uses
-- valid range is 0..(24k-1)
local _24k = 24*1024

-- min and  max values of IP Next Hop table
local ipNextHop_base_index = 10
local ipNextHop_last_index = _24k - 1

-- hold fields: callerIndexName
local ipNextHop_db = {}
-- hold fields: ipNextHopIndex
local ipNextHop_users_names = {}

-- get new ipNextHop index or 'retrieve' from DB for this 'caller'
-- inputs:
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the ipNextHop index
--
function allocIpNextHopIndexFromPool(callerIndexName,fromEnd)
    local minVal
    local maxVal
    local newIpNextHopIndex
    local isNew = true

    if fromEnd == nil then fromEnd = false end

    minVal = ipNextHop_base_index
    maxVal = ipNextHop_last_index

    if ipNextHop_users_names[callerIndexName] == nil then
        -- need to allocate new index in ipNextHop_db

        newIpNextHopIndex = getFreeIndexInTable(ipNextHop_db,minVal,maxVal,1,1,fromEnd)

        ipNextHop_db[newIpNextHopIndex] = {}
        ipNextHop_db[newIpNextHopIndex].callerName = callerIndexName

        ipNextHop_users_names[callerIndexName] = {}
        ipNextHop_users_names[callerIndexName].ipNextHopIndex = newIpNextHopIndex
    else
        -- index was already allocated for this 'caller'
        newIpNextHopIndex = ipNextHop_users_names[callerIndexName].ipNextHopIndex
        isNew = false
    end

    if newIpNextHopIndex == nil then
        -- error (table is full)
        return nil
    end

    if isNew == true then
        _debug("allocated new ipNextHopIndex = " .. newIpNextHopIndex .. " for client " .. callerIndexName)
    else
        _debug("retrieved ipNextHopIndex = " .. newIpNextHopIndex .. " for client " .. callerIndexName)
    end

    -- index is new or 'retrieved' from DB for this 'caller'
    return newIpNextHopIndex
end

--
-- get new ipNextHop index or 'retrieve' from DB for this 'caller' - lookfor index staring from end of table
-- inputs:
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the ipNextHop index
--
local function allocIpNextHopIndexFromPool_fromEnd(callerIndexName)
    return allocIpNextHopIndexFromPool(callerIndexName,true)
end

-- delete entry(s) from ipNextHop_db and from ipNextHop_users_names
local function local_freeIpNextHopIndexToPool(ipNextHopIndex)
    local callerIndexName = ipNextHop_db[ipNextHopIndex].callerName

    if ipNextHop_users_names[callerIndexName] == nil then
        _debug("ipNextHopIndex = "..ipNextHopIndex.." caller was not set for index in table")
    else
        ipNextHop_db[ipNextHopIndex] = nil
    end

    ipNextHop_users_names[callerIndexName] = nil
    ipNextHop_db[ipNextHopIndex] = nil

    _debug("local_freeIpNextHopIndexToPool : free from DB index :" .. ipNextHopIndex .. " for : " .. callerIndexName)
end

--
-- release the ipNextHopIndex back to the DB (the pool)
-- inputs:
-- ipNextHopIndex  - the ipNextHop index (as returned by allocipNextHopIndexFromPool(...))
--
-- returns:
-- nil
--
function freeIpNextHopIndexToPool(ipNextHopIndex)
    local callerIndexName

    if ipNextHopIndex <  ipNextHop_base_index or
            ipNextHopIndex >  ipNextHop_last_index
    then
        _debug("ipNextHopIndex = "..ipNextHopIndex.." over max or below min")
        return nil
    end

    if ipNextHop_db[ipNextHopIndex] == nil then
        _debug("ipNextHopIndex = "..ipNextHopIndex.." index was not occupied in ipNextHop_db")
        -- index was not occupied
        return nil
    end
    local_freeIpNextHopIndexToPool(ipNextHopIndex)
    return nil
end

-------------------------------------------------
------------- LPM LEAF Index management table -----
-------------------------------------------------
-- first index that the 'appDemo' uses
-- valid range is 0..(8k-1)
local _8k = 8*1024

-- min and  max values of LPM LEAF table
local lpmLeaf_base_index = 0
local lpmLeaf_last_index = _8k - 1

-- hold fields: callerIndexName
local lpmLeaf_db = {}
-- hold fields: lpmLeafIndex
local lpmLeaf_users_names = {}

-- get new lpmLeaf index or 'retrieve' from DB for this 'caller'
-- inputs:
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the lpmLeaf index
--
function allocLpmLeafIndexFromPool(callerIndexName,fromEnd)
    local minVal
    local maxVal
    local newLpmLeafIndex
    local isNew = true

    if fromEnd == nil then fromEnd = false end

    minVal = lpmLeaf_base_index
    maxVal = lpmLeaf_last_index

    if lpmLeaf_users_names[callerIndexName] == nil then
        -- need to allocate new index in lpmLeaf_db

        newLpmLeafIndex = getFreeIndexInTable(lpmLeaf_db,minVal,maxVal,1,1,fromEnd)

        lpmLeaf_db[newLpmLeafIndex] = {}
        lpmLeaf_db[newLpmLeafIndex].callerName = callerIndexName

        lpmLeaf_users_names[callerIndexName] = {}
        lpmLeaf_users_names[callerIndexName].lpmLeafIndex = newLpmLeafIndex
    else
        -- index was already allocated for this 'caller'
        newLpmLeafIndex = lpmLeaf_users_names[callerIndexName].lpmLeafIndex
        isNew = false
    end

    if newLpmLeafIndex == nil then
        -- error (table is full)
        return nil
    end

    if isNew == true then
        _debug("allocated new lpmLeafIndex = " .. newLpmLeafIndex .. " for client " .. callerIndexName)
    else
        _debug("retrieved lpmLeafIndex = " .. newLpmLeafIndex .. " for client " .. callerIndexName)
    end

    -- index is new or 'retrieved' from DB for this 'caller'
    return newLpmLeafIndex
end

--
-- get new lpmLeaf index or 'retrieve' from DB for this 'caller' - lookfor index staring from end of table
-- inputs:
-- callerIndexName - string that unique the allocator (the purpose of the allocation)
--
-- returns:
-- the lpmLeaf index
--
local function allocLpmLeafIndexFromPool_fromEnd(callerIndexName)
    return allocLpmLeafIndexFromPool(callerIndexName,true)
end

-- delete entry(s) from lpmLeaf_db and from lpmLeaf_users_names
local function local_freeLpmLeafIndexToPool(lpmLeafIndex)
    local callerIndexName = lpmLeaf_db[lpmLeafIndex].callerName

    if lpmLeaf_users_names[callerIndexName] == nil then
        _debug("lpmLeafIndex = "..lpmLeafIndex.." caller was not set for index in table")
    else
        lpmLeaf_db[lpmLeafIndex] = nil
    end

    lpmLeaf_users_names[callerIndexName] = nil
    lpmLeaf_db[lpmLeafIndex] = nil

    _debug("local_freeLpmLeafIndexToPool : free from DB index :" .. lpmLeafIndex .. " for : " .. callerIndexName)
end

--
-- release the lpmLeafIndex back to the DB (the pool)
-- inputs:
-- lpmLeafIndex  - the lpmLeaf index (as returned by alloclpmLeafIndexFromPool(...))
--
-- returns:
-- nil
--
function freeLpmLeafIndexToPool(lpmLeafIndex)
    local callerIndexName

    if lpmLeafIndex <  lpmLeaf_base_index or
            lpmLeafIndex >  lpmLeaf_last_index
    then
        _debug("lpmLeafIndex = "..lpmLeafIndex.." over max or below min")
        return nil
    end

    if lpmLeaf_db[lpmLeafIndex] == nil then
        _debug("lpmLeafIndex = "..lpmLeafIndex.." index was not occupied in lpmLeaf_db")
        -- index was not occupied
        return nil
    end
    local_freeLpmLeafIndexToPool(lpmLeafIndex)
    return nil
end

CLI_addHelp("debug", "show PCL", "show PCL related info")
CLI_addHelp("debug", "show PCL internal-db", "show PCL internal DB managed by the LUA commands")

CLI_addCommand("debug", "show PCL internal-db tcam", {
    func = function(params)

        print ("\n start entries for TCAM-PCL")
        print ("index   , purpose ")
        debug_table_print(pcl_db)
        print ("end PCL 0 \n")
    end,
    help="Dump internal DB of PCL tcam"
})

CLI_addHelp("debug", "show ts", "show ts related info")
CLI_addHelp("debug", "show ts internal-db", "show TS internal DB managed by the LUA commands")

CLI_addCommand("debug", "show ts internal-db", {
    func = function(params)

        print ("\n start entries for TS")
        print ("index   , purpose ")
        debug_table_print(ts_db)
        print ("end TS 0 \n")
    end,
    help="Dump internal DB of TS"
})

CLI_addHelp("debug", "show mac2me", "show mac2me related info")
CLI_addHelp("debug", "show mac2me internal-db", "show MAC2ME internal DB managed by the LUA commands")

CLI_addCommand("debug", "show mac2me internal-db", {
    func = function(params)

        print ("\n start entries for MAC2ME")
        print ("index   , purpose ")
        debug_table_print(mac2me_db)
        print ("end MAC2ME 0 \n")
    end,
    help="Dump internal DB of MAC2ME"
})

CLI_addHelp("debug", "show CW", "show CW related info")
CLI_addHelp("debug", "show CW internal-db", "show CW internal DB managed by the LUA commands")

CLI_addCommand("debug", "show CW internal-db", {
    func = function(params)

        print ("\n start entries for CW")
        print ("index   , purpose ")
        debug_table_print(cw_db)
        print ("end CW 0 \n")
    end,
    help="Dump internal DB of CW"
})

CLI_addHelp("debug", "show ipNextHop", "show ipNextHop related info")
CLI_addHelp("debug", "show ipNextHop internal-db", "show IP Next Hop internal DB managed by the LUA commands")

CLI_addCommand("debug", "show ipNextHop internal-db", {
    func = function(params)

        print ("\n start entries for IP-NEXTHOP")
        print ("index   , purpose ")
        debug_table_print(ipNextHop_db)
        print ("end IP-NEXTHOP 0 \n")
    end,
    help="Dump internal DB of IP-NEXTHOP"
})

CLI_addHelp("debug", "show lpmLeaf", "show ipNextHop related info")
CLI_addHelp("debug", "show lpmLeaf internal-db", "show LPM LEAF internal DB managed by the LUA commands")

CLI_addCommand("debug", "show lpmLeaf internal-db", {
    func = function(params)

        print ("\n start entries for IP-NEXTHOP")
        print ("index   , purpose ")
        debug_table_print(lpmLeaf_db)
        print ("end LPM-LEAF 0 \n")
    end,
    help="Dump internal DB of LPM-LEAF"
})