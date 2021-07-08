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
--*       setting of E-channel commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpeMulticastEChannelCreate")
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpeMulticastEChannelUpdate") 
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpeUnicastEChannelCreate") 

--constants
CLI_type_dict["interfaceType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "enumerator for interface type",
    enum = {
        ["PORT"] = { value="CPSS_PX_HAL_BPE_INTERFACE_PORT_E", help="the interface is of port type" },
        ["TRUNK"] = { value="CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E", help="the interface is of trunk type" }       
    }
}
 
 
CLI_type_dict["bpePort"] = {
    checker = CLI_check_param_number,
    min=0,
    max=16, 
    complete = CLI_complete_param_number,
    help = "Enter port number"
}

local echn_data

-- ************************************************************************
---
--  unicast_echannel_create_func
--        @description  Create Unicast E-Channel and add downstream interface
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function unicast_echannel_create_func(params)

    local command_data = Command_Data()

    echn_data = {
        device_id = params.device_id,
        cid_id = params.cid_id,
        array={}
    }
    if params.interface_type == "typePort" then
        tableAppend(echn_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.interface_device_id,
            portNum=params.interface_port_id
        }})
    else
        tableAppend(echn_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E",
        trunkId=params.interface_trunk_id
        })
    end
 
    status = wrlCpssPxHalBpeUnicastEChannelCreate(echn_data.device_id,echn_data.cid_id,echn_data.array)
    if status ~= 0 then
        local error_string = "ERROR calling function wrlCpssPxHalBpeUnicastEChannelCreate"
            command_data:addErrorAndPrint(error_string)
    end
    
    return isError 

end

-- ************************************************************************
---
--  multicast_echannel_create_commit_func
--        @description  Create Multicast E-Channel and commit
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--

local function multicast_echannel_create_commit_func(params)
    
    local command_data = Command_Data()
    tableAppend(echn_data.array, {type="CPSS_PX_HAL_BPE_INTERFACE_NONE_E"})  
    status = wrlCpssPxHalBpeMulticastEChannelCreate(echn_data.device_id,echn_data.cid_id,echn_data.array)
    if status ~= 0 then
        local error_string = "ERROR calling function wrlCpssPxHalBpeMulticastEChannelCreate"
            command_data:addErrorAndPrint(error_string)
    end

    CLI_change_mode_pop()
    return isError     
        
end

-- ************************************************************************
---
--  multicast_echannel_create_func
--        @description  Create Multicast E-Channel and add interfaces
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function multicast_echannel_create_func(params)
    echn_data = {
        device_id = params.device_id,
        cid_id = params.cid_id,
        array={}
    }
    CLI_prompts["bpe_multi_echn_create"] = string.format("(create multi echn %d#)",params.cid_id)
    CLI_change_mode_push("bpe_multi_echn_create")
    return true
end

-- ************************************************************************
---
--  multicast_echannel_create_add_port_func
--        @description  Create Multicast E-Channel and add port
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function multicast_echannel_create_add_port_func(params)      
    tableAppend(echn_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.device_id,
            portNum=params.port_id
        }})  
end

-- ************************************************************************
---
--  multicast_echannel_create_add_trunk_func
--        @description  Create Multicast E-Channel and add TRUNK
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function multicast_echannel_create_add_trunk_func(params)   
    tableAppend(echn_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E",
        trunkId=params.trunk_id
        })
end


-- ************************************************************************
---
--  multicast_echannel_update_commit_func
--        @description  Update Multicast E-Channel and commit
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function multicast_echannel_update_commit_func(params)
    
    local command_data = Command_Data()
    tableAppend(echn_data.array, {type="CPSS_PX_HAL_BPE_INTERFACE_NONE_E"})  
    status = wrlCpssPxHalBpeMulticastEChannelUpdate(echn_data.device_id,echn_data.cid_id,echn_data.array)
    if status ~= 0 then
        local error_string = "ERROR calling function wrlCpssPxHalBpeMulticastEChannelUpdate"
            command_data:addErrorAndPrint(error_string)
    end

    CLI_change_mode_pop()
    return isError     
        
end

-- ************************************************************************
---
--  multicast_echannel_update_func
--        @description  Update Multicast E-Channel and add interfaces
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function multicast_echannel_update_func(params)
    echn_data = {
        device_id = params.device_id,
        cid_id = params.cid_id,
        array={}
    }
    CLI_prompts["bpe_multi_echn_update"] = string.format("(update multi echn %d#)",params.cid_id)
    CLI_change_mode_push("bpe_multi_echn_update")
    return true
end

-- ************************************************************************
---
--  multicast_echannel_update_add_port_func
--        @description  Update Multicast E-Channel and add port
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function multicast_echannel_update_add_port_func(params)      
    tableAppend(echn_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.device_id,
            portNum=params.port_id
        }})  
end

-- ************************************************************************
---
--  multicast_echannel_update_add_trunk_func
--        @description  Update Multicast E-Channel and add TRUNK
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function multicast_echannel_update_add_trunk_func(params)   
    tableAppend(echn_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E",
        trunkId=params.trunk_id
        })
end

-- ************************************************************************
---
--  num_of_channels_get_func
--        @description  Get the number of E-channels supported by the BPE (unicast+multicast channels)
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function num_of_channels_get_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeNumOfChannelsGet", {
            { "IN", "GT_SW_DEV_NUM",    "devNum",                 params.device_id },
            { "OUT","GT_U32",           "numOfChannelsPtr",       numOfChannelsPtr }         
        })
    
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeNumOfChannelsGet"
        command_data:addErrorAndPrint(error_string)        
    end
    print("num-of-channels ",to_string(values.numOfChannelsPtr)) 
    return isError 
        
end

-- ************************************************************************
---
--  num_of_channels_set_func
--        @description  Set the maximum number of unicast E-channels and multicast E-channels
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function num_of_channels_set_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeNumOfChannelsSet", {
            { "IN", "GT_SW_DEV_NUM",    "devNum",                 params.device_id },
            { "IN", "GT_U32",           "numOfUnicastChannels",   params.unicast_channels },
            { "IN", "GT_U32",           "numOfMulticastChannels", params.multicast_channels }

        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeNumOfChannelsSet"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end


-- ************************************************************************
---
--  unicast_echannel_delete_func
--        @description  Delete Unicast E-channel
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function unicast_echannel_delete_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeUnicastEChannelDelete", {
            { "IN", "GT_SW_DEV_NUM",    "devNum",                 params.device_id },
            { "IN", "GT_U32",           "cid",                    params.cid_id }            
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeUnicastEChannelDelete"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

-- ************************************************************************
---
--  multicast_echannel_delete_func
--        @description  Delete Multicast E-channel
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function multicast_echannel_delete_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeMulticastEChannelDelete", {
            { "IN", "GT_SW_DEV_NUM",    "devNum",                 params.device_id },
            { "IN", "GT_U32",           "cid",                    params.cid_id }            
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeMulticastEChannelDelete"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

--------------------------------------------
-- command registration: 
--------------------------------------------

-- the command looks like :
-- Console(bpe)# multicast-echannel-create device 0 cid 22 
CLI_addCommand("bpe", "multicast-echannel-create", {
    func   = multicast_echannel_create_func,
    help   = "Create Multicast E-Channel and add interfaces",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "cid %GT_U32", name = "cid_id", help = "cid in range GT_U32"},
            mandatory = { "device_id", "cid_id"},
            requirements = {
                ["cid_id"] = { "device_id" },                
            },                                                  
        }
    }
}) 

-- the command looks like :
-- Console(create multi echn 33#)# add-port interface-device-id 0 interface-port-id 4
CLI_addCommand("bpe_multi_echn_create", "add-port", {
    func   = multicast_echannel_create_add_port_func,
    help   = "Create Multicast E-Channel and add port",
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
-- Console(create multi echn 33#)# add-trunk interface-trunk-id 5
CLI_addCommand("bpe_multi_echn_create", "add-trunk", {
    func   = multicast_echannel_create_add_trunk_func,
    help   = "Create Multicast E-Channel and add TRUNK",
    params = {
        { type = "named",
            { format = "interface-trunk-id %GT_U16", name = "trunk_id", help = "trunk in range GT_U16"},
            mandatory = { "trunk_id"},                                                           
        }
    }
}) 

-- the command looks like :
-- Console(create multi echn 33#)# commit
CLI_addCommand("bpe_multi_echn_create", "commit", {
    func   = multicast_echannel_create_commit_func,
    help   = "Create Multicast E-Channel commit",
    params = {}    
}) 

-- the command looks like :
-- Console(bpe)# multicast-echannel-update device 0 cid 22 
CLI_addCommand("bpe", "multicast-echannel-update", {
    func   = multicast_echannel_update_func,
    help   = "Update Multicast E-Channel and add interfaces",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "cid %GT_U32", name = "cid_id", help = "cid in range GT_U32"},
            mandatory = { "device_id", "cid_id"},
            requirements = {
                ["cid_id"] = { "device_id" },                
            },                                                  
        }
    }
}) 

-- the command looks like :
-- Console(update multi echn 33#)# add-port interface-device-id 0 interface-port-id 4
CLI_addCommand("bpe_multi_echn_update", "add-port", {
    func   = multicast_echannel_update_add_port_func,
    help   = "Update Multicast E-Channel and add port",
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
-- Console(update multi echn 33#)# add-trunk interface-trunk-id 5
CLI_addCommand("bpe_multi_echn_update", "add-trunk", {
    func   = multicast_echannel_update_add_trunk_func,
    help   = "Update Multicast E-Channel and add TRUNK",
    params = {
        { type = "named",
            { format = "interface-trunk-id %GT_U16", name = "trunk_id", help = "trunk in range GT_U16"},
            mandatory = { "trunk_id"},                                                           
        }
    }
}) 

-- the command looks like :
-- Console(update multi echn 33#)# commit
CLI_addCommand("bpe_multi_echn_update", "commit", {
    func   = multicast_echannel_update_commit_func,
    help   = "Update Multicast E-Channel commit",
    params = {}    
}) 

-- the command looks like :
-- Console(bpe)# unicast-echannel-create device 0 cid 22 interface-type PORT interface-device-id 2 interface-port-id 6
-- Console(bpe)# unicast-echannel-create device 0 cid 22 interface-type TRUNK interface-trunk-id 8 
CLI_addCommand("bpe", "unicast-echannel-create", {
    func   = unicast_echannel_create_func,
    help   = "Create Unicast E-Channel and add downstream interface",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "cid %GT_U32", name = "cid_id", help = "cid in range GT_U32"},     
            { format = "interface-type", name="int_type", help="choose interface type" },                     
            { format = "PORT", name = "typePort", help = "interface type Port"},   
            { format = "TRUNK", name = "typeTrunk", help = "interface type Trunk"},   
            { format = "interface-device-id %GT_U8", name = "interface_device_id", help = "device in range GT_U8"},
            { format = "interface-port-id %bpePort", name = "interface_port_id", help = "port in range 0..16"},
            { format = "interface-trunk-id %GT_U16", name = "interface_trunk_id", help = "trunk in range GT_U16"},
            alt = { interface_type = { "typePort", "typeTrunk" }},
            mandatory = { "device_id", "cid_id", "interface_type"},            
            requirements = {
                ["cid_id"] = { "device_id" },    
                ["int_type"] = { "cid_id" },  
                ["typePort"] = { "int_type" }, 
                ["typeTrunk"] = { "int_type" },              
                ["interface_device_id"] = { "typePort" }, 
                ["interface_port_id"] = { "interface_device_id" }, 
                ["interface_trunk_id"] = { "typeTrunk" }                               
            },
        }        
    }
})

-- the command looks like :
-- Console(bpe)# num-of-channels-get device 0
CLI_addCommand("bpe", "num-of-channels-get", {
    func   = num_of_channels_get_func,
    help   = "Get the number of E-channels supported by the BPE ",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" }                                                              
        }
    }
}) 

-- the command looks like :
-- Console(bpe)# num-of-channels-set device 0 num-unicast-channels 100 num-multicast-channels 300
CLI_addCommand("bpe", "num-of-channels-set", {
    func   = num_of_channels_set_func,
    help   = "Set the maximum number of unicast/multicast E-channels",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },
            { format = "num-unicast-channels %GT_U32", name="unicast_channels", help="number of unicast channels in range GT_U32" },
            { format = "num-multicast-channels %GT_U32", name="multicast_channels", help="number of multicast channels in range GT_U32" },
            mandatory = { "device_id", "unicast_channels", "multicast_channels"},            
            requirements = {
                ["cid_id"] = { "device_id" },    
                ["interface_type"] = {"cid_id" },               
                ["interface_device_id"] = { "typePort" }, 
                ["interface_port_id"] = { "interface_device_id" }, 
                ["interface_trunk_id"] = { "typeTrunk" }                               
            },                                                               
        }
    }
}) 

-- the command looks like :
-- Console(bpe)# unicast-echannel-delete device 0 cid 22 
CLI_addCommand("bpe", "unicast-echannel-delete", {
    func   = unicast_echannel_delete_func,
    help   = "Delete Unicast E-channel",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "cid %GT_U32", name = "cid_id", help = "cid in range GT_U32"},
            mandatory = { "device_id", "cid_id"},            
            requirements = {
                ["cid_id"] = { "device_id" }                                          
            },                                                
        }
    }
}) 


-- the command looks like :
-- Console(bpe)# multicast-echannel-delete device 0 cid 22 
CLI_addCommand("bpe", "multicast-echannel-delete", {
    func   = multicast_echannel_delete_func,
    help   = "Delete Multicast E-channel",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "cid %GT_U32", name = "cid_id", help = "cid in range GT_U32"},
            mandatory = { "device_id", "cid_id"},            
            requirements = {
                ["cid_id"] = { "device_id" }                                          
            },                                                
        }
    }
}) 


