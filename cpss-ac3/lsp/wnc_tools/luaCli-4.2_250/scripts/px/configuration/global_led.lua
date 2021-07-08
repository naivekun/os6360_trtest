--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* global_led.lua
--*
--* DESCRIPTION:
--*       LED interface related commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--constants
local help_led_interface = "A list of LED interfaces. Examples: 0-4; 1,3\n"

local function checkLedClassRange(range,devNum)
    if range[table.getn(range)]>=6 then
        print("Invalid LED class: maximum index of LED class is 6")
        return false
    else
        return true
    end
end

local function fillClassRange(devNum)
    local class_range={}
    for iter=1,6 do
        class_range[iter]=iter-1
    end
    return class_range
end

local function checkLedClassNum(params, devNum)
    local numOfLedClasses=6
    if (not params.a) then
        print("Parameter class-a is mandatory")
        return false
    end
    if (not params.b) then
        print("Parameter class-b is mandatory")
        return false
    end
    if (not params.c) then
        print("Parameter class-c is mandatory")
        return false
    end
    if (not params.d) then
        print("Parameter class-d is mandatory")
        return false
    end
    if (params.a>=6) or
       (params.b>=6) or
       (params.c>=6) or
       (params.d>=6) then
        print("Invalid LED class: the maximum index of LED class is 5")
        return false
    else
        return true
    end
end

local function ledInterfaceGroupHandler(command_data, params, devNum)
    local groupParamsPtr
    local rc
    local i
    local groupNum

    if (params.flagNo==false or params.flagNo==nil) then
        if checkLedClassNum(params, devNum)==false then
            return false
        end
        groupParamsPtr={classA=params.a,classB=params.b,classC=params.c,classD=params.d}
    else
        groupParamsPtr={classA=0,classB=0,classC=0,classD=0}
    end

    for groupNum=0,1 do
        rc= myGenWrapper("cpssPxLedStreamGroupConfigSet",{
                    { "IN","GT_SW_DEV_NUM","devNum",devNum },
                    { "IN","GT_U32","groupNum",groupNum },
                    { "IN","CPSS_LED_GROUP_CONF_STC","groupParamsPtr",groupParamsPtr }
                })
        command_data:handleCpssErrorDevice(
            rc,
            string.format("setting LED group classes "),
            devNum)
    end
    return true
end

local function ledStreamParamSet(command_data, params, values)
    local enable = not (params.flagNo == true)
    if enable == true then
        if params["ledOrder"] ~= nil then
            values["ledConfPtr"]["ledOrganize"] = params["ledOrder"]

        elseif params["disableOnLinkDown"] ~= nil then
            values["ledConfPtr"]["disableOnLinkDown"] = true

        elseif params["streamLength"] ~= nil then
            if params["start"] == nil then
                params["start"] = values["ledConfPtr"]["ledStart"]
            end
            if params["end"] == nil then
                params["end"] = values["ledConfPtr"]["ledEnd"]
            end

            if params["start"] > params["end"] then
                print("ERROR : stream bit start:".. params["start"] .. " greater than stream bit end:".. params["end"])
                command_data:setFailStatus()
                command_data:setFailLocalStatus()
                return false
            elseif (params["end"] - params["start"]) == 0 then
                print("ERROR : zero length stream")
                command_data:setFailStatus()
                command_data:setFailLocalStatus()
                return false
            end
            values["ledConfPtr"]["ledStart"] = params["start"]
            values["ledConfPtr"]["ledEnd"] = params["end"]

        elseif params["blinkCfg"] ~= nil then
            local blinkDutyCycle = "blink"..params["blinkCfg"].."DutyCycle"
            local blinkDuration = "blink"..params["blinkCfg"].."Duration"
            if params["dutyCycle"] then
                values["ledConfPtr"][blinkDutyCycle] = params["dutyCycle"]
            end
            if params["duration"] then
                values["ledConfPtr"][blinkDuration] = params["duration"]
            end

        elseif params["stretch"] ~= nil then
            values["ledConfPtr"]["pulseStretch"] = params["stretch"]

        elseif params["ledClockFrequency"] ~= nil then
            values["ledConfPtr"]["ledClockFrequency"]=params["ledClockFrequency"]
        end

    -- No commands
    else

        if params["ledOrder"] ~= nil then
            values["ledConfPtr"]["ledOrganize"] = "CPSS_LED_ORDER_MODE_BY_CLASS_E"

        elseif params["disableOnLinkDown"] ~= nil then
            values["ledConfPtr"]["disableOnLinkDown"] = false

        elseif params["streamLength"] ~= nil then
            values["ledConfPtr"]["ledStart"] = 0
            values["ledConfPtr"]["ledEnd"] = 255

        elseif params["blinkCfg"] ~= nil then
            local blinkDutyCycle = "blink"..params["blinkCfg"].."DutyCycle"
            local blinkDuration = "blink"..params["blinkCfg"].."Duration"
            values["ledConfPtr"][blinkDutyCycle] = "CPSS_LED_BLINK_DUTY_CYCLE_0_E"
            values["ledConfPtr"][blinkDuration] = "CPSS_LED_BLINK_DURATION_1_E"

        elseif params["stretch"] ~= nil then
            values["ledConfPtr"]["pulseStretch"] = "CPSS_LED_PULSE_STRETCH_1_E"

        elseif params["ledClockFrequency"] ~= nil then
            values["ledConfPtr"]["ledClockFrequency"]="CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E"
        end
    end
end

-- implement 'per device' : 'config' --> global config commands
local function ledStreamGlobal(command_data, devNum, params)
    --check if selected command is "group"
    if (params.group) then
        local rc=ledInterfaceGroupHandler(command_data, params, devNum, interface)
    else
        local apiName = "cpssPxLedStreamConfigGet"
        local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_SW_DEV_NUM",           "devNum",       devNum },
            { "OUT",    "CPSS_PX_LED_CONF_STC",    "ledConfPtr"}
        })
        if isError then
            return
        end
        ledStreamParamSet(command_data, params, values)
        apiName = "cpssPxLedStreamConfigSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_SW_DEV_NUM",           "devNum",       devNum },
            { "IN",     "CPSS_PX_LED_CONF_STC",    "ledConfPtr",   values["ledConfPtr"]}
        })
    end
end

local function ledStreamGlobalFunc(params)
    return generic_all_device_func(ledStreamGlobal, params)
end

--led class manipulation function
local function ledClassManipulation(params)
    local classParams
    local command_data = Command_Data()
    local class_range
    local rc, value
    local devices = (params.devNum=="all") and wrLogWrapper("wrlDevList") or {params.devNum}

    --Checking LED interface and class range for selected devices
    for dev_iter,devNum in pairs(devices) do
        class_range=(params.class_range=="all") and fillClassRange(devNum) or params.class_range
         --Checking LED class range
        if checkLedClassRange(class_range,devNum)==false then
            return
        end
    end

    for dev_iter, devNum in pairs(devices) do
        class_range=(params.class_range=="all") and fillClassRange(devNum) or params.class_range
        for class_iter, classNum in pairs(class_range) do
            rc, value= myGenWrapper("cpssPxLedStreamClassManipulationGet",{
                { "IN","GT_SW_DEV_NUM","devNum",devNum },
                { "IN","GT_U32","classNum",classNum },
                { "OUT","CPSS_PX_LED_CLASS_MANIPULATION_STC","classParamsPtr" }
            })
            command_data:handleCpssErrorDevice(
                rc,
                string.format("getting LED stream class manipulation configuration, class #"..classNum),
                devNum)
            if (rc==0) then
                classParams=value["classParamsPtr"]
                if (params["use_blink"]) then
                    if (params.flagNo==true) then
                        classParams["blinkEnable"]=false
                        classParams["blinkSelect"]=0
                    else
                        classParams["blinkEnable"]=true
                        classParams["blinkSelect"]=params["use_blink"]
                    end
                elseif (params["force"]) then
                    if (params.flagNo==true) then
                        classParams["forceEnable"]=false
                        classParams["forceData"]=0
                    else
                        classParams["forceEnable"]=true
                        classParams["forceData"]=params["force"]
                    end
                elseif (params["enable_stretch"]) then
                    classParams["pulseStretchEnable"]=not params.flagNo
                elseif (params["disable_on_link_down"]) then
                    classParams["disableOnLinkDown"]=not params.flagNo
                end
                rc= myGenWrapper("cpssPxLedStreamClassManipulationSet",{
                    { "IN","GT_SW_DEV_NUM","devNum",devNum },
                    { "IN","GT_U32","classNum",classNum },
                    { "IN","CPSS_PX_LED_CLASS_MANIPULATION_STC","classParamsPtr",classParams }
                })
                command_data:handleCpssErrorDevice(
                    rc,
                    string.format("setting LED stream class manipulation configuration "),
                    devNum)
            end
        end
    end
    command_data:updateStatus()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

-- *****************************************************************************
--  callApiWithOneParamForDevices
--
--  @description iterate through all devices specified by parameter and call
--               a CPSS function with parameters devNum and <user's parameter>.
--               An API name and parameter's type/name/value are passed
--               through function's arguments
--
--  @param dev       - a device number or 'all'.
--  @param api       - a CPSS API name
--  @param ptype     - a CPSS function parameter's type
--  @param pname     - a CPSS function parameter's name
--  @param pvalue    - a CPSS function parameter's value
--
--  @return  true on success, otherwise false and error message
--
-- *****************************************************************************
local function callApiWithOneParamForDevices(dev, api, ptype, pname, pvalue)
    local devices = (dev == "all") and wrLogWrapper("wrlDevList") or {dev}
    local command_data = Command_Data()

    for _,devNum in ipairs(devices) do
        command_data:clearLocalStatus()

        local rc = myGenWrapper(
            api, {
                {"IN", "GT_SW_DEV_NUM", "devNum", devNum},
                {"IN", ptype, pname, pvalue}})
        if rc == 0x1e or rc == 0x10 then  -- GT_NOT_APPLICABLE_DEVICE, GT_NOT_SUPPORTED
            command_data:addWarning("The device #" ..devNum..
                      " doesn't support this command. Ignored.")
        elseif rc ~= 0 then
            command_data:setFailLocalStatus()
            command_data:addError(returnCodes[rc])
        end
        command_data:updateStatus()
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--********************** CLI COMMANDS DECLARATION ******************************

--------------------------------------------
-- command registration: stream-parameters
--------------------------------------------
CLI_addCommand("config", "led interface", {
  func=ledStreamGlobalFunc,
  help="Configure a LED interface data stream ordering",
  params={
        { type="named",
            { format="device %devID_all",  name="all_device",    help="The device number\n" } ,

            -- led interface stream-order-by
            { format="stream-order-by %ledOrder", name = "ledOrder", help = "LED stream ordering mode" },

            -- led interface stream-length
            { format="stream-length", name = "streamLength", help = "Set LED interface stream length " },
                { format="start %streamBitPosition", name = "start", help = "Set start significant bit in stream " },
                { format="end %streamBitPosition", name = "end", help = "Set end significant bit in stream " },
--                    requirements={["start"]={"streamLength"}, ["end"]={"start"}},

            -- led interface blink-cfg
            { format="blink-cfg %blinkNumber", name = "blinkCfg", help = "Set a LED interface blink configurations" },
                { format="duty-cycle %dutyCycle", name = "dutyCycle", help = "Set a LED blink duty cycle signal" },
                { format="duration  %duration", name = "duration", help = "Set a divider of blink frequency" },
--                    requirements={["dutyCycle"]={"blinkCfg"}, ["duration"]={"dutyCycle"}},

            -- led interface stretch-cfg
            { format="stretch-cfg %stretch", name = "stretch", help = "Configure the indication stretching divider" },

            -- led interface LEDCLK frequency
            { format="clock-freq %clockOutFrequency", name="ledClockFrequency", help="LED output clock frequency" },

            { format="group %LED_group_num", name="group", help="LED group" },
            -- led interface group
            { format="class-a %LED_class_num", name="a", help="class A number" },
            { format="class-b %LED_class_num", name="b", help="class B number" },
            { format="class-c %LED_class_num", name="c", help="class C number" },
            { format="class-d %LED_class_num", name="d", help="class D number" },
            requirements={["dutyCycle"]={"blinkCfg"}, ["duration"]={"blinkCfg"},
                ["start"] = {"streamLength"}, ["end"] = {"streamLength"},
                ["a"] = {"group"},
                ["b"] = {"group"},
                ["c"] = {"group"},
                ["d"] = {"group"}
            },

            alt = { any_stream_param = { "ledOrder", "disableOnLinkDown", "streamLength", "blinkCfg", "stretch", "clockInvert" ,"ledClockFrequency",
                "dualMediaClass5", "dualMediaClass13", "group", "directModeEn"}},
            mandatory = { "any_stream_param", "all_device" }
      }
  }
})

--------------------------------------------
-- command registration: no stream-parameters
--------------------------------------------
CLI_addCommand("config", "no led interface", {
func=function(params)
     params.flagNo=true
     return ledStreamGlobalFunc(params)
  end,
 help="Returns list values to default",
 params={
       { type="named",
           { format="device %devID_all",  name="all_device",    help="The device number\n" } ,
           { format="stream-order-by", name = "ledOrder", help = "Set LED stream ordering mode by class" },
           { format="stream-length", name = "streamLength", help = "Set LED interface default stream length " },
           { format="blink-cfg %blinkNumber", name = "blinkCfg", help = "Set a LED interface default blink configurations" },
           { format="stretch-cfg", name = "stretch", help = "Configure the indication stretching divider" },
           { format="clock-freq", name="ledClockFrequency", help="LED output clock frequency" },
           { format="group %LED_group_num", name="group", help="LED group" },

           alt = { any_stream_param = { "ledOrder", "disableOnLinkDown",
            "streamLength", "blinkCfg", "stretch", "ledClockFrequency", "group"}},
           mandatory = { "any_stream_param", "all_device" }
       }
    }
})
--------------------------------------------------------------------------------
-------------------------------------------------
--LED class manipulation command definition
-------------------------------------------------
CLI_addCommand("config", "led class", {
    func   = ledClassManipulation,
    help   = "LED interface class manipulation",
    params={
        {
            type = "values",
            { format="%LED_class_range", name = "class_range" }
        },
        { type="named",
            { format="device %devID_all", name="devNum", help="device id" },
            { format="use-blink %LED_blink_select",name="use_blink" },
            { format="force %LED_force_value", name="force" },
            { format="enable-stretch", name="enable_stretch" },
            { format="disable-on-link-down",name="disable_on_link_down" },
            alt = {cmd={"use_blink","force","enable_stretch","disable_on_link_down","indication_name"}},
            mandatory = {"devNum", "cmd"}
        }
    }
})

CLI_addCommand("config", "no led class", {
    func=function(params)
        params.flagNo=true
        return ledClassManipulation(params)
    end,
    help   = "LED interface class manipulation",
    params={
        {
            type = "values",
            { format="%LED_class_range", name = "class_range" }
        },
        {
            type="named",
            { format="device %devID_all", name="devNum", help="device id" },
            { format="use-blink",name="use_blink" },
            { format="force", name="force" },
            { format="enable-stretch", name="enable_stretch" },
            { format="disable-on-link-down",name="disable_on_link_down" },
            alt = { cmd={ "use_blink","force","enable_stretch","disable_on_link_down","indication_name"}},
            mandatory = { "devNum", "cmd" }

        }
    }
})

local cmdParamsDevIdOnly = {{ type = "named",
                              {format = "device %devID_all", name = "devNum"},
                              mandatory = {"devNum"}}}

CLI_addHelp("config", "led", "configure LED specific settings (interfaces, classes, etc")
