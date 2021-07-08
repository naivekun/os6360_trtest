--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* micro_init_tool.lua
--*
--* DESCRIPTION:
--*       Create init configurations file by the micro-init tool, reset the system
--*       and test fast boot
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
cmdLuaCLI_registerCfunction("luaCLI_getTraceDBLine")

SUPPORTED_FEATURE_DECLARE(devEnv.dev, "MICRO_INIT_TOOL")

local skip = false
--this test is temporary skipped
if skip == true then
    setTestStateSkipped();
    return;
end 

local error = false
local port1
local port2
local port3
local port4
local commonCpuPort = 63

-- special definition for customer
local devNum  = devEnv.dev
local devFamily, subFamily
local configFileName
local portsMembers = {}
local portsMembers100G = {}
local port_range_start, port_range_end
local familyName = nil
local dmaNum = 0x48
tid = nil
local result, values = cpssGenWrapper("osTaskGetSelf",{{"OUT","GT_U32","tid"}}, true)

if result == 0 then
    tid = values.tid
else
    print ("osTaskGetSelf failed, ret="..to_string(result))
    return false
end

local CETUS_DEV_ID = 0xBE00
local CAELUM_DEV_ID = 0xBC00

local VLAN_ID = 2

devFamily, subFamily = wrlCpssDeviceFamilyGet(devNum)
devType = wrlCpssDeviceTypeGet(devNum)

if (("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) and ("CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily)) then
    if (math.floor(devType / 0x10000) == CETUS_DEV_ID) then
        port1   = 56
        port2   = 64 -- dummy the port has link down
        port3   = 59
        port4   = 71 -- dummy the port has link down

        configFileName = "micro_init_tool_cetus"
        familyName = "bobk"

        portsMembers = {ports = {[0]=0x00000000, [1]=0x0F000000, [2]=0x000000FF}} -- { 56,57,58,59,64,65,66,67,68,69,70,71 }
    -- elseif (math.floor(devType / 0x10000) == CAELUM_DEV_ID) then
    --     port1   = 0
    --     port2   = 18 -- dummy the port has link down
    --     port3   = 36
    --     port4   = 24 -- dummy the port has link down

    --     configFileName = "micro_init_tool_caelum"
    --     familyName = "bobk"
    else
        setTestStateSkipped()
        return
    end
elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) then
    port1   = 0
    port2   = 4
    port3   = 8
    port4   = 12

    configFileName = "micro_init_tool_bobcat3"
    familyName = "bobcat3"
    portsMembers100G = {ports = {[0]=0x00111111}}
    portsMembers = {ports = {[0]=0xFF000000, [1]=0x07FFFFFF, [2]=0x00007FFF}}
    dmaNum = 74
elseif ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily) then
    port1   = 16
    port2   = 58
    port3   = 0
    port4   = 76

    configFileName = "micro_init_tool_aldrin2"
    familyName = "aldrin2"
    portsMembers100G = {ports = {[0]=0x00111111}}
    portsMembers = {ports = {[0]=0xFF000000, [1]=0x07FFFFFF, [2]=0x00003FFF}}
    dmaNum = 74
elseif ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) then
    port1   = 0
    port2   = 8
    port3   = 12
    port4   = 30

    configFileName = "micro_init_tool_aldrin"
    familyName = "aldrin"

    portsMembers = {ports = {[0]=0xFFFFFFFF}} -- { 0 - 31}
elseif ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily) then
    port1   = 0
    port2   = 8
    port3   = 18
    port4   = 23

    configFileName = "micro_init_tool_xcat3"
    familyName = "xcat3"

    portsMembers = {ports = {[0]=0x00FFFFFF}} -- { 0 - 31}
else
    setTestStateSkipped()
    return
end

-- special definition for customer

local pktInfo
local payload
local rc
local result,value
local cntrValue

local debug_on = false
local function _debug(debugString)
    if debug_on == true then
        print (debugString)
        local ret = cpssGenWrapper("simLogTaskAddLog",{{"IN","string","str","LUA: "..debugString.."\n"}}, true)
    end
end

-- enable trace
local function enable_microInit_tracing()

    local ret,devices
    
    ret = myGenWrapper("appDemoTraceHwAccessOutputModeSet",{{"IN",TYPE["ENUM"],"mode",2}})  --output to db
    if ret ~= 0 then
        print("appDemoTraceHwAccessOutputModeSet failed, ret="..to_string(ret))
    end

    devices=wrLogWrapper("wrlDevList")

    for j=1,#devices do
        ret = myGenWrapper("appDemoTraceHwAccessEnable",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"accessType",1},  -- 0 -read, 1--write 2-both 3-delay&write 4-all
            {"IN","GT_BOOL","enable",true}
        })
        if (ret~=0) then
            print("Could not perform trace action, status is:"..ret)
        end
    end
end

-- disable trace
local function disable_microInit_tracing()

    local ret,devices

    devices=wrLogWrapper("wrlDevList")
    
    for j=1,#devices do
        ret = myGenWrapper("appDemoTraceHwAccessEnable",{
            {"IN","GT_U8","devNum",devices[j]},
            {"IN",TYPE["ENUM"],"accessType",1},  -- 0 -read, 1--write 2-both 3-delay&write 4-all
            {"IN","GT_BOOL","enable",false}
        })
        if (ret~=0) then
            print("Could not perform trace action, status is:"..ret)
        end
    end
end

local function traceClear()
    
    local ret
    
    ret = myGenWrapper("appDemoTraceHwAccessClearDb",{})
    if (ret~=0) then
        print("Could clear HW access db, status is:"..ret)
        return false
    else
        return true
    end
end

-- saves configuration in micro-init file
local function saveConfig(filename)

    local save_tracing=[[
    end
    debug
    trace save file ]]..filename..[[ as micro-init saveToHd
    end
    examples
    ]]

    if (luaCLI_getTraceDBLine(0)==nil) then
        print("No trace is stored")
        return false
    end
    executeStringCliCommands(save_tracing)
    traceClear()
end

-- loads configuration from micro-init file
local function loadConfig(filename)

    local load_tracing=[[
    end
    debug-mode
    load eeprom ]]..filename..[[ 
    end
    examples
    ]]
    executeStringCliCommands(load_tracing)
end

local function callWithErrorHandling(cpssApi, params)
   _debug("Try to call "..cpssApi)
   local rc, value = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
       setFailState()
       _debug("ERROR of calling "..cpssApi..": "..returnCodes[rc])
   else
       _debug("OK of calling "..cpssApi)

   end
   return rc, value
end

local delayTimeInSec = 4.095

local function clearCounters()
    _debug("\n---------- CLEAR counters ------------")

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_RCV_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    -- read egress counters
    result, value = myGenWrapper(
        "appDemoDxChPortEgressCntrsGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",       devNum },
        { "IN",     "GT_U8",                        "cntrSetNum",   0 },
        { "OUT",    "CPSS_PORT_EGRESS_CNTR_STC",    "egrCntrPtr"}
    })

    --read the drop event counter
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    --read the drop event counter
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })

    -- read Ingress Drop Counter
    result, value = myGenWrapper(
        "appDemoDxChCfgIngressDropCntrGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",    "devNum",    devNum  },
        { "OUT",    "GT_U32",   "counterPtr"}
    })
end

local function readCounters()

    _debug("\n---------- read counters ------------")

    --Get counter after sleep
    _debug("delay for 1 second")
    delay(1000 * delayTimeInSec) -- one second

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_RCV_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value.cntrValuePtr))
    _debug("port "..port1.." CPSS_GOOD_OCTETS_RCV_E  "..cntrValue)
    if tonumber(cntrValue) == 0 then
        print("ERROR in value of counter CPSS_GOOD_OCTETS_RCV_E "..cntrValue.." on port "..port1)
        error = true
    end

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value.cntrValuePtr))
    _debug("port "..port1.." CPSS_GOOD_OCTETS_SENT_E  "..cntrValue)
    if tonumber(cntrValue) == 0 then
        print("ERROR in value of counter CPSS_GOOD_OCTETS_SENT_E "..cntrValue.." on port "..port1)
        error = true
    end

    --read the counters
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_GOOD_OCTETS_SENT_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value.cntrValuePtr))
    _debug("port "..port3.." CPSS_GOOD_OCTETS_SENT_E  "..cntrValue)
    if tonumber(cntrValue) == 0 then
        print("ERROR in value of counter CPSS_GOOD_OCTETS_SENT_E "..cntrValue.." on port "..port3)
        error = true
    end

    --------- read drop counters ------------

    _debug("\n---------- read DROP counters ------------")

    -- read egress counters
    result, value = myGenWrapper(
        "appDemoDxChPortEgressCntrsGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",       devNum },
        { "IN",     "GT_U8",                        "cntrSetNum",   0 },
        { "OUT",    "CPSS_PORT_EGRESS_CNTR_STC",    "egrCntrPtr"}
    })
    _debug("Egress drop counters: ")
    _debug("brgEgrFilterDisc "..to_string(value.egrCntrPtr.brgEgrFilterDisc))
    _debug("txqFilterDisc "..to_string(value.egrCntrPtr.txqFilterDisc))
    _debug("egrFrwDropFrames "..to_string(value.egrCntrPtr.egrFrwDropFrames))
    _debug("mcFifo3_0DropPkts "..to_string(value.egrCntrPtr.mcFifo3_0DropPkts))
    _debug("mcFifo7_4DropPkts "..to_string(value.egrCntrPtr.mcFifo7_4DropPkts))
    -- ***** Waiting for bug fix in simulation
    --if tonumber(value.egrCntrPtr.brgEgrFilterDisc) ~= 0 then
    --    print("ERROR in value of egress drop counter brgEgrFilterDisc "..to_string(value.egrCntrPtr.brgEgrFilterDisc))
    --    error = true
    --end
    if tonumber(value.egrCntrPtr.txqFilterDisc) ~= 0 then
        print("ERROR in value of egress drop counter txqFilterDisc "..to_string(value.egrCntrPtr.txqFilterDisc))
        error = true
    end
    if tonumber(value.egrCntrPtr.egrFrwDropFrames) ~= 0 then
        print("ERROR in value of egress drop counter egrFrwDropFrames "..to_string(value.egrCntrPtr.egrFrwDropFrames))
        error = true
    end
    if tonumber(value.egrCntrPtr.mcFifo3_0DropPkts) ~= 0 then
        print("ERROR in value of egress drop counter mcFifo3_0DropPkts "..to_string(value.egrCntrPtr.mcFifo3_0DropPkts))
        error = true
    end
    if tonumber(value.egrCntrPtr.mcFifo7_4DropPkts) ~= 0 then
        print("ERROR in value of egress drop counter mcFifo7_4DropPkts "..to_string(value.egrCntrPtr.mcFifo7_4DropPkts))
        error = true
    end

    --read the drop event counter
    _debug("\nDrop event counters:")
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port1 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value["cntrValuePtr"]))
    _debug("port "..port1.." CPSS_DROP_EVENTS_E  "..cntrValue)
    if tonumber(cntrValue) ~= 0 then
        print("ERROR in value of counter CPSS_DROP_EVENTS_E "..cntrValue.." on port "..port1)
        error = true
    end

    --read the drop event counter
    result, value = myGenWrapper(
        "appDemoDxChMacCounterGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",                        "devNum",    devNum  },
        { "IN",     "GT_PHYSICAL_PORT_NUM",         "portNum",   port3 },
        { "IN",     "CPSS_PORT_MAC_COUNTERS_ENT",   "cntrName",  "CPSS_DROP_EVENTS_E" },
        { "OUT",    "GT_U64",                       "cntrValuePtr"}
    })
    cntrValue = string.format("0x%X", wrlCpssGtU64StrGet(value["cntrValuePtr"]))
    _debug("port "..port3.." CPSS_DROP_EVENTS_E  "..cntrValue)
    if tonumber(cntrValue) ~= 0 then
        print("ERROR in value of counter CPSS_DROP_EVENTS_E "..cntrValue.." on port "..port3)
        error = true
    end

    -- read Ingress Drop Counter
    result, value = myGenWrapper(
        "appDemoDxChCfgIngressDropCntrGet_HardCodedRegAddr", {
        { "IN",     "GT_U8",    "devNum",    devNum  },
        { "OUT",    "GT_U32",   "counterPtr"}
    })
    _debug("\nIngress drop counter "..to_string(value.counterPtr))
    if tonumber(value.counterPtr) ~= 0 then
        print("ERROR in value of ingress drop counter "..to_string(value.counterPtr))
        error = true
    end

end

-- restore Eeprom configuration for Bobcat2
local function restoreEepromConfigBobcat2()

    print("\nRestore Eeprom configuration")

    local tcQueue={0,1,2,3,4,5,6,7}
    for i=1,#tcQueue do
        --Enable enqueuing to a Traffic Class queue
        callWithErrorHandling (
            "prvCpssDxChPortTxQueueingEnableSet", {
            { "IN", "GT_U8",                "devNum",      devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",     58  },
            { "IN", "GT_U8",                "tcQueue",     tcQueue[i]  },
            { "IN", "GT_BOOL",              "enable",      true  }
        })

    end

    local ports={58}
    for i=1,#ports do
        --Enable a specified port
        callWithErrorHandling (
            "prvCpssDxChPortEnableSet", {
            { "IN", "GT_U8",                    "devNum",   devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",  ports[i] },
            { "IN", "GT_BOOL",                   "enable",  true   }
        })

    end
    callWithErrorHandling(
        "appDemoDxChPortModeSpeedSet", {
        { "IN", "GT_U8",        "devNum", devNum  }
    })
end

-- run pre-test config
local function preTestConfig()

    local rc, value

    --reset system
    callWithErrorHandling(
        "cpssResetSystem",{
        {"IN", "GT_BOOL", "doHwReset",  true}
    })

    --create configuration files by the micro-init tool
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName))

    enable_microInit_tracing()
    --Partial ports configurations (instead of FW configurations, loaded by the micro-init application)
    if ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily) then
        rc, value = callWithErrorHandling(
            "appDemoDxChPortMicroInitConfigSet",{
            { "IN", "GT_U8",                        "devNum",   devNum },
            { "IN", "CPSS_PORTS_BMP_STC",           "portsBmp", portsMembers },
            { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode",   "CPSS_PORT_INTERFACE_MODE_QSGMII_E" },
            { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    "CPSS_PORT_SPEED_1000_E" }
        })
        portsMembers = {ports = {[0]=0x0F000000}}
        rc, value = callWithErrorHandling(
            "appDemoDxChPortMicroInitConfigSet",{
            { "IN", "GT_U8",                        "devNum",   devNum },
            { "IN", "CPSS_PORTS_BMP_STC",           "portsBmp", portsMembers },
            { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode",   "CPSS_PORT_INTERFACE_MODE_KR_E" },
            { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    "CPSS_PORT_SPEED_10000_E" }
        })
    elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
            ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily)
    then
        -- Configure 100G ports
        rc, value = callWithErrorHandling(
            "appDemoDxChPortMicroInitConfigSet",{
            { "IN", "GT_U8",                        "devNum",   devNum },
            { "IN", "CPSS_PORTS_BMP_STC",           "portsBmp", portsMembers100G },
            { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode",   "CPSS_PORT_INTERFACE_MODE_KR4_E" },
            { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    "CPSS_PORT_SPEED_100G_E" }
        })
        --Configure 10G ports
        rc, value = callWithErrorHandling(
            "appDemoDxChPortMicroInitConfigSet",{
            { "IN", "GT_U8",                        "devNum",   devNum },
            { "IN", "CPSS_PORTS_BMP_STC",           "portsBmp", portsMembers },
            { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode",   "CPSS_PORT_INTERFACE_MODE_KR_E" },
            { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    "CPSS_PORT_SPEED_10000_E" }
        })
    else
        rc, value = callWithErrorHandling(
            "appDemoDxChPortMicroInitConfigSet",{
            { "IN", "GT_U8",                        "devNum",   devNum },
            { "IN", "CPSS_PORTS_BMP_STC",           "portsBmp", portsMembers },
            { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode",   "CPSS_PORT_INTERFACE_MODE_KR_E" },
            { "IN", "CPSS_PORT_SPEED_ENT",          "speed",    "CPSS_PORT_SPEED_10000_E" }
        })
    end
    disable_microInit_tracing()
    -- save configuration to file
    if rc == 0 then
        saveConfig("test-config-ports-"..familyName.."-"..tid..".txt")
    else 
        print("appDemoDxChPortMicroInitConfigSet failed, ret="..to_string(rc))
        return false
    end

    --add support of reset system
    callWithErrorHandling(
        "cpssDevSupportSystemResetSet",{
        { "IN", "GT_U8",   "devNum",  devNum  },
        { "IN", "GT_BOOL", "enable",  true }
    })

    --reset system
    callWithErrorHandling(
        "cpssResetSystem",{
        {"IN", "GT_BOOL", "doHwReset",  true}
    })

    -- reset SLAN unbind flag because soft reset binds them back according to INI file
    unbindSlansAlreadyDone = false

    -- change mode to PEX only
    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","initRegDefaults"},
        {"IN","GT_U32","value",1}
    })
    
    if (("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily)) then
        callWithErrorHandling("cpssInitSystem",{
            {"IN","GT_U32","boardIdx",29},
            {"IN","GT_U32","boardRevId",1},
            {"IN","GT_U32","reloadEeprom",0}
        })
    elseif ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily) then
        callWithErrorHandling("cpssInitSystem",{
            {"IN","GT_U32","boardIdx",19},
            {"IN","GT_U32","boardRevId",2},
            {"IN","GT_U32","reloadEeprom",0}
        })
    end

    --Enable device
    callWithErrorHandling(
        "cpssDxChCfgDevEnable",{
        { "IN", "GT_U8",   "devNum",  devNum },
        { "IN", "GT_BOOL", "enable",  true }
    })

    _debug('Load config')

    --load configuration files
    if ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily)
    then
        loadConfig("config-"..familyName.."-"..tid.."-device0.txt")
		loadConfig("config-"..familyName.."-"..tid.."-device1.txt")
    else
		loadConfig("config-"..familyName.."-"..tid.."-device.txt")
	end
    if ("CPSS_PP_FAMILY_DXCH_XCAT3_E" ~= devFamily) then
        loadConfig("config-"..familyName.."-"..tid.."-map.txt")
    end
    loadConfig("config-"..familyName.."-"..tid.."-pizza.txt")
    loadConfig("config-"..familyName.."-"..tid.."-smi.txt")
    loadConfig("config-other-"..familyName.."-"..tid..".txt")
    -- configure Interface mode and speed on a specified port
    loadConfig("test-config-ports-"..familyName.."-"..tid..".txt")

    _debug('Config loaded')

    --generate test packets
    do
        if ((("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) and ("CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily)) or
            ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
            ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily) or
            ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
            ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily) or
            ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily))
        then
            result, payload = pcall(dofile, "dxCh/examples/packets/fastBoot_Cetus_trafficAfterEeprom.lua")
        end
    end

    if not result then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

-- run post-test config
local function postTestConfig()

    _debug("\npostTestConfig() ")
    if (("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily))
    then
        _debug("\rRestore port mapping ")
        --restore port mapping  
        callWithErrorHandling (
        "appDemoDxChPortMappingRestore", {
            { "IN", "GT_U8", "devNum", devNum}
        })
    end
    if ((("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) and ("CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily)) or
        ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily))
    then
        callWithErrorHandling(
        "appDemoDxChPortModeSpeedSet", {
            { "IN", "GT_U8",        "devNum", devNum  }
        })
    end

    --unset set force link up for port3
    callWithErrorHandling(
        -- opposite of "appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr"
        "cpssDxChPortForceLinkPassEnableSet", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port3 },
        { "IN", "GT_BOOL",              "state",    false }
    })

    --unset set force link up for port1
    callWithErrorHandling(
        -- opposite of "appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr"
        "cpssDxChPortForceLinkPassEnableSet", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port1 },
        { "IN", "GT_BOOL",              "state",    false }
    })

    callWithErrorHandling (
        -- opposite of "appDemoDxChPortLoopbackModeEnableSet_HardCodedRegAddr"
        "cpssDxChPortInternalLoopbackEnableSet", {
        { "IN", "GT_U8",                    "devNum",               devNum  },
        { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",              port1  },
        { "IN", "GT_BOOL",                  "enable",               false }
    })

    callWithErrorHandling (
        -- opposite of "appDemoDxChPortLoopbackModeEnableSet_HardCodedRegAddr"
        "cpssDxChPortInternalLoopbackEnableSet", {
        { "IN", "GT_U8",                    "devNum",               devNum  },
        { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",              port3  },
        { "IN", "GT_BOOL",                  "enable",               false }
    })

    --unset port1 as RX mirror
    callWithErrorHandling(
        "appDemoDxChMirrorRxPortSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_PORT_NUM",  "mirrPort",         port1   },
        { "IN", "GT_BOOL",      "enable",           false },
        { "IN", "GT_U32",       "index",            0 }
    })

    interface =  {interface={type    = "CPSS_INTERFACE_PORT_E",
                  devPort = { devNum  = 0x10,
                              portNum = port1}}}
    --unset port1 as analyzer port
    callWithErrorHandling(
        -- opposite of "appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr"
        "cpssDxChMirrorAnalyzerInterfaceSet", {
        { "IN", "GT_U8",                                    "devNum",       devNum },
        { "IN", "GT_U32",                                   "index",        0 },
        { "IN", "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC",  "interfacePtr", interface }
    })
    callWithErrorHandling(
        -- opposite of "appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr"
        "cpssDxChMirrorAnalyzerInterfaceSet", {
        { "IN", "GT_U8",                                    "devNum",       devNum },
        { "IN", "GT_U32",                                   "index",        1 },
        { "IN", "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC",  "interfacePtr", interface }
    })

    -- change mode from fastBoot test
    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","fastBootSkipOwnDeviceInit"},
        {"IN","GT_U32","value",0}
    })

    -- change mode from fastBoot test
    callWithErrorHandling(
        "cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet", {
        { "IN", "GT_BOOL",  "enable",       false }
    })

    if ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily) then
        ports={0,2,4,6,8,9}
        for i=1,#ports do
            callWithErrorHandling (
                    "cpssDxChBrgFdbNaStormPreventSet", {
                    { "IN", "GT_U8",                    "devNum",      devNum  },
                    { "IN", "GT_PORT_NUM",              "portNum",     ports[i] },
                    { "IN", "GT_BOOL",                  "enable",      false }
            })
        end
    end

    -- restore configurtion from Eeprom
    if (("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) and ("CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" == subFamily)) then
        restoreEepromConfigBobcat2()
        fillDeviceEnvironment()
    end
    if (("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily))
    then
        fillDeviceEnvironment()
    end

    --restore default vlan and pvid
        callWithErrorHandling (
        "appDemoDxChVlanPvidRestore", {
            { "IN", "GT_U8", "devNum", devNum}
        })

    --remove VLAN 2
    executeLocalConfig(luaTgfBuildConfigFileName('micro_init_tool_deconfig'))
end


local function mainFunction()
    _debug("\nmainFunction() ")

    printLog("\n Start ... \n")
    --appDemoDxChRestorePortsMapShadowArray()
    ------------------------------ SYSTEM CONFIGURATION PART ---------------------
    --[[ System configuration
      1.    ports port1, port3 belong to vlan X.
      2.    port1 is in 'loopback' mode.
      3.    sending packet with 'dsa' tag 'from_cpu' to cpu port 63.
      4.    the packet egress from port1 without 'dsa' tag on vlan X.
      5.    the loopback ingress the packet again to port1.
      6.    the packet egress from port3 and the replication to port1 is filtered.
      for bobcat2: X=4094
    ]]--

    if (("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily))
    then
        --bind the DQ port to the DMA number
        callWithErrorHandling(
            "appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "txqNum",           0x3F },
            { "IN", "GT_U32",       "txDmaNum",         0x48 } --for BOBCAT2 SDMA CPU port number is 0x48
        })


        callWithErrorHandling(
            "appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "txqNum",           port3 },
            { "IN", "GT_U32",       "txDmaNum",         port3 }
        })

        callWithErrorHandling(
            "appDemoDxChPortMappingTxQPort2TxDMAMapSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "txqNum",           port1 },
            { "IN", "GT_U32",       "txDmaNum",         port1 }
        })

        --configure BMA local physical port 2 RxDMA port  mapping
        callWithErrorHandling(
            "appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "physPort",         0x3F },
            { "IN", "GT_U32",       "rxDMAPort",        0x48 } --for BOBCAT2 SDMA CPU port number is 0x48
        })

        if (("CPSS_PP_FAMILY_DXCH_ALDRIN_E" ~= devFamily) and
            ("CPSS_PP_FAMILY_DXCH_AC3X_E" ~= devFamily))
        then
            callWithErrorHandling(
            "appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr", {
                { "IN", "GT_U8",        "devNum",           devNum  },
                { "IN", "GT_U32",       "physPort",         port1 },
                { "IN", "GT_U32",       "rxDMAPort",        port1 }
            })

            callWithErrorHandling(
            "appDemoDxChPortMappingBMAMapOfLocalPhys2RxDMASet_HardCodedRegAddr", {
                { "IN", "GT_U8",        "devNum",           devNum  },
                { "IN", "GT_U32",       "physPort",         port3 },
                { "IN", "GT_U32",       "rxDMAPort",        port3 }
            })
        end

        --configure RxDMA 2 phys port mapping
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         0x48 }, --for BOBCAT2 SDMA CPU port number is 0x48
            { "IN", "GT_U32",       "physPort",         0x3F },

        })

        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port1 },
            { "IN", "GT_U32",       "physPort",         port1 },

        })

        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port3 },
            { "IN", "GT_U32",       "physPort",         port3 },

        })

        --Bobcat2 Port mapping initial configuration
        callWithErrorHandling(
            "appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "physPort",         port1 }

        })

        callWithErrorHandling(
            "appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "physPort",         port3 }

        })

        callWithErrorHandling(
            "appDemoDxChBobcat2PortMappingConfigSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "physPort",         63 }

        })

        local interface  ={type    = "CPSS_INTERFACE_PORT_E",
                           devPort = { devNum  = 0x10,
                                       portNum = port1}}

        --Set physical info for ePort 4 in the E2PHY mapping table
        callWithErrorHandling(
            "appDemoDxChBrgEportToPhysicalPortTargetMappingTableSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                    "devNum",           devNum  },
            { "IN", "GT_PORT_NUM",              "portNum",          port1 },
            { "IN", "CPSS_INTERFACE_INFO_STC",  "physicalInfoPtr",  interface }
        })
    elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
            ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily)
    then
        callWithErrorHandling(
            "prvCpssDxChTablesAccessInit", {
            { "IN", "GT_U8",        "devNum",           devNum  }
        })
        -- Configure configure RxDMA 2 phys port mapping for CPU ports
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         74 },
            { "IN", "GT_U32",       "physPort",         63 },
        })

        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         75 },
            { "IN", "GT_U32",       "physPort",         80 },
        })

        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         76 },
            { "IN", "GT_U32",       "physPort",         81 },
        })

        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         77 },
            { "IN", "GT_U32",       "physPort",         82 },
        })
    -- Configure configure RxDMA 2 phys port mapping for port1
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port1 },
            { "IN", "GT_U32",       "physPort",         port1 },
        })
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port1 + 1 },
            { "IN", "GT_U32",       "physPort",         port1 + 1 },
        })
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port1 + 2 },
            { "IN", "GT_U32",       "physPort",         port1 + 2 },
        })
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port1 + 3 },
            { "IN", "GT_U32",       "physPort",         port1 + 3 },
        })
    -- Configure configure RxDMA 2 phys port mapping for port3
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port3 },
            { "IN", "GT_U32",       "physPort",         port3 },
        })
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port3 + 1 },
            { "IN", "GT_U32",       "physPort",         port3 + 1 },
        })
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port3 + 2 },
            { "IN", "GT_U32",       "physPort",         port3 + 2 },
        })
        callWithErrorHandling(
            "appDemoDxChPortMappingRxDMA2LocalPhysSet_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "rxDmaNum",         port3 + 3 },
            { "IN", "GT_U32",       "physPort",         port3 + 3 },
        })
        -- Configure BMA local physical port 2 RxDMA port mapping for CPU ports
        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         63 },
            { "IN", "GT_U32",       "rxDMAPort",        74 }
        })

        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         80 },
            { "IN", "GT_U32",       "rxDMAPort",        75 }
        })

        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         81 },
            { "IN", "GT_U32",       "rxDMAPort",        76 }
        })

        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         82 },
            { "IN", "GT_U32",       "rxDMAPort",        77 }
        })

        -- Configure BMA local physical port 2 RxDMA port mapping for port1
        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         port1 },
            { "IN", "GT_U32",       "rxDMAPort",        port1 }
        })
        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         port1 + 1 },
            { "IN", "GT_U32",       "rxDMAPort",        port1 + 1 }
        })
        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         port1 + 2 },
            { "IN", "GT_U32",       "rxDMAPort",        port1 + 2 }
        })
        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         port1 + 3 },
            { "IN", "GT_U32",       "rxDMAPort",        port1 + 3 }
        })
        -- Configure BMA local physical port 2 RxDMA port mapping for port3
        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         port3 },
            { "IN", "GT_U32",       "rxDMAPort",        port3 }
        })
        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         port3 + 1 },
            { "IN", "GT_U32",       "rxDMAPort",        port3 + 1 }
        })
        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         port3 + 2 },
            { "IN", "GT_U32",       "rxDMAPort",        port3 + 2 }
        })
        callWithErrorHandling(
            "prvCpssDxChPortMappingBMAMapOfLocalPhys2RxDMASet", {
            { "IN", "GT_U8",        "devNum",           devNum  },
            { "IN", "GT_U32",       "portGroupId",      0 },
            { "IN", "GT_U32",       "physPort",         port3 + 3 },
            { "IN", "GT_U32",       "rxDMAPort",        port3 + 3 }
        })

        callWithErrorHandling(
            "appDemoDxChBrgEportToPhysicalPortTargetMappingTableSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                    "devNum",           devNum  },
            { "IN", "GT_PORT_NUM",              "portNum",          port1 },
            { "IN", "CPSS_INTERFACE_INFO_STC",  "physicalInfoPtr",  interface }
        })
    end

    if (("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily))
    then
        --set cpu port as ingress cascade
        callWithErrorHandling (
            "appDemoDxChIngressCscdPortSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                   "devNum",       devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",      commonCpuPort   },
            { "IN", "GT_U32",                  "portRxDmaNum", dmaNum   },
            { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType",     "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E" }
        })
    elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
            ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily)
    then
        --set cpu ports as ingress cascade
        callWithErrorHandling (
            "appDemoDxChIngressCscdPortSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                   "devNum",       devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",      63   },
            { "IN", "GT_U32",                  "portRxDmaNum", 74   },
            { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType",     "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E" }
        })
        callWithErrorHandling (
            "appDemoDxChIngressCscdPortSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                   "devNum",       devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",      80   },
            { "IN", "GT_U32",                  "portRxDmaNum", 75   },
            { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType",     "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E" }
        })
        callWithErrorHandling (
            "appDemoDxChIngressCscdPortSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                   "devNum",       devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",      81   },
            { "IN", "GT_U32",                  "portRxDmaNum", 76   },
            { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType",     "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E" }
        })
        callWithErrorHandling (
            "appDemoDxChIngressCscdPortSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                   "devNum",       devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",      82   },
            { "IN", "GT_U32",                  "portRxDmaNum", 77   },
            { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType",     "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E" }
        })
    else
        --set cpu port as ingress cascade
        callWithErrorHandling (
            "appDemoDxChIngressCscdPortSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                   "devNum",       devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",    "portNum",      commonCpuPort   },
            { "IN", "GT_U32",                  "portRxDmaNum", commonCpuPort   },
            { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType",     "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E" }
        })
    end

    --[[
    if (("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily))
    then
        --init cpu port resources
        callWithErrorHandling(
            "appDemoDxChCpuPortPizzaResources_HardCodedRegAddr", {
            { "IN", "GT_U8",        "devNum",           devNum  }
        })
    end
    --]]

    local interface  ={type    = "CPSS_INTERFACE_PORT_E",
                       devPort = { devNum  = devNum,
                                   portNum = port1}}
    --startHwTrace()
    -- set port1 as MAC loopback

    if ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily) then
        -- loopback enable for GE ports
        callWithErrorHandling (
            "appDemoDxChPortLoopbackModeEnableSet_HardCodedRegAddr", {
            { "IN", "GT_U8",                    "devNum",               devNum  },
            { "IN", "CPSS_INTERFACE_INFO_STC",  "portInterfacePtr",     interface  },
            { "IN", "GT_BOOL",                  "enable",               true }
        })
    elseif ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
            ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily)
    then
        callWithErrorHandling (
            "portBobcat3HardCodedInternalLoopbackXGEnableSet", {
            { "IN", "GT_U8",                    "devNum",      devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",     port1  },
            { "IN", "GT_BOOL",                  "enable",      true }
        })
        callWithErrorHandling (
            "portBobcat3HardCodedInternalLoopbackXGEnableSet", {
            { "IN", "GT_U8",                    "devNum",      devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",     port3  },
            { "IN", "GT_BOOL",                  "enable",      true }
        })
    else
        -- loopback enable for XG ports
        callWithErrorHandling (
            "portBobcat2HardCodedInternalLoopbackXGEnableSet", {
            { "IN", "GT_U8",                    "devNum",      devNum  },
            { "IN", "GT_PHYSICAL_PORT_NUM",     "portNum",     port1  },
            { "IN", "GT_BOOL",                  "enable",      true }
        })
    end
    
    --stopHwTrace()

    --set port1 as RX mirror
    callWithErrorHandling(
        "appDemoDxChMirrorRxPortSet_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  },
        { "IN", "GT_PORT_NUM",  "mirrPort",         port1   },
        { "IN", "GT_BOOL",      "enable",           true },
        { "IN", "GT_U32",       "index",            0 }
    })

    --set own device
    callWithErrorHandling(
        "setOwnDevice_HardCodedRegAddr", {
        { "IN", "GT_U8",        "devNum",           devNum  }
    })

    --set force link up for port1
    callWithErrorHandling(
        "appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port1 },
        { "IN", "GT_BOOL",              "state",    true }
    })

    local interface =  {interface={type    = "CPSS_INTERFACE_PORT_E",
                       devPort = { devNum  = 0x10,
                                   portNum = port1}}}
    --set port1 as analyzer port
    callWithErrorHandling(
        "appDemoDxChMirrorAnalyzerInterfaceSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                                    "devNum",       devNum },
        { "IN", "GT_U32",                                   "index",        0 },
        { "IN", "CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC",  "interfacePtr", interface }
    })

    --set analyzer interface index
    callWithErrorHandling(
        "appDemoDxChMirrorRxGlobalAnalyzerInterfaceIndexSet_HardCodedRegAddr", {
        { "IN", "GT_U8",    "devNum",       devNum },
        { "IN", "GT_BOOL",  "enable",       true },
        { "IN", "GT_U32",   "index",        0 }
    })

    --set set force link up for port3
    callWithErrorHandling(
        "appDemoDxChPortForceLinkPassEnableSet_HardCodedRegAddr", {
        { "IN", "GT_U8",                "devNum",   devNum },
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum",  port3 },
        { "IN", "GT_BOOL",              "state",    true }
    })

    if ("CPSS_PP_FAMILY_DXCH_XCAT3_E" == devFamily) then
        ports={0,2,4,6,8,9}
        for i=1,#ports do
            -- prevent NA messages storm
            callWithErrorHandling (
                    "cpssDxChBrgFdbNaStormPreventSet", {
                    { "IN", "GT_U8",                    "devNum",      devNum  },
                    { "IN", "GT_PORT_NUM",              "portNum",     ports[i] },
                    { "IN", "GT_BOOL",                  "enable",      true }

            })
        end
    end

    ------------------------------ MAIN PART ---------------------------------------

    clearCounters()

    --send continuous traffic
    printLog("\nSend continuous traffic ")
    callWithErrorHandling(
        "appDemoDxChSdmaTxPacketSend_HardCodedRegAddr", {
        { "IN",     "GT_U8",       "devNum",                devNum  },
        { "IN",     "GT_BOOL",     "isPacketWithVlanTag",   false},
        { "IN",     "GT_U8",       "vid",                   VLAN_ID  },
        { "IN",     "TGF_PACKET_PAYLOAD_STC", "packet",     payload["broadcast_1_to_all_vlan_X"] },
        { "IN",     "GT_U8",       "dstPortNum",            port1   }
    })

    readCounters()

    -- change mode from PEX only
    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","initRegDefaults"},
        {"IN","GT_U32","value",0}
    })

    -- change mode from PEX only
    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","initRegDefaults"},
        {"IN","GT_U32","value",0}
    })

    -- change mode to fastBoot test
    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","fastBootSkipOwnDeviceInit"},
        {"IN","GT_U32","value",1}
    })

    if (("CPSS_PP_FAMILY_DXCH_BOBCAT2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily) or
        ("CPSS_PP_FALILY_DXCH_ALDRIN2_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_ALDRIN_E" == devFamily) or
        ("CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily))
    then
        -- special setting in appDemo that analyzer port will not be '0' but port1
        callWithErrorHandling(
            "appDemoDbEntryAdd",{
            {"IN","string","namePtr","fastBootSetMirrorAnalyzerPort"},
            {"IN","GT_U32","value", port1}
        })
    end

    --no DB reset, only remove device
    callWithErrorHandling(
        "cpssResetSystemNoSoftReset", {})

    local systemRecoveryInfo = {}
    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
    }
    --Set system recovery state init
    printLog("\nSet system recovery state INIT ")
    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    -- run full init system
    printLog("\nRun full init system ")
    callWithErrorHandling("cpssReInitSystem",{})

    -- Don't check counters for SIP_5.20 because traffic is stopped in prvBridgeLibInit
    if ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" ~= devFamily) and
        ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" ~= devFamily)
    then
        readCounters()
    end

    local systemRecoveryInfo = {}
    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
    }

    -- Don't check counters for SIP_5.20 because traffic is stopped in prvBridgeLibInit
    if ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" ~= devFamily) and
        ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" ~= devFamily)
    then
        readCounters()
    end

    --Set system recovery state catchup
    printLog("\nSet system recovery state CATCH_UP ")
    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    -- Don't check counters for SIP_5.20 because traffic is stopped in prvBridgeLibInit
    if ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" ~= devFamily) and
        ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" ~= devFamily)
    then
        readCounters()
    end

    local systemRecoveryInfo = {}
    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E"
    }
    --Set system recovery state completion
    printLog("\nSet system recovery state COMPLETION ")
    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    --Open interrupts
    callWithErrorHandling (
               "appDemoDxChEventRestore", {})
    delay(1000);

    --[[
        NOTE: next operation with appDemoAllowProcessingOfAuqMessages() needed because:
        1. the full wire speed packet generated 'AUQ FULL' with (the same) mac addresses.
        2. the appDemo that wanted to handle those messages was ignored by CPSS in function
        prvDxChBrgFdbAuFuMsgBlockGet() in code of:
            if ((systemRecoveryInfo.systemRecoveryState != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E) &&
                 (systemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_FALSE))
            {
                *numOfAuFuPtr = 0;/* no messages to retrieve */
                return /* it's not error for log */ GT_NO_MORE;
            }

        this means that while 'systemRecoveryState != 'COMPLETION_STATE' the appDemo could not handle the messages.

        but once the 'AUQ FULL' reached ... no more NA messages will come , and not more interrupts on that register (interrupt cause,mask register)

        so now that the systemRecoveryState == 'COMPLETION_STATE' ... we need to handle all those waiting messages!
    ]]--
    printLog("\n Allow AppDemo to handle the AUQ messages that were ignored during 'systemRecoveryState != 'COMPLETION_STATE' ")
    -- we disable and then enable to allow the appDemo to force polling into the AUQ
    callWithErrorHandling("appDemoAllowProcessingOfAuqMessages",{{"IN",  "GT_BOOL", "enable",  false}})
    callWithErrorHandling("appDemoAllowProcessingOfAuqMessages",{{"IN",  "GT_BOOL", "enable",   true}})

    -- Don't check counters for SIP_5.20 because traffic is stopped in prvBridgeLibInit
    if ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" ~= devFamily) and
        ("CPSS_PP_FAMILY_DXCH_ALDRIN2_E" ~= devFamily)
    then
        readCounters()
    end

    -- called from initSysem function that handles cpssInitSystem command ("exec" context)
    -- but should also be automatically called at reloading LUA CLI
    fillDeviceEnvironment()
end


-- run pre-test config
preTestConfig()
-- run the test
mainFunction()
-- run post-test config
postTestConfig()

if error == true then setFailState() end
