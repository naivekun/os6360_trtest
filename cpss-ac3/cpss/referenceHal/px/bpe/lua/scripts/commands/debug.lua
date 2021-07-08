--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* debug.lua
--*
--* DESCRIPTION:
--*       setting of debug commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

CLI_type_dict["cncBitmap"] = {
    checker = CLI_check_param_number,
    min=0,
    max=3, 
    complete = CLI_complete_param_number,
    help = "Enter bitmap of CNC Blocks"
}

CLI_type_dict["blockNum"] = {
    checker = CLI_check_param_number,
    min=0,
    max=1, 
    complete = CLI_complete_param_number,
    help = "Enter CNC block number"
}

CLI_type_dict["clientType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "enumerator for CNC clients type",
    enum = {
        ["INGRESS_PROCESSING"] =          { value="CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E", help="Ingress processing client" },
        ["EGRESS_QUEUE_PASS_DROP_QCN"] =  { value="CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E",  help="Egress queue pass/tail-drop and QCN client" }       
    }
}

-- ************************************************************************
---
--  debug_cnc_blocks_set_func
--        @description  Set the bitmap of CNC Blocks that can be used by 802.1BR refHal for debug
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function debug_cnc_blocks_set_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeDebugCncBlocksSet", {
            { "IN", "GT_SW_DEV_NUM",        "devNum",              params.device_id },            
            { "IN", "GT_U32",               "cncBitmap",           params.cncBitmap_id}
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeDebugCncBlocksSet"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

-- ************************************************************************
---
--  debug_cnc_config_client_func
--        @description  Configure CNC for client
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function debug_cnc_config_client_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeCncConfigClient", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id },            
            { "IN", "CPSS_PX_CNC_CLIENT_ENT",   "client",           params.client},
            { "IN", "GT_U32",                   "blockNum",         params.block_id}
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeCncConfigClient"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end


-- ************************************************************************
---
--  debug_cnc_dump_func
--        @description  Display CNC information for all clients
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function debug_cnc_dump_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeCncDump", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id },            
            { "IN", "GT_BOOL",                  "displayAll",       params.display_all}          
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeCncDump"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end


-- ************************************************************************
---
--  debug_ingress_tables_dump_func
--        @description  Display Ingress tables information
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function debug_ingress_tables_dump_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeIngressTablesDump", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id },            
            { "IN", "GT_BOOL",                  "displayAll",       params.display_all}          
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeIngressTablesDump"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end


-- ************************************************************************
---
--  debug_ingress_errors_dump_func
--        @description  Display Ingress Errors information
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function debug_ingress_errors_dump_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeIngressErrorsDump", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id }                    
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeIngressErrorsDump"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

-- ************************************************************************
---
--  debug_egress_tables_dump_func
--        @description  Display Egress tables information
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function debug_egress_tables_dump_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeEgressTablesDump", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id },            
            { "IN", "GT_BOOL",                  "displayAll",       params.display_all}                      
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeEgressTablesDump"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

-- ************************************************************************
---
--  debug_cos_tables_dump_func
--        @description  Display Cos tables information
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function debug_cos_tables_dump_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeCosTablesDump", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id }                    
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeCosTablesDump"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end


-- ************************************************************************
---
--  debug_ingress_hash_tables_dump_func
--        @description  Display Ingress Hash tables information
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function debug_ingress_hash_tables_dump_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeIngressHashTablesDump", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id }                    
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeIngressHashTablesDump"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end


-- ************************************************************************
---
--  debug_internal_tables_dump_func
--        @description  Display Internal tables information
--
--        @param        params- the parameters
--
--        @return       true if successfule otherwise false
--
local function debug_internal_tables_dump_func(params)
    
    local command_data = Command_Data()
    local isError, result, values =
        genericCpssApiWithErrorHandler(command_data,"cpssPxHalBpeInternalDbDump", {
            { "IN", "GT_SW_DEV_NUM",            "devNum",           params.device_id }                    
        })
    if isError then
        local error_string = "ERROR calling function cpssPxHalBpeInternalDbDump"
        command_data:addErrorAndPrint(error_string)        
    end
    return isError 
        
end

--------------------------------------------
-- command registration: 
--------------------------------------------
 
-- the command looks like :
-- Console(bpe)# debug-cnc-blocks-set device 0 cnc-bitmap 3
CLI_addCommand("bpe", "debug-cnc-blocks-set", {
    func   = debug_cnc_blocks_set_func,
    help   = "Set the bitmap of CNC Blocks that can be used by 802.1BR refHal for debug",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "cnc-bitmap %cncBitmap", name = "cncBitmap_id", help = "cncBitmap in range 0..3"},
            mandatory = { "device_id", "cncBitmap_id"},
            requirements = {
                ["cncBitmap_id"] = { "device_id" },                
            },                                              
        },        
    }
})

-- the command looks like :
-- Console(bpe)# debug-cnc-config-client device 0 client INGRESS_PROCESSING block-num 1
CLI_addCommand("bpe", "debug-cnc-config-client", {
    func   = debug_cnc_config_client_func,
    help   = "Configure CNC for client",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "client %clientType", name = "client", help = "CNC client"},
            { format = "block-num %blockNum", name = "block_id", help = "CNC block number in range 0..1"},
            mandatory = { "device_id", "client", "block_id"},
            requirements = {
                ["client"] = { "device_id" },     
                ["block_id"] = { "client" }                
            },                                              
        },        
    }
})
 

-- the command looks like :
-- Console(bpe)# debug-cnc-dump device 0 display-all true
CLI_addCommand("bpe", "debug-cnc-dump", {
    func   = debug_cnc_dump_func,
    help   = "Display CNC information for all clients",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "display-all %bool", name = "display_all", help = "display all entries or only valid entries"},           
            mandatory = { "device_id", "display_all"},
            requirements = {
                ["display_all"] = { "device_id" }                    
            },                                              
        },        
    }
})

-- the command looks like :
-- Console(bpe)# debug-ingress-tables-dump device 0 display-all false
CLI_addCommand("bpe", "debug-ingress-tables-dump", {
    func   = debug_ingress_tables_dump_func,
    help   = "Display Ingress tables information",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "display-all %bool", name = "display_all", help = "display all entries or only valid entries"},           
            mandatory = { "device_id", "display_all"},
            requirements = {
                ["display_all"] = { "device_id" }                    
            },                                              
        },        
    }
})

-- the command looks like :
-- Console(bpe)# debug-ingress-errors-dump device 0
CLI_addCommand("bpe", "debug-ingress-errors-dump", {
    func   = debug_ingress_errors_dump_func,
    help   = "Display Ingress Errors information",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },
            mandatory = { "device_id"}                                                         
        },        
    }
})

-- the command looks like :
-- Console(bpe)# debug-egress-tables-dump device 0 display-all true
CLI_addCommand("bpe", "debug-egress-tables-dump", {
    func   = debug_egress_tables_dump_func,
    help   = "Display Egress tables information",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },   
            { format = "display-all %bool", name = "display_all", help = "display all entries or only valid entries"},           
            mandatory = { "device_id", "display_all"},
            requirements = {
                ["display_all"] = { "device_id" }                    
            },                                                                                                                               
        },        
    }
})


-- the command looks like :
-- Console(bpe)# debug-cos-tables-dump device 0
CLI_addCommand("bpe", "debug-cos-tables-dump", {
    func   = debug_cos_tables_dump_func,
    help   = "Display Cos tables information",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },                       
            mandatory = { "device_id"}                                                         
        },        
    }
})

-- the command looks like :
-- Console(bpe)# debug-ingress-hash-tables-dump device 0
CLI_addCommand("bpe", "debug-ingress-hash-tables-dump", {
    func   = debug_ingress_hash_tables_dump_func,
    help   = "Display Ingress Hash tables information",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },                       
            mandatory = { "device_id"}                                                         
        },        
    }
})

-- the command looks like :
-- Console(bpe)# debug-internal-tables-dump device 0 display-all true
CLI_addCommand("bpe", "debug-internal-tables-dump", {
    func   = debug_internal_tables_dump_func,
    help   = "Display Internal tables information",
    params = {
        { type = "named",
            { format = "device %GT_U8", name="device_id", help="device in range GT_U8" },            
            { format = "display-all %bool", name = "display_all", help = "display all entries or only valid entries"},           
            mandatory = { "device_id", "display_all"},
            requirements = {
                ["display_all"] = { "device_id" }                    
            },                                              
        },        
    }
})
