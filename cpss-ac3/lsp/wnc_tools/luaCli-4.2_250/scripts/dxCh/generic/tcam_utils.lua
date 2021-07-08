--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tcam_utils.lua
--*
--* DESCRIPTION:
--*       contain auxiliary functions to get info about how TCAM memory is divided
--*       between clients  etc.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("wrlCpssDxChTcamGeomGet")
cmdLuaCLI_registerCfunction("wrlCpssDxChTcamIndexRangeHitNumAndGroupGet")

local floorColNum = 12
local floorRowsNum = 256
local floorItemsNum = floorColNum * floorRowsNum


-- ************************************************************************
--  tcamBlocksUsageInfo
--
--  @description return information about how Group/Client/hitNum
--               are assigned to TCAM blocks
--               Relevant to sip5 devices only.
--
--  @param devNum - a device number
--
--  @return two  values.
--          if failed: nil, <error message>
--          if ok: table {<floor_index> = {[<block_index>]={groupNum = <groupNum>,
--                                                           hitNum  = <hitNum>}
--                                           , ...}
--                         , ...}
--                 , number of blocks on each TCAM floor
--
-- ************************************************************************
local function tcamBlocksUsageInfo(devNum)
    if devNum == nil then
        return nil
    end

    local floorsNum, blocksNum, itemsNum  = wrlCpssDxChTcamGeomGet(devNum)

    local tcamInfo = {}
    local group = -1       -- impossible value for TCAM group numbers
    -- get group, hitNum values for every block
    for floor=0,floorsNum-1 do
        rc, floorInfo = wrLogWrapper(
            "wrlCpssDxChTcamIndexRangeHitNumAndGroupGet", "(devNum, floor)",
            devNum, floor)

        if rc ~= 0 then
            return nil, "wrlCpssDxChTcamIndexRangeHitNumAndGroupGet: " .. (returnCodes[rc] or to_string(rc))
        end
        tcamInfo[floor] = floorInfo
    end
    return tcamInfo, blocksNum
end


-- ************************************************************************
--  optimizeChunks
--
--  @description optimize chunks in place. 'Optimize' means merge neighbor
--               chunks without any gap between them.
--
--  @param chunks -  an  array of values
--                   {{base=<base>, size=<size>}, ...}
--
--  @return if failed: nil, error message
--          if ok:   return true
--
-- ************************************************************************
local function optimizeChunks(chunks)
    local last = -2           -- a value different from any base at least for 2.
    local prev = nil
    table.sort(chunks, function (a, b) return  a.base < b.base end)
    for key, ch in ipairs(chunks) do
        if ch.base - last < 1 then
            return nil, "optimizeChunks(): overlapping"
        elseif ch.base - last == 1 then
            prev.size = prev.size + ch.size
            chunks[key] = nil
        end
        if key == 1 then
            prev = ch
        end
        last = ch.base + ch.size - 1
    end
    return true
end

-- ************************************************************************
--  blocksToChunks
--
--  @description transforms a table of blocks  into an array of chunks.
--
--  @param blocks - a table {floor_i = <array of blocks>, floor_2=..., ...}
--  @param blocksPerFloor - number of blocks on each TCAM floor.
--
--  @return if  fail:  nil, <error  message>
--          if ok: array of chunks: {{base=<base>, size=<size>}, ...}
--
-- ************************************************************************
local function blocksToChunks(blocks, blocksPerFloor)

    local chunks = {}
    local fisrt = nil
    local last = nil
    local  blockColNum = floorColNum / blocksPerFloor
    if floorColNum % blocksPerFloor ~= 0 then
        return nil, string.format(
            "Floor banks count(%d) is not multiple to blocks count(%d)",
            floorColNum, blocksPerFloor)
    end
    for floor, floorBlocks in pairs(blocks) do
        table.sort(floorBlocks)
        local left = floorBlocks[1] -- nil, if floorBlocks is empty. It's ok.
        local right = left

        for i = 1,#floorBlocks+1 do -- +1 to get nil as last item.
            local block = floorBlocks[i]
            if (block == nil) or (block - right > 1) then
                local topLeft = floor * floorItemsNum + left * blockColNum
                local width =  (right-left + 1) * blockColNum
                if width == floorColNum then
                    table.insert(chunks, {base = topLeft, size = width * floorRowsNum})
                else
                    for i=0,floorRowsNum-1 do
                        table.insert(chunks, {base = topLeft, size = width})
                        topLeft = topLeft + floorColNum
                     end

                end
                left = block
            end
            right = block
        end
    end
    local isOk,errMsg = optimizeChunks(chunks)
    if isOk then
        return chunks
    else
        return nil, errorMsg
    end
end

-- use the same name as the name of the ENH-UT MACRO
local function HARD_WIRE_TCAM_MAC(devNum)
    if (DeviceFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT3_E" ) then
        return true
    else
        return false
    end
end

-- ************************************************************************
--  tcamClientBlocksGet
--
--  @description get information about TCAM blocks used by specified
--               clients.
--
--  @param devNum - a device number
--  @param clients - can be a single value of arrays of
--                   'tti', 'pcl', 'ipcl0', 'ipcl1', 'ipcl2', 'ipcl3', 'epcl'
--                   'pcl' means get chunks relevant to all clients
--                   IPCL0, IPC1, IPC2, EPCL.
--  @param hitNum - a hitNum. if nil  blocks with any hitNum will be return
--
--  @return  three values.
--           if failed: nil, <error message>
--           if ok: table { [client] = 
--                      table {floor=  {block_1, block_2, ...}, ...} 
--                          , 'next client' ...}
--                  , number of blocks on each TCAM floor
--              is 'multi groups' - true/false indication that the client are in multiple groups
--                  (see isMultipleGroups)
--
--          NOTE: if all clients belong to the same group --> no [client] indication
--
-- ************************************************************************
function tcamClientBlocksGet(devNum, clients, hitNum)

    if not clients then
        return nil, "clients are not specified"
    end

    if type(clients) == "string" then
        clients = {clients}
    end

    local tcamClients = { CPSS_DXCH_TCAM_IPCL_0_E = false,
                          CPSS_DXCH_TCAM_IPCL_1_E = false,
                          CPSS_DXCH_TCAM_IPCL_2_E = false,
                          CPSS_DXCH_TCAM_EPCL_E   = false,
                          CPSS_DXCH_TCAM_TTI_E    = false }


    for _, cl in pairs(clients) do
        if cl == "tti" then
            tcamClients.CPSS_DXCH_TCAM_TTI_E = true
        elseif cl == "ipcl0" then
            tcamClients.CPSS_DXCH_TCAM_IPCL_0_E = true
        elseif cl == "ipcl1" then
            tcamClients.CPSS_DXCH_TCAM_IPCL_1_E = true
        elseif cl == "ipcl2" then
            tcamClients.CPSS_DXCH_TCAM_IPCL_2_E = true
        elseif cl == "epcl" then
            tcamClients.CPSS_DXCH_TCAM_EPCL_E = true
        elseif cl == "pcl" then
            tcamClients.CPSS_DXCH_TCAM_IPCL_0_E = true
            tcamClients.CPSS_DXCH_TCAM_IPCL_1_E = true
            tcamClients.CPSS_DXCH_TCAM_IPCL_2_E = true
            tcamClients.CPSS_DXCH_TCAM_EPCL_E = true
        else
            return nil, "an unknown client " .. cl
        end
    end

    -- determane which TCAM groups we need to read.
    local neededGroups = {}
    local neededGroups_clients = {}
    for client, enabled in pairs(tcamClients) do
        if enabled then
            if(HARD_WIRE_TCAM_MAC(devNum))then
                -- 'hard wired'
                local bc3HardWiredTcamGroups = {
                    ["CPSS_DXCH_TCAM_IPCL_0_E"] = {tcamGroup = 1},
                    ["CPSS_DXCH_TCAM_IPCL_1_E"] = {tcamGroup = 2},
                    ["CPSS_DXCH_TCAM_IPCL_2_E"] = {tcamGroup = 3},
                    ["CPSS_DXCH_TCAM_EPCL_E"]   = {tcamGroup = 4},
                    ["CPSS_DXCH_TCAM_TTI_E"]    = {tcamGroup = 0}
                    }
                --print ("bc3HardWiredTcamGroups=",to_string(bc3HardWiredTcamGroups))
                
                clientInfo = bc3HardWiredTcamGroups[client]
                local tcamGroup
                
                if (clientInfo ~= nil) then
                    tcamGroup = clientInfo.tcamGroup
                    neededGroups[tcamGroup] = true
                    neededGroups_clients[tcamGroup] = client
                    --print ("bc3HardWiredTcamGroups: tcamGroup = " .. to_string(tcamGroup) .. " to client = " .. to_string(client))
                else
                    --print ("ERROR : bc3HardWiredTcamGroups: not supports client = " .. to_string(client))
                    return nil, "bc3HardWiredTcamGroups: not supports client = " .. to_string(client)
                end
                
            else --not 'hard wired'
                local rc, out  = myGenWrapper(
                    "cpssDxChTcamPortGroupClientGroupGet",
                    {
                        {"IN",  "GT_U8",                     "devNum",        devNum},
                        {"IN",  "GT_PORT_GROUPS_BMP",        "portGroupsBmp", 0xFFFFFFFF}, -- CPSS_PORT_GROUP_UNAWARE_MODE_CNS
                        {"IN",  "CPSS_DXCH_TCAM_CLIENT_ENT", "tcamClient",    client},
                        {"OUT", "GT_U32",                    "tcamGroup"},
                        {"OUT", "GT_BOOL",                   "enable"}
                })
                if rc ~= 0 then
                    return nil, "cpssDxChTcamPortGroupClientGroupGet: " .. (returnCodes[rc] or to_string(rc))
                end

                if out.enable then
                    neededGroups[out.tcamGroup] = true
                    neededGroups_clients[out.tcamGroup] = client
                end
            end--not 'hard wired'
        end
    end

    --print("neededGroups=" , to_string(neededGroups))
    --print("#neededGroups=" , to_string(#neededGroups))
    local isMultipleGroups = false
    local clients_blocks = {}
    local blocksPerFloor =  nil
    if neededGroups ~= {} then
        local tcamInfo
        tcamInfo, blocksPerFloor = tcamBlocksUsageInfo(devNum)
        
        --print("tcamInfo", to_string(tcamInfo))
        --print("blocksPerFloor", to_string(blocksPerFloor))
        
        blocks = {}
        
        if not tcamInfo then
            return nil, errMsg
        end

        for floor, blocksPerFloor in pairs(tcamInfo) do
            for block, info in pairs(blocksPerFloor) do
                --print("block",to_string(block))
                --print("info",to_string(info))
            
                if  neededGroups[info.group] and ((hitNum == nil)  or (hitNum  == info.hitNum)) then
                    -- get 'per client'
                    local client = neededGroups_clients[info.group]
                    if (#neededGroups > 1) then
                        isMultipleGroups = true
                        if(clients_blocks[client] == nil) then
                            clients_blocks[client] = {}
                        end
                        -- per (group) that represents client(s)
                        blocks = clients_blocks[client]
                    else
                        -- aggregation
                        blocks = clients_blocks
                    end
                
                    if blocks[floor] == nil then
                        blocks[floor] =  {}
                    end
                    table.insert(blocks[floor], block)
                end
            end
        end
    end
    return clients_blocks, blocksPerFloor , isMultipleGroups
end


-- ************************************************************************
--  tcamClientsChunksGet
--
--  @description get information about TCAM blocks used by specified
--               clients.
--
--  @param devNum - a device number
--  @param clients - can be a single value of arrays of
--                   'tti', 'pcl', 'ipcl0', 'ipcl1', 'ipcl2', 'ipcl3', 'epcl'
--                   'pcl' means get chunks relevant to all clients
--                   IPCL0, IPC1, IPC2, EPCL.
--  @param hitNum - a hitNum. if nil  blocks with any hitNum will be return
--
--  @return array of chunks: {{base=<base>, size=<size>}, ...}
--
-- ************************************************************************
function tcamClientsChunksGet(devNum, clients, hitNum)
    local  blocks, blocksPerFloor , isMultipleGroups = tcamClientBlocksGet(devNum, clients, hitNum)
    if blocks == nil then
        return  nil, blocksPerFloor -- nil  and  error message
    end

    --print("clients ".. clients .." ::: blocks : ".. to_string(blocks))

    if(isMultipleGroups) then
        local multipleGroupsChunks = {}
        for index, entry in pairs(blocks) do
            multipleGroupsChunks[index] = blocksToChunks(entry, blocksPerFloor)
        end
        
        --print("multipleGroupsChunks",to_string(multipleGroupsChunks))
        return multipleGroupsChunks
    else
        return blocksToChunks(blocks, blocksPerFloor)
    end
end

