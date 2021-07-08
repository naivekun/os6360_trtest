--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* eee_example.lua
--*
--* DESCRIPTION:
--*       test configuration and traffic related to EEE.
--*       Port's IEEE 802.3az Energy Efficient Ethernet (EEE) Support.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
cmdLuaCLI_registerCfunction("prvLuaTgfPortLoopbackModeEnableSet")

local devNum  = devEnv.dev
-- this test is relevant for SIP_5 devices (BOBCAT 2 and above)
--- AC3X need proper ports for this test
-- the call to SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM may change the value of devEnv.port[x] !!!
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum, "EEE" , "ports" , nil--[[all ports]]) 



local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
--NOTE: port3 is EEE disabled
--port4 is XG port .. don't use it
-- so work with ports 2
local testedPort = port2


--##################################
--##################################
local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"
local macDa   = "000000000058"
local macSa   = "000000001111"
local dummyPacket = macDa..macSa..packetPayload

local subEvent_eee_pcs_rx = {name = "eee_pcs_rx" , value = 0}
local subEvent_eee_pcs_tx = {name = "eee_pcs_tx" , value = 1}
local subEvent_eee_mac_rx = {name = "eee_mac_rx" , value = 2}
local CPSS_PP_PORT_EEE_E = 356 -- from H file - cpss\common\h\cpss\generic\events\cpssGenEventUnifyTypes.h
local CPSS_EVENT_MASK_E  = 0
local CPSS_EVENT_UNMASK_E= 1

-- enable/disable appDemo printings of EEE events
local function appDemoEeeInterruptsPrint(enable)
    local value
    if enable == true then
        value = 1
    else
        value = 0
    end

    local appDemoEeeInterruptsEnableString = [[
        end
        shell-execute appDemoPrintPortEeeInterruptInfoSet ]] .. value .. [[

    ]]

    executeStringCliCommands(appDemoEeeInterruptsEnableString)

end

-- set event mask/unmask (disable/enable)
-- when portNum and subEvent are nil ... imply to all ports and all sub fields
-- when subEvent is nil .. imply to all sub fields of the port
local function eeeEventCommand(devNum,portNum,subEvent,enable)
    local mask
    local extData

    if enable then
        mask = CPSS_EVENT_UNMASK_E
    else
        mask = CPSS_EVENT_MASK_E
    end

    if(portNum == nil and subEvent == nil) then
        --[[
        GT_STATUS cpssEventDeviceMaskSet
        (
            IN GT_U8                    devNum,
            IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
            IN CPSS_EVENT_MASK_SET_ENT  operation
        );
        ]]--
        -- call CPSS with direct call as 'shell command' because no 'cpss-api' support for this
        local eeeEventSetString = [[
            end

            shell-execute cpssEventDeviceMaskSet ]]..devNum..","..CPSS_PP_PORT_EEE_E..","..mask..[[

        ]]

        executeStringCliCommands(eeeEventSetString)

        return-- no more to do
    end

    if(subEvent == nil) then
        -- explicit call to the 3 sub events of the EEE
        -- pcs Rx
        eeeEventCommand(devNum,portNum,subEvent_eee_pcs_rx,enable)
        -- pcs Tx
        eeeEventCommand(devNum,portNum,subEvent_eee_pcs_tx,enable)
        -- Mac Rx
        eeeEventCommand(devNum,portNum,subEvent_eee_mac_rx,enable)

        return-- no more to do
    end

--[[
GT_STATUS cpssEventDeviceMaskWithEvExtDataSet
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_U32                   evExtData,
    IN CPSS_EVENT_MASK_SET_ENT  operation
);
]]--
    extData = (portNum * 256) + subEvent.value


    -- call CPSS with direct call as 'shell command' because no 'cpss-api' support for this
    local eeeEventSetString = [[
        end

        shell-execute cpssEventDeviceMaskWithEvExtDataSet ]]..devNum..","..CPSS_PP_PORT_EEE_E..","..extData..","..mask..[[

    ]]

    executeStringCliCommands(eeeEventSetString)
end

local function eeeEventDisableAllPorts()
        eeeEventCommand(devNum,nil,nil,false)-- disable
end
local function eeeEventEnableAllPorts()
        eeeEventCommand(devNum,nil,nil,true)-- enable
end

local CPSS_PP_EVENTS = 0
-- clear the event table
local function eeeEventsClear(devNum)
    -- clear the table
    wrlCpssGenEventTableClear(devNum, CPSS_PP_EVENTS)
end
-- get the number of events that we got for the ports/sub-events that are enabled for the EEE event
local function eeeEventsGetOnPort(devNum,portNum)
    local countersTable = wrlCpssGenEventTableGet(devNum, CPSS_PP_EVENTS)
    for index,currCounter in pairs(countersTable) do
        local currEventType = cpssGenWrapperCheckParam("CPSS_UNI_EV_CAUSE_ENT", tonumber(index))

        if currEventType == "CPSS_PP_PORT_EEE_E" then
            return currCounter
        end
    end

    -- we got no EEE events
    return 0
end


-- send packet and check that the LPI stopped to send the packet , and then
-- was restored to LPI again after the 'timer' expired
local function sendTestPacket()
    local transmitInfo = {portNum = testedPort , pktInfo = {fullPacket = dummyPacket} }
    local egressInfoTable = {
        {portNum = port1  },--check flooding
        {portNum = port3  },--check flooding
        {portNum = port4  } --check flooding
    }

    -- important : state to transmit function not to modify the MAC loopback port mode.
    transmitInfo.doNotModifyMacLoopback = true
    -- send packet without checking it's egress
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    local sectionName = "transmit packet to modify LPI state"
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED"
        printLog ("ENDED : " .. pass_string .. "\n")
        testAddPassString(pass_string)
    else
        local error_string = "Section " .. sectionName .. " FAILED"
        printLog ("ENDED : " .. error_string .. "\n")
        testAddPassString(error_string)
    end
end

-- set/unset port as MAC loopback
local function setLoopbackPort(devNum,portNum,enable)
    local rc = prvLuaTgfPortLoopbackModeEnableSet(devNum,portNum,enable)

    if rc ~= 0 then
        testAddErrorString("Unable to set {dev/port} [" ..devNum .."/"..portNum.."] with MAC loopback set to: " .. to_string(enable))
    end

end

local ports_link_up = {}
local command_data = {}
local function eeeInitPortsLinkUp()
    command_data = Command_Data()

    local sysInfo_link_up = luaCLI_getDevInfo(devNum,true)--link up only
    if type(sysInfo_link_up) ~= "table" or sysInfo_link_up[devNum] == nil then
        -- case of invoking before cpssInitSystem
        testAddPassString("case of invoking before cpssInitSystem")
        return
    end
    ports_link_up = sysInfo_link_up[devNum]--link up only
end

-- return indication is the port enabled for EEE ('link up' and 'eee enabled' and 'GE port')
-- return isError , isLinkUp , isGePort , eeeMode
local function eeeIsPortEnabled(portNum)
    local myError = false
    local function eeeMode(portNum)
        --portInfo --> {mode , tw_limit , li_limit  , ts_limit , phyMacSecMode }
        local isError , portInfo = eee_get_info_port_config(command_data, devNum, portNum)
        if isError then
            myError = true
            testAddPassString("ERROR calling eee_get_info_port_config for port " .. portNum)
            return
        end

        return portInfo.mode
    end


    local function isLinkUp(portNum)
        for i = 1,#(ports_link_up) do
            local portNum_link_up = ports_link_up[i]
            if portNum_link_up == portNum then
                return true
            end
        end
        return false
    end

    -- check if port is 'GE' port and not XG
    local function isGePort(portNum)
        local apiName = "cpssDxChPortSpeedGet"
        local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN",     "GT_U8",      "devNum",     devNum },
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
            { "OUT",    "CPSS_PORT_SPEED_ENT",    "speedPtr"}
        })

        if isError then
            myError = true
            return false
        end

        local port_speed_str,port_speed_number = speedStrGet(values["speedPtr"])
        printLog("portNum = " .. portNum .. "," .. values["speedPtr"])

        if port_speed_number and
           port_speed_number <= 1000 and
           port_speed_number >= 10 then
            return true
        end
        -- speed more then 1000 (1G)
        return false
    end

    local link,ge,mode = isLinkUp(portNum) , isGePort(portNum) , eeeMode(portNum)

    return myError , link , ge , mode
end

local portInfo_validRxTxPort = {macTxPathLpi=true    ,macTxPathLpWait = false,macRxPathLpi = true    ,pcsTxPathLpi = true    ,pcsRxPathLpi = true }
local portInfo_validTxPort   = {macTxPathLpi=true    ,macTxPathLpWait = false,macRxPathLpi = false   ,pcsTxPathLpi = true    ,pcsRxPathLpi = false }
local portInfo_NotEeeValid   = {macTxPathLpi=false   ,macTxPathLpWait = false,macRxPathLpi = false   ,pcsTxPathLpi = false   ,pcsRxPathLpi = false }
local checkedPorts_status =
    {
         {portNum = testedPort} --rx and tx
        ,{portNum = port1     } -- tx only
        ,{portNum = port3     } -- none (eee disabled)
        ,{portNum = port4     } -- none  (XG)
    }
-- entry is from checkedPorts_status[]
local function wrapper__eee_get_info_port_status(command_data, entry)
    local portNum = entry.portNum
    local expectedValue
    local description = "port " .. portNum .." EEE LPI status."
    if entry.note then
        description = description .. entry.note
    end

    local isError , linkup , gePort , eeeMode = eeeIsPortEnabled(portNum)
    if isError then
        return
    end

    if linkup and gePort and eeeMode and eeeMode ~= eee_disable then
        if portNum ~= testedPort then
            expectedValue = portInfo_validTxPort
        else
            expectedValue = portInfo_validRxTxPort
        end
    else
        expectedValue = portInfo_NotEeeValid
    end

    --      portInfo --> {macTxPathLpi,macTxPathLpWait,macRxPathLpi,pcsTxPathLpi,pcsRxPathLpi}
    local isError , portInfo = eee_get_info_port_status(command_data, devNum, portNum)

    return isError , portInfo , expectedValue , description
end

local function expected_checkValuesAfterConfigFile()
    local checkedPorts =
        {
             {portNum = testedPort  , portInfo = {mode="auto" , tw_limit=0x020 , li_limit=0x016  , ts_limit=0x024 , phyMacSecMode="not valid" }}
            ,{portNum = port1       , portInfo = {mode="auto" , tw_limit=0x020 , li_limit=0x016  , ts_limit=0x024 , phyMacSecMode="not valid" }}
            ,{portNum = port3       , portInfo = {mode="disable",tw_limit=0x011, li_limit=0x006  , ts_limit=0x013 , phyMacSecMode="not valid" }}
            ,{portNum = port4       , portInfo = {mode="auto"   ,tw_limit=0x011, li_limit=0x006  , ts_limit=0x013 , phyMacSecMode="not valid" }}
        }
    -- entry is from checkedPorts[]
    local function wrapper__eee_get_info_port_config(command_data, entry)
        local portNum = entry.portNum
        local expectedValue = entry.portInfo
        local description = "port " .. portNum .." EEE LPI config (after config)."
        if entry.note then
            description = description .. entry.note
        end
        --      portInfo --> {mode , tw_limit , li_limit  , ts_limit , phyMacSecMode }
        local isError , portInfo = eee_get_info_port_config(command_data, devNum, portNum)

        return isError , portInfo , expectedValue , description
    end
    --check array checkedPorts
    testCheckExpectedValues(command_data,wrapper__eee_get_info_port_config,checkedPorts)

    --check array checkedPorts_status
    testCheckExpectedValues(command_data,wrapper__eee_get_info_port_status,checkedPorts_status)

end

local function expected_checkValuesAfterDeconfigFile()
    local portInfo_deconfig = {mode="disable" , tw_limit=0x010 , li_limit=0x004  , ts_limit=0x010 , phyMacSecMode="not valid" }
    local checkedPorts =
        {
             {portNum = testedPort  , portInfo = portInfo_deconfig}
            ,{portNum = port1       , portInfo = portInfo_deconfig}
            ,{portNum = port3       , portInfo = portInfo_deconfig}
            ,{portNum = port4       , portInfo = portInfo_deconfig}
        }
    -- entry is from checkedPorts[]
    local function wrapper__eee_get_info_port_config(command_data, entry)
        local portNum = entry.portNum
        local expectedValue = entry.portInfo
        local description = "port " .. portNum .." EEE LPI config (after de-config)."
        if entry.note then
            description = description .. entry.note
        end
        --      portInfo --> {mode , tw_limit , li_limit  , ts_limit , phyMacSecMode }
        local isError , portInfo = eee_get_info_port_config(command_data, devNum, portNum)

        return isError , portInfo , expectedValue , description
    end
    --check array checkedPorts
    testCheckExpectedValues(command_data,wrapper__eee_get_info_port_config,checkedPorts)

    --check array checkedPorts_status
    testCheckExpectedValues(command_data,wrapper__eee_get_info_port_status,checkedPorts_status)
end
--##################################
--##################################
local function generalInit()
    -- enable appDemo printings of EEE events
    appDemoEeeInterruptsPrint(true)

    -- before configurations set the port into MAC loopback , so it may go into LPI mode.
    setLoopbackPort(devNum,testedPort,true)

    executeLocalConfig(luaTgfBuildConfigFileName("eee_example"))

    delay(500) -- stable the port

    -- called after the 'config file' so we can check it's configurations
    expected_checkValuesAfterConfigFile()

    -- disable the events of EEE on all ports
    eeeEventDisableAllPorts()
    -- clear the event table
    eeeEventsClear(devNum)
end


local function generalEnd()
    setLoopbackPort(devNum,testedPort,false)

    -- enable the events of EEE on all ports
    eeeEventEnableAllPorts()

    --unset config
    executeLocalConfig(luaTgfBuildConfigFileName("eee_example",true))

    -- called after the 'deconfig file' so we can check it restored configurations
    expected_checkValuesAfterDeconfigFile()

    -- disable appDemo printings of EEE events
    appDemoEeeInterruptsPrint(false)


    -- print results summary
    testPrintResultSummary("EEE")
end

-- test list of ports with sending traffic for verification
local function testPorts(listOfPorts,steps)
    local portNum
    local subEvent
    local eventCounter
    local counter
    local note,extraNote
    local tableEvents = {}
    local portIsEnabled
    local mapInfo

    if nil == steps then
        steps = 1
    end

    for i = 1,#(listOfPorts),steps do
        portNum = listOfPorts[i]

        -- check if port enabled for eee
        local isError , linkup , gePort , eeeMode = eeeIsPortEnabled(portNum)
        if isError then
            return
        end

        if false == linkup then
            portIsEnabled = false
            extraNote = "Link Down (no EEE interrupts expected)"
        elseif false == gePort then
            extraNote = "Not GE port (no EEE interrupts expected)"
            portIsEnabled = false
        elseif eeeMode == eee_disable then
            extraNote = "EEE Disabled (no EEE interrupts expected)"
            portIsEnabled = false
        else
            extraNote = "EEE mode :[" .. eeeMode .. "](EEE interrupts expected)"
            print("++++++++++++++++++++++++++++++++EEE mode :[" .. eeeMode .. "](EEE interrupts expected) | portNum = " .. portNum)
            portIsEnabled = true
        end


        if portNum == testedPort and portIsEnabled == true then
            tableEvents = {
                 {subEvent = nil                 ,eventCounter=3   } -- all 3 events
                ,{subEvent =subEvent_eee_pcs_rx  ,eventCounter=1   } -- single event
                ,{subEvent =subEvent_eee_pcs_tx  ,eventCounter=1   } -- single event
                ,{subEvent =subEvent_eee_mac_rx  ,eventCounter=1   } -- single event
            }

            extraNote = extraNote .. "(Rx Port)"
        elseif portIsEnabled == true then
            tableEvents = {
                 {subEvent = nil                 ,eventCounter=1   } -- as this is egress port , only 'pcs-tx' raise event
                ,{subEvent =subEvent_eee_pcs_rx  ,eventCounter=0   } -- as this is egress port , only 'pcs-tx' raise event
                ,{subEvent =subEvent_eee_pcs_tx  ,eventCounter=1   } -- as this is egress port , only 'pcs-tx' raise event
                ,{subEvent =subEvent_eee_mac_rx  ,eventCounter=0   } -- as this is egress port , only 'pcs-tx' raise event
            }
            extraNote = extraNote .. "(Tx Port - EEE enabled)"
        else
            tableEvents = {
                 {subEvent = nil                 ,eventCounter=0   } -- port not enabled for EEE --> no events
            }
        end

        mapInfo = getPortMapInfo(devNum,portNum) 

        if (is_sip_5_20(devNum) and (portNum >= 37)) or (mapInfo.mappingType ~= "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E") then
            -- ports does not have GIG MACs and interrupt managment will fail test.
            -- skip such ports
            -- skip remote ports also
        else
            for index,entry in pairs(tableEvents) do
                subEvent = entry.subEvent
                eventCounter = entry.eventCounter

                if subEvent then
                    note = "test EEE subEvent = " .. subEvent.name
                else -- subEvent == nil
                    note = "test EEE all subEvents"
                end

                note = note  .. " on portNum = " .. portNum .. " expected numOfEvent = " .. eventCounter .. " " .. extraNote

                -- enable the sub event
                eeeEventCommand(devNum,portNum,subEvent,true)
                -- clear the table after the mask is enabled
                -- so a 'hidden' interrupt will be generated and cleared.
                -- so do delay to process the event
                delay(100)
                -- clear the events
                eeeEventsClear(devNum)
                -- send a packet that should cause LPI change of mode .. cause interrupt.
                sendTestPacket()

                counter = eeeEventsGetOnPort(devNum,portNum)

                testCheckExpectedValue(note , eventCounter , counter)

                -- disable the sub event
                eeeEventCommand(devNum,portNum,subEvent,false)
            end -- loop on tableEvents[]
        end -- port is valid for test
    end -- loop on ports[]
end

local function testSelectedPorts()
    local ports = {devEnv.port[1],devEnv.port[2],devEnv.port[3],devEnv.port[4],devEnv.port[5],devEnv.port[6]}
    -- test ports[1..6]
    testPorts(ports)
end

local function testDevicePorts()
    local sysInfo = luaCLI_getDevInfo(devNum)
    if type(sysInfo) ~= "table" or sysInfo[devNum] == nil then
        -- case of invoking before cpssInitSystem
        testAddPassString("case of invoking before cpssInitSystem")
        return
    end

    local ports = sysInfo[devNum]

    local steps = 7 -- test port out of 7 ports --> to reduce iterations

    testPorts(ports,steps)
end

--set config
printLog("--------start generic EEE configuration : ")
eeeInitPortsLinkUp()
generalInit()
-- test ports[1..6]
testSelectedPorts()
-- test other ports from the device
testDevicePorts()

printLog("--------restore EEE configuration : ")
generalEnd()
printLog("-------- EEE test ended --- \n")
