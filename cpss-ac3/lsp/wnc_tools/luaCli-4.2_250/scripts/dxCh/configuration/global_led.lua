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

-- includes
cmdLuaCLI_registerCfunction("wrlCpssDeviceFamilyGet")

--constants
local help_led_interface = "A list of LED interfaces. Examples: 0-4; 1,3\n"

-- *******LED_IF_INFO Table******
-- LED_IF_INFO table contains device information that related to LED interface
-- Fields of structure
--        numOfLedInterfacesInCore -- number of LED interfaces in device core
--        numOfLedClasses     -- number of LED classes
--        numOfCores          -- number of PP cores in one device
--        portTypeRelevant    -- variable that indicate that device
--                               needs portType in LED interface commands
LED_IF_INFO={
    CPSS_PP_FAMILY_DXCH_BOBCAT3_E={
        numOfLedInterfacesInCore=4,
        numOfLedClasses=6,
        numOfCores=1,
        portTypeRelevant=false
    },

    CPSS_PP_FAMILY_DXCH_ALDRIN2_E={
        numOfLedInterfacesInCore=4,
        numOfLedClasses=6,
        numOfCores=1,
        portTypeRelevant=false
    },

    CPSS_PP_FAMILY_DXCH_ALDRIN_E={
        numOfLedInterfacesInCore=2,
        numOfLedClasses=6,
        numOfCores=1,
        portTypeRelevant=false
    },
    CPSS_PP_FAMILY_DXCH_AC3X_E={
        numOfLedInterfacesInCore=2,
        numOfLedClasses=6,
        numOfCores=1,
        portTypeRelevant=false
    },
    CPSS_PP_FAMILY_DXCH_BOBCAT2_E={
        numOfLedInterfacesInCore=5,
        numOfLedClasses=6,
        numOfCores=1,
        portTypeRelevant=false
    },
    CPSS_PP_FAMILY_DXCH_LION_E={
        numOfLedInterfacesInCore=1,
        numOfLedClasses=14,
        numOfCores=4,
        portTypeRelevant=false
    },
    CPSS_PP_FAMILY_DXCH_LION2_E={
        numOfLedInterfacesInCore=1,
        numOfLedClasses=14,
        numOfCores=8,
        portTypeRelevant=false
    },
    CPSS_PP_FAMILY_CHEETAH_E={
        numOfLedInterfacesInCore=1,
        numOfLedClasses=14,
        numOfCores=1,
        portTypeRelevant=true
    },
    CPSS_PP_FAMILY_CHEETAH2_E={
        numOfLedInterfacesInCore=1,
        numOfLedClasses=14,
        numOfCores=1,
        portTypeRelevant=true
    },
    CPSS_PP_FAMILY_CHEETAH3_E={
        numOfLedInterfacesInCore=1,
        numOfLedClasses=14,
        numOfCores=1,
        portTypeRelevant=true
    },
    CPSS_PP_FAMILY_DXCH_XCAT_E={
        numOfLedInterfacesInCore=2,
        numOfLedClasses=14,
        numOfCores=1,
        portTypeRelevant=true
    },
    CPSS_PP_FAMILY_DXCH_XCAT2_E={
        numOfLedInterfacesInCore=2,
        numOfLedClasses=14,
        numOfCores=1,
        portTypeRelevant=true
    },
    CPSS_PP_FAMILY_DXCH_XCAT3_E={
        numOfLedInterfacesInCore=2,
        numOfLedClasses=14,
        numOfCores=1,
        portTypeRelevant=true
    }
}


-- ************************************************************************
--
--  getLedInfoFieldForDevice
--        @description  returns field value from LED_IF_INFO structure for the specified device number
--
--        @param field            - name of field in LED_IF_INFO substructure
--
--        @param devNum           - device number
--
--        @return                 rc,value
--                                -rc=false device isn't included in LED_IF_INFO table
--                                -rc=true device is found in LED_IF_INFO table
--                                -value - value of the specified field
--
local function getLedInfoFieldForDevice(field,devNum)
    local value
    local devFamily=wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
    local info
    if (devFamily~=nil) then
        local info=LED_IF_INFO[devFamily]
        if (info~=nil) then
            value=info[field]
            return true,value
        else
            print("Device Family not found in LED_IF_INFO structure")
        end
    end
    return false
end

local function getNumLedInterfaces(devNum)
    local rc,value1,value2
    rc,value1=getLedInfoFieldForDevice("numOfLedInterfacesInCore",devNum)
    if rc==true then
        rc,value2=getLedInfoFieldForDevice("numOfCores",devNum)
        if rc==true then
            return value1*value2
        else
            return 0
        end
    else
        return 0
    end
end

local function getNumLedClasses(devNum)
    local rc,value
    rc,value=getLedInfoFieldForDevice("numOfLedClasses",devNum)
    if rc==true then
        return value
    else
        return 0
    end
end

local function checkLedInterfaceRange(range,devNum)
    local numOfLEDIfInDevice=getNumLedInterfaces(devNum)
    if range[table.getn(range)]>=numOfLEDIfInDevice then
        print("Invalid LED interface range: for device family "..wrlCpssDeviceFamilyGet(devNum).." the maximum index of LED interface is "..numOfLEDIfInDevice-1)
        return false
    else
        return true
    end
end

local function checkLedClassRange(range,devNum)
    local numOfLedClasses=getNumLedClasses(devNum)
    if range[table.getn(range)]>=numOfLedClasses then
        print("Invalid LED class: for device family "..wrlCpssDeviceFamilyGet(devNum).." the maximum index of LED class is "..numOfLedClasses-1)
        return false
    else
        return true
    end
end

-- ************************************************************************
---
--  getPortGroupBmpAndInterface
--        @description  returns port group bitmap and physical number led interface
--
--        @param ledIfInfo          - structure that contains LED interface related information. Can be read from LED_IF_INFO structure
--
--        @param numOfLedInterface  - number of led interface
--
--        @return                   portGroupsBmp, interface

local function getPortGroupBmpAndInterface(ledIfInfo,numOfLedInterface)
    local core=math.floor(numOfLedInterface/ledIfInfo.numOfLedInterfacesInCore)
    local interface=math.fmod(numOfLedInterface,ledIfInfo.numOfLedInterfacesInCore)
    return math.pow(2,core),interface
end
local function fillClassRange(devNum)
    local class_range={}
    for iter=1,getNumLedClasses(devNum) do
        class_range[iter]=iter-1
    end
    return class_range
end
local function fillInterfaceRange(devNum)
    local if_range={}
    for iter=1,getNumLedInterfaces(devNum) do
        if_range[iter]=iter-1
    end
    return if_range
end


local function checkLedClassNum(params, devNum)
    local numOfLedClasses=getNumLedClasses(devNum)
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
    if (params.a>=numOfLedClasses) or
       (params.b>=numOfLedClasses) or
       (params.c>=numOfLedClasses) or
       (params.d>=numOfLedClasses) then
        print("Invalid LED class: for device family "..wrlCpssDeviceFamilyGet(devNum).." the maximum index of LED class is "..numOfLedClasses-1)
        return false
    else
        return true
    end
end
local function ledInterfaceGroupHandler(command_data, params, devNum, interface)
    local portGroupsBmp,ledInterfaceNum
    local groupParamsPtr
    local rc
    local portTypes
    local pType=params.portType
    local ledIfInfo=LED_IF_INFO[wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)]
    if (params.flagNo==false or params.flagNo==nil) then
        if checkLedClassNum(params, devNum)==false then
            return false
        end
        groupParamsPtr={classA=params.a,classB=params.b,classC=params.c,classD=params.d}
    else
        groupParamsPtr={classA=0,classB=0,classC=0,classD=0}
    end
    portGroupsBmp,ledInterfaceNum=getPortGroupBmpAndInterface(ledIfInfo,interface)
    if pType==nil then
        if ledIfInfo.portTypeRelevant==true then
            pType="all"
        else
            pType="CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E"
        end
    end
    portTypes=(pType=="all") and {"CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E","CPSS_DXCH_LED_PORT_TYPE_XG_E"} or {pType}
    for port_type_iter, portType in pairs(portTypes) do
        rc= myGenWrapper("cpssDxChLedStreamPortGroupGroupConfigSet",{
                    { "IN","GT_U8","devNum",devNum },
                    { "IN","GT_PORT_GROUPS_BMP","portGroupsBmp",portGroupsBmp },
                    { "IN","GT_U32","ledInterfaceNum",ledInterfaceNum },
                    { "IN","CPSS_DXCH_LED_PORT_TYPE_ENT","portType",portType },
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

        elseif params["clockInvert"] ~= nil then
            values["ledConfPtr"]["clkInvert"] = true

        elseif params["ledClockFrequency"] ~= nil then
            values["ledConfPtr"]["ledClockFrequency"]=params["ledClockFrequency"]

        elseif params["dualMediaClass5"] ~= nil then
            values["ledConfPtr"]["class5select"]=params["dualMediaClass5"]

        elseif params["dualMediaClass13"] ~= nil then
            values["ledConfPtr"]["class13select"]=params["dualMediaClass13"]
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

        elseif params["clockInvert"] ~= nil then
            values["ledConfPtr"]["clkInvert"] = false

        elseif params["ledClockFrequency"] ~= nil then
            values["ledConfPtr"]["ledClockFrequency"]="CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E"

        elseif params["dualMediaClass5"] ~= nil then
            values["ledConfPtr"]["class5select"]="CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E"

        elseif params["dualMediaClass13"] ~= nil then
            values["ledConfPtr"]["class13select"]="CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E"

        end
    end
end

--*****************************************************************************
-- ledIfDirectModeEn
--
-- @description  Enable direct-mode on LED interfaces.
--               Relevant for xCat, xCat2, xCat3 devices only
--
-- @param  commanda_data - a Command_Data() instance. Used for errors handling
-- @param  devNum - a device number
-- @params - a Lua CLI command's parameters
--
-- @return nothing
--
local function ledIfDirectModeEn(command_data, devNum, ledIf, params)
    local family = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
    if family ~= "CPSS_PP_FAMILY_DXCH_XCAT_E" and
       family ~= "CPSS_PP_FAMILY_DXCH_XCAT2_E" and
       family ~= "CPSS_PP_FAMILY_DXCH_XCAT3_E"
    then
        return
    end

    local enable =  not params.flagNo

    local isError, result = genericCpssApiWithErrorHandler(
        command_data,
        "cpssDxChLedStreamDirectModeEnableSet",
        {
            { "IN", "GT_U8",   "devNum",          devNum},
            { "IN", "GT_U32",  "ledInterfaceNum", ledIf},
            { "IN", "GT_BOOL", "enable",          enable}
    })
end


-- implement 'per device' : 'config' --> global config commands
local function ledStreamGlobal(command_data, devNum, params)
    local ledif_range=(params.ledif_range=="all") and fillInterfaceRange(devNum) or params.ledif_range
    if checkLedInterfaceRange(ledif_range,devNum)==true then
        for i,interface in pairs(ledif_range) do
            --check if selected command is "group"
            if (params.group) then
                local rc=ledInterfaceGroupHandler(command_data, params, devNum, interface)
            elseif params.directModeEn then
                ledIfDirectModeEn(command_data, devNum, interface, params)
            else
                local ledIfInfo=LED_IF_INFO[wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)]
                local portGroupsBmp,ledInterfaceNum=getPortGroupBmpAndInterface(ledIfInfo,interface)
                local apiName = "cpssDxChLedStreamPortGroupConfigGet"
                local isError , result, values =
                genericCpssApiWithErrorHandler(command_data,
                    apiName, {
                    { "IN",     "GT_U8",                "devNum",             devNum },
                    { "IN",     "GT_PORT_GROUPS_BMP",  "portGroupsBmp",       portGroupsBmp },
                    { "IN",     "GT_U32",               "ledInterfaceNum",    ledInterfaceNum },
                    { "OUT",    "CPSS_LED_CONF_STC",    "ledConfPtr"}
                })
                if isError then
                    return
                end
                ledStreamParamSet(command_data, params, values)
                apiName = "cpssDxChLedStreamPortGroupConfigSet"
                genericCpssApiWithErrorHandler(command_data,
                    apiName, {
                    { "IN",     "GT_U8",                "devNum",             devNum },
                    { "IN",     "GT_PORT_GROUPS_BMP",  "portGroupsBmp",       portGroupsBmp },
                    { "IN",     "GT_U32",               "ledInterfaceNum",    ledInterfaceNum },
                    { "IN",     "CPSS_LED_CONF_STC",    "ledConfPtr",         values["ledConfPtr"]}
                })
            end
        end
    end
end

local function ledStreamGlobalFunc(params)
    return generic_all_device_func(ledStreamGlobal, params)
end

--led class manipulation function
local function ledClassManipulation(params)
    local portGroupsBmp
    local classParams
    local command_data = Command_Data()
    local ledif_range
    local class_range
    local ledIfInfo
    local rc, value
    local devices = (params.devNum=="all") and wrLogWrapper("wrlDevList") or {params.devNum}
    local pType = params.portType
    --Checking LED interface and class range for selected devices
    for dev_iter,devNum in pairs(devices) do
        ledif_range=(params.ledif_range=="all") and fillInterfaceRange(devNum) or params.ledif_range
        class_range=(params.class_range=="all") and fillClassRange(devNum) or params.class_range
        if checkLedInterfaceRange(ledif_range,devNum)==false then
            return
        end
         --Checking LED class range
        if checkLedClassRange(class_range,devNum)==false then
            return
        end
    end
    --Check for portType parameter. Default value of portType parameter is all.
    --If device doesn't support portType argument portType is set to tri-speed
    if pType==nil then
        ledIfInfo=LED_IF_INFO[wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)]
        if ledIfInfo.portTypeRelevant==true then
            pType="all"
        else
            pType="CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E"
        end
    end
    local portTypes=(pType=="all") and {"CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E","CPSS_DXCH_LED_PORT_TYPE_XG_E"} or {pType}
    for port_type_iter, portType in pairs(portTypes) do
        for dev_iter, devNum in pairs(devices) do
            ledif_range=(params.ledif_range=="all") and fillInterfaceRange(devNum) or params.ledif_range
            class_range=(params.class_range=="all") and fillClassRange(devNum) or params.class_range
            ledIfInfo=LED_IF_INFO[wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)]
            for class_iter, classNum in pairs(class_range) do
                for if_iter, numOfLedInterface in pairs(ledif_range) do
                    if (params["indication_name"]) then
                        local indication=(params.flagNo==true) and "CPSS_DXCH_LED_INDICATION_PRIMARY_E" or params["indication_name"]
                        rc= myGenWrapper("cpssDxChLedStreamClassIndicationSet",{
                            { "IN","GT_U8","devNum",devNum },
                            { "IN","GT_U32","ledInterfaceNum",ledInterfaceNum },
                            { "IN","GT_U32","classNum",classNum },
                            { "IN","CPSS_DXCH_LED_INDICATION_ENT","indication", indication}
                        })
                        command_data:handleCpssErrorDevice(
                            rc,
                            string.format("configuring indication for flex ports "),
                            devNum)
                    else
                        portGroupsBmp,ledInterfaceNum=getPortGroupBmpAndInterface(ledIfInfo,numOfLedInterface)
                        rc, value= myGenWrapper("cpssDxChLedStreamPortGroupClassManipulationGet",{
                            { "IN","GT_U8","devNum",devNum },
                            { "IN","GT_PORT_GROUPS_BMP","portGroupsBmp",portGroupsBmp },
                            { "IN","GT_U32","ledInterfaceNum",ledInterfaceNum },
                            { "IN","CPSS_DXCH_LED_PORT_TYPE_ENT","portType", portType },
                            { "IN","GT_U32","classNum",classNum },
                            { "OUT","CPSS_LED_CLASS_MANIPULATION_STC","classParamsPtr" }
                        })
                        command_data:handleCpssErrorDevice(
                            rc,
                            string.format("getting LED stream class manipulation configuration, class #"..classNum..", interface #"..numOfLedInterface.." "),
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
                            elseif (params["invert"]) then
                                classParams["invertEnable"]=not params.flagNo
                            elseif (params["force"]) then
                                if (params.flagNo==true) then
                                    classParams["forceEnable"]=false
                                    classParams["forceData"]=0
                                else
                                    classParams["forceEnable"]=true
                                    classParams["forceData"]=params["force"]
                                end
                            elseif (params["enable_stretch"]) then
                                if (params.earch) then
                                    classParams["pulseStretchEnable"]=not params.flagNo
                                else
                                    print("Warning: enable-stretch command is not applicable for device family "..wrlCpssDeviceFamilyGet(devNum))
                                end
                            elseif (params["disable_on_link_down"]) then
                                if (params.earch) then
                                    classParams["disableOnLinkDown"]=not params.flagNo
                                else
                                    print("Warning: disable-on-link-down command is not applicable for device family "..wrlCpssDeviceFamilyGet(devNum))
                                end
                            end
                            rc= myGenWrapper("cpssDxChLedStreamPortGroupClassManipulationSet",{
                                { "IN","GT_U8","devNum",devNum },
                                { "IN","GT_PORT_GROUPS_BMP","portGroupsBmp",portGroupsBmp },
                                { "IN","GT_U32","ledInterfaceNum",ledInterfaceNum },
                                { "IN","CPSS_DXCH_LED_PORT_TYPE_ENT","portType", portType },
                                { "IN","GT_U32","classNum",classNum },
                                { "IN","CPSS_LED_CLASS_MANIPULATION_STC","classParamsPtr",classParams }
                            })
                            command_data:handleCpssErrorDevice(
                                rc,
                                string.format("setting LED stream class manipulation configuration "),
                                devNum)
                        end
                    end
                end
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
                {"IN", "GT_U8", "devNum", devNum},
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
        { type="values",
            { format = "%LED_if_range", name="ledif_range", help = help_led_interface }
        },
        { type="named",
            { format="device %devID_earch",  name="all_device",    help="The device number\n" } ,

            -- led interface stream-order-by
            { format="stream-order-by %ledOrder", name = "ledOrder", help = "LED stream ordering mode" },

            -- led interface disable-on-link-down
            { format="disable-on-link-down", name = "disableOnLinkDown", help = "Disable LED indication on link down" },

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

            -- led interface clock-invert
            { format="clock-invert ", name = "clockInvert", help = "Enables inversion of LEDCLK signal" },

            -- led interface LEDCLK frequency
            { format="clock-freq %clockOutFrequency", name="ledClockFrequency", help="LED output clock frequency" },

            -- led interface indication displayed on Class 5
            { format="dual-media-class-5 %ledClass5", name="dualMediaClass5", help="Selects the indication displayed on Class 5" },

            -- led interface indication displayed on Class 13
            { format="dual-media-class-13 %ledClass13", name="dualMediaClass13", help="Selects the indication displayed on Class 13" },
            { format="group %LED_group_num", name="group", help="LED group" },
            -- led interface group
            { format="class-a %LED_class_num", name="a", help="class A number" },
            { format="class-b %LED_class_num", name="b", help="class B number" },
            { format="class-c %LED_class_num", name="c", help="class C number" },
            { format="class-d %LED_class_num", name="d", help="class D number" },
            { format="enable-direct-mode", name = "directModeEn", help = "enable LED Direct Mode" },
            { format="port-type %port_type", name="portType", help="Port Type" },
            requirements={["dutyCycle"]={"blinkCfg"}, ["duration"]={"blinkCfg"},
                ["start"] = {"streamLength"}, ["end"] = {"streamLength"},
                ["ledClockFrequency"] = {"earch"},
                ["clockInvert"] = {"legacy"},
                ["disableOnLinkDown"] = {"legacy"},
                ["dualMediaClass5"] = {"legacy"},
                ["dualMediaClass13"] = {"legacy"},
                ["a"] = {"group"},
                ["b"] = {"group"},
                ["c"] = {"group"},
                ["d"] = {"group"},
                ["portType"] = { "isPortTypeRelevant","group" }
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
       { type="values",
           { format = "%LED_if_range", name="ledif_range", help = help_led_interface }
       },
       { type="named",
           { format="device %devID_earch",  name="all_device",    help="The device number\n" } ,
           { format="stream-order-by", name = "ledOrder", help = "Set LED stream ordering mode by class" },
           { format="disable-on-link-down", name = "disableOnLinkDown", help = "Disable LED indication on link down" },
           { format="stream-length", name = "streamLength", help = "Set LED interface default stream length " },
           { format="blink-cfg %blinkNumber", name = "blinkCfg", help = "Set a LED interface default blink configurations" },
           { format="stretch-cfg", name = "stretch", help = "Configure the indication stretching divider" },
           { format="clock-invert ", name = "clockInvert", help = "Enables inversion of LEDCLK signal" },
           { format="clock-freq", name="ledClockFrequency", help="LED output clock frequency" },
           { format="dual-media-class-5", name="dualMediaClass5", help="Selects the indication displayed on Class 5" },
           { format="dual-media-class-13", name="dualMediaClass13", help="Selects the indication displayed on Class 13" },
           { format="group %LED_group_num", name="group", help="LED group" },
           { format="enable-direct-mode", name = "directModeEn", help = "disable LED Direct Mode" },
           requirements={
               ["ledClockFrequency"] = {"earch"},
               ["clockInvert"] = {"legacy"},
               ["disableOnLinkDown"] = {"legacy"},
               ["dualMediaClass5"] = {"legacy"},
               ["dualMediaClass13"] = {"legacy"}},

           alt = { any_stream_param = { "ledOrder", "disableOnLinkDown", "streamLength", "blinkCfg", "stretch",
                     "clockInvert" , "ledClockFrequency", "dualMediaClass5", "dualMediaClass13", "group", "directModeEn"}},
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
            { format="device %devID_earch", name="devNum", help="device id" },
            { format="led-if %LED_if_range", name="ledif_range"},
            { format="use-blink %LED_blink_select",name="use_blink" },
            { format="invert", name="invert" },
            { format="force %LED_force_value", name="force" },
            { format="enable-stretch", name="enable_stretch" },
            { format="disable-on-link-down",name="disable_on_link_down" },
            { format="custom-indication %LED_indication_name", name="indication_name"},
            { format="port-type %port_type", name="portType", help="Port Type" },
            requirements = { ["portType"] = { "isPortTypeRelevant" } },
            alt = {cmd={"use_blink","invert","force","enable_stretch","disable_on_link_down","indication_name"}},
            mandatory = {"devNum","ledif_range", "cmd"}
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
            { format="device %devID_earch", name="devNum", help="device id" },
            { format="led-if %LED_if_range", name="ledif_range", help="LED interface range" },
            --{ format="%LED_class_command",name="command"},
            { format="use-blink",name="use_blink" },
            { format="invert", name="invert" },
            { format="force", name="force" },
            { format="enable-stretch", name="enable_stretch" },
            { format="disable-on-link-down",name="disable_on_link_down" },
            { format="custom-indication", name="indication_name"},
            { format="port-type %port_type", name="portType", help="Port Type" },
            requirements = { ["portType"] = { "isPortTypeRelevant" } },
            alt = { cmd={ "use_blink","invert","force","enable_stretch","disable_on_link_down","indication_name"} },
            mandatory = { "devNum","ledif_range", "cmd" }

        }
    }
})

local cmdParamsDevIdOnly = {{ type = "named",
                              {format = "device %devID_all", name = "devNum"},
                              mandatory = {"devNum"}}}

CLI_addHelp("config", "led", "configure LED specific settings (interfaces, classes, etc")
--******************************************************************************
local cpssApi = "cpssDxChLedStreamCpuOrPort27ModeSet"
CLI_addCommand({"config"}, "led enable-cpu-indication",
{
    func = function(params)
        return callApiWithOneParamForDevices(
            params.devNum, cpssApi,
            "CPSS_DXCH_LED_CPU_OR_PORT27_ENT", "indicatedPort", "CPSS_DXCH_LED_CPU_E")
    end,
    params = cmdParamsDevIdOnly,
    help = "enable CPU port indications instead of indications of 27 port.",
})

CLI_addCommand({"config"}, "no led enable-cpu-indication",
{
    func = function(params)
        return callApiWithOneParamForDevices(
            params.devNum, cpssApi,
            "CPSS_DXCH_LED_CPU_OR_PORT27_ENT", "indicatedPort", "CPSS_DXCH_LED_PORT27_E")
    end,
    params = cmdParamsDevIdOnly,
    help = "disable CPU port indications instead of indications of 27 port."
})


--******************************************************************************
local cpssApi = "cpssDxChLedStreamHyperGStackTxQStatusEnableSet"
CLI_addCommand({"config"}, "led enable-txq-status-indication",
{
    func = function(params)
        return callApiWithOneParamForDevices(
            params.devNum, cpssApi, "GT_BOOL", "enable", true)
    end,
    params = cmdParamsDevIdOnly,
    help = "enable indicating of Transmit Queues status for FlexLink ports working in XG mode.",
})

CLI_addCommand({"config"}, "no led enable-txq-status-indication",
{
    func = function(params)
        return callApiWithOneParamForDevices(
            params.devId, cpssApi, "GT_BOOL", "enable", false)
    end,
    params = cmdParamsDevIdOnly,
    help = "disable indicating of Transmit Queues status for FlexLink ports working in XG mode."
})

--******************************************************************************
local cpssApi = "cpssDxChLedStreamSyncLedsEnableSet"
CLI_addCommand({"config"}, "led synchronize-streams",
{
    func = function(params)
        return callApiWithOneParamForDevices(
            params.devNum, cpssApi, "GT_BOOL", "enable", true)
    end,
    params = cmdParamsDevIdOnly,
    help = "enable synchronizing of both LED streams.",
})

CLI_addCommand({"config"}, "no led synchronize-streams",
{
    func = function(params)
        return callApiWithOneParamForDevices(
            params.devNum, cpssApi, "GT_BOOL", "enable", false)
    end,
    params = cmdParamsDevIdOnly,
    help = "disable synchronizing of both LED streams."
})
