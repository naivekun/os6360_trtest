--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pip_example.lua
--*
--* DESCRIPTION:
--*       test configuration and traffic related to PIP.
--*       Pre-Ingress Prioritization (PIP)
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev

-- this test is relevant for SIP_5_10 devices (BOBCAT 2 B0 and above)
--- AC3X need proper ports for this test : all ports will be taken from Aldrin
-- the call to SUPPORTED_FEATURE_DECLARE may change the value of devEnv.port[1..6] !!!
SUPPORTED_FEATURE_DECLARE(devNum,"RXDMA_PIP")


local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4] 

local ingressPort = port2
local egressPort  = port3
local cpuPort = 63


local isSimulation = wrlCpssIsAsicSimulation()

local function getSimOrBeforeSIP520(devNum)
  
  if isSimulation then
    return true 
  end
  
  if ( is_sip_5_20(devNum) 
             --  or is_sip_5_25(devNum)
  ) then
    return false
  else
    return true
  end
end

local isSimOrBeforeSIP520 = getSimOrBeforeSIP520(devEnv.dev)



local priorityNamesArr
local prioritySet
if is_sip_5_20(devEnv.dev) then
    priorityNamesArr = { [0] = "Very high" , [1] = "High" , [2] = "Medium" , [3] = "Low" }
    prioritySet = {"medium", "low", "high", "very-high"}
    lowPriorityNum = 3
else
    priorityNamesArr = { [0] = "High" , [1] = "Medium" , [2] = "Low" }
    prioritySet = {"medium", "low", "high"}
    lowPriorityNum = 2
end

--##################################
--##################################

local delayVal = 250 -- 0.25 sec by default
local disableCheckDropCounters = false
local dropOnPortTotalCounter = 0
local dropByPriorityTotalCounter = 0

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"
-- unicast macDa
local macDaUc = "000000000058" --> 00:00:00:00:00:58 --> as set in the configuration file !
local macSa   = "000000001111"

local function buildPacketUc(partAfterMac)
    if not partAfterMac then
        partAfterMac = ""
    end
    
    return macDaUc .. macSa ..  partAfterMac .. packetPayload
end
-- build mac da - mc (multi-cast)
local function buildPacketMc(macDaMc)
    return macDaMc .. macSa ..  packetPayload
end

-- build packet with 4 bytes of vlan tag according to TPID , vid , up
local function buildPacketVlanTag(tpid,vid,up)
    local value = bit_shl(tpid,16) + bit_shl(up,13) + vid 
    local vlanTagString = string.format("%8.8x",value) 
    return buildPacketUc(vlanTagString)
end

-- build packet with 6 bytes of : 2 bytes mpls ethertype + 4 bytes of MPLS label according to exp
local function buildPacketMpls(mpls_exp)
    local ethertype = "8847"-- mpls unicast
    local label = 0x12345
    local ttl   = 0xaa
    local stopBit = 1
    local value = bit_shl(0x12345,12) + bit_shl(mpls_exp,9) + bit_shl(stopBit,8) + ttl 
    local mplsString = string.format("%8.8x",value) 
    return buildPacketUc(ethertype .. mplsString)
end

-- build packet with 22 bytes of : 2 bytes ipv4 ethertype +  20 bytes of IPv4
local function buildPacketIpv4(tos)
    local ethertype = "0800"
    local version_and_ihl = 0x45
    local totalLength = 60
    local value = bit_shl(version_and_ihl,24) + bit_shl(tos,16) + totalLength
    local word1 = string.format("%8.8x",value) 
    local word2 = string.format("%8.8x",0) 

    local ttl   = 0xbb
    local protocol = 0x55
    local checksum = 0 -- unknown 
    local value = bit_shl(ttl,24) + bit_shl(protocol,16) + checksum
    local word3 = string.format("%8.8x",value) 

    local word4 = "01020304"--sip
    local word5 = "01020305"--dip

    return buildPacketUc(ethertype .. word1 .. word2 .. word3 .. word4 .. word5)
end

-- build packet with 44 bytes : 2 bytes ipv6 ethertype + 40 bytes of ip bytes of IPv6
local function buildPacketIpv6(tc)
    local ethertype = "86dd"
    local version = 0x6
    local flowLabel = 0x12345
    local value = bit_shl(version,28) + bit_shl(tc,20) + flowLabel
    local word1 = string.format("%8.8x",value) 

    local hopLimit = 0xdd
    local nextHeader = 0x55
    local payloadLength = 40 
    local value = bit_shl(payloadLength,16) + bit_shl(nextHeader,8) + hopLimit
    local word2 = string.format("%8.8x",value) 

    local sip = "11111111" .. "11223344" .. "22334455" .. "11111111"
    local dip = "22222222" .. "aabbccdd" .. "bbccddee" .. "22222222"

    return buildPacketUc(ethertype .. word1 .. word2 .. sip .. dip)
end

--build packet with 2 bytes of UDE
local function buildPacketUde(etherType)
    local bytes_0_1 = string.format("%4.4x",etherType) 
    return buildPacketUc(bytes_0_1)
end

--build packet with MC mac da
local function buildPacketMacDaMc(macDa)
    return buildPacketMc(macDa)
end

-- set/unset port as cascade
local function generic_cscd_port_set(devNum, portNum, unset , portType)
    local command_data = Command_Data()

    if unset == true then
        portType = "CPSS_CSCD_PORT_NETWORK_E"
    elseif portType then
        -- already have portType value
    elseif is_sip_5(devNum) then
        portType = "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E"
    else
        portType = "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"
    end

    apiName = "cpssDxChCscdPortTypeSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
        { "IN", "CPSS_PORT_DIRECTION_ENT", "portDirection", "CPSS_PORT_DIRECTION_BOTH_E"},
        { "IN", "CPSS_CSCD_PORT_TYPE_ENT", "portType", portType}
    })
end 

local function updateGlobalCountersByPriority(devNum)
    -- read actual values of the priority drop counters 
    local isErr
    local counterPrioVeryhigh, counterPrioHigh, counterPrioMedium, counterPrioLow
    local vhDropCounter, hDropCounter, mDropCounter, lDropCounter


    local dropPacketsDetected = false
    local saveCounter = dropByPriorityTotalCounter


    if is_sip_5_20(devEnv.dev) then
      isErr, counterPrioVeryhigh, counterPrioHigh, counterPrioMedium, counterPrioLow = pip_get_info_drop_global_counter(command_data, devNum)
      vhDropCounter = tonumber(string.format("0x%08X%08X", counterPrioVeryhigh.l[1], counterPrioVeryhigh.l[0]))
    else
      isErr, counterPrioHigh, counterPrioMedium, counterPrioLow = pip_get_info_drop_global_counter(command_data, devNum)
      vhDropCounter = 0
    end
    hDropCounter = tonumber(string.format("0x%08X%08X", counterPrioHigh.l[1], counterPrioHigh.l[0]))
    mDropCounter = tonumber(string.format("0x%08X%08X", counterPrioMedium.l[1], counterPrioMedium.l[0]))
    lDropCounter = tonumber(string.format("0x%08X%08X", counterPrioLow.l[1], counterPrioLow.l[0]))

    -- calculate total number of packets dropped by priority
    dropByPriorityTotalCounter = dropByPriorityTotalCounter + vhDropCounter + hDropCounter + mDropCounter + lDropCounter

    if isErr == true then 
      local error_string = "Error reading global drop counters by priority"
      printLog ("ERROR : " .. error_string)
      testAddErrorString(error_string)
    end

    -- counters changed
    if saveCounter ~= dropByPriorityTotalCounter then
      dropPacketsDetected = true
    end
    
    return dropPacketsDetected
end

--##################################
--##################################

-- function to run section
local function testSection(sectionInfo)
    local sectionName       = sectionInfo.sectionName    
    local purpose           = sectionInfo.purpose        
    local transmitInfo      = sectionInfo.transmitInfo   
    local egressInfoTable   = sectionInfo.egressInfoTable
    local configFunc        = sectionInfo.configFunc     
    local deConficFunc      = sectionInfo.deConficFunc
    local thresholdChangeFunc = sectionInfo.thresholdChangeFunc
    
    local dropped_on_ingress = {
        -- expect ALL ports NOT to get it
        {portNum = port1  , packetCount = 0},
        {portNum = port2  , packetCount = 0},
        {portNum = port3  , packetCount = 0},
        {portNum = port4  , packetCount = 0}
    }
    local ingressPort = transmitInfo.portNum
    
    local index
    -- remove ingress port from dropped_on_ingress
    for ii,entry in pairs(dropped_on_ingress) do
        if entry.portNum == ingressPort then
            -- move last entry to override this entry
            index = ii
            break
        end
    end
    
    -- remove last index
    if index then
        dropped_on_ingress[index] = nil
    end
    
    printLog (delimiter)

    local egress_expected_result_string = "--> 'threshold' allow 'packet to pass'"

    printLog("START : Section " .. sectionName .. egress_expected_result_string)
    printLog("Purpose of section : " .. purpose)
    
    if configFunc then
        configFunc()
    end
    
    -- check that packet egress the needed port(s) , when the 'threshold' allow it
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    
    printLog (delimiter)
    if rc == 0 then
        local pass_string = "Section " .. sectionName .. " PASSED , when " .. egress_expected_result_string
        printLog ("ENDED : " .. pass_string .. "\n")
        testAddPassString(pass_string)
    else
        local error_string = "Section " .. sectionName .. " FAILED , when " .. egress_expected_result_string
        printLog ("ENDED : " .. error_string .. "\n")
        testAddErrorString(error_string)
    end 

    printLog (delimiter)

    if (thresholdChangeFunc and (disableCheckDropCounters ~= true)) then
        -- call to change the needed threshold
        thresholdChangeFunc()
    
        egress_expected_result_string = "--> 'threshold' NOT allow 'packet to pass' --> expected 'DROP'"
        printLog("START : Section " .. sectionName .. egress_expected_result_string)

      if (isSimOrBeforeSIP520 == false) then        
        transmitInfo.burstCount = transmit_continuous_wire_speed
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)

        -- the task still running
        
        delay(delayVal) -- delay

        transmitInfo.burstCount = stop_transmit_continuous_wire_speed
        rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)
        transmitInfo.burstCount = nil


        local command_data = Command_Data()

        local isError , ingDropCounterLo, ingDropCounterHi = pip_get_info_drop_port_counter(command_data, devNum, ingressPort)
        if isError then
            local error_string = "ERROR : FAILED to get drop counters on " .. tostring(ingressPort) .. "port"
            -- use command_data:addErrorAndPrint to be able to see also errors that came from 
            -- command_data that got it inside pip_get_info_drop_port_counter
            command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
           
            --printLog ("ERROR : " .. error_string)
            testAddErrorString(error_string)
        end
        local ingressDropCounter = tonumber(string.format("0x%08X%08X", ingDropCounterHi, ingDropCounterLo))

        dropOnPortTotalCounter = dropOnPortTotalCounter + ingressDropCounter 

        local isError , cpuDropCounterLo, cpuDropCounterHi = pip_get_info_drop_port_counter(command_data, devNum, cpuPort)
        if isError then
            local error_string = "ERROR : FAILED to get drop counters on " .. tostring(cpuPort) .. "port"
            -- use command_data:addErrorAndPrint to be able to see also errors that came from 
            -- command_data that got it inside pip_get_info_drop_port_counter
            command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
           
            --printLog ("ERROR : " .. error_string)
            testAddErrorString(error_string)
        end
        local cpuDropCounter = tonumber(string.format("0x%08X%08X", cpuDropCounterHi, cpuDropCounterLo))

        dropOnPortTotalCounter = dropOnPortTotalCounter + cpuDropCounter 

        local globalCntByPrioChangesDetected = updateGlobalCountersByPriority(devNum) 

        
        if ((cpuDropCounter == 0) and (ingressDropCounter == 0)) then
          if globalCntByPrioChangesDetected then
            printLog(" Global Priority Drop  Counters  detected! ")
          else
            printLog(" Drop  Counters  failed! ")
            rc = 1
          end
        end

      else

        -- check that packet egress no port , when the 'threshold' NOT allow it
        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,dropped_on_ingress)
      end
        printLog (delimiter)
        if rc == 0 then
            local pass_string = "Section " .. sectionName .. " PASSED , when " .. egress_expected_result_string
            printLog ("ENDED : " .. pass_string .. "\n")
            testAddPassString(pass_string)
        else
            local error_string = "Section " .. sectionName .. " FAILED , when " .. egress_expected_result_string
            printLog ("ENDED : " .. error_string .. "\n")
            testAddErrorString(error_string)
        end 
        printLog (delimiter)
    end

    
    if deConficFunc then
        deConficFunc()
    end
end
--##################################
--##################################
local command_restoreThresholds
if is_sip_5_20(devEnv.dev) then
    command_restoreThresholds =
    [[
end
configure
pip threshold 65535 device ${dev} priority very-high
pip threshold 65535 device ${dev} priority high
pip threshold 65535 device ${dev} priority medium
pip threshold 65535 device ${dev} priority low
    ]]
else
    command_restoreThresholds =
    [[
end
configure
pip threshold 1023 device ${dev} priority high
pip threshold 1023 device ${dev} priority medium
pip threshold 1023 device ${dev} priority low
    ]]
end

local command_threshold_veryhigh_reset =
[[
end
configure
pip threshold 0 device ${dev} priority very-high
]]

local command_threshold_high_reset =
[[
end
configure
pip threshold 0 device ${dev} priority high
]]
local command_threshold_medium_reset = 
[[
end
configure
pip threshold 0 device ${dev} priority medium
]]
local command_threshold_low_reset = 
[[
end
configure
pip threshold 0 device ${dev} priority low
]]
-- set the egress port to keep it's TPID
local command_set_egress_port_keep_tpid = 
[[
end
debug-mode
cpss-api call cpssDxChBrgVlanEgressTagTpidSelectModeSet devNum ${dev} portNum ]] .. egressPort .. [[ ethMode CPSS_VLAN_ETHERTYPE0_E mode CPSS_DXCH_BRG_VLAN_VID_TO_TPID_SELECT_TABLE_MODE_E
exit
]]
-- set the egress port to get TPID according to egress port index in TPID table(default setting)
local command_set_egress_port_not_keep_tpid = 
[[
end
debug-mode
cpss-api call cpssDxChBrgVlanEgressTagTpidSelectModeSet devNum ${dev} portNum ]] .. egressPort .. [[ ethMode CPSS_VLAN_ETHERTYPE0_E mode CPSS_DXCH_BRG_VLAN_EGRESS_TAG_TPID_SELECT_MODE_E
exit
]]
-- set pvid 15 on ingress port
local command_set_ingress_port_pvid_15 = 
[[
end
configure
interface ethernet ${dev}/]] .. ingressPort ..
[[

switchport pvid 15
exit
]]
-- restor pvid 1 on ingress port
local command_set_ingress_port_pvid_1 = 
[[
end
configure
interface ethernet ${dev}/]] .. ingressPort ..
[[

switchport pvid 1
exit
]]

-- Set priority for FROM_CPU traffic
local command_from_cpu_veryhigh_set =
[[
end
configure
pip profile 0 device ${dev} type dsa-up field 0 priority very-high
]]

local command_from_cpu_high_set =
[[
end
configure
pip profile 0 device ${dev} type dsa-up field 0 priority high
]]

local command_from_cpu_low_set =
[[
end
configure
pip profile 0 device ${dev} type dsa-up field 0 priority low
]]

-- the CPU port should restore pip trust
local command_cpuPortPipTrust = [[
end
configure
interface ethernet ${dev}/]] .. cpuPort .. [[

    pip trust
exit
]]

local command_allPortPipTrust = [[
end
configure
interface range ethernet all ${dev}
pip trust
exit
]]




-- set 1023 threshold for veryhigh priority
local command_set_thr_veryhigh = [[
end
configure
pip threshold 1023 device ${dev} priority very-high
expect pip threshold 1023 device ${dev} priority very-high
exit
]]

-- set dafault threshold for veryhigh priority
local command_set_dflt_thr_veryhigh = [[
end
configure
pip threshold 65535 device ${dev} priority very-high
exit
]]


--##################################
--##################################
local ingressPacket_1 = buildPacketVlanTag(0x9100 --[[tpid]] , 5 --[[vid]] , 7 --[[up]])
local transmitInfo_1 = {portNum = ingressPort , pktInfo = {fullPacket = ingressPacket_1} }
local egressInfoTable_1 = {
    -- expected egress port .. unmodified
    {portNum = egressPort  , pktInfo = {fullPacket = ingressPacket_1}},
    -- expect other ports not to get it
    {portNum = port1  , packetCount = 0},
    --{portNum = port2  , packetCount = 0}, --> the ingress port
    --{portNum = port3  , packetCount = 0}, --> the egress port
    {portNum = port4  , packetCount = 0}
}

--[[
1.	On the 'ingress port'
a.	enable the "tpid+vid" classification
b.	set priority for "tpid+vid" classification --> medium

2. set the egress port to egress packets with the same TPID as ingress.
]]--
local function  configFunc_1()
    local command_configString = 
        [[
        end
        configure
        interface ethernet ${dev}/]] .. ingressPort ..
        [[
        
        pip vid-ethertype-classification
        pip priority-vid-ethertype-classification medium
        exit
        ]]

    executeStringCliCommands(command_configString)
end

local function deConficFunc_1()
    local command_configString = 
        [[
        end
        configure
        interface ethernet ${dev}/]] .. ingressPort ..
        [[
        
        no pip vid-ethertype-classification
        exit
        ]]

    executeStringCliCommands(command_configString)
    executeStringCliCommands(command_restoreThresholds)
end
-- function to change the proper threshold
local function thresholdChangeFunc_1()
    executeStringCliCommands(command_threshold_medium_reset)
end
local sectionInfo_1_sectionName = "TPID+vid classifications - part-1"
local sectionInfo_1 = {
    sectionName = sectionInfo_1_sectionName,
    purpose = "Test need to check that packet classified as 'vlan tag' for specific 'vid' " .. 
              "and get pip-priority according to 'per-port' < priority for 'tpid+vid' classification>",
    transmitInfo = transmitInfo_1,
    egressInfoTable = egressInfoTable_1,
    configFunc = configFunc_1,
    deConficFunc = deConficFunc_1,
    thresholdChangeFunc = thresholdChangeFunc_1
}
--TPID+vid classifications
local function test_TPID_vid_classifications()
    printLog (delimiter)
    --vid 5
    sectionInfo_1.sectionName = sectionInfo_1_sectionName .. "with tpid 0x9100 , vid = 5 , up = 7"

    -- Set FROM CPU Traffic to be high(very high) priority to avoid drop on CPU Port
    if is_sip_5_20(devEnv.dev) then
        executeStringCliCommands(command_from_cpu_veryhigh_set)
    else
        executeStringCliCommands(command_from_cpu_high_set)
    end
    -->> test now
    testSection(sectionInfo_1)
    
    printLog (delimiter)
    --vid 4094
    local ingressPacket_2 = buildPacketVlanTag(0x9100 --[[tpid]] , 4094 --[[vid]] , 7 --[[up]])
    sectionInfo_1.transmitInfo.pktInfo.fullPacket = ingressPacket_2
    sectionInfo_1.egressInfoTable[1].pktInfo.fullPacket = ingressPacket_2

    sectionInfo_1.sectionName = sectionInfo_1_sectionName .. "with tpid 0x9100 , vid = 4094 , up = 7"
    -->> test now
    testSection(sectionInfo_1)
    
    -- restore FROM CPU Traffic to be low priority
    executeStringCliCommands(command_from_cpu_low_set)
    
    printLog (delimiter)
    -- keep the priority of port
    -- not enable 'tpid+vid' classification
    sectionInfo_1.sectionName = "TPID+vid classifications - part-2" .. " not enable 'tpid+vid' classification "
    sectionInfo_1.configFunc   = nil
    -- sectionInfo_1.deConficFunc = nil
    -- function to change the proper threshold
    if is_sip_5_20(devEnv.dev) then
        sectionInfo_1.thresholdChangeFunc =
            function()
                executeStringCliCommands(command_threshold_veryhigh_reset)
            end
    else
        sectionInfo_1.thresholdChangeFunc =
            function()
                executeStringCliCommands(command_threshold_high_reset)
            end
    end

    -->> test now
    testSection(sectionInfo_1)
end

--##################################
--##################################
local vlan_tag = "vlan_tag"
local mpls = "mpls"
local ipv4 = "ipv4"
local ipv6 = "ipv6"
local ude = "ude"
local mac_da = "mac_da"
local dsa_tag_qos = "dsa_tag_qos"
local dsa_tag_up = "dsa_tag_up"

local function test_classifications( packetsToSend,sectionName)
    local pip_profile
    local transmitInfo = {portNum = ingressPort , pktInfo = {fullPacket = nil --[[filled in runtime]]} }
    local egressInfoTable = {
        -- expected egress port .. unmodified
        {portNum = egressPort  , pktInfo = {fullPacket = nil --[[filled in runtime]]}},
        -- expect other ports not to get it
        {portNum = port1  , packetCount = 0},
        --{portNum = port2  , packetCount = 0}, --> the ingress port
        --{portNum = port3  , packetCount = 0}, --> the egress port
        {portNum = port4  , packetCount = 0}
    }
    local egressInfoTable_multicast = {
        -- expected egress port .. unmodified
        {portNum = egressPort  , pktInfo = {fullPacket = nil --[[filled in runtime]]}},
        {portNum = port1  , pktInfo = {fullPacket = nil --[[filled in runtime]]}},
        --{portNum = port2  }, --> the ingress port
        --{portNum = port3  }, --> the egress port
        {portNum = port4  , pktInfo = {fullPacket = nil --[[filled in runtime]]}}
    }
    -- function to change the proper threshold
    local function thresholdChangeFunc()
        if is_sip_5_20(devEnv.dev) then
            if pip_profile == 2 then
                executeStringCliCommands(command_threshold_veryhigh_reset)
            else -- pip_profile = 3
                executeStringCliCommands(command_threshold_high_reset)
            end
        else
            if pip_profile == 2 then
                executeStringCliCommands(command_threshold_high_reset)
            else -- pip_profile = 3
                executeStringCliCommands(command_threshold_medium_reset)
            end

        end
    end
    -- function to set configuration for the test
    local function configFunc(new_pip_profile)
        if new_pip_profile == nil then 
            new_pip_profile = pip_profile
        end
        -- set pip-profile on the ingress port
        local command_portPipProfile = [[
        end
        configure
        interface ethernet ${dev}/]] .. ingressPort .. [[
        
            pip profile ]] .. new_pip_profile .. [[
            
        exit
        ]]
        executeStringCliCommands(command_portPipProfile)
    end
    -- function to de-configuration for the test
    local function deConficFunc()
        -- restore pip profile
        --[[the orig pip profile from the configuration file]]
        local new_pip_profile = 2
        configFunc(new_pip_profile)
        executeStringCliCommands(command_restoreThresholds)
    end

    local sectionInfo = {
        sectionName = sectionName,
        purpose = "Test need to check that packet classified as expected " .. 
                  "and get pip-priority according to 'per-profile' per 'field' (for this packet type)",
        transmitInfo = transmitInfo,
        egressInfoTable = egressInfoTable,
        configFunc = configFunc,
        deConficFunc = deConficFunc,
        thresholdChangeFunc = thresholdChangeFunc
    }
    
    for index,entry in pairs(packetsToSend) do
        printLog (delimiter)
        
        if entry.type == mac_da then
            sectionInfo.egressInfoTable = egressInfoTable_multicast
        else
            sectionInfo.egressInfoTable = egressInfoTable
        end
        
        -- the ingress packet
        sectionInfo.transmitInfo.pktInfo.fullPacket = entry.packet
        if entry.cascade then
            generic_cscd_port_set(devNum, ingressPort,false,entry.cascade)
            sectionInfo.egressInfoTable[1].pktInfo = nil
            sectionInfo.egressInfoTable[1].packetCount = 1 -- we not want to check the content of packet that removed the DSA tag
        else
            -- expect egress packet .. unmodified (did 'simple' bridging)
            sectionInfo.egressInfoTable[1].pktInfo = {fullPacket = sectionInfo.transmitInfo.pktInfo.fullPacket}
            if entry.type == mac_da then
                sectionInfo.egressInfoTable[2].pktInfo = {fullPacket = sectionInfo.transmitInfo.pktInfo.fullPacket}
                sectionInfo.egressInfoTable[3].pktInfo = {fullPacket = sectionInfo.transmitInfo.pktInfo.fullPacket}
            end
        end

        if index == 1 then
            --startSimulationLog()
        end

        for ii = 2,3 do
            pip_profile = ii

            if is_sip_5_20(devEnv.dev) then
                if pip_profile == 2 then
                    pip_priority = "very-high"
                else
                    pip_priority = "high"

                    -- Set FROM CPU Traffic to be high priority to avoid drop on CPU Port
                    executeStringCliCommands(command_from_cpu_veryhigh_set)
                end
            else
                if pip_profile == 2 then
                    pip_priority = "high"
                else
                    pip_priority = "medium"

                    -- Set FROM CPU Traffic to be high priority to avoid drop on CPU Port
                    executeStringCliCommands(command_from_cpu_high_set)
                end

            end

            sectionInfo.sectionName = sectionName .. "--> currently :" .. entry.type .. 
                                    " with 'field index' = " .. entry.value ..
                                    " on profile = " .. pip_profile .. " expected pip_priority = " .. pip_priority
            -->> test now
            testSection(sectionInfo)

            -- restore FROM CPU Traffic to be low priority
            executeStringCliCommands(command_from_cpu_low_set)
            
            -- debug specific part 
            --break
        end
        
        if entry.cascade then
            generic_cscd_port_set(devNum, ingressPort,true--[[unset]])
        end

        -- debug specific part 
        if index == 1 then
            --stopSimulationLog()
        end
        --break
    end
end

--packet classifications - vlan-tag(no vid),mpls,ipv4,ipv6,ude,mac-da 
local function test_non_dsa_classifications()
    local test2_packets = {
        {type = vlan_tag , value = 6 , packet = buildPacketVlanTag(0x9100 --[[tpid]] ,   15 --[[vid]] , 6 --[[up]])},
        {type = vlan_tag , value = 6 , packet = buildPacketVlanTag(0x9100 --[[tpid]] , 4093 --[[vid]] , 6 --[[up]])},
        {type = vlan_tag , value = 6 , packet = buildPacketVlanTag(0x9100 --[[tpid]] , 4095 --[[vid]] , 6 --[[up]])},
        {type = mpls     , value = 4           , packet = buildPacketMpls(4--[[exp]])},
        {type = ipv4     , value = 0xab        , packet = buildPacketIpv4(0xab--[[TOS]])},
        {type = ipv6     , value = 0xe7        , packet = buildPacketIpv6(0xe7--[[TC]])},
        {type = ude      , value = 2--[[index]], packet = buildPacketUde(0xabcd--[[etherType]])},
        {type = mac_da   , value = 1--[[index]], packet = buildPacketMacDaMc("0180C2000004")}, --01:80:C2:00:00:04
        {type = mac_da   , value = 3--[[index]], packet = buildPacketMacDaMc("01000C005566")}, --01:00:0C:00:55:66 --> Cisco macs 
    }
    
    local sectionName = "packet classifications - vlan-tag(no vid),mpls,ipv4,ipv6,ude,mac-da "
    
    test_classifications(test2_packets,sectionName)
end

--##################################
--##################################
--packet classifications - use default port's priority
local function test_default_port_priority(test_cpu_port)
    local pip_priority
    local ingressPacket = buildPacketUc(nil) -- etherType is 0x6666 from packetPayload

    local transmitInfo = {portNum = ingressPort , pktInfo = {fullPacket = ingressPacket} }
    local egressInfoTable = {
        -- expected egress port .. unmodified
        {portNum = egressPort  , pktInfo = {fullPacket = ingressPacket}},
        -- expect other ports not to get it
        {portNum = port1  , packetCount = 0},
        --{portNum = port2  , packetCount = 0}, --> the ingress port
        --{portNum = port3  , packetCount = 0}, --> the egress port
        {portNum = port4  , packetCount = 0}
    }
    
    local function convertTestPipPriorityToCpuPortPriority(pip_priority)
        local cpu_pip_priority
        if is_sip_5_20(devEnv.dev) then
            if pip_priority == "low" then
                cpu_pip_priority = "medium"
            elseif pip_priority == "medium" then
                cpu_pip_priority = "high"
            elseif pip_priority == "high" then
                cpu_pip_priority = "very-high"
            else --pip_priority == "very-high"
                cpu_pip_priority = "low"
            end
        else
            if pip_priority == "low" then
                cpu_pip_priority = "medium"
            elseif pip_priority == "medium" then
                cpu_pip_priority = "high"
            else --pip_priority == "high"
                cpu_pip_priority = "low"
            end
        end
        return cpu_pip_priority
    end
    
    -- function to set configuration for the test
    local function configFunc(new_pip_priority)
        local param_new_pip_priority = new_pip_priority
        if new_pip_priority == nil then 
            new_pip_priority = pip_priority
        end
        -- set pip-profile on the ingress port
        local command_portPipPriority = [[
        end
        configure
        interface ethernet ${dev}/]] .. ingressPort .. [[
        
            pip priority-default ]] .. new_pip_priority .. [[
            
        exit
        ]]
        executeStringCliCommands(command_portPipPriority)
        
        local configCpuPort , cpu_pip_priority
        if param_new_pip_priority == nil and pip_priority == "low" and not test_cpu_port then
            configCpuPort = true
            cpu_pip_priority = "high"
        elseif param_new_pip_priority == nil and test_cpu_port then
            configCpuPort = true
            cpu_pip_priority = convertTestPipPriorityToCpuPortPriority(pip_priority)
        else
            configCpuPort = false
        end

        if configCpuPort then
            -- the CPU port is also in 'low' priority
            -- so changing the 'low' threshold to 'drop' the packets
            -- will cause the packets not to reach from the cpu port to out 'ingress port'
            -- so move the CPU port out from the "low" --> to "high"
            -- use 'no trust' with default's port 'high'
            printLog("Set CPU port ".. cpuPort .. " with default priority : " .. cpu_pip_priority .. " and 'not trust' pip")
            local command_cpuPortPipPriority = [[
            end
            configure
            interface ethernet ${dev}/]] .. cpuPort .. [[
            
                pip priority-default ]] .. cpu_pip_priority .. [[
                
                no pip trust
            exit
            ]]
            executeStringCliCommands(command_cpuPortPipPriority)
        end
        
    end
    -- function to de-configuration for the test
    local function deConficFunc()
        -- restore port default priority
        --[[the orig pip priority from the configuration file]]
        local new_pip_priority = "low"
        configFunc(new_pip_priority)
        executeStringCliCommands(command_restoreThresholds)
       
        if pip_priority == "low" then
            -- the CPU port should restore pip configurations
            local command_cpuPortPipPriority = [[
            end
            configure
            interface ethernet ${dev}/]] .. cpuPort .. [[
            
                pip priority-default ]] .. "low" .. [[
                
            exit
            ]]
            executeStringCliCommands(command_cpuPortPipPriority)
        end
        
        if test_cpu_port then
            executeStringCliCommands(command_cpuPortPipTrust)
        end
        
    end
    -- function to change the proper threshold
    local function thresholdChangeFunc()
        local local_priority
        if test_cpu_port then
            local_priority = convertTestPipPriorityToCpuPortPriority(pip_priority)
            
            -- make sure that the CPU port is the one that dropped the packet
            if (isSimOrBeforeSIP520 == true) then
               -- not valid for BC3,Aldrin2 test scenario 
               transmitInfo.expectPacketFilteredBeforLoopback = true
            end

        else
            local_priority = pip_priority
        end

        if local_priority == "very-high" then --The value is impossible for <SIP 5.20
            executeStringCliCommands(command_threshold_veryhigh_reset)
        elseif local_priority == "high" then
            executeStringCliCommands(command_threshold_high_reset)
        elseif local_priority == "medium" then
            executeStringCliCommands(command_threshold_medium_reset)
        else -- low
            executeStringCliCommands(command_threshold_low_reset)
        end
    end

    local sectionInfo = {
        sectionName = "packet classifications - use default port's priority",
        purpose = "Test need to check that packet use default port's priority",
        transmitInfo = transmitInfo,
        egressInfoTable = egressInfoTable,
        configFunc = configFunc,
        deConficFunc = deConficFunc,
        thresholdChangeFunc = thresholdChangeFunc
    }
    
    local orig_sectionName = sectionInfo.sectionName
    local pip_priority_list
    
    for index,entry in pairs(prioritySet) do
        pip_priority = entry
        
        sectionInfo.sectionName = orig_sectionName .. "--> currently : port's priority = " .. pip_priority
        if test_cpu_port then
            sectionInfo.sectionName = sectionInfo.sectionName .. "--> but CPU port tested with : " .. convertTestPipPriorityToCpuPortPriority(pip_priority)
        end

        if pip_priority == "medium" then
            -- Set FROM CPU Traffic to be high priority to avoid drop on CPU Port
            executeStringCliCommands(command_from_cpu_high_set)
        end

        -->> test now
        testSection(sectionInfo)
        
        -- restore FROM CPU Traffic to be low priority
        executeStringCliCommands(command_from_cpu_low_set)

        transmitInfo.expectPacketFilteredBeforLoopback = nil-- restore value
        
        -- debug specific part 
        --break
    end

    -- restore CPU Port trust
    executeStringCliCommands(command_cpuPortPipTrust)
    
end

--##################################
--##################################
local function buildPacketDsaTag_FRW(isEdsa , qosProfile)
    local vlanId = 15
    local result, trgHwDevNum =  device_to_hardware_format_convert(devNum)
    local srcHwDevNum = trgHwDevNum + 1
    local dsaTagType
    
    if not isEdsa then
        dsaTagType = "CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT"
    else
        dsaTagType = "CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT"
    end
    
    local trgEport = egressPort
    local srcEPort = 0
    local origSrcPhyPort = 0
    local srcId = 0
    
    dsaTagStc = {
      dsaInfo={
        forward={
          dstEport=trgEport,
          source={
            portNum=srcEPort
          },
          tag0TpidIndex=0,
          phySrcMcFilterEnable=false,
          srcIsTagged="CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
          origSrcPhy={
            portNum=origSrcPhyPort
          },
          srcIsTrunk=false,
          egrFilterRegistered=true,
          qosProfileIndex = qosProfile,
          srcId=srcId,
          isTrgPhyPortValid = 1,
          dstInterface={
            type="CPSS_INTERFACE_PORT_E",
            devPort={
              portNum=egressPort,
              devNum=trgHwDevNum
            }
          },
          srcHwDev=srcHwDevNum
        }
      },
      commonParams={
        dsaTagType=dsaTagType,
        vpt=1,
        vid=vlanId
      },
      dsaType="CPSS_DXCH_NET_DSA_CMD_FORWARD_E"
    }
    
    local dsaString = luaTgfDsaStringFromSTC(devNum,dsaTagStc)
    
    return buildPacketUc(dsaString)
end

local function buildPacketDsaTag_FROM_CPU(isEdsa , up)
    local vlanId = 15
    local result, trgHwDevNum =  device_to_hardware_format_convert(devNum)
    local srcHwDevNum = trgHwDevNum + 1
    local dsaTagType
    
    if not isEdsa then
        dsaTagType = "CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT"
    else
        dsaTagType = "CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT"
    end
    
    local trgEport = egressPort
    local srcEPort = 0
    local origSrcPhyPort = 0
    local srcId = 0

    dsaTagStc={
        dsaInfo={
            fromCpu={
                  cascadeControl=true,
                  dp="CPSS_DP_GREEN_E",
                  tag0TpidIndex=7,
                  dstEport=0,
                  egrFilterRegistered=true,
                  isTrgPhyPortValid=true,
                  srcId=srcId,
                  srcHwDev=srcHwDevNum,
                  tc=7,
                  egrFilterEn=true,
                  dstInterface={
                    type="CPSS_INTERFACE_PORT_E",
                    devPort={
                      portNum=egressPort,
                      devNum=trgHwDevNum
                    }
                  },
            }
        },
        commonParams={
            dsaTagType=dsaTagType,
            vpt=up,
            vid=vlanId
        },
        dsaType="CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E"
    }
    
    local dsaString = luaTgfDsaStringFromSTC(devNum,dsaTagStc)
    
    return buildPacketUc(dsaString)
    
end

--packet classifications - dsa-qos,dsa-up
local function test_dsa_classifications()
--[[
a.	FRW DSA , 2 words , qos-profile = 0x64 , trgPort = egress port
b.	FRW DSA , 4 words , qos-profile = 0x64 , trgPort = egress port
c.	FROM_CPU DSA , 2 words , up = 5  , trgPort = egress port
d.	FROM_CPU DSA, 4 words , up = 5 , trgPort = egress port

]]--
    local test3_packets = {
        {type = dsa_tag_qos , value = 0x64 , packet = buildPacketDsaTag_FRW(true--[[eDsa]] , 0x64--[[qosProfile]]) , cascade = "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E"},
        {type = dsa_tag_qos , value = 0x64 , packet = buildPacketDsaTag_FRW(false--[[EXT-dsa]] , 0x64--[[qosProfile]]) , cascade = "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"},
        {type = dsa_tag_up  , value = 5    , packet = buildPacketDsaTag_FROM_CPU(true--[[eDsa]] , 5--[[up]]) , cascade = "CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E"},
        {type = dsa_tag_up  , value = 5    , packet = buildPacketDsaTag_FROM_CPU(false--[[EXT-dsa]] , 5--[[up]]) , cascade = "CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E"},
    }
    
    local sectionName = "packet classifications - dsa-qos,dsa-up "
    
    test_classifications(test3_packets,sectionName)

end
--##################################
--##################################
--[[ 
    need to check configurations by using next functions :
    NOTE: the 'config' file already did 'show' --> so counters should be 0
        so we can check counters only before we call the 'deconfig' that also calls 'show'
    
    -- for dev,port get drop counter :
    -- function returns isError , counter
    function pip_get_info_drop_port_counter(command_data, devNum, portNum)    

    -- for dev get all 3 priority drop counter :
    -- function returns isError , counterPrioHigh,counterPrioMedium,counterPrioLow
    function pip_get_info_drop_global_counter(command_data,devNum)
    
    -- for dev,port get info :
    -- function returns isError , portInfo
    function pip_get_info_port(command_data, devNum, portNum)
    
    -- for dev get info :
    -- function returns isError , devInfo
    function pip_get_info_device(command_data, devNum)
--]]

-- function to check expected values compared to actual values and print and register the error for the test
-- note : this function not care about the format of 'arrayToCheck'
--
-- callBackFunction(command_data, entry) --> callback function that will 
--          return with : isError , actualValue , expectedValue , description
--
-- arrayToCheck[] - array of 'entry'
--
local function checkExpectedValues(command_data,callBackFunction,arrayToCheck)
    for index,entry in pairs(arrayToCheck) do
        local isError , actualValue , expectedValue , description = callBackFunction(command_data, entry)
        
        
        if isError then
            local error_string = "ERROR : FAILED to get " .. description
            -- use command_data:addErrorAndPrint to be able to see also errors that came from 
            -- command_data that got it inside pip_get_info_drop_port_counter
            command_data:addErrorAndPrint(error_string) -- print and reset the errors string array
           
            --printLog ("ERROR : " .. error_string)
            testAddErrorString(error_string)
        else
            check_expected_value(description,expectedValue,actualValue)
        end
    end
end

-- check drop counters of test ports and global drop counters
local function expected_checkDropCounters()
    local command_data = Command_Data()
    local note_clear_on_read = "(check clear on read)."
    local sum_the_drop_counters = { perPort = 0 , perPriority = 0}
    
    local checkedPorts
    if is_sip_5_20(devEnv.dev) then
       if (isSimulation) then 
         checkedPorts = {
             {portNum = ingressPort , expectedValue = 0x21--[[33]]} -- empiric values we got ... and are ok
            ,{portNum = cpuPort     , expectedValue = 0x04        } -- empiric values we got ... and are ok
            ,{portNum = port1 , expectedValue =  0}
            ,{portNum = port3 , expectedValue =  0}
            ,{portNum = port4 , expectedValue =  0}

            ,{portNum = ingressPort , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
            ,{portNum = cpuPort     , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
         }
       else
         checkedPorts = {
             {portNum = port1 , expectedValue =  0}
            ,{portNum = port3 , expectedValue =  0}
            ,{portNum = port4 , expectedValue =  0}
            ,{portNum = ingressPort , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
            ,{portNum = cpuPort     , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
         }
       end
    else
        checkedPorts = {
             {portNum = ingressPort , expectedValue = 0x20--[[32]]} -- empiric values we got ... and are ok
            ,{portNum = cpuPort     , expectedValue = 0x03        } -- empiric values we got ... and are ok
            ,{portNum = port1 , expectedValue =  0}
            ,{portNum = port3 , expectedValue =  0}
            ,{portNum = port4 , expectedValue =  0}

            ,{portNum = ingressPort , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
            ,{portNum = cpuPort     , expectedValue = 0 , note = note_clear_on_read} -- check clear on read
        }
    end
    
    printLog("Start check drop counters - per port : ")
    
    -- entry is from checkedPorts[]
    local function wrapper__pip_get_info_drop_port_counter(command_data, entry)
        local portNum = entry.portNum
        local expectedValue = { l = {[0] = entry.expectedValue , [1] = 0}}
        sum_the_drop_counters.perPort = sum_the_drop_counters.perPort + entry.expectedValue
        local description = "port " .. portNum .." drop counters (at end of test)."
        if entry.note then
            description = description .. entry.note
        end

        local isError , cpssCounterLo, cpssCounterHi = pip_get_info_drop_port_counter(command_data, devNum, portNum)
        local cpssCounter = { l = {[0] = cpssCounterLo , [1] = cpssCounterHi}}
        
        return isError , cpssCounter , expectedValue , description
    end
    --check array checkedPorts
    checkExpectedValues(command_data,wrapper__pip_get_info_drop_port_counter,checkedPorts)
    
    printLog("Ended check drop counters - per port . ")

    printLog("Start check drop counters - per priority : ")
    local checkGlobalDrops

    if is_sip_5_20(devEnv.dev) then
      if (isSimulation) then 
        checkGlobalDrops = {
             {priority = 0 , expectedValue =  0x10} --very-high
            ,{priority = 1 , expectedValue =  0x0f} --high
            ,{priority = 2 , expectedValue =  0x04} --medium
            ,{priority = 3 , expectedValue =  0x02} --low

            ,{priority = 0 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --very-high
            ,{priority = 1 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --high
            ,{priority = 2 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --medium
            ,{priority = 3 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --low
        }
      else
        checkGlobalDrops = {
             {priority = 0 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --very-high
            ,{priority = 1 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --high
            ,{priority = 2 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --medium
            ,{priority = 3 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --low
        }
      end
    else
        checkGlobalDrops = {
             {priority = 0 , expectedValue =  0x10} --high
            ,{priority = 1 , expectedValue =  0x11} --medium
            ,{priority = 2 , expectedValue =  0x02} --low

            ,{priority = 0 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --high
            ,{priority = 1 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --medium
            ,{priority = 2 ,expectedValue =  0, note = note_clear_on_read} -- check clear on read --low
        }
    end

    local saved_drop_global_counter = {}
    -- entry is from checkGlobalDrops[]
    local function wrapper__pip_get_info_drop_global_counter(command_data, entry)
        local priority = entry.priority
        local priorityName = priorityNamesArr[priority]
        local expectedValue = { l = {[0] = entry.expectedValue , [1] = 0}}
        sum_the_drop_counters.perPriority = sum_the_drop_counters.perPriority + entry.expectedValue
        local description = "priority [" .. priorityName .."] drop counters (at end of test)."
        if entry.note then
            description = description .. entry.note
        end

        if(priority == 0) then -- priority is highest
            local counterPrioHighest
            local isError
            local counterPrioVeryhigh, counterPrioHigh, counterPrioMedium, counterPrioLow
            if is_sip_5_20(devEnv.dev) then
                isError,counterPrioVeryhigh,counterPrioHigh,counterPrioMedium,counterPrioLow = pip_get_info_drop_global_counter(command_data, devNum, priority)
                -- save values for the next calles
                saved_drop_global_counter[1] = counterPrioHigh
                saved_drop_global_counter[2] = counterPrioMedium
                saved_drop_global_counter[3] = counterPrioLow
                counterPrioHighest = counterPrioVeryhigh
            else
                isError,counterPrioHigh,counterPrioMedium,counterPrioLow = pip_get_info_drop_global_counter(command_data, devNum, priority)
                -- save values for the next calles
                saved_drop_global_counter[1] = counterPrioMedium
                saved_drop_global_counter[2] = counterPrioLow
                counterPrioHighest = counterPrioHigh
            end

            return isError, counterPrioHighest, expectedValue, description
        else
            local isError = false
            return isError, saved_drop_global_counter[priority], expectedValue, description
        end
    end
    --check array checkGlobalDrops
    checkExpectedValues(command_data,wrapper__pip_get_info_drop_global_counter,checkGlobalDrops)

    printLog("Ended check drop counters - per priority . ")

    if (isSimOrBeforeSIP520 == true) then
      if sum_the_drop_counters.perPriority ~= sum_the_drop_counters.perPort then
          -- error the sum of all ports should be the same as the sum of the global.
          local error_string = "The sum of drops 'per port' [" .. sum_the_drop_counters.perPort .. "] not equal sum of drops 'per priority' [" .. sum_the_drop_counters.perPriority .."]"
          printLog ("ERROR : " .. error_string)
          testAddErrorString(error_string)
      end
    end

end

local function expected_checkValuesAfterConfigFile_perPort(command_data)
    local checkedPorts =
        {
            -- empiric values we got ... and are ok
             {portNum = ingressPort , expectedValue = {trusted = true,profile = 2,vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
            ,{portNum = cpuPort     , expectedValue = {trusted = true,            vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
            ,{portNum = port1       , expectedValue = {trusted = true,            vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
            ,{portNum = port3       , expectedValue = {trusted = true,            vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
            ,{portNum = port4       , expectedValue = {trusted = true,            vidTpidPriority = lowPriorityNum, defaultPriority = lowPriorityNum}}
        }

    -- entry is from checkedPorts[]
    local function wrapper__pip_get_info_port(command_data, entry)
        local portNum = entry.portNum
        local expectedValue = entry.expectedValue
        local description = "port " .. portNum .." info (after config file)."
        
        local isError , info = pip_get_info_port(command_data, devNum, portNum)

        return isError , info , expectedValue , description
    end
    --check array checkedPorts
    checkExpectedValues(command_data,wrapper__pip_get_info_port,checkedPorts)
    
end
local function expected_checkValuesAfterConfigFile_global(command_data)
    local   macDaClassification={
      [0]={macAddrValue="01:80:c1:00:00:00",macAddrMask="ff:ff:ff:ff:00:00",dbName="mac-mc-IEEE-c1"},
          {macAddrValue="01:80:c2:00:00:00",macAddrMask="ff:ff:ff:00:00:00",dbName="mac-mc-IEEE-c2"},
          {macAddrValue="01:80:c3:00:00:00",macAddrMask="ff:00:ff:00:00:00",dbName="mac-mc-IEEE-c3"},
          {macAddrValue="01:00:0c:00:00:00",macAddrMask="ff:ff:ff:ff:00:00",dbName="mac-mc-CISCO"}
    }
    local vidClassification={
      [0]={vid=1,   etherType=0x8100},
          {vid=5,   etherType=0x8100},
          {vid=5,   etherType=0x9100},
          {vid=4094,etherType=0x9100}
      }
    local thresholds
    if is_sip_5_20(devEnv.dev) then
        thresholds ={
            [0] = {value=1023},
                  {value=1023},
                  {value=1023},
                  {value=1023}
        }
    else
        thresholds ={
            [0] = {value=1023},
                  {value=1023},
                  {value=1023}
        }

    end
    local udeClassification={
      [0]={etherType=0x6789,    dbName="ude0x6789"},
          {etherType=0x789A,    dbName="ude0x789a"},
          {etherType=0xabcd,    dbName="ude0xabcd"},
          {etherType=0xfedc,    dbName="ude0xfedc"}
    }
    
    local profiles = { [0] = {},{},{},{} } -- fill in runtime
    local checkedDev = 
        { 
            -- empiric values we got ... and are ok
             {macDaClassification = macDaClassification , vidClassification = vidClassification ,thresholds = thresholds,
                udeClassification = udeClassification , 
                profiles = profiles , enabled = true
             } 
        }
    
    -- set all values with default priority ('low')
    for profile = 0,3 do
        local perProfile = profiles[profile]
        for pip_type,max_value in pairs(pip_type_max_value) do
            perProfile[pip_type] = {}
            local perType = perProfile[pip_type]
            for index = 0,max_value do
                perType[index] = lowPriorityNum
            end
        end
    end
    
    -- override values:
--[[
    pip-priority --> specific
    1.	On pip-profile  = 2 
        a.	Priority high for 'vlan-tag' for 'up' = 6 , 7
        b.	Priority high for 'mpls' for 'EXP' = 4
        c.	Priority high for 'ipv4' for 'TOS' = 0xab
        d.	Priority high for 'ipv6' for 'TC' = 0xe7
        e.	Priority high for 'ude' for 'index' = 2
        f.	Priority high for 'mac-da' for 'index' = 1 , 3
        g.	Priority high for 'dsa-qos' for 'qos' = 0x64
        h.	Priority high for 'dsa-up' for 'up' = 5

    2.	On pip-profile  = 3
        a.	Set like for profile 2 but with pip-priority medium
]]    
    local ipv6ErratumImpact = (0xe7 % 64)
    local overridePriority = {
        -- type                 fieldIndex
        [VLAN_TAG_UP]           = { 6,      7   },
        [MPLS_EXP]              = { 4           },
        [IPV4_TOS]              = {0xab         },
        [IPV6_TC]               = {0xe7         },
        [UDE_INDEX]             = {2            },
        [MAC_DA_INDEX]          = {1,       3   },
        [DSA_QOS_PROFILE]       = {0x64         },
        [DSA_UP]                = {5            }
    }

    if pip_is_device_ipv6_erratum(devNum) then
        overridePriority[IPV6_TC] = {ipv6ErratumImpact + 0 , ipv6ErratumImpact + 64 , ipv6ErratumImpact+128,ipv6ErratumImpact+192 } -- due to erratum
    end

    for profile = 2,3 do
        local perProfile = profiles[profile]
        for pip_type,fields in pairs(overridePriority) do
            local perType = perProfile[pip_type]
            for jj,field in pairs(fields) do
                if profile == 2 then 
                    perType[field] = 0 -- high (very high) priority
                else
                    perType[field] = 1 -- medium (high) priority
                end
            end
        end
    end

    -- entry is from checkedDev[]
    local function wrapper__pip_get_info_device(command_data, entry)
        local expectedValue = entry
        local description = "Global info (after config file)."

        local isError , info = pip_get_info_device(command_data, devNum)

        return isError , info , expectedValue , description
    end
    --check array checkedDev
    checkExpectedValues(command_data,wrapper__pip_get_info_device,checkedDev)
end

-- called after the 'config file' so we can check it's configurations
local function expected_checkValuesAfterConfigFile()
    local command_data = Command_Data()
    
    expected_checkValuesAfterConfigFile_perPort(command_data)
    expected_checkValuesAfterConfigFile_global(command_data)
end

local beforeTestStartedInfo_perPort = {
     {portNum = ingressPort , expectedValue = nil}
    ,{portNum = cpuPort     , expectedValue = nil}
    ,{portNum = port1       , expectedValue = nil} 
    ,{portNum = port3       , expectedValue = nil} 
    ,{portNum = port4       , expectedValue = nil} 
}
local function learnValuesBeforeConfigFile_perPort(command_data)
    -- entry is from beforeTestStartedInfo_perPort[]
    for index,entry in pairs(beforeTestStartedInfo_perPort) do
        local portNum = entry.portNum
        local isError , info = pip_get_info_port(command_data, devNum, portNum)
        entry.expectedValue = info
    end
end
-- global DB that hold value frmo 'before' calling the config file
-- so we can compare it to values after the deconfig file
local beforeTestStartedInfo_global = {}
-- entry is from beforeTestStartedInfo_global[]
local function learnValuesBeforeConfigFile_global(command_data)
    local isError , info = pip_get_info_device(command_data, devNum)

    beforeTestStartedInfo_global[1] = info
end

-- called BEFORE any 'config file' so we can learn info 'before test'
local function learnValuesBeforeConfigFile()
    local command_data = Command_Data()
    
    learnValuesBeforeConfigFile_perPort(command_data)
    learnValuesBeforeConfigFile_global(command_data)
end

local function expected_checkValuesAfterDeconfigFile_perPort(command_data)
    -- entry is from beforeTestStartedInfo_perPort[]
    local function wrapper__pip_get_info_port(command_data, entry)
        local portNum = entry.portNum
        local expectedValue = entry.expectedValue
        local description = "port " .. portNum .." info (after deconfig file --> end of test)."
        
        local isError , info = pip_get_info_port(command_data, devNum, portNum)

        return isError , info , expectedValue , description
    end
    --check array beforeTestStartedInfo_perPort
    checkExpectedValues(command_data,wrapper__pip_get_info_port,beforeTestStartedInfo_perPort)
end

local function expected_checkValuesAfterDeconfigFile_global(command_data)
    -- entry is from beforeTestStartedInfo_global[]
    local function wrapper__pip_get_info_device(command_data, entry)
        local expectedValue = entry
        local description = "Global info (after deconfig file --> end of test)."

        local isError , info = pip_get_info_device(command_data, devNum)

        return isError , info , expectedValue , description
    end
    --check array beforeTestStartedInfo_global
    checkExpectedValues(command_data,wrapper__pip_get_info_device,beforeTestStartedInfo_global)
end

-- called after the 'deconfig file' so we can check it restored configurations
local function expected_checkValuesAfterDeconfigFile()
    local command_data = Command_Data()
    
    expected_checkValuesAfterDeconfigFile_perPort(command_data)
    expected_checkValuesAfterDeconfigFile_global(command_data)
end


--##################################
--##################################
local function generalInit()
    if is_sip_5_20(devEnv.dev) then
        executeStringCliCommands(command_allPortPipTrust);
        executeStringCliCommands(command_restoreThresholds);
    end

    xcat3x_start(true)
    -- called BEFORE any 'config file' so we can learn info 'before test'
	
    learnValuesBeforeConfigFile()

    if is_sip_5_20(devEnv.dev) then
        global_test_data_env.priority0 = "very-high";
        global_test_data_env.priority1 = "high";
        global_test_data_env.defaultThreshold = 65535;
    else
        global_test_data_env.priority0 = "high";
        global_test_data_env.priority1 = "medium";
        global_test_data_env.defaultThreshold = 1023;
    end
    executeLocalConfig(luaTgfBuildConfigFileName("pip_example"))
    executeStringCliCommands(command_set_egress_port_keep_tpid)
    executeStringCliCommands(command_set_ingress_port_pvid_15)
    if is_sip_5_20(devEnv.dev) then
        executeStringCliCommands(command_set_thr_veryhigh)
    end


    -- called after the 'config file' so we can check it's configurations
    expected_checkValuesAfterConfigFile()
    
end

local function generalEnd()
    xcat3x_start(false)

    -- check drop counters of test ports and global drop counters
    expected_checkDropCounters()-- need to be before 'deconfig' 

    executeStringCliCommands(command_restoreThresholds)
    executeStringCliCommands(command_set_egress_port_not_keep_tpid)
    executeStringCliCommands(command_set_ingress_port_pvid_1)
    --unset config
    executeLocalConfig(luaTgfBuildConfigFileName("pip_example",true))

    if is_sip_5_20(devEnv.dev) then
        executeStringCliCommands(command_set_dflt_thr_veryhigh)
    end

    -- called after the 'deconfig file' so we can check it restored configurations
    expected_checkValuesAfterDeconfigFile()

    -- print results summary
    testPrintResultSummary("PIP")
end


--set config
printLog("--------start generic PIP configuration : ")
generalInit()

--TPID+vid classifications
test_TPID_vid_classifications()
--packet classifications - vlan-tag(no vid),mpls,ipv4,ipv6,ude,mac-da 
test_non_dsa_classifications()
-- use default port's priority
test_default_port_priority(false)--not testing CPU port

--packet classifications - dsa-qos,dsa-up
if (isSimOrBeforeSIP520 == false) then
  disableCheckDropCounters = true
end
test_dsa_classifications()
if (isSimOrBeforeSIP520 == false) then
  disableCheckDropCounters = false
end

-- CPU port - use default port's priority
test_default_port_priority(true)--testing CPU port

printLog("--------restore PIP configuration : ")
generalEnd()
printLog("-------- PTP test ended --- \n")


