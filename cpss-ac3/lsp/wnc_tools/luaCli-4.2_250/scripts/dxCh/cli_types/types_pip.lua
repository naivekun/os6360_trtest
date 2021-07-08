--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* types_pip.lua
--*
--* DESCRIPTION:
--*       define types and DB for PIP.
--*       Pre-Ingress Prioritization (PIP)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[
--********************************************************************************
        types
--********************************************************************************
    -- the interfaces that the PIP supports (only ethernet)
    pip_supported_interfaces

--********************************************************************************
        DB utilities
--********************************************************************************
    -- init pip DB manager
    function pip_db_manager_init()
---------
    -- get the index for the {ethertypeString,vid}
    function pip_db_vid_index_get(ethertypeString,vid)
    
    -- check that the {ethertypeString,vid} already exists 
    function pip_db_vid_index_check(ethertypeString,vid)

    -- reset the {ethertypeString,vid} from DB
    function pip_db_vid_index_reset(ethertypeString,vid)
---------
    -- get the index for the ude of ethertypeString
    function pip_db_ude_index_get(ethertypeString)
    
    -- check if ude of ethertypeString already exists
    function pip_db_ude_index_check(ethertypeString)

    -- get the ethertypeString used at specific index 
    function pip_db_ude_ethertype_get(index)
    
    -- reset the index used for the ude of ethertypeString
    function pip_db_ude_ethertype_index_reset(index)
---------
    -- set the index of the 'ude-name'
    function pip_db_ude_name_index_set(ude_name,index)
    
    -- get the index of the 'ude-name'
    function pip_db_ude_name_index_get(ude_name)

    -- get by index the 'ude-name'
    pip_db_ude_name_by_index_get(index)

    -- reset the index of the 'ude-name'
    function pip_db_ude_name_reset(ude_name)

    -- add device to the 'ude-name'
    function pip_db_ude_name_device_add(ude_name,devNum)

    -- get the device number bound to 'ude-name'
    function pip_db_ude_name_device_num_get(ude_name)

    -- get the device id at specific index (index is 1 based) to 'ude-name'
    function pip_db_ude_name_device_id_get(ude_name,index)
    
    -- get index of device in 'ude-name'
    function pip_db_ude_name_device_index_get(ude_name,devNum)
---------
    -- get the index for the mac-da {pattern,mask} 
    function pip_db_mac_da_index_get(macAddrString,macAddrMaskString)

    -- get the mac-da {pattern,mask} already exists
    function pip_db_mac_da_index_check(ethertypeString)
    
    -- get the macDa entry {pattern,mask} used at specific index 
    function pip_db_mac_da_entry_get(index)

    -- reset the index used for the  macDa entry {pattern,mask}
    function pip_db_mac_da_index_reset(index)
---------
    -- set the index of the 'mac_da-name'
    function pip_db_mac_da_name_index_set(mac_da_name,index)

    -- get the index of the 'mac_da-name'
    function pip_db_mac_da_name_index_get(mac_da_name)

    -- get by index the 'mac_da-name'
    pip_db_mac_da_name_by_index_get(index)

    -- reset the index of the 'mac_da-name'
    function pip_db_mac_da_name_reset(mac_da_name)

    -- add device to the 'mac_da-name'
    function pip_db_mac_da_name_device_add(mac_da_name,devNum)
    
    -- get the device number bound to 'mac_da-name'
    function pip_db_mac_da_name_device_num_get(mac_da_name)
    
    -- get the device id at specific index (index is 1 based) to 'mac_da-name'
    function pip_db_mac_da_name_device_id_get(mac_da_name,index)

    -- get index of device in 'mac_da-name'
    function pip_db_mac_da_name_device_index_get(mac_da_name,devNum)
    
]]--


--##################################
--##################################
--##################################
--##################################
--##################################
--##################################
-- check if the device hold IPv6 TC erratum that limit it's TC range from 0..255 to 0..63
function pip_is_device_ipv6_erratum(devNum)
    if devNum == "all" then
        local all_devices = wrLogWrapper("wrlDevList")
        for index, currDevNum in pairs(all_devices) do
            if pip_is_device_ipv6_erratum(currDevNum) == true then
                -- at least one of the devices hold the erratum
                return true
            end
        end
        -- no device of the 'all' is with the erratum
        return false
    end 

    if is_sip_5_16(devNum) then
        return false
    end

    -- single device
    return is_sip_5_10(devNum)
end

-- limit due to erratum instead of 255
pip_type_max_value_ipv6_erratum = 63

pip_ipv6_erratum_string = "Due to Erratum only 6 LSBits of the TC considered (instead of 8)."

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################
-- the interfaces that the PIP supports (only ethernet)
--[[local]] pip_supported_interfaces = {ethernet = true}

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################
--[[local]] pip_help_vlan_tag_ethertype = "The ethertype to classify packet as 'vlan tag' . format : 'HEX' number without '0x' (8100 / 88a8)"

CLI_type_dict["pip_vlan_tag_ethertype"] = {
    checker = CLI_check_param_hexstring,
    min=4,
    max=4,
    help=pip_help_vlan_tag_ethertype
}

--[[local]] pip_help_generic_ethertype = "The ethertype to classify packet as 'generic' . format : 'HEX' number without '0x' (6666/abcd/789a)"

CLI_type_dict["pip_generic_ethertype"] = {
    checker = CLI_check_param_hexstring,
    min=4,
    max=4,
    help=pip_help_generic_ethertype
}

--[[local]] pip_help_pip_profile = "one of 4 pip-profiles (0..3) , support 'range'"
CLI_type_dict["pip_profile"] = {
    checker = CLI_check_param_number_range,
    min=0,
    max=3,
    complete = CLI_complete_param_number_range,
    help=pip_help_pip_profile
}

--[[local]] DSA_QOS_PROFILE =  "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_QOS_PROFILE_E"
--[[local]] DSA_UP          =  "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_DSA_UP_E"
--[[local]] VLAN_TAG_UP     =  "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_VLAN_TAG_UP_E"
--[[local]] MPLS_EXP        =  "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MPLS_EXP_E"
--[[local]] IPV4_TOS        =  "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV4_TOS_E"
--[[local]] IPV6_TC         =  "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_IPV6_TC_E"
--[[local]] UDE_INDEX       =  "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_UDE_INDEX_E"
--[[local]] MAC_DA_INDEX    =  "CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_MAC_DA_INDEX_E"

-- the max index allowed for the 'field' for those 'types'
pip_type_max_value = {
    [DSA_QOS_PROFILE]   = 127,
    [DSA_UP]            = 7,
    [VLAN_TAG_UP]       = 7,
    [MPLS_EXP]          = 7,
    [IPV4_TOS]          = 255,
    [IPV6_TC]           = 255,
    [UDE_INDEX]         = 3,
    [MAC_DA_INDEX]      = 3
}

--[[local]] pip_type_enum = {
        ["dsa-qos"]     = { value=DSA_QOS_PROFILE, help="use <qos profile> from DSA"   },
        ["dsa-up"]      = { value=DSA_UP, help="use <UP> from DSA"   },
        ["vlan-tag-up"] = { value=VLAN_TAG_UP, help="use <UP> from vlan tag (most outer)" },
        ["mpls-exp"]    = { value=MPLS_EXP, help="use <EXP> from mpls tag (most outer)" },
        ["ipv4-tos"]    = { value=IPV4_TOS, help="use <TOS> from ipv4 header"},
        ["ipv6-tc"]     = { value=IPV6_TC, help="use <TC> from ipv6 header" },
        ["user-defined-ethertype"]    = { value=UDE_INDEX, help="use one of user-defined-ethertypes"},
        ["mac-da"]    = { value=MAC_DA_INDEX, help="use one of mac-da {pattern,mask}" },
        
        ["all"]     = {value="all" , help="all of the above"},
   }
   
--[[local]] pip_help_pip_type = "select one of the packet classifications : dsa-tag/vlan-tag/mpls/ipv4/ipv6/ude/mac-da"
CLI_type_dict["pip_type_enum"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = help_pip_type,
    enum = pip_type_enum
}    

local pip_help_pip_vid_value = "Specify IEEE 802.1Q VLAN ID (vid 0 for restore/remove 'HW defaults')"
CLI_addParamDictAndType_number("pip-vid-value","vid",pip_help_pip_vid_value, 0,4095)

local pip_help_all_named_indexes = "all 4 indexes (regardless to names)"
local pip_field_mac_da_names_enum = {}
local function pip_fill_pip_field_mac_da_names_enum()
    local enum_size = 0
    pip_field_mac_da_names_enum = {}--reset the enum
    for index = 0,3 do
        local name = pip_db_mac_da_name_by_index_get(index)
        if name then
            pip_field_mac_da_names_enum[name] = {value=name , help="known mac-da name"}
        end
    end
    
    pip_field_mac_da_names_enum["all"] = {value="all" , help=pip_help_all_named_indexes}
end
local pip_field_ude_names_enum = {}
local function pip_fill_pip_field_ude_names_enum()
    local enum_size = 0
    pip_field_ude_names_enum = {}--reset the enum
    for index = 0,3 do
        local name = pip_db_ude_name_by_index_get(index)
        if name then
            pip_field_ude_names_enum[name] = {value=name , help="known UDE name"}
        end
    end
    pip_field_ude_names_enum["all"] = {value="all" , help=pip_help_all_named_indexes}
end

local function CLI_check_param_pip_field_mac_da(param,name,desc,varray,params)
    if params.pip_type ~= MAC_DA_INDEX then
        return false,""
    end

    pip_fill_pip_field_mac_da_names_enum()
    local my_dictionary = {enum = pip_field_mac_da_names_enum}
    local my_name = "mac-da name"
    return CLI_check_param_enum(param,my_name,my_dictionary,varray,params)
end
    
local function CLI_check_param_pip_field_ude(param,name,desc,varray,params)
    if params.pip_type ~= UDE_INDEX then
        return false,""
    end

    pip_fill_pip_field_ude_names_enum()
    local my_dictionary = {enum = pip_field_ude_names_enum}
    local my_name = "ude name"
    return CLI_check_param_enum(param,my_name,my_dictionary,varray,params)
end

local function CLI_check_param_pip_field_number(param,name,desc,varray,params)
    if params.pip_type ~= UDE_INDEX and
       params.pip_type ~= MAC_DA_INDEX 
    then
        if pip_type_max_value[params.pip_type] then
            local my_dictionary = {min = 0 , max = pip_type_max_value[params.pip_type]}
            return CLI_check_param_number_range(param,name,my_dictionary,varray,params)
        end
    end

    return false,""
end

local function CLI_check_param_pip_field____all(param,name,desc,varray,params)
    if params.pip_type == "all" and param == "all" then
        --filled by command
        return true,{"all"}
    end
    return false,""    
end

local function CLI_complete_param_pip_field_mac_da(param,name,desc,varray,params)
    if params.pip_type ~= MAC_DA_INDEX then
        return {},{}
    end

    pip_fill_pip_field_mac_da_names_enum()
    local my_dictionary = {enum = pip_field_mac_da_names_enum}
    local my_name = "mac-da name"
    local ret1,ret2 =  CLI_complete_param_enum(param,my_name,my_dictionary,varray,params)
    if #ret1 == 0 then
        CLI_type_dict["pip_field"].help = "one of mac-da names but no 'name' was defined yet"
    end
    
    return ret1,ret2
end

local function CLI_complete_param_pip_field_ude(param,name,desc,varray,params)
    if params.pip_type ~= UDE_INDEX then
        return {},{}
    end

    pip_fill_pip_field_ude_names_enum()
    local my_dictionary = {enum = pip_field_ude_names_enum}
    local my_name = "ude name"
    local ret1,ret2 = CLI_complete_param_enum(param,my_name,my_dictionary,varray,params)
    
    if #ret1 == 0 then
        CLI_type_dict["pip_field"].help = "one of UDE names but no 'name' was defined yet"
    end
    
    return ret1,ret2
end

local function CLI_complete_param_pip_field_number(param,name,desc,varray,params)
    if params.pip_type ~= UDE_INDEX and
       params.pip_type ~= MAC_DA_INDEX 
    then
        if pip_type_max_value[params.pip_type] then
            local my_dictionary = {min = 0 , max = pip_type_max_value[params.pip_type]}
            local ret1,ret2 =  CLI_complete_param_number_range(param,name,my_dictionary,varray,params)

            --print("ret1" , to_string(ret1),"ret2",to_string(ret2))

            if params.pip_type == IPV6_TC and true == pip_is_device_ipv6_erratum(params.all_device) then
                -- the device hold erratum
                ret2.def = ret2.def .. ".\n IMPORTANT NOTE : " .. pip_ipv6_erratum_string
            end
            
            CLI_type_dict["pip_field"].help = ret2.def

            return ret1,ret2
        end
    end
    return {},{}
end
local function CLI_complete_param_pip_field____all(param,name,desc,varray,params)
    if params.pip_type == "all" then
        return {"all" , "all values supported by all types"}
    end
    return {},{}
end
local  new_line_plus_tab = "\n\t\t"

--[[local]] help_pip_field = 
            "(support range : all or 5 or 1-3,6 or 1,2-4,6)" .. new_line_plus_tab ..
            "the field value : "                        .. new_line_plus_tab ..
            "all full range of field"                   .. new_line_plus_tab ..
			"for dsa-qos , value = 0..127"              .. new_line_plus_tab ..
			"for dsa-up , value = 0..7"                 .. new_line_plus_tab ..
			"for vlan-tag-up, value = 0.. 7"            .. new_line_plus_tab ..
			"for mpls-exp, value = 0..7"                .. new_line_plus_tab ..
			"for ipv4-tos, value = 0..255"              .. new_line_plus_tab ..
			"for ipv6-tc, value = 0..255"               .. new_line_plus_tab ..
			"for user-defined-ethertype : one of its 'names'" .. new_line_plus_tab ..
			"for mac-da : one of its 'names'"

CLI_type_dict["pip_field"] = {
    checker  = {CLI_check_param_pip_field_number      , CLI_check_param_pip_field_mac_da     , CLI_check_param_pip_field_ude     , CLI_check_param_pip_field____all},
    complete = {CLI_complete_param_pip_field_number   , CLI_complete_param_pip_field_mac_da  , CLI_complete_param_pip_field_ude , CLI_complete_param_pip_field____all },
    --min=0,
    --max=255,
    help=help_pip_field --[[actually set in runtime by proper 'complete' function]]
}    

 
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################


local pip_vid_index_db_name = nil
local pip_vid_hw_default = 0
--[[local]] pip_tpid_hw_default = {[0]="0x8100",[1]="0x88A8",[2] = "0x8100" , [3]= "0x8100"}
--[[local]] pip_ude_default = 0x0800
local pip_mac_da_default = "00:00:00:00:00:00"

-- DB to convert UDE-ethertype to index
local pip_ude_index_db_name = "pip_ude_index_db_name"
-- DB to convert UDE-name to {index}
local pip_ude_db = {
    --[[ ude_name = index , devices = {list}}]]}

-- DB to convert mac-da {pattern,mask} to index
local pip_mac_da_index_db_name = "pip_mac_da_index_db_name"
-- DB to convert mac-da-name to {index}
local pip_mac_da_db = {
    --[[ mac_da_name = 
        {index = index , devices = {list}}]]}

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

local function pip_vid_key_name_get(ethertypeString,vid)
    return "ethertype=" .. ethertypeString .. ",vid=" .. tostring(vid)
end

-- init pip DB manager
function pip_db_manager_init()
    if pip_vid_index_db_name then 
        -- already initialized
        return
    end

    pip_vid_index_db_name = "pip_vid_index_db_name"

    local key_name
    local index
    local vid = pip_vid_hw_default
    -- manager for pip vid
    general_resource_manager_init(pip_vid_index_db_name,0,3)
        -- state the entries that already in HW
        index = 0
        key_name = pip_vid_key_name_get(pip_tpid_hw_default[index],vid)
        general_resource_manager_use_specific_index(pip_vid_index_db_name,index, key_name)
        index = 1
        key_name = pip_vid_key_name_get(pip_tpid_hw_default[index],vid)
        general_resource_manager_use_specific_index(pip_vid_index_db_name,index, key_name)
        
    -- manager for pip UDE
    general_resource_manager_init(pip_ude_index_db_name,0,3)

    -- manager for pip mac-da
    general_resource_manager_init(pip_mac_da_index_db_name,0,3)

    
end

-- get the index for the {ethertypeString,vid}
-- function returns 2 indications:
-- index            - new index / existing index for {ethertypeString,vid}
-- isAlreadyExists  - is {ethertypeString,vid} was already exists
function pip_db_vid_index_get(ethertypeString,vid)
    local table_name = pip_vid_index_db_name
    local key_name = pip_vid_key_name_get(ethertypeString,vid)
    -- check existence before adding the entry
    local isAlreadyExists = general_resource_manager_entry_is_used_get(table_name,key_name)
    -- get index of the new/existing entry
    local index = general_resource_manager_entry_get(table_name,key_name)

    return index , -- can be nil --> the {ethertypeString,vid} not found and no place for it
           isAlreadyExists
end

-- check that the {ethertypeString,vid} already exists 
-- function returns :
-- isAlreadyExists  - is {ethertypeString,vid} exists
function pip_db_vid_index_check(ethertypeString,vid)
    local table_name = pip_vid_index_db_name
    local key_name = pip_vid_key_name_get(ethertypeString,vid)
    -- check existence before adding the entry
    local isAlreadyExists = general_resource_manager_entry_is_used_get(table_name,key_name)
    
    return isAlreadyExists
end

-- reset the {ethertypeString,vid} from DB
function pip_db_vid_index_reset(ethertypeString,vid)
    local table_name = pip_vid_index_db_name
    local key_name = pip_vid_key_name_get(ethertypeString,vid)

    local isAlreadyExists = general_resource_manager_entry_is_used_get(table_name,key_name)
    
    if isAlreadyExists then
        -- keep DB with those defaults as in HW
        local new_vid = pip_vid_hw_default
        local existing_index = general_resource_manager_entry_get(table_name,key_name)
        local newEthertypeString = pip_tpid_hw_default[existing_index]
        
        -- we try to remove the 2 entries that need to be restored to HW defaults.
        local new_key_name = pip_vid_key_name_get(pip_tpid_hw_default[existing_index],new_vid)
        
        if existing_index < 2   and 
            (new_key_name ~= key_name)
        then 
            general_resource_manager_use_specific_index(table_name,existing_index, new_key_name)
            return--we are done
        end
    end

    -- reset the {ethertypeString,vid} from DB
    general_resource_manager_release_entry(table_name,key_name)
    
    
end
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

-- get the index for the ude of ethertypeString
-- function returns 2 indications:
-- index            - new index / existing index for {ethertypeString}
-- isAlreadyExists  - is {ethertypeString} was already exists
function pip_db_ude_index_get(ethertypeString)
    local table_name = pip_ude_index_db_name
    local key_name = ethertypeString
    -- check existence before adding the entry
    local isAlreadyExists = general_resource_manager_entry_is_used_get(table_name,key_name)
    -- get index of the new/existing entry
    local index = general_resource_manager_entry_get(table_name,key_name)

    if(isAlreadyExists) then
        --print("get index for " , ethertypeString , to_string(index))
    else
        --print("add index for " , ethertypeString , to_string(index))
    end
    
    return index , -- can be nil --> the {ethertypeString} not found and no place for it
           isAlreadyExists
end

-- check if ude of ethertypeString already exists
-- function returns indication:
-- isAlreadyExists  - is {ethertypeString} was already exists
function pip_db_ude_index_check(ethertypeString)
    local table_name = pip_ude_index_db_name
    local key_name = ethertypeString
    -- check existence 
    return general_resource_manager_entry_is_used_get(table_name,key_name)
end

-- get the ethertypeString used at specific index 
function pip_db_ude_ethertype_get(index)
    local table_name = pip_ude_index_db_name
    -- check existence before adding the entry
    local key_name = general_resource_manager_get_key_by_index(table_name,index)

    --print("get ethertypeString for " ,to_string(index) , to_string(key_name))
    
    local ethertypeString = key_name
    
    return ethertypeString
end

-- reset the index used for the ude of ethertypeString
function pip_db_ude_ethertype_index_reset(index)
    local table_name = pip_ude_index_db_name
    
    --print("reset index " ,to_string(index))

    -- reset the index
    general_resource_manager_free_specific_index(table_name,index)
end
--##################################
--##################################
--##################################
--##################################
--##################################
--##################################


-- set the index of the 'ude-name'
function pip_db_ude_name_index_set(ude_name,index)
    if pip_ude_db[ude_name] == nil then
        pip_ude_db[ude_name] = {}
    end
    pip_ude_db[ude_name].index = index
end
-- get the index of the 'ude-name'
-- function returns 2 indications:
-- index            - existing index (or nil) for 'ude-name'
function pip_db_ude_name_index_get(ude_name)
    if pip_ude_db[ude_name] == nil then
        return nil
    end
    return pip_ude_db[ude_name].index
end

-- get by index the 'ude-name'
-- function returns 1 indication:
-- name            - 'ude-name' or nil
function pip_db_ude_name_by_index_get(index)
    for _index,entry in pairs(pip_ude_db) do
        if entry.index == index then
            return _index
        end
    end
    return nil
end

-- reset the index of the 'ude-name'
function pip_db_ude_name_reset(ude_name)
    pip_ude_db[ude_name] = nil
end
-- add device to the 'ude-name'
function pip_db_ude_name_device_add(ude_name,devNum)
    if pip_ude_db[ude_name] == nil then
        pip_ude_db[ude_name] = {}
    end
    if(pip_ude_db[ude_name].devices == nil) then
        pip_ude_db[ude_name].devices = {}
        pip_ude_db[ude_name].device_index = 1
    end
    local index = pip_ude_db[ude_name].device_index
    pip_ude_db[ude_name].devices[index] = devNum
    pip_ude_db[ude_name].device_index = index + 1
end
-- get the device number bound to 'ude-name'
function pip_db_ude_name_device_num_get(ude_name)
    if pip_ude_db[ude_name] == nil then
        return 0
    end
    if(pip_ude_db[ude_name].devices == nil) then
        return 0
    end
    return pip_ude_db[ude_name].device_index - 1
end

-- get the device id at specific index (index is 1 based) to 'ude-name'
function pip_db_ude_name_device_id_get(ude_name,index)
    if pip_ude_db[ude_name] == nil then
        return nil
    end
    if(pip_ude_db[ude_name].devices == nil) then
        return nil
    end
    return pip_ude_db[ude_name].devices[index]
end

-- get index of device in 'ude-name'
function pip_db_ude_name_device_index_get(ude_name,devNum)
    if pip_ude_db[ude_name] == nil then
        return nil
    end
    if(pip_ude_db[ude_name].devices == nil) then
        return nil
    end
    
    for index , entry in pairs(pip_ude_db[ude_name].devices) do 
        if entry == devNum then
            -- found
            return index
        end
    end
    -- not found
    return nil
end

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################

local function pip_mac_da_key_name_get(macDaPatternString,macDaMaskString)
    return "pattern=" .. macDaPatternString .. ",mask=" .. macDaMaskString
end

local function macDaParse(key_name) 
    return "????" , "!!!!"
end

-- get the index for the mac-da {pattern,mask} 
-- function returns 2 indications:
-- index            - new index / existing index for mac-da {pattern,mask}
-- isAlreadyExists  - is mac-da {pattern,mask} was already exists
function pip_db_mac_da_index_get(macAddrString,macAddrMaskString)
    local table_name = pip_mac_da_index_db_name
    local key_name = pip_mac_da_key_name_get(macAddrString,macAddrMaskString)
    -- check existence before adding the entry
    local isAlreadyExists = general_resource_manager_entry_is_used_get(table_name,key_name)
    -- get index of the new/existing entry
    local index = general_resource_manager_entry_get(table_name,key_name)

    return index , -- can be nil --> the {macAddrString,macAddrMaskString} not found and no place for it
           isAlreadyExists
end

-- get the mac-da {pattern,mask} already exists
-- function returns indication:
-- isAlreadyExists  - is mac-da {pattern,mask} was already exists
function pip_db_mac_da_index_check(macAddrString,macAddrMaskString)
    local table_name = pip_mac_da_index_db_name
    local key_name = pip_mac_da_key_name_get(macAddrString,macAddrMaskString)
    -- check existence 
    return general_resource_manager_entry_is_used_get(table_name,key_name)
end

-- get the macDa entry {pattern,mask} used at specific index 
function pip_db_mac_da_entry_get(index)
    local table_name = pip_mac_da_index_db_name
    -- check existence before adding the entry
    local key_name = general_resource_manager_get_key_by_index(table_name,index)

    local macAddrString,macAddrMaskString = macDaParse(key_name)
    
    return macAddrString,macAddrMaskString
end

-- reset the index used for the  macDa entry {pattern,mask}
function pip_db_mac_da_index_reset(index)
    local table_name = pip_mac_da_index_db_name
    -- reset the index
    general_resource_manager_free_specific_index(table_name,index)
end

--##################################
--##################################
--##################################
--##################################
--##################################
--##################################


-- set the index of the 'mac_da-name'
function pip_db_mac_da_name_index_set(mac_da_name,index)
    if pip_mac_da_db[mac_da_name] == nil then
        pip_mac_da_db[mac_da_name] = {}
    end
    pip_mac_da_db[mac_da_name].index = index
end
-- get the index of the 'mac_da-name'
-- function returns 2 indications:
-- index            - existing index (or nil) for 'mac_da-name'
function pip_db_mac_da_name_index_get(mac_da_name)
    if pip_mac_da_db[mac_da_name] == nil then
        return nil
    end
    return pip_mac_da_db[mac_da_name].index
end
-- get by index the 'mac_da-name'
-- function returns 1 indication:
-- name            - 'mac_da-name' or nil
function pip_db_mac_da_name_by_index_get(index)
    for _index,entry in pairs(pip_mac_da_db) do
        if entry.index == index then
            return _index
        end
    end
    return nil
end
-- reset the index of the 'mac_da-name'
function pip_db_mac_da_name_reset(mac_da_name)
    pip_mac_da_db[mac_da_name] = nil
end
-- add device to the 'mac_da-name'
function pip_db_mac_da_name_device_add(mac_da_name,devNum)
    if pip_mac_da_db[mac_da_name] == nil then
        pip_mac_da_db[mac_da_name] = {}
    end
    if(pip_mac_da_db[mac_da_name].devices == nil) then
        pip_mac_da_db[mac_da_name].devices = {}
        pip_mac_da_db[mac_da_name].device_index = 1
    end
    local index = pip_mac_da_db[mac_da_name].device_index
    pip_mac_da_db[mac_da_name].devices[index] = devNum
    pip_mac_da_db[mac_da_name].device_index = index + 1
end
-- get the device number bound to 'mac_da-name'
function pip_db_mac_da_name_device_num_get(mac_da_name)
    if pip_mac_da_db[mac_da_name] == nil then
        return 0
    end
    if(pip_mac_da_db[mac_da_name].devices == nil) then
        return 0
    end
    return pip_mac_da_db[mac_da_name].device_index - 1
end

-- get the device id at specific index (index is 1 based) to 'mac_da-name'
function pip_db_mac_da_name_device_id_get(mac_da_name,index)
    if pip_mac_da_db[mac_da_name] == nil then
        return nil
    end
    if(pip_mac_da_db[mac_da_name].devices == nil) then
        return nil
    end
    return pip_mac_da_db[mac_da_name].devices[index]
end

-- get index of device in 'mac_da-name'
function pip_db_mac_da_name_device_index_get(mac_da_name,devNum)
    if pip_mac_da_db[mac_da_name] == nil then
        return nil
    end
    if(pip_mac_da_db[mac_da_name].devices == nil) then
        return nil
    end
    
    for index , entry in pairs(pip_mac_da_db[mac_da_name].devices) do 
        if entry == devNum then
            -- found
            return index
        end
    end
    -- not found
    return nil
end
