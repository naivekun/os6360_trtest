--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* interface.lua
--*
--* DESCRIPTION:
--*       setting of BPE interface commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpeInterfaceTypeSet") 
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpeInterfaceExtendedPcidSet")
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpePortExtendedUpstreamSet")
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpePortExtendedUntaggedVlanAdd")
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpePortExtendedUntaggedVlanDel")
cmdLuaCLI_registerCfunction("wrlCpssPxHalBpePortExtendedQosMapSet")

--constants
CLI_type_dict["bpeInterfaceTypes"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Enumeration of bridge port extender interface types",
    enum = {
        ["IDLE"]     = { value="CPSS_PX_HAL_BPE_INTERFACE_MODE_IDLE_E",     help="default mode, not BPE port, discard all. relevant to CPSS_BPE_INTERFACE_PORT_E" },
        ["TRUSTED"]  = { value="CPSS_PX_HAL_BPE_INTERFACE_MODE_TRUSTED_E",  help="port potentially connected to CB. relevant to CPSS_BPE_INTERFACE_PORT_E" },       
        ["UPSTREAM"] = { value="CPSS_PX_HAL_BPE_INTERFACE_MODE_UPSTREAM_E", help="upstream interface, connected to CB" },
        ["EXTENDED"] = { value="CPSS_PX_HAL_BPE_INTERFACE_MODE_EXTENDED_E", help="extended interface, connected stations" },
        ["CASCADE"]  = { value="CPSS_PX_HAL_BPE_INTERFACE_MODE_CASCADE_E",  help="cascaded interface" },
        ["INTERNAL"] = { value="CPSS_PX_HAL_BPE_INTERFACE_MODE_INTERNAL_E", help="inter-device interface (aka Marvell DSA tag port)" }
    }
}
 
CLI_type_dict["bpeQueue"] = {
    checker = CLI_check_param_number,
    min=0,
    max=7,
    complete = CLI_complete_param_number,
    help="Enter egress queue "
}
 

CLI_type_dict["bpePcp"] = {
    checker = CLI_check_param_number,
    min=0,
    max=7, 
    complete = CLI_complete_param_number,
    help = "Enter class of service "
}


CLI_type_dict["bpeDei"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1, 
    complete = CLI_complete_param_number,
    help = "Enter drop eligibility indication "
}

CLI_type_dict["bpeVid"] = {
    checker = CLI_check_param_number,
    min=1,
    max=4095, 
    complete = CLI_complete_param_number,
    help = "Enter vlan Id "
}

local interface_data
local qos_data

-- ************************************************************************
---
--  interface_type_set_func
--        @description  Set BPE interface mode
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function interface_type_set_func(params)

    local command_data = Command_Data()

    interface_data = {
        device_id = params.device_id,
        mode = params.mode,
        array={}
    }

    if params.interface_type == "typePort" then
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.interface_device_id,
            portNum=params.interface_port_id
        }})
    else
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E",
        trunkId=params.interface_trunk_id
        })
    end
        
    status = wrlCpssPxHalBpeInterfaceTypeSet(interface_data.device_id,interface_data.array,interface_data.mode)
    if status ~= 0 then
        local error_string = "ERROR calling function wrlCpssPxHalBpeInterfaceTypeSet"
            command_data:addErrorAndPrint(error_string)
    end
    
    return isError 

end


-- ************************************************************************
---
--  interface_extended_pcid_set_func
--        @description  Set Extended interface Pcid
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function interface_extended_pcid_set_func(params)

    local command_data = Command_Data()

    interface_data = {
        device_id = params.device_id,
        pcid = params.pcid_id,
        array={}
    }
    if params.interface_type == "typePort" then
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.interface_device_id,
            portNum=params.interface_port_id
        }})
    else
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E",
        trunkId=params.interface_trunk_id
        })
    end
        
    status = wrlCpssPxHalBpeInterfaceExtendedPcidSet(interface_data.device_id,interface_data.array,interface_data.pcid)
    if status ~= 0 then
        local error_string = "ERROR calling function wrlCpssPxHalBpeInterfaceExtendedPcidSet"
            command_data:addErrorAndPrint(error_string)
    end
    
    return isError 

end

-- ************************************************************************
---
--  port_extended_upstream_set_func
--        @description  Set Extended interface Pcid
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function port_extended_upstream_set_func(params)

    local command_data = Command_Data()

    interface_data = {
        device_id = params.device_id,
        port_num = params.port_id,
        array={}
    }
    if params.interface_type == "typePort" then
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.interface_device_id,
            portNum=params.interface_port_id
        }})
    else
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E",
        trunkId=params.interface_trunk_id
        })
    end
        
    status = wrlCpssPxHalBpePortExtendedUpstreamSet(interface_data.device_id,interface_data.port_num,interface_data.array)
    if status ~= 0 then
        local error_string = "ERROR calling function wrlCpssPxHalBpePortExtendedUpstreamSet"
            command_data:addErrorAndPrint(error_string)
    end
    
    return isError 

end

-- ************************************************************************
---
--  upstream_qos_map_set_func
--        @description  Set CoS mapping between E-PCP to QUEUE for Upstream port
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function upstream_qos_map_set_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeUpstreamQosMapSet", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id },           
            { "IN", "GT_U32",                   "pcp",              params.pcp },
            { "IN", "GT_U32",                   "queue",            params.queue },
                                
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeUpstreamQosMapSet"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end


-- ************************************************************************
---
--  extended_qos_map_set_func
--        @description  Set CoS mapping between E-PCP to QUEUE for Extended/Cascade port
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function extended_qos_map_set_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeExtendedQosMapSet", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id },
            { "IN", "GT_PORT_NUM",              "portNum",          params.port_id },
            { "IN", "GT_U32",                   "pcp",              params.pcp },
            { "IN", "GT_U32",                   "queue",            params.queue },
                                
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeExtendedQosMapSet"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

-- ************************************************************************
---
--  extended_qos_default_map_set_func
--        @description  Set Extended port default E-PCP and E-DEI
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function extended_qos_default_map_set_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpePortExtendedQosDefaultPcpDeiSet", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id },
            { "IN", "GT_PORT_NUM",              "portNum",          params.port_id },
            { "IN", "GT_U32",                   "pcp",              params.pcp },
            { "IN", "GT_U32",                   "dei",              params.dei },
                                
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpePortExtendedQosDefaultPcpDeiSet"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

-- ************************************************************************
---
--  port_extended_untagged_vlan_add_func
--        @description  Configure the Extended interface to egress packets untagged
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function port_extended_untagged_vlan_add_func(params)

    local command_data = Command_Data()

    interface_data = {
        device_id = params.device_id,
        vid = params.vid_id,
        array={}
    }
    if params.interface_type == "typePort" then
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.interface_device_id,
            portNum=params.interface_port_id
        }})
    else
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E",
        trunkId=params.interface_trunk_id
        })
    end
        
    status = wrlCpssPxHalBpePortExtendedUntaggedVlanAdd(interface_data.device_id,interface_data.array,interface_data.vid)
    if status ~= 0 then
        local error_string = "ERROR calling function wrlCpssPxHalBpePortExtendedUntaggedVlanAdd"
            command_data:addErrorAndPrint(error_string)
    end
    
    return isError 

end


-- ************************************************************************
---
--  port_extended_untagged_vlan_Del_func
--        @description  Configure the Extended interface to egress packets tagged
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function port_extended_untagged_vlan_del_func(params)

    local command_data = Command_Data()

    interface_data = {
        device_id = params.device_id,
        vid = params.vid_id,
        array={}
    }
    if params.interface_type == "typePort" then
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_PORT_E",
        devPort={
            devNum=params.interface_device_id,
            portNum=params.interface_port_id
        }})
    else
        tableAppend(interface_data.array, {
        type="CPSS_PX_HAL_BPE_INTERFACE_TRUNK_E",
        trunkId=params.interface_trunk_id
        })
    end
        
    status = wrlCpssPxHalBpePortExtendedUntaggedVlanDel(interface_data.device_id,interface_data.array,interface_data.vid)
    if status ~= 0 then
        local error_string = "ERROR calling function wrlCpssPxHalBpePortExtendedUntaggedVlanDel"
            command_data:addErrorAndPrint(error_string)
    end
    
    return isError 

end

-- ************************************************************************
---
--  port_extended_qos_map_set_func
--        @description  Set CoS mapping between C-UP,C-CFI to E-PCP,E-DEI for Extended port
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function port_extended_qos_map_set_func(params)
    qos_data = {
        device_id = params.device_id,
        port_id = params.port_id,
        pcp_index = 0,
        dei_index = 0,
        array={}
    }
    CLI_prompts["bpe_port_extended_qos_map"] = string.format("(port extended pcp %d dei %d#)",qos_data.pcp_index, qos_data.dei_index)
    CLI_change_mode_push("bpe_port_extended_qos_map")
    return true
end

-- ************************************************************************
---
--  port_extended_qos_map_func
--        @description  Set CoS mapping between a single C-UP,C-CFI to E-PCP,E-DEI for Extended port
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function port_extended_qos_map_func(params)   
    local command_data = Command_Data()
       
    if (qos_data.array == nil) then
        qos_data.array = {}        
    end
    if (qos_data.array[qos_data.pcp_index] == nil) then
        qos_data.array[qos_data.pcp_index] = {}        
    end
    qos_data.array[qos_data.pcp_index][qos_data.dei_index] = { newPcp=params.pcp, newDei=params.dei}
    --print(to_string(qos_data.array))    

    if (qos_data.dei_index == 1) then
        qos_data.pcp_index = qos_data.pcp_index + 1
        qos_data.dei_index = 0
    else
        qos_data.dei_index = 1 
    end
       
    if (qos_data.pcp_index == 8) then
        status = wrlCpssPxHalBpePortExtendedQosMapSet(qos_data.device_id,qos_data.port_id,qos_data.array)
        if status ~= 0 then
            local error_string = "ERROR calling function wrlCpssPxHalBpePortExtendedQosMapSet"
                command_data:addErrorAndPrint(error_string)
        end

        CLI_change_mode_pop()
        return isError     
    end

    CLI_prompts["bpe_port_extended_qos_map"] = string.format("(port extended pcp %d dei %d#)",qos_data.pcp_index, qos_data.dei_index) 
    CLI_change_mode_push("bpe_port_extended_qos_map")
    return true
end

--------------------------------------------
-- command registration: 
--------------------------------------------

-- the command looks like :
-- Console(bpe)# interface-type-set device 0 interface-type PORT interface-device-id 2 interface-port-id 6 mode IDLE
-- Console(bpe)# interface-type-set device 0 interface-type TRUNK interface-trunk-id 7 mode IDLE
CLI_addCommand("bpe", "interface-type-set", {
    func   = interface_type_set_func,
    help   = "Set BPE interface mode",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" }, 
            { format = "interface-type", name="int_type", help="choose interface type" }, 
            { format = "PORT", name = "typePort", help = "interface type Port"},   
            { format = "TRUNK", name = "typeTrunk", help = "interface type Trunk"},                                                  
            { format = "interface-device-id %GT_U8", name = "interface_device_id", help = "device in range GT_U8"},
            { format = "interface-port-id %bpePort", name = "interface_port_id", help = "port in range 0..16"},
            { format = "interface-trunk-id %GT_U16", name = "interface_trunk_id", help = "trunk in range GT_U16"},                                                  
            { format = "mode %bpeInterfaceTypes", name = "mode", help = "interface mode"},
            alt = { interface_type = { "typePort", "typeTrunk" }, interface_is_set={"interface_port_id","interface_trunk_id"}},                 
            mandatory = { "device_id", "interface_type", "mode", "interface_is_set" },            
            requirements = {
                ["int_type"] = { "device_id" },  
                ["typePort"] = { "int_type" }, 
                ["typeTrunk"] = { "int_type" },                         
                ["interface_device_id"] = { "typePort" }, 
                ["interface_port_id"] = { "interface_device_id" },                
                ["interface_trunk_id"] = { "typeTrunk" },
                ["mode"] = { "interface_is_set"}                                           
            },  
        }        
    }
})

-- the command looks like :
-- Console(bpe)# interface-extended-pcid-set device 0 interface-type PORT interface-device-id 2 interface-port-id 6 pcid 88
-- Console(bpe)# interface-extended-pcid-set device 0 interface-type TRUNK interface-trunk-id 4 pcid 12
CLI_addCommand("bpe", "interface-extended-pcid-set", {
    func   = interface_extended_pcid_set_func,
    help   = "Set Extended interface Pcid",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },  
            { format = "interface-type", name="int_type", help="choose interface type" },  
            { format = "PORT", name = "typePort", help = "interface type Port"},   
            { format = "TRUNK", name = "typeTrunk", help = "interface type Trunk"},                                      
            { format = "interface-device-id %GT_U8", name = "interface_device_id", help = "device in range GT_U8"},
            { format = "interface-port-id %bpePort", name = "interface_port_id", help = "port in range 0..16"},
            { format = "interface-trunk-id %GT_U16", name = "interface_trunk_id", help = "trunk in range GT_U16"},                                                     
            { format = "pcid %GT_U32", name = "pcid_id", help = "cid in range GT_U32"},
            alt = { interface_type = { "typePort", "typeTrunk" }, interface_is_set={"interface_port_id","interface_trunk_id"}},                 
            mandatory = { "device_id", "interface_type", "pcid_id", "interface_is_set" },            
            requirements = {
                ["int_type"] = { "device_id" },  
                ["typePort"] = { "int_type" }, 
                ["typeTrunk"] = { "int_type" },                              
                ["interface_device_id"] = { "typePort" }, 
                ["interface_port_id"] = { "interface_device_id" },                
                ["interface_trunk_id"] = { "typeTrunk" },
                ["pcid_id"] = { "interface_is_set"}                                           
            },  
        }        
    }
})

-- the command looks like :
-- Console(bpe)# port-extended-upstream-set device 0 port-id 8 interface-type PORT interface-device-id 2 interface-port-id 6 
-- Console(bpe)# port-extended-upstream-set device 0 port-id 8 interface-type TRUNK interface-trunk-id 12 
CLI_addCommand("bpe", "port-extended-upstream-set", {
    func   = port_extended_upstream_set_func,
    help   = "Set Extended port Upstream interface",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },   
            { format = "port-id %bpePort", name = "port_id", help = "port in range 0..16"},  
            { format = "interface-type", name="int_type", help="choose interface type" },  
            { format = "PORT", name = "typePort", help = "interface type Port"},   
            { format = "TRUNK", name = "typeTrunk", help = "interface type Trunk"},                                    
            { format = "interface-device-id %GT_U8", name = "interface_device_id", help = "device in range GT_U8"},
            { format = "interface-port-id %bpePort", name = "interface_port_id", help = "port in range 0..16"},
            { format = "interface-trunk-id %GT_U16", name = "interface_trunk_id", help = "trunk in range GT_U16"},
            alt = { interface_type = { "typePort", "typeTrunk" }, interface_is_set={"interface_port_id","interface_trunk_id"}},                 
            mandatory = { "device_id", "interface_type", "port_id", "interface_is_set" },            
            requirements = {
                ["port_id"] = { "device_id" },    
                ["int_type"] = { "port_id" },    
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
-- Console(bpe)# upstream-qos-map-set device 0 pcp 3 queue 2
CLI_addCommand("bpe", "upstream-qos-map-set", {
    func   = upstream_qos_map_set_func,
    help   = "Set CoS mapping between E-PCP to QUEUE for Upstream port",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },             
            { format = "pcp %bpePcp", name = "pcp", help = "class of service in range 0..7"},                                                  
            { format = "queue %bpeQueue", name = "queue", help = "egress queue in range 0..7"},                         
            mandatory = { "device_id","pcp", "queue" },            
            requirements = {
                ["pcp"] = { "device_id" },                                               
                ["queue"] = { "pcp" }                                       
            },  
        }        
    }
})

-- the command looks like :
-- Console(bpe)# extended-qos-map-set device 0 port-id 6 pcp 3 queue 2
CLI_addCommand("bpe", "extended-qos-map-set", {
    func   = extended_qos_map_set_func,
    help   = "Set CoS mapping between E-PCP to QUEUE for Extended/Cascade port",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },  
            { format = "port-id %bpePort", name = "port_id", help = "port in range 0..16"},
            { format = "pcp %bpePcp", name = "pcp", help = "class of service in range 0..7"},                                                  
            { format = "queue %bpeQueue", name = "queue", help = "egress queue in range 0..7"},                         
            mandatory = { "device_id", "port_id", "pcp", "queue" },            
            requirements = {
                ["port_id"] = { "device_id" },                               
                ["pcp"] = { "port_id" }, 
                ["queue"] = { "pcp" }                                       
            },  
        }        
    }
})

-- the command looks like :
-- Console(bpe)# extended-qos-default-map-set device 0 port-id 6 pcp 3 dei 1
CLI_addCommand("bpe", "extended-qos-default-map-set", {
    func   = extended_qos_default_map_set_func,
    help   = "Set Extended port default E-PCP and E-DEI",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },  
            { format = "port-id %bpePort", name = "port_id", help = "port in range 0..16"},
            { format = "pcp %bpePcp", name = "pcp", help = "class of service in range 0..7"},                                                  
            { format = "dei %bpeDei", name = "dei", help = "drop eligibility indication in range 0..1"},                         
            mandatory = { "device_id", "port_id", "pcp", "dei" },            
            requirements = {
                ["port_id"] = { "device_id" },                               
                ["pcp"] = { "port_id" }, 
                ["dei"] = { "pcp" }                                       
            },  
        }        
    }
})

-- the command looks like :
-- Console(bpe)# port-extended-untagged-vlan-add device 0 interface-type PORT interface-device-id 2 interface-port-id 6 vid 8
-- Console(bpe)# port-extended-untagged-vlan-add device 0 interface-type TRUNK interface-trunk-id 4 vid 12
CLI_addCommand("bpe", "port-extended-untagged-vlan-add", {
    func   = port_extended_untagged_vlan_add_func,
    help   = "Configure the Extended interface to egress packets untagged ",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },
            { format = "interface-type", name="int_type", help="choose interface type" },   
            { format = "PORT", name = "typePort", help = "interface type Port"},   
            { format = "TRUNK", name = "typeTrunk", help = "interface type Trunk"},                                      
            { format = "interface-device-id %GT_U8", name = "interface_device_id", help = "device in range GT_U8"},
            { format = "interface-port-id %bpePort", name = "interface_port_id", help = "port in range 0..16"},
            { format = "interface-trunk-id %GT_U16", name = "interface_trunk_id", help = "trunk in range GT_U16"},                                                     
            { format = "vid %bpeVid", name = "vid_id", help = "vlan id in range 1..4095"},
            alt = { interface_type = { "typePort", "typeTrunk" }, interface_is_set={"interface_port_id","interface_trunk_id"}},                 
            mandatory = { "device_id", "interface_type", "vid_id", "interface_is_set" },            
            requirements = {
                ["int_type"] = { "device_id" },  
                ["typePort"] = { "int_type" }, 
                ["typeTrunk"] = { "int_type" },                                   
                ["interface_device_id"] = { "typePort" }, 
                ["interface_port_id"] = { "interface_device_id" },                
                ["interface_trunk_id"] = { "typeTrunk" },
                ["vid_id"] = { "interface_is_set"}                                           
            },  
        }        
    }
})

-- the command looks like :
-- Console(bpe)# port-extended-untagged-vlan-del device 0 interface-type PORT interface-device-id 2 interface-port-id 6 vid 8
-- Console(bpe)# port-extended-untagged-vlan-del device 0 interface-type TRUNK interface-trunk-id 4 vid 12
CLI_addCommand("bpe", "port-extended-untagged-vlan-del", {
    func   = port_extended_untagged_vlan_del_func,
    help   = "Configure the Extended interface to egress packets tagged ",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" }, 
            { format = "interface-type", name="int_type", help="choose interface type" },  
            { format = "PORT", name = "typePort", help = "interface type Port"},   
            { format = "TRUNK", name = "typeTrunk", help = "interface type Trunk"},                                      
            { format = "interface-device-id %GT_U8", name = "interface_device_id", help = "device in range GT_U8"},
            { format = "interface-port-id %bpePort", name = "interface_port_id", help = "port in range 0..16"},
            { format = "interface-trunk-id %GT_U16", name = "interface_trunk_id", help = "trunk in range GT_U16"},                                                     
            { format = "vid %bpeVid", name = "vid_id", help = "vlan id in range 1..4095"},
            alt = { interface_type = { "typePort", "typeTrunk" }, interface_is_set={"interface_port_id","interface_trunk_id"}},                 
            mandatory = { "device_id", "interface_type", "vid_id", "interface_is_set" },            
            requirements = {
                ["int_type"] = { "device_id" },  
                ["typePort"] = { "int_type" }, 
                ["typeTrunk"] = { "int_type" },                                           
                ["interface_device_id"] = { "typePort" }, 
                ["interface_port_id"] = { "interface_device_id" },                
                ["interface_trunk_id"] = { "typeTrunk" },
                ["vid_id"] = { "interface_is_set"}                                           
            },  
        }        
    }
})


-- the command looks like :
-- Console(bpe)# port-extended-qos-map-set device 0 port-id 2 
CLI_addCommand("bpe", "port-extended-qos-map-set", {
    func   = port_extended_qos_map_set_func,
    help   = "Set CoS mapping between C-UP,C-CFI to E-PCP,E-DEI for Extended port",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "port-id %bpePort", name = "port_id", help = "port in range 0..16"},
            mandatory = { "device_id", "port_id"},
            requirements = {
                ["port_id"] = { "device_id" },                
            },                                                  
        }
    }
}) 

-- the command looks like :
-- Console(port extended pcp 0 dei 0#)# qos-map pcp 2 dei 0
-- Console(port extended pcp 0 dei 1#)# qos-map pcp 0 dei 1
-- Console(port extended pcp 1 dei 0#)# qos-map pcp 3 dei 0
--...
-- Console(port extended pcp 7 dei 1#)# qos-map pcp 3 dei 0
CLI_addCommand("bpe_port_extended_qos_map", "qos-map", {
    func   = port_extended_qos_map_func,
    help   = "Set CoS mapping between a single C-UP,C-CFI to E-PCP,E-DEI for Extended port",
    params = {
        { type = "named",
            { format = "pcp %bpePcp", name = "pcp", help = "class of service in range 0..7"},  
            { format = "dei %bpeDei", name = "dei", help = "drop eligibility indication in range 0..1"},         
             mandatory = { "pcp", "dei"},
            requirements = {
                ["dei"] = { "pcp" },                
            },                                               
        }
    }
}) 

