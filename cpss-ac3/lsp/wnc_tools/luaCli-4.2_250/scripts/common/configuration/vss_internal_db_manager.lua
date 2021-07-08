--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vss_internal_db_manager.lua
--*
--* DESCRIPTION:
--*     manage 'internal-db' dedicated for the 'vss of bc2-ac3'
--*     according to doc:
--*     http://docil.marvell.com/webtop/drl/objectId/0900dd88801a06b3
--*       cc – Control Card device --  Bc2
--*       lc – line-card device - Ac3
--*	    Native traffic received on AC3 LC are sent with 8B Extended DSA to BC2
--*	    BC2 transmits 16B eDSA packets to AC3 line cards
--*	    Cross platform VSS traffic
--*	    AC3 passes eDSA packets transparently over the VSS interface to remote platform
--*	    AC3 passes eDSA packets received over VSS to BC2
--*	    BC2 device learns MAC address on remote platform based on eDSA <Src ePort>
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

-- debug utility
local debug_on = false
local function _debug(...)
    if debug_on == true then
        printLog (...)
    end
end
-- extern function _debug_to_string(...)
-- 'on lc' reserved pcli-id for epcl for traffic out to egress vss ports
vss_internal_db_on_lc_used_for_vss_ports_epcl_pcl_id = 0x332
-- 'on lc' reserved pcli-id for ipcl for traffic from 'uplink'
vss_internal_db_on_lc_used_uplink_ipcl_pcl_id = 0x333
-- 'on lc' reserved src-d : needed to flood from vss trunk port to the uplink trunk and not to other ports
vss_internal_db_on_lc_used_src_id_for_vss_trunk = 31


local curr_tbl

--[[
-- NOTE:
--  prefix 'on_cc' means that the part of the db is for the 'cc' only
--  prefix 'on_lc' means that the part of the db is for the 'lc' only

the format of DB : vss_internal_db_global
    1.	my_role
    2.	my_name
    3.	my_vss_id
    9.  my_hw_devNum
    4.	global_eport_range
        a.	start_eport
        b.	end_eport
    5.	other_vss[vss-name]  -- array on vss systems
        a.	vss_id
        b.	vss_trunk_id
        c.	connections[the_dev_name_in_local_vss]  –> array of devices connecting the other vss to local vss
            i.	ports[portNum]   –> array of port on the device connecting the other vss to the device
            ii.	on_lc_dummy_hw_dev_num_for_vss_trunk
    6.	neighbors[neighbor-name] – array of local vss neighbors
        a.	on_cc_info
            i.	role – ‘lc’
            ii.	connection – one of the next is valid : port/trunk
                1.	portNum
                2.	trunkId
    7.	on_lc_info
        a.	uplink
            i.	connection – one of the next is valid : port/trunk
                1.	portNum
                2.	trunkId
                3.  the_neighbor_device_name - the name of the neighbor device
            ii.	pcl_id – pcl-id for IPCL rules
            iii. src_id_for_vss_trunk -- needed to flood from vss trunk port to the uplink trunk and not to other ports
        b.	physical_port_info[physical_port] –array of physical ports
            i.	eport_on_cc - the eport on CC that represent current lc physical port
            ii.	source_id = current_port + 1 ? value 0 not to be used for ports
        c.	eports_aggregator[] – array of aggregated ‘eports’ (ports of LC represented as eports on the CC) –
                this aggregation acts as ‘user-trunk’ with special up to total 8 members (should be those for LB of mc traffic).
            i.	members[] – add member as ‘last’ (if not already exist) order important !
                1.	neighbor_vss_name
                2.	lc_name
                3.	lc_port
            ii.	source_id – source_id equal to the source_id of ‘first’ ‘own uc member’
            iii. trunk_id - trunk-id to manage the 'MC designated table' table.
                            the device not really use the 'trunk-id' , but the 'MC designated table' used for flood in
                            the vlan/vidx to the aggregation members
        d.	epcl_pcl_id – pcl-id for EPCL rules
        e.  members_in_aggregations[] - array of ports that are members of eports_aggregator
            i member
                1.	neighbor_vss_name
                2.	lc_name
                3.	lc_port
            ii . eport_aggregator
    8.	on_cc_info
        a.	eports[the_eport] -- array of eports
            i.	lc_name
            ii.	lc_portNum
            iii.	aggregated_eports[secondary_eport]= secondary_eport ? array for ‘global-eport’ that hold ‘multi-secondary-eports’ (L2 ECMP)
--]]
vss_internal_db_global = {
     my_role = nil
    ,my_name = nil
    ,global_eport_range =
    {
         start_eport = vss_global_eport_range_start_eport
        ,end_eport   = vss_global_eport_range_end_eport
    }
    ,other_vss = {} -- array of devices connecting the other vss to local vss
    ,neighbors = {} -- array of local vss neighbors
    ,on_lc_info =
    {
        uplink =
        {
             connection = {
                 portNum = nil
                ,trunkId = nil
                ,the_neighbor_device_name = nil
                }
            ,pcl_id = vss_internal_db_on_lc_used_uplink_ipcl_pcl_id
            ,src_id_for_vss_trunk = vss_internal_db_on_lc_used_src_id_for_vss_trunk
        }
        ,physical_port_info = {} --array of physical ports
        ,eports_aggregator = {} -- array of aggregated ‘eports’ (ports of LC represented as eports on the CC) –
                                 -- this aggregation acts as ‘user-trunk’ with special up to total 8 members (should be those for LB of mc traffic).
        ,epcl_pcl_id = vss_internal_db_on_lc_used_for_vss_ports_epcl_pcl_id
        ,members_in_aggregations = {} -- array of port that are members of eports_aggregator{}
    }
    ,on_cc_info = {
        eports = {}, -- array of eports
        --[[ eport representing LC port inside
            [eport] =
            {
                lc_name="lc1",
                lc_portNum=4
                aggregate_eport = 0x1011 -- if member of aggregation
            }
        --]]
        --[[ aggregate_eports elements inside:
            [aggregate_eport] =
            {
                L2ECMP_entries = {dummy_index_0_31 = {eport = 0, neighbor_vss_name = "cc2"}},
                L2ECMP_base = 0, L2ECMP_allocated_size = 32
            }
        --]]
    }
}

-- convention : all 'global function' must start with prefix : vss_global_func_

-- convert port number to src-id
function vss_global_func_convert_port_to_src_id(portNum)
    -- make sure that src-id is unique per port and is not 0
    return portNum + 1
end

--[[
    debug 'show' commands to see the internal DB of the VSS manager
--]]
CLI_addHelp("debug", "show vss", "show VSS related info")
CLI_addHelp("debug", "show vss internal-db", "show VSS internal DB managed by the LUA commands")

CLI_addCommand("debug", "show vss internal-db", {
    func = function(params)

        print ("\n start the VSS internal DB")
        print ("vss_internal_db_global = " , to_string(vss_internal_db_global))
        print ("end the VSS internal DB \n")

    end,
    help="Dump internal DB of VSS"
})

---------------------------------------
---------------------------------------
--[[-------------------------------------
-- general manager of (simple) resources
-- like tables in the HW that each entry
-- no depended on other entries
---------------------------------------
the list of 'APIs' of the manager:
    -- init the DB of the table
    function general_resource_manager_init(table_name,startIndex,endIndex)

    -- indication that a specific index in the table need to be used
    -- if key_name omitted --> the manager gives internal 'key' to the index --> key = table_name .. " index = " .. index
    function general_resource_manager_use_specific_index(table_name,index,key_name)

    -- indication that a specific index in the table not used any more
    -- if key_name omitted --> the manager gives internal 'key' to the index --> key = table_name .. " index = " .. index
    function general_resource_manager_free_specific_index(table_name,index,key_name)

    -- return the 'index' that is associated with the 'key_name' in the 'table_name'
    function general_resource_manager_entry_get(table_name,key_name)

    -- check if 'key_name' used in the 'table_name'
    function general_resource_manager_entry_is_used_get(table_name,key_name)

    -- return the 'index' that is associated with the 'key_name' in the 'table_name'
    -- but in limited range (not from full range if the table)
    function general_resource_manager_entry_get_in_limited_range(table_name,key_name,limited_start_index,limited_end_index)

    -- release the DB resources of entry in the table
    function general_resource_manager_release_entry(table_name,key_name)

    -- release the DB resources of the table
    function general_resource_manager_release_table(table_name)

    -- get the list of used 'indexes'
    function general_resource_manager_get_used_indexes(table_name)

    -- get the 'key_name' by 'index'
    function general_resource_manager_get_key_by_index(table_name,index_in_table)

    -- dump the table info
    function general_resource_manager_dump_table(table_name)

---------------------------------------
--]]-------------------------------------
-- list known tables
general_resource_manager_trunk_table = "trunk_table"
local known_tables_to_init = {
    -- table_name                           startIndex , endIndex
    [general_resource_manager_trunk_table] = {1,          511}
    -- add here additional tables ...
}


local function internal_key_from_index(table_name,index)
    return table_name .. " index = " .. index
end

-- general resource manager
-- the 'table' we get from the 'caller
local manager = {}

-- init the DB of the table
function general_resource_manager_init(table_name,startIndex,endIndex)
    if manager[table_name] ~= nil then
        manager[table_name] = nil
    end

    manager[table_name] = {startIndex = startIndex , endIndex = endIndex , usedEntriesArr = {}}

end

-- indication that a specific index in the table need to be used
-- if key_name omitted --> the manager gives internal 'key' to the index --> key = table_name .. " index = " .. index
function general_resource_manager_use_specific_index(table_name,index,key_name)

    curr_tbl = manager[table_name]

    if key_name == nil then
        key_name = internal_key_from_index(table_name,index)
    end

    curr_tbl.usedEntriesArr[key_name] = index
end

-- indication that a specific index in the table not used any more
-- if key_name omitted --> the manager gives internal 'key' to the index --> key = table_name .. " index = " .. index
function general_resource_manager_free_specific_index(table_name,index,key_name)
    curr_tbl = manager[table_name]

    if key_name == nil then
        key_name = internal_key_from_index(table_name,index)

        if not curr_tbl.usedEntriesArr[key_name] then
            -- check the possibility that the entry was added 'by name' and not by index !!!
            for ii,entry in pairs(curr_tbl.usedEntriesArr) do
                if index == entry then
                    key_name = ii -- found it !!! --> was added 'by name' and we remove it 'by index' !!!
                    break
                end
            end
        end
    end
    
    --print("curr_tbl : ",to_string(curr_tbl))
    --print("table - free index: " ,to_string(table_name) , to_string(index) , to_string(key_name))

    curr_tbl.usedEntriesArr[key_name] = nil
end

-- return the 'index' that is associated with the 'key_name' in the 'table_name'
-- but in limited range (not from full range if the table)
function general_resource_manager_entry_get_in_limited_range(table_name,key_name,limited_start_index,limited_end_index)
    curr_tbl = manager[table_name]

    local startIndex, endIndex
    
    if curr_tbl.usedEntriesArr[key_name] then
        -- already have index for this key
        --print("table - already exists index: " ,to_string(table_name) , to_string(key_name) , to_string(curr_tbl.usedEntriesArr[key_name]))

        return curr_tbl.usedEntriesArr[key_name]
    end

    if limited_start_index == nil then
        startIndex = curr_tbl.startIndex
    else
        startIndex = limited_start_index
    end

    if limited_end_index == nil then
        endIndex = curr_tbl.endIndex
    else
        endIndex = limited_end_index
    end

    -- need new index
    for ii = startIndex, endIndex do
        index_used = false
        for index,entry in pairs(curr_tbl.usedEntriesArr) do
            if ii == entry then
                index_used = true
                break
            end
        end

        if index_used == false then
            curr_tbl.usedEntriesArr[key_name] = ii

            --print("table - new index: " ,to_string(table_name) , to_string(key_name) , to_string(curr_tbl.usedEntriesArr[key_name]))
            return ii
        end
    end

    _debug_to_string("the table".. table_name .."is full , for new key " .. key_name .. "(range[".. startIndex .. ".." ..  endIndex .."])")
    _debug_to_string("manager[table_name] = ", manager[table_name])
    -- the table is full ?!
    return nil
end

-- return the 'index' that is associated with the 'key_name' in the 'table_name'
function general_resource_manager_entry_get(table_name,key_name)
    return general_resource_manager_entry_get_in_limited_range(table_name,key_name)
end

-- check if 'key_name' used in the 'table_name'
-- return : true  - key_name used
--          false - key_name not used
function general_resource_manager_entry_is_used_get(table_name,key_name)
    curr_tbl = manager[table_name]
    
    return not (curr_tbl.usedEntriesArr[key_name] == nil)
end
    
-- release the DB resources of entry in the table
function general_resource_manager_release_entry(table_name,key_name)
    curr_tbl = manager[table_name]

    if curr_tbl.usedEntriesArr[key_name] == nil then
        return 
    end
    
    -- free the entry
    curr_tbl.usedEntriesArr[key_name] = nil
    
end

-- release the DB resources of the table
local function general_resource_manager_release_table(table_name)
    manager[table_name] = nil
end

-- get the list of used 'indexes'
local function general_resource_manager_get_used_indexes(table_name)
    return curr_tbl.usedEntriesArr
end

-- get the 'key_name' by 'index'
function general_resource_manager_get_key_by_index(table_name,index_in_table)
    curr_tbl = manager[table_name]
    --print("table - reset index: " ,to_string(table_name) , to_string(index_in_table))

    for key_name,entry in pairs(curr_tbl.usedEntriesArr) do
        --print("key_name,entry: " ,to_string(key_name) , to_string(entry))

        if index_in_table == entry then
            return key_name 
        end
    end
end
-- dump the table info
local function general_resource_manager_dump_table(table_name)
    if manager[table_name] == nil then
        print ("The table " .. to_string(table_name) .. " not exists in the DB ")
        return
    end

    print ("\n start entries for " .. table_name)

    print(to_string(manager[table_name]))

    print ("\n end entries for " .. table_name)
end

-- init the known tables 'predefined tables'
function general_resource_manager_init_pre_defined_tables()
    for index,entry in pairs(known_tables_to_init)do
        general_resource_manager_init(index,entry[1],entry[2])
    end
end


--[[
    debug 'show' commands to see the internal DB of 'general resource manager tables'
--]]
CLI_addHelp("debug", "show general-resource-manager", "show general resource manager tables")

CLI_addCommand("debug", "show general-resource-manager names", {
    func = function(params)

        local ii = 1
        print ("\n start the names in 'general-resource-manager' ")
        for index,entry in pairs(manager) do
            print("index = " .. ii .. " name = " .. to_string(index) )
            ii = ii + 1
        end
        print ("\n end the names in 'general-resource-manager' ")

    end,
    help="show the names of tables managed by the general manager"
})
CLI_addCommand("debug", "show general-resource-manager all-tables", {
    func = function(params)

        print ("\n start the tables in 'general-resource-manager' ")
        for index,entry in pairs(manager) do
            general_resource_manager_dump_table(index)
        end
        print ("\n end the tables in 'general-resource-manager' ")

    end,
    help="show the  content in all tables managed by the general manager"
})
CLI_addCommand("debug", "show general-resource-manager specific-table", {
    func   = function(params)
        general_resource_manager_dump_table(params.tableName)
    end
    ,
    help   = "Bind PCID to 'Extended port' (current local interface) in BPE system",
    params = {
        { type = "values",
            { format = "%s",    name = "tableName",    help = "The specific table name"}
        }
    }
})
