--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* trunk.lua
--*
--* DESCRIPTION:
--*       setting of trunk commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpeTrunkPortsAdd") 
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpeTrunkPortsDelete") 

--constants
CLI_type_dict["trunkType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "enumerator for trunk type",
    enum = {
        ["UPSTREAM"] = { value="CPSS_PX_HAL_BPE_TRUNK_UPSTREAM_E", help="the trunk consists of upstream ports" },
        ["EXTENDED"] = { value="CPSS_PX_HAL_BPE_TRUNK_EXTENDED_E", help="the trunk consists of extended ports" },
        ["CASCADE"]  = { value="CPSS_PX_HAL_BPE_TRUNK_CASCADE_E",  help="the trunk consists of cascade ports" },
        ["INTERNAL"] = { value="CPSS_PX_HAL_BPE_TRUNK_INTERNAL_E", help="the trunk consists of internal ports" }
    }
} 

CLI_type_dict["trunkLoadBalance"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "enumerator for trunk load balance",
    enum = {
        ["MAC-IP-TCP"] = { value="CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_IP_TCP_E", help="trunk load balance according to MAC,IP,TCP " },
        ["PORT-BASE"]  = { value="CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_PORT_BASE_E",  help="trunk load balance according to port" },
        ["MAC"]        = { value="CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_MAC_E",        help="trunk load balance according to MAC" },
        ["IP"]         = { value="CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_IP_E",         help="trunk load balance according to IP" }
    }
} 

local trunk_data

-- ************************************************************************
---
--  trunk_create_func
--        @description  Create a TRUNK
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function trunk_create_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeTrunkCreate", {
            { "IN", "GT_SW_DEV_NUM",                    "devNum",         params.device_id },
            { "IN", "GT_TRUNK_ID",                      "trunkId",        params.trunk_id},
            { "IN", "CPSS_PX_HAL_BPE_TRUNK_TYPE_ENT",   "trunkType",      params.trunk_type}
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeTrunkCreate"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end


-- ************************************************************************
---
--  trunk_remove_func
--        @description  Remove a TRUNK
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function trunk_remove_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeTrunkRemove", {
            { "IN", "GT_SW_DEV_NUM",                    "devNum",         params.device_id },
            { "IN", "GT_TRUNK_ID",                      "trunkId",        params.trunk_id}           
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeTrunkRemove"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

-- ************************************************************************
---
--  trunk_load_balance_func
--        @description  Set the Load Balance Algorithm of the TRUNK
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function trunk_load_balance_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeTrunkLoadBalanceModeSet", {
            { "IN", "GT_SW_DEV_NUM",                                 "devNum",              params.device_id },            
            { "IN", "CPSS_PX_HAL_BPE_TRUNK_LOAD_BALANCE_MODE_ENT",   "loadBalanceMode",     params.trunk_load_balance}
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeTrunkLoadBalanceModeSet"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

-- ************************************************************************
---
--  trunk_ports_add_func
--        @description  Add ports to a TRUNK
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--

local function trunk_ports_add_func(params)
    trunk_data = {
        device_id = params.device_id,
        trunk_id = params.trunk_id,
        portList_len = params.portList_len,
        numPortsAdded = 0,
        array={}
    }
    CLI_prompts["bpe_trunk_ports_add"] = string.format("(add ports trunk %d#)",params.trunk_id)
    CLI_change_mode_push("bpe_trunk_ports_add")
    return true
end


-- ************************************************************************
---
--  trunk_port_add_func
--        @description  Add single port to a TRUNK
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function trunk_port_add_func(params) 
    trunk_data.numPortsAdded = trunk_data.numPortsAdded + 1     
    tableAppend(trunk_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.device_id,
            portNum=params.port_id
        }})  
end


-- ************************************************************************
---
--  trunk_ports_add_commit_func
--        @description  Add ports to a TRUNK and commit
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--

local function trunk_ports_add_commit_func(params)
    
    local command_data = Command_Data()

    if trunk_data.numPortsAdded ~= trunk_data.portList_len then
        local error_string = string.format("ERROR number of ports added to the trunk should be %d",trunk_data.portList_len)
        command_data:addErrorAndPrint(error_string)
    end

    status = wrlCpssPxHalBpeTrunkPortsAdd(trunk_data.device_id,trunk_data.trunk_id,trunk_data.portList_len,trunk_data.array)
    if status ~= 0 then
        error_string = "ERROR calling function wrlCpssPxHalBpeTrunkPortsAdd"
        command_data:addErrorAndPrint(error_string)
    end

    CLI_change_mode_pop()
    return isError     
        
end

-- ************************************************************************
---
--  trunk_ports_delete_func
--        @description  Delete ports from a TRUNK
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function trunk_ports_delete_func(params)
    trunk_data = {
        device_id = params.device_id,
        trunk_id = params.trunk_id,
        portList_len = params.portList_len,
        numPortsAdded = 0,
        array={}
    }
    CLI_prompts["bpe_trunk_ports_delete"] = string.format("(delete ports trunk %d#)",params.trunk_id)
    CLI_change_mode_push("bpe_trunk_ports_delete")
    return true
end


-- ************************************************************************
---
--  trunk_port_delete_func
--        @description  Delete single port from a TRUNK
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function trunk_port_delete_func(params) 
    trunk_data.numPortsAdded = trunk_data.numPortsAdded + 1     
    tableAppend(trunk_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.device_id,
            portNum=params.port_id
        }})  
end


-- ************************************************************************
---
--  trunk_ports_delete_commit_func
--        @description  Delete ports from a TRUNK and commit
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--

local function trunk_ports_delete_commit_func(params)
    
    local command_data = Command_Data()

    if trunk_data.numPortsAdded ~= trunk_data.portList_len then
        local error_string = string.format("ERROR number of ports deleted from the trunk should be %d",trunk_data.portList_len)
        command_data:addErrorAndPrint(error_string)
    end

    status = wrlCpssPxHalBpeTrunkPortsDelete(trunk_data.device_id,trunk_data.trunk_id,trunk_data.portList_len,trunk_data.array)
    if status ~= 0 then
        error_string = "ERROR calling function wrlCpssPxHalBpeTrunkPortsDelete"
        command_data:addErrorAndPrint(error_string)
    end

    CLI_change_mode_pop()
    return isError     
        
end

--------------------------------------------
-- command registration: 
--------------------------------------------

-- the command looks like :
-- Console(bpe)# trunk-create device 0 trunk-id 2 trunk-type UPSTREAM
CLI_addCommand("bpe", "trunk-create", {
    func   = trunk_create_func,
    help   = "Create a TRUNK",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "trunk-id %GT_U16", name = "trunk_id", help = "trunk in range GT_U16"},
            { format = "trunk-type %trunkType", name = "trunk_type", help = "type of trunk ports"},
            mandatory = { "device_id", "trunk_id", "trunk_type" },            
            requirements = {
                ["trunk_id"] = { "device_id" },                               
                ["trunk_type"] = { "trunk_id" }                                                   
            },                                     
        }
    }
}) 


-- the command looks like :
-- Console(bpe)# trunk-remove device 0 trunk-id 2 
CLI_addCommand("bpe", "trunk-remove", {
    func   = trunk_remove_func,
    help   = "Remove a TRUNK",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "trunk-id %GT_U16", name = "trunk_id", help = "trunk in range GT_U16"},
            mandatory = { "device_id", "trunk_id" },            
            requirements = {
                ["trunk_id"] = { "device_id" }                                                                                               
            },                                                    
        }
    }
})
 
-- the command looks like :
-- Console(bpe)# trunk-load-balance device 0 mode MAC-IP-TCP
CLI_addCommand("bpe", "trunk-load-balance", {
    func   = trunk_load_balance_func,
    help   = "Set the Load Balance Algorithm of the TRUNK",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "mode %trunkLoadBalance", name = "trunk_load_balance", help = "load balance mode algorithm"},
            mandatory = { "device_id", "trunk_load_balance" },            
            requirements = {
                ["trunk_load_balance"] = { "device_id" }                                                                                               
            },                                                
        }
    }
})
 
-- the command looks like :
-- Console(bpe)# trunk-ports-add device 0 trunk-id 2 port-list-len 4 
CLI_addCommand("bpe", "trunk-ports-add", {
    func   = trunk_ports_add_func,
    help   = "Add ports to a TRUNK",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },  
            { format = "trunk-id %GT_U16", name = "trunk_id", help = "trunk in range GT_U16"},          
            { format = "port-list-len %GT_U32", name = "portList_len", help = "port list length range GT_U32"},
            mandatory = { "device_id", "trunk_id", "portList_len"},
            requirements = {
                ["trunk_id"] = { "device_id" },  
                ["portList_len"] = { "trunk_id" }                
            },                                                  
        }
    }
}) 

-- the command looks like :
-- Console(add ports trunk 33#)# add-port interface-device-id 0 interface-port-id 4
CLI_addCommand("bpe_trunk_ports_add", "add-port", {
    func   = trunk_port_add_func,
    help   = "Add single port to a TRUNK",
    params = {
        { type = "named",
            { format = "interface-device-id %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "interface-port-id %bpePort", name = "port_id", help = "port in range 0..16"},
            mandatory = { "device_id", "port_id"},
            requirements = {
                ["port_id"] = { "device_id" },                
            },                                               
        }
    }
}) 

-- the command looks like :
-- Console(add ports trunk 33#)# commit
CLI_addCommand("bpe_trunk_ports_add", "commit", {
    func   = trunk_ports_add_commit_func,
    help   = "Add ports to a TRUNK commit",
    params = {}    
}) 

-- the command looks like :
-- Console(bpe)# trunk-ports-delete device 0 trunk-id 2 port-list-len 4 
CLI_addCommand("bpe", "trunk-ports-delete", {
    func   = trunk_ports_delete_func,
    help   = "Delete ports from a TRUNK",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },  
            { format = "trunk-id %GT_U16", name = "trunk_id", help = "trunk in range GT_U16"},          
            { format = "port-list-len %GT_U32", name = "portList_len", help = "port list length range GT_U32"},
            mandatory = { "device_id", "trunk_id", "portList_len"},
            requirements = {
                ["trunk_id"] = { "device_id" },  
                ["portList_len"] = { "trunk_id" }                
            },                                                  
        }
    }
}) 

-- the command looks like :
-- Console(delete ports trunk 33#)# delete-port interface-device-id 0 interface-port-id 4
CLI_addCommand("bpe_trunk_ports_delete", "delete-port", {
    func   = trunk_port_delete_func,
    help   = "Delete single port from a TRUNK",
    params = {
        { type = "named",
            { format = "interface-device-id %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "interface-port-id %bpePort", name = "port_id", help = "port in range 0..16"},
            mandatory = { "device_id", "port_id"},
            requirements = {
                ["port_id"] = { "device_id" },                
            },                                               
        }
    }
}) 

-- the command looks like :
-- Console(delete ports trunk 33#)# commit
CLI_addCommand("bpe_trunk_ports_delete", "commit", {
    func   = trunk_ports_delete_commit_func,
    help   = "Delete ports from a TRUNK commit",
    params = {}    
}) 


