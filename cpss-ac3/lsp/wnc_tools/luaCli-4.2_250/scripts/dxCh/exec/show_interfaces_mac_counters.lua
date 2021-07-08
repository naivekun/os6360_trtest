--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_mac_counters.lua
--*
--* DESCRIPTION:
--*       showing of statistics on traffic seen by the physical interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 7 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssGtU64StrGet")
cmdLuaCLI_registerCfunction("wrlCpssGtU64MathAdd")

--constants


-- ************************************************************************
---
--  get_counters
--        @description  For each packet processor, get the Rx packet counters
--                     from its SDMA
--
--        @param command_data   - command execution data object
--        @param devNum         - device number
--        @param portNum        - port number
--
--        @return       array with counters on success, otherwise nill
--
local function get_counters(command_data, devNum, portNum)
    local result, values
    if portNum == 63 and DeviceCpuPortMode == "CPSS_NET_CPU_PORT_MODE_SDMA_E" then
        local i
        local pkts = nil
        local bytes = nil
        local rxerrs = nil
        for i=0,7 do
            result, values = myGenWrapper("cpssDxChNetIfSdmaRxCountersGet", {
                { "IN",  "GT_U8",  "devNum", devNum },
                { "IN",  "GT_U8",  "queueIdx", i },
                { "OUT", "CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC", "rxCounters" }
            })
            if result == 0 then
                if pkts == nil then
                    pkts = { l={[0] = 0, [1] = 0 }}
                    bytes = { l={[0] = 0, [1] = 0 }}
                end
                local p = values.rxCounters.rxInPkts
                local b = values.rxCounters.rxInOctets
                pkts = wrlCpssGtU64MathAdd(pkts, {l={[0]=p,[1]=0}})
                bytes = wrlCpssGtU64MathAdd(bytes, {l={[0]=b,[1]=0}})
            else
                print("Eror: cpssDxChNetIfSdmaRxCountersGet returned: result")
            end
        end
        result, values = myGenWrapper("cpssDxChNetIfSdmaRxErrorCountGet", {
            { "IN",  "GT_U8",  "devNum", devNum },
            { "OUT", "CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC", "rxErrCount" }
        })
        if result == 0 then
            rxerrs = { l={[0] = 0, [1] = 0 }}
            for i=0,7 do
                local e = values.rxErrCount.counterArray[i]
                rxerrs = wrlCpssGtU64MathAdd(rxerrs, {l={[0]=e,[1]=0}})
            end
        else
            print("Eror: cpssDxChNetIfSdmaRxErrorCountGet returned: result")
        end
        if bytes == nil then
            return nil
        end
        return { goodOctetsRcv = bytes,
                ucPktsRcv = pkts,
                macRcvError = rxerrs,
                badOctetsRcv = {l={[0]=0,[1]=0}}
            }
    else
        result, values = cpssPerPortParamGet("cpssDxChPortMacCountersOnPortGet",
                                                 devNum, portNum, 
                                               "portMacCounterSetArray", 
                                               "CPSS_PORT_MAC_COUNTER_SET_STC")
        if 0x10 == result then
            command_data:setFailLocalStatus()
            command_data:addWarning("Traffic statistic getting is not allowed")
            return nil
        elseif 0 ~= result then
            command_data:setFailLocalStatus()
            command_data:addError("Error at traffic statistic getting")
            return nil
        end
        return values.portMacCounterSetArray
    end
end


-- ************************************************************************
---
--  get_cg_counters
--        @description  For Bobcat3 packet processor, get the Rx packet counters
--
--        @param command_data   - command execution data object
--        @param devNum         - device number
--        @param portNum        - port number
--
--        @return       array with counters on success, otherwise nill
--
local function get_cg_counters(command_data, devNum, portNum)
    local result, values

    -- Parameters check --
    if portNum == nil then
        command_data:setFailStatus()
        command_data:addNotExistendPort(devNum, portNum)
        return nil
    end

    if devNum == nil then
        command_data:setFailStatus()
        command_data:addNotExistendPort(devNum, portNum)
        return nil
    end

    if false == does_port_exist(devNum, portNum) then
        command_data:setFailStatus()
        command_data:addNotExistendPort(devNum, portNum)
        return nil
    end

    result, values = cpssPerPortParamGet("cpssDxChPortMacCountersOnCgPortGet",
                                             devNum, portNum,
                                           "cgMibStcPtr",
                                           "CPSS_PORT_MAC_CG_COUNTER_SET_STC")
    if 0x10 == result then
        command_data:setFailStatus()
        command_data:addError("Traffic statistic getting is not supported on this port")
        return nil
    elseif 0 ~= result then
        command_data:setFailStatus()
        command_data:addError("Error at traffic statistic getting")
        return nil
    end
    return values.cgMibStcPtr
end



-- ************************************************************************
---
--  show_interfaces_mac_counters_single_port
--        @description  shows statistics on traffic seen by the physical 
--                      interface with single port 
--
--        @param params         - params["all"]: all devices port or all 
--                                ports of given device cheking switch, 
--                                could be irrelevant;
--                                params["devID"]: checked device number, 
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil;
--                                params["port-channel"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil
--        @param command_data   - command execution data object
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_mac_counters_single_port(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    local counters_t = {
        "goodOctetsRcv",
        "badOctetsRcv",
        "macTransmitErr",
        "brdcPktsRcv",
        "mcPktsRcv",
        "pkts64Octets",
        "pkts65to127Octets",
        "pkts128to255Octets",
        "pkts256to511Octets",
        "pkts512to1023Octets",
        "pkts1024tomaxOoctets",
        "goodOctetsSent",
        "excessiveCollisions",
        "mcPktsSent",
        "brdcPktsSent",
        "fcSent",
        "goodFcRcv",
        "dropEvents",
        "undersizePkts",
        "fragmentsPkts",
        "oversizePkts",
        "jabberPkts",
        "macRcvError",
        "badCrc",
        "collisions",
        "lateCollisions",
        "fc_received",
        "ucPktsRcv",
        "ucPktsSent",
        "multiplePktsSent",
        "deferredPktsSent"
    }

    local cnt_string_prefix = {
        goodOctetsRcv = "Good Octets Received:\t\t",
        badOctetsRcv = "Bad Octets Received:\t\t",
        macTransmitErr = "MAC Transmit Error:\t\t",
        brdcPktsRcv = "BRDC Packets Received:\t\t",
        mcPktsRcv = "MC Packets Received:\t\t" ,
        pkts64Octets = "Size 64:\t\t\t",
        pkts65to127Octets = "Size 65 to 127:\t\t\t",
        pkts128to255Octets = "Size 128 to 255:\t\t",
        pkts256to511Octets = "Size 256 to 511:\t\t",
        pkts512to1023Octets = "Size 512 to 1023:\t\t",
        pkts1024tomaxOoctets = "Size 1024 to Max:\t\t",
        goodOctetsSent = "Good Octets Sent:\t\t",
        excessiveCollisions = "Excessive Collision:\t\t",
        mcPktsSent = "MC Packets Sent:\t\t",
        brdcPktsSent = "BRDC Packets Sent:\t\t",
        fcSent = "FC Sent:\t\t\t",
        goodFcRcv = "Good FC Received:\t\t",
        dropEvents = "Drop Events:\t\t\t",
        undersizePkts = "Undersize Packets:\t\t",
        fragmentsPkts = "Fragments Packets:\t\t",
        oversizePkts = "Oversize Packets:\t\t",
        jabberPkts = "Jabber Packets:\t\t\t",
        macRcvError = "MAC RX Error Packets Received:\t",
        badCrc = "Bad CRC:\t\t\t",
        collisions = "Collisions:\t\t\t",
        lateCollisions = "Late Collision:\t\t\t",
        ucPktsRcv = "Good UC Packets Received:\t",
        ucPktsSent = "Good UC Packets Sent:\t\t",
        multiplePktsSent = "Multiple Packets Sent:\t\t",
        deferredPktsSent = "Deferred Packets Sent:\t\t",
        fc_received = "FC Received:\t\t\t"
    }
    local header_string, footer_string    
    local resulting_string
    
    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()

    -- Command specific variables initialization.    
    header_string                                 = ""

    command_data:clearLocalStatus()

    -- Traffic statistic getting.    
    local counters
    if true  == command_data["local_status"] then
        counters = get_counters(command_data, devNum, portNum)

        if ((counters ~= nil and counters.goodFcRcv ~= nil and counters.badFcRcv ~= nil) == true ) then
            counters.fc_received =
                wrlCpssGtU64MathAdd(counters.goodFcRcv, counters.badFcRcv)
        else
            counters.fc_received = 0
        end
    end

    command_data:updateStatus()

    -- Traffic statistic strings formatting and adding.
    local i
    for i=1,#counters_t do
        local nm = counters_t[i]
        local pref = cnt_string_prefix[nm]
        local suff = ""
        if i == #counters_t then suff = "\n" end
        local str
        if counters == nil then
            str = "n/a"
        else
            str = wrlCpssGtU64StrGet(counters[nm])
        end
        command_data:setResultStr(pref, str, suff)
        command_data:addResultToResultArray()  
    end

    -- Resulting string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStr(header_string, command_data["result"],
                              footer_string)

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

	return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  show_interfaces_mac_counters_single_cg_port
--        @description  shows statistics on traffic seen by the physical
--                      CG interface with single port
--
--        @param params         - params["devID"]: checked device number;
--                                params["ethernet"]: checked interface
--                                name
--        @param command_data   - command execution data object
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_mac_counters_single_cg_port(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    local counters_t = {
        "ifOutOctets",
        "ifOutMulticastPkts",
        "ifOutErrors",
        "ifOutUcastPkts",
        "ifOutBroadcastPkts",
        "ifInOctets",
        "ifInErrors",
        "ifInBroadcastPkts",
        "ifInMulticastPkts",
        "ifInUcastPkts",
        "aFramesTransmittedOK",
        "aFramesReceivedOK",
        "aFrameCheckSequenceErrors",
        "aMACControlFramesTransmitted",
        "aMACControlFramesReceived",
        "aPAUSEMACCtrlFramesTransmitted",
        "aPAUSEMACCtrlFramesReceived",
        "aCBFCPAUSEFramesTransmitted_0",
        "aCBFCPAUSEFramesTransmitted_1",
        "aCBFCPAUSEFramesTransmitted_2",
        "aCBFCPAUSEFramesTransmitted_3",
        "aCBFCPAUSEFramesTransmitted_4",
        "aCBFCPAUSEFramesTransmitted_5",
        "aCBFCPAUSEFramesTransmitted_6",
        "aCBFCPAUSEFramesTransmitted_7",
        "aCBFCPAUSEFramesReceived_0",
        "aCBFCPAUSEFramesReceived_1",
        "aCBFCPAUSEFramesReceived_2",
        "aCBFCPAUSEFramesReceived_3",
        "aCBFCPAUSEFramesReceived_4",
        "aCBFCPAUSEFramesReceived_5",
        "aCBFCPAUSEFramesReceived_6",
        "aCBFCPAUSEFramesReceived_7",
        "etherStatsOctets",
        "etherStatsUndersizePkts",
        "etherStatsOversizePkts",
        "etherStatsJabbers",
        "etherStatsDropEvents",
        "etherStatsFragments",
        "etherStatsPkts",
        "etherStatsPkts64Octets",
        "etherStatsPkts65to127Octets",
        "etherStatsPkts128to255Octets",
        "etherStatsPkts256to511Octets",
        "etherStatsPkts512to1023Octets",
        "etherStatsPkts1024to1518Octets",
        "etherStatsPkts1519toMaxOctets",
        "aAlignmentErrors",
        "aFrameTooLongErrors",
        "aInRangeLengthErrors",
        "VLANTransmittedOK",
        "VLANRecievedOK"
    }

    local cnt_string_prefix = {
        ifOutOctets = "ifOutOctets:\t\t",
        ifOutMulticastPkts = "ifOutMulticastPkts:\t\t",
        ifOutErrors = "ifOutErrors:\t\t",
        ifOutUcastPkts = "ifOutUcastPkts:\t\t",
        ifOutBroadcastPkts = "ifOutBroadcastPkts:\t\t",
        ifInOctets = "ifInOctets:\t\t",
        ifInErrors = "ifInErrors:\t\t",
        ifInBroadcastPkts = "ifInBroadcastPkts:\t\t",
        ifInMulticastPkts = "ifInMulticastPkts:\t\t",
        ifInUcastPkts = "ifInUcastPkts:\t\t",
        aFramesTransmittedOK = "aFramesTransmittedOK:\t\t",
        aFramesReceivedOK = "aFramesReceivedOK:\t\t",
        aFrameCheckSequenceErrors = "aFrameCheckSequenceErrors:\t\t",
        aMACControlFramesTransmitted = "aMACControlFramesTransmitted:\t\t",
        aMACControlFramesReceived = "aMACControlFramesReceived:\t\t",
        aPAUSEMACCtrlFramesTransmitted = "aPAUSEMACCtrlFramesTransmitted:\t\t",
        aPAUSEMACCtrlFramesReceived = "aPAUSEMACCtrlFramesReceived:\t\t",
        aCBFCPAUSEFramesTransmitted_0 = "aCBFCPAUSEFramesTransmitted_0:\t\t",
        aCBFCPAUSEFramesTransmitted_1 = "aCBFCPAUSEFramesTransmitted_1:\t\t",
        aCBFCPAUSEFramesTransmitted_2 = "aCBFCPAUSEFramesTransmitted_2:\t\t",
        aCBFCPAUSEFramesTransmitted_3 = "aCBFCPAUSEFramesTransmitted_3:\t\t",
        aCBFCPAUSEFramesTransmitted_4 = "aCBFCPAUSEFramesTransmitted_4:\t\t",
        aCBFCPAUSEFramesTransmitted_5 = "aCBFCPAUSEFramesTransmitted_5:\t\t",
        aCBFCPAUSEFramesTransmitted_6 = "aCBFCPAUSEFramesTransmitted_6:\t\t",
        aCBFCPAUSEFramesTransmitted_7 = "aCBFCPAUSEFramesTransmitted_7:\t\t",
        aCBFCPAUSEFramesReceived_0 = "aCBFCPAUSEFramesReceived_0:\t\t",
        aCBFCPAUSEFramesReceived_1 = "aCBFCPAUSEFramesReceived_1:\t\t",
        aCBFCPAUSEFramesReceived_2 = "aCBFCPAUSEFramesReceived_2:\t\t",
        aCBFCPAUSEFramesReceived_3 = "aCBFCPAUSEFramesReceived_3 :\t\t",
        aCBFCPAUSEFramesReceived_4 = "aCBFCPAUSEFramesReceived_4:\t\t",
        aCBFCPAUSEFramesReceived_5 = "aCBFCPAUSEFramesReceived_5:\t\t",
        aCBFCPAUSEFramesReceived_6 = "aCBFCPAUSEFramesReceived_6:\t\t",
        aCBFCPAUSEFramesReceived_7 = "aCBFCPAUSEFramesReceived_7:\t\t",
        etherStatsOctets = "etherStatsOctets:\t\t",
        etherStatsUndersizePkts = "etherStatsUndersizePkts:\t\t",
        etherStatsOversizePkts = "etherStatsOversizePkts:\t\t",
        etherStatsJabbers = "etherStatsJabbers:\t\t",
        etherStatsDropEvents = "etherStatsDropEvents:\t\t",
        etherStatsFragments = "etherStatsFragments:\t\t",
        etherStatsPkts = "etherStatsPkts:\t\t",
        etherStatsPkts64Octets = "etherStatsPkts64Octets:\t\t",
        etherStatsPkts65to127Octets = "etherStatsPkts65to127Octets:\t\t",
        etherStatsPkts128to255Octets = "etherStatsPkts128to255Octets:\t\t",
        etherStatsPkts256to511Octets = "etherStatsPkts256to511Octets:\t\t",
        etherStatsPkts512to1023Octets = "etherStatsPkts512to1023Octets:\t\t",
        etherStatsPkts1024to1518Octets = "etherStatsPkts1024to1518Octets:\t\t",
        etherStatsPkts1519toMaxOctets = "etherStatsPkts1519toMaxOctets:\t\t",
        aAlignmentErrors = "aAlignmentErrors:\t\t",
        aFrameTooLongErrors = "aFrameTooLongErrors:\t\t",
        aInRangeLengthErrors = "aInRangeLengthErrors:\t\t",
        VLANTransmittedOK = "VLANTransmittedOK:\t\t",
        VLANRecievedOK = "VLANRecievedOK:\t\t"
    }
    local header_string, footer_string
    local resulting_string

    -- Fetting devNum and portNum --
    local dev, ports, index, port
    for dev, ports in pairs(command_data["dev_port_range"]) do
        for index, port in pairs(ports) do
            devNum = dev
            portNum = port
        end
    end

    -- Command specific variables initialization.
    header_string                                 = ""

    command_data:clearLocalStatus()

    -- Traffic statistic getting.
    local counters

    counters = get_cg_counters(command_data, devNum, portNum)

    -- Handle error --
    if true ~= command_data:getCommandExecutionResults() then
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
    end

    command_data:updateStatus()   

    -- Traffic statistic strings formatting and adding.   
    local i
    for i=1,#counters_t do
        local nm = counters_t[i]
        local pref = cnt_string_prefix[nm]
        local suff = ""
        if i == #counters_t then suff = "\n" end
        local str
        if counters == nil then
            str = "n/a"
        else
            str = wrlCpssGtU64StrGet(counters[nm])
        end
        command_data:setResultStr(pref, str, suff)
        command_data:addResultToResultArray()  
    end
        
    -- Resulting string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStr(header_string, command_data["result"], 
                              footer_string)        
        
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  show_interfaces_mac_counters_port_range
--        @description  shows statistics on traffic seen by the physical 
--                      interface with many ports
--
--        @param params         - params["all_device"]: all or given 
--                                devices iterating property, could be 
--                                irrelevant
--                                params["mac-address"]: mac-address, could 
--                                be irrelevant;
--                                params["ethernet"]: interface range, 
--                                could be irrelevant;
--                                params["port-channel"]: trunk id,could be 
--                                irrelevant;
--                                params["vlan"]: vlan Id, could be 
--                                irrelevant;
--                                params["skipped"]: skipping property, 
--                                could be irrelevant 
--        @param command_data   - command execution data object                     
--
--        @return        true on success, otherwise false and error message
--
local function show_interfaces_mac_counters_port_range(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local counters_t = {
        "ucPktsRcv",
        "mcPktsRcv",
        "brdcPktsRcv",
        "octets_received",
        "ucPktsSent",
        "mcPktsSent",
        "brdcPktsSent",
        "goodOctetsSent"
    }

    local header_string1, header_string2, footer_string1, footer_string2
   
    -- Command specific variables initialization.   
    header_string1 = 
        "\n" ..
        "Interface      UC Received          MC Received          BC Received       Octets Received  \n" ..
        "--------- -------------------- -------------------- -------------------- -------------------\n"
    footer_string1 = "\n"    
    header_string2 = 
        "\n" ..
        "Interface        UC Sent              MC Sent             BRDC Sent           Octets Sent     \n" ..
        "--------- -------------------- -------------------- --------------------- --------------------\n"        
    footer_string2 = "\n"
    
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()     

if portNum == "CPU" then portNum = 63 end
            -- DevNum/PortNum string forming.
            devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) .. 
                                                         '/' .. 
                                                         tostring(portNum), 9)
            
            -- Traffic statistic getting.
            command_data:clearLocalStatus()
            
            local counters, cs
            if true == command_data["local_status"] then    
                counters = get_counters(command_data, devNum, portNum)
                cs = {}
                if counters ~= nil then
                    counters.octets_received =
                        wrlCpssGtU64MathAdd(counters.goodOctetsRcv, counters.badOctetsRcv)
                else
                    counters = {}
                end
                local i
                for i=1,#counters_t do
                    local nm = counters_t[i]
                    if counters[nm] ~= nil then
                        cs[nm] = wrlCpssGtU64StrGet(counters[nm])
                    else
                        cs[nm] = "n/a"
                    end
                end
                
            end
            
            command_data:updateStatus()            
            
            -- First resulting string formatting and adding.
            command_data["result"] = 
                string.format("%-10s%-21s%-21s%-21s%-21s", 
                              devnum_portnum_string, 
                              alignLeftToCenterStr(cs.ucPktsRcv, 20), 
                              alignLeftToCenterStr(cs.mcPktsRcv, 20), 
                              alignLeftToCenterStr(cs.brdcPktsRcv, 20), 
                              alignLeftToCenterStr(cs.octets_received, 20))
            command_data:addResultToResultArray()

            -- Second resulting string formatting and adding.
            command_data["result"] = 
                string.format("%-10s%-21s%-21s%-21s%-21s", 
                              devnum_portnum_string, 
                              alignLeftToCenterStr(cs.ucPktsSent, 20),    
                              alignLeftToCenterStr(cs.mcPktsSent, 20), 
                              alignLeftToCenterStr(cs.brdcPktsSent, 20),   
                              alignLeftToCenterStr(cs.goodOctetsSent, 20))
            command_data:addResultToResultArray()
            
            command_data:updatePorts()
        end
    end            
        
    -- First resulting table string formatting.
    command_data:setResultArrayToResultStr(1, 2)
    command_data:setResultStrOnPortCount(header_string1, command_data["result"], 
                                         footer_string1,
                                         "There is no port information to show.\n")
                                         
    -- Second resulting table string formatting.
    command_data:addToResultStrOnPortCount(
        header_string2, command_data:getInterlacedResultArray(2, 2), 
        footer_string2)                                         
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()
end   
  

-- ************************************************************************
---
--  show_interfaces_mac_counters
--        @description  shows statistics on traffic seen by the physical 
--                      interface 
--
--        @param params         - params["all"]: all devices port or all 
--                                ports of given device cheking switch, 
--                                could be irrelevant;
--                                params["devID"]: checked device number, 
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil;
--                                params["port-channel"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_mac_counters(params)
    local result, values 
    -- Common variables declaration
    local command_data = Command_Data() 
    
    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)    
    if params.all ~= nil then
        local dev, ports
        for dev,ports in pairs(command_data.dev_port_range) do
            table.insert(ports,"CPU")
        end
    end
    
    -- System specific data initialization.
    command_data:enablePausedPrinting()       
    
    if true == command_data:isOnePortInPortRange() then
        result, values = show_interfaces_mac_counters_single_port(params, command_data)
    else
        result, values = show_interfaces_mac_counters_port_range(params, command_data)
    end
    
    return result, values
end


-- ************************************************************************
---
--  show_interfaces_mac_cg_counters
--        @description  shows statistics on traffic seen by the physical CG
--                      interface
--
--        @param params         - params["devID"]: checked device number;
--                                params["ethernet"]: checked interface
--                                name, relevant if params["all"] is not
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_mac_cg_counters(params)
    local result, values 
    -- Common variables declaration
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    -- System specific data initialization.
    command_data:enablePausedPrinting()

    -- Perform action --
    result, values = show_interfaces_mac_counters_single_cg_port(params, command_data)

    return result, values
end


--------------------------------------------------------------------------------
-- command registration: show interfaces mac counters
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show interfaces mac", "Interface(s) MIB Counters")
CLI_addCommand("exec", "show interfaces mac counters", {
    func = show_interfaces_mac_counters,
    help = "Interface(s) MIB Counters",
    params = {
        { type = "named",
            "#all_interfaces",
          mandatory = { "all_interfaces" }
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: show interfaces mac counters
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show interfaces mac", "CG Interface(s) MIB Counters")
CLI_addCommand("exec", "show interfaces mac counters cg ", {
    func = show_interfaces_mac_cg_counters,
    help = "Interface(s) MIB Counters",
    params = {{ type = "named", "#ethernet_devId", mandatory = { "ethernet_devId" }}}
})

