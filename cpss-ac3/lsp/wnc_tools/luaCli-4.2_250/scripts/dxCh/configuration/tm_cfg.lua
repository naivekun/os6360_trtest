--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tm_tree.lua
--*
--* DESCRIPTION:
--*       tm tree building
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
--require("dxCh/configuration/?")

--constants 

-- todo: get tm_hw_params from cpss API

-- later include tm hw params, types, defs from common tm file

-------------------------------- tm hw params --------------------------------
-- BC2 params
local tm_hw_params_bc2 = {}
tm_hw_params_bc2["max_c_node"] = 512
tm_hw_params_bc2["max_b_node"] = 1024
tm_hw_params_bc2["max_a_node"] = 2048
tm_hw_params_bc2["max_q_node"] = 16383

-- BobK params
local tm_hw_params_bk = {}

-- todo: assign tm_hw_params per device ID
local tm_hw_params

if (DeviceFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") then
	tm_hw_params = tm_hw_params_bc2
else
	tm_hw_params = tm_hw_params_bk
end

--------------------------------   end of     --------------------------------
-------------------------------- tm hw params --------------------------------


---------------------------------- tm types ---------------------------------- 


----------------------------------  end of  ---------------------------------- 
---------------------------------- tm types ---------------------------------- 


-------------------------------- tm tree defs -------------------------------- 
-- local CPSS_TM_NO_DROP_PROFILE_CNS = 0

--------------------------------    end of    -------------------------------- 
-------------------------------- tm tree defs -------------------------------- 


----------------------------- tm tree variables ------------------------------ 

-----------------------------      end of       ------------------------------ 
----------------------------- tm tree variables ------------------------------ 


------------------------------- tm tree utils -------------------------------- 

-- enable/disable debug prints
local tm_debug_on = true
local function tm_debug(msg)
    if tm_debug_on == true then
        print (msg)
    end
end


-------------------------------    end of     -------------------------------- 
------------------------------- tm tree utils -------------------------------- 


------------------------------- tm tree callbacks -------------------------------- 

---------------------------------------------------------------------------------- 

local function tm_init_func(params)
--	if (params["devID"]=="all") then devices=wrlDevList() else devices={params["devID"]} end
    local i, devNum
    local devices = wrLogWrapper("wrlDevList")
    local command_data = Command_Data()

    local apiName = "cpssTmInit"
--[[
    GT_STATUS cpssTmInit
    (
        IN GT_U8 devNum
    );
]]--
    for i = 1, #devices do
        devNum = devices[i]
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",                    "devNum",       devNum          }
        })
    end
end

---------------------------------------------------------------------------------- 

local function tm_close_func(params)
--	if (params["devID"]=="all") then devices=wrlDevList() else devices={params["devID"]} end
    local i, devNum
    local devices = wrLogWrapper("wrlDevList")
    local command_data = Command_Data()


    local apiName = "cpssTmClose"
--[[
    GT_STATUS cpssTmClose
    (
        IN GT_U8 devNum
    );
]]--
    for i = 1, #devices do
        devNum = devices[i]
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",                    "devNum",       devNum          }
        })
    end
end

---------------------------------------------------------------------------------- 

local function tm_wred_curve_traditional_func(params)
--  print(to_string(params))

    local i, devNum  
    local devices
    local curve_index
    local command_data = Command_Data()

    local apiName = "cpssTmDropWredTraditionalCurveCreate"
    
    devNum = params.devID
    if (devNum=="all") then devices=wrlDevList() else devices={devNum} end 
    
    local level = params.level
    local max_probability = params["max-probability"]
     
    for i = 1, #devices do
        devNum = devices[i]
        local isFailed,rc,out_param = genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",                    "devNum",       devNum          },
            { "IN",     "CPSS_TM_LEVEL_ENT",        "level",    level },
            { "IN",     "GT_U32",                   "cos",    0 },
            { "IN",     "GT_U32",                   "maxProbability", max_probability},
            { "OUT",     "GT_U32",                   "curve_index"}
          })
        if isFailed == true then
          print ("cpssTmDropWredTraditionalCurveCreate call failed")
        else
          curve_index = out_param.curve_index
          print("created wred curve " .. curve_index)
        end
   end
end


CLI_type_dict["level"] = {
    checker = CLI_check_param_number,
    min=1,
    max=5,
    help="TM node level: 0 for Queue .. 4 for Port"
}

CLI_type_dict["probability"] = {
    checker = CLI_check_param_number,
    min=0,
    max=100,
    help="probability in %"
}


-------------------------------      end of       -------------------------------- 
------------------------------- tm tree callbacks -------------------------------- 


------------------------------- tm tree registrations -------------------------------- 

CLI_addHelp("config", "tm", "TM configuration commands") 

CLI_addCommand("config", "tm close", {
    func   =  tm_close_func,
    help   = "Close TM library.",
    params = {}
})

CLI_addCommand("config", "tm init", {
    func   =  tm_init_func,
    help   = "Initialize TM library.",
    params = {}
})

local wred_curve_traditional___params = {
     {  type="named",
        { format = "device %devID_all", name = "devID",
                help = "a device number"},
              
        { format = "level %level", name = "level",
                help = "TM node level"},

        { format = "max-probability %probability", name = "max-probability",
                help = "WRED mac probability"},


        mandatory = {"devID", "level", "max-probability"}
     }
  }
CLI_addHelp("config", "tm wred-curve", "TM WRED curve commands") 
CLI_addCommand("config", "tm wred-curve traditional", {
    func   =  tm_wred_curve_traditional_func,
    help   = "Create traditional WRED curve",
    params = wred_curve_traditional___params
})


-------------------------------        end of         -------------------------------- 
------------------------------- tm tree registrations -------------------------------- 
