--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* asicSimulaticn.lua
--*
--* DESCRIPTION: commands to allow call so 'asic simulation' functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation")
if not cli_C_functions_registered("wrlSimulationLog", "wrlSimulationSlanManipulations") then
    function wrlSimulationLog()
        return 0x10 -- GT_NOT_SUPPORTED
    end
    function wrlSimulationSlanManipulations()
        return 0x10 -- GT_NOT_SUPPORTED
    end
    function wrlSimulationLogDevFilter()
        return 0x10 -- GT_NOT_SUPPORTED
    end
else
    cmdLuaCLI_registerCfunction("wrlSimulationLogDevFilter")
end


--constants
local ERROR_NOT_SIMULATION_CNS = "ERROR : command supported only on 'simulation' "

local mode_stop                     = 0
local mode_start                    = 1
local mode_start_with_from_cpu      = 2
local mode_start_full               = 3

local function internal_func_startSimulationLog(params,mode)
    local param_full_path_name 
    if params.param_full_path_name then
        -- remove '"' 
        param_full_path_name = string.gsub(params.param_full_path_name , "\"" , "")
        
        --print("param_full_path_name = " .. param_full_path_name)
    end

    if wrlCpssIsAsicSimulation() then
        wrlSimulationLog(mode,param_full_path_name)
    else
        printLog(ERROR_NOT_SIMULATION_CNS)
    end
end

local function func_startSimulationLog(params)
    internal_func_startSimulationLog(params,mode_start)
end
local function func_startSimulationLogWithFromCpu(params)
    internal_func_startSimulationLog(params,mode_start_with_from_cpu)
end
local function func_startSimulationLogFull(params)
    internal_func_startSimulationLog(params,mode_start_full)
end
local function func_stopSimulationLog()
    if wrlCpssIsAsicSimulation() then
        wrlSimulationLog(mode_stop)
    else
        printLog(ERROR_NOT_SIMULATION_CNS)
    end
end

-------------------------------------------------------
-- commands registration
-------------------------------------------------------
CLI_addHelp("debug", "simulation", "Commands relevant only to simulation environment (WM/GM)")

local help_full_path_name = "(optional) Full path of the log file to create (including file name)"
local param_full_path_name = { format="full-path-name %s",    name = "param_full_path_name" ,       help = help_full_path_name}


CLI_addCommand("debug", "simulation startSimulationLog", {
    func=func_startSimulationLog,
    help="Start the simulation log",
    params={ 
        { type="named",
            param_full_path_name            
        }
    }
})
CLI_addCommand("debug", "simulation startSimulationLogWithFromCpu", {
    func=func_startSimulationLogWithFromCpu,
    help="Start the simulation log (include 'from_cpu' traffic)",
    params={ 
        { type="named",
            param_full_path_name            
        }
    }
})
CLI_addCommand("debug", "simulation startSimulationLogFull", {
    func=func_startSimulationLogFull,
    help="Start the simulation log (include 'from_cpu' traffic and memory access)",
    params={ 
        { type="named",
            param_full_path_name            
        }
    }
})

CLI_addCommand("debug", "simulation stopSimulationLog", {
    func=func_stopSimulationLog,
    help="Disable the simulation from sending trace into the log"
})

---
--  func_slanConnectDisconnect
--        @description  Connects or disconnects port to SLAN
--
--        @param
--           slan_mode      - SLAN Mode: 0 - connect, 1 - disconnect
--           params         - The parameters
--
--        @return       none
--
local function func_slanConnectDisconnect(slan_mode, params)

    local devNum, portNum
    local keep_slan_connected

    --_debug_to_string(params);
    
    -- get device and port from dev_port table
    devNum         = params["dev_port"]["devId"]
    portNum        = params["dev_port"]["portNum"]

    -- can be nil
    keep_slan_connected = params["keep_slan_connected"]
    
    if wrlCpssIsAsicSimulation() then
        wrlSimulationSlanManipulations(slan_mode ,devNum, portNum, params.slan_name , keep_slan_connected)
    else
        printLog(ERROR_NOT_SIMULATION_CNS)
    end

end

CLI_addCommand("debug", "simulation slan connect ethernet", {
    func=function(params)
        return func_slanConnectDisconnect(0 --[[bind]], params)
    end,
    help="Connect port to SLAN",
    params = {
        {
            type = "values",
                { format = "%dev_port",  name="dev_port", help="The ethernet port" },
                { format = "%slan_name", name="slan_name", help="SLAN name, up to 8 symbols string e.g. SLAN_23" },
            mandatory = {"dev_port"}
        },
        {
            type = "named",
            { format = "keep-other-ports-on-slan %enable_disable",   name="keep_slan_connected",   help="(optional) indication to keep (or detach) the slan connected to other ports" }
        }
    }
})

CLI_addCommand("debug", "no simulation slan connect", {
    func=function(params)
        return func_slanConnectDisconnect(1 --[[unbind]], params)
    end,
    help="Disconnect port from SLAN",
    params = {
        {
            type = "named",
            { format = "ethernet %dev_port",   name="dev_port",   help="The ethernet port" },
            mandatory = {"dev_port"}
        }
    }
})
-------------------------------------------------------
-- new command : simulation log filter device
-------------------------------------------------------
local function func_logFilterDevice(params)
    local command_data = Command_Data()

    if not wrlCpssIsAsicSimulation() then
        printLog(ERROR_NOT_SIMULATION_CNS)
        return
    end

    local doFilter = not params.flagNo
    local result = wrlSimulationLogDevFilter(params.devID,params.sim_log_dev_filter_type,doFilter)
    if 0 ~= result then
        command_data:addErrorAndPrint(returnCodes[result]) -- print and reset the errors string array 
    end 
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults() 
end

local sim_log_dev_filter_type_help = "simulation log device filter type"
local sim_log_dev_filter_type_enum = {
        ["filter_all_exclude_me"]   = { value=0, help="filter all other devices (exclude me)" },
        ["filter_only_me"]          = { value=1, help="filter only my device (exclude others)" },
        ["filter_me"]               = { value=2, help="add me to filtered devices (that already filtered)" }
   }
--[[
CLI_type_dict["pip_priority_enum"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = pip_help_pip_priority,
    enum = pip_pririty_enum
} 
]]--
CLI_addParamDictAndType_enum("sim_log_dev_filter_type_enum","sim_log_dev_filter_type",sim_log_dev_filter_type_help, sim_log_dev_filter_type_enum)
  
CLI_addHelp("debug", "simulation log ", "simulation log related")
CLI_addHelp("debug", "simulation log filter ", "allow to define filters to the log")
CLI_addHelp("debug", "simulation log filter device", "allow to filter device from log")
CLI_addCommand("debug", "simulation log filter device", {
    func=func_logFilterDevice,
    help="allow to filter device from log",
    params = {
        {
            type = "values",
            { format="%devID", name="devID", help="The device number" },
            { format="%sim_log_dev_filter_type_enum", name="sim_log_dev_filter_type", help=sim_log_dev_filter_type_help },
            mandatory = {"devID","sim_log_dev_filter_type"}
        }
    }
})

CLI_addCommand("debug", "no simulation log filter device", {
    func=function(params)
        params.flagNo = true 
        return func_logFilterDevice(params)
    end,
    help="allow to disable the filter device from log",
    params = {
        {
            type = "values",
            { format="%devID", name="devID", help="The device number" },
            { format="%sim_log_dev_filter_type_enum", name="sim_log_dev_filter_type", help=sim_log_dev_filter_type_help },
            mandatory = {"devID","sim_log_dev_filter_type"}
        }
    }
})

