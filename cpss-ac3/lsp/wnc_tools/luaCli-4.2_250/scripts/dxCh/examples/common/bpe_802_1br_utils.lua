--[[

    utilities for testing the BPE 'system' .. the system is described below.

--]]

--[[            the 'system'
--
                      station_1
                            &
                            &
                            &
              ******************************************
              *            port 54                     *
              *                                        *
              *        Control bridge (CB)      port 0 *&&&&&& station_7 (not tested)
              *                                        *
              *       port 36             port 18      *
              ******************************************
                     #                         #
                     #                         #
                     
----------------------------------  PE ports for BC2 device -------------                     
                     
                     #                         #
       ***********************      ***********************
       *          port 18    *      *       port 36       *
       *                     *      *                     *
       *        PE3          *      *        PE2   port 0 *&&&&&&   station_2
       *                     *      *                     *         pcid=1004
       *    port 0           *      *    port 18          *         eport=1004
       ***********************      ***********************
              &                              # station_6
              &                              # pcid=1005
              &                              # eport=1005
        station_5                   ***********************
        pcid=1001                   *     port 54         *
        eport=1101                  *                     *
                                    *        PE1  port 36 *&&&&&& station_8 (not tested)
                                    *                     *       pcid=1003
                                    * port 0      port 18 *       eport=1003
                                    ***********************
                                       &              &
                                       &              &
                                       &              &
                                   station_4
                                   station_9       station_3
                                   pcid=1001       pcid=1002
                                   eport=1001      eport=1002
                                   
                                   
                                   
----------------------------------  PE ports for xCat3 device -------------                     

                     #                         #
       ***********************      ***********************
       *          port 18    *      *       port  8       *
       *       (xcat3)       *      *      (xcat3)        *
       *        PE3          *      *        PE2   port 0 *&&&&&&   station_2
       *                     *      *                     *         pcid=1004
       *    port 0           *      *    port  18         *         eport=1004
       ***********************      ***********************
              &                              # station_6
              &                              # pcid=1005
              &                              # eport=1005
        station_5                   ***********************
        pcid=1001                   *     port 23         *
        eport=1101                  *      (xcat3)        *
                                    *        PE1  port  8 *&&&&&& station_8 (not tested)
                                    *                     *       pcid=1003
                                    * port 0      port 18 *       eport=1003
                                    ***********************
                                       &              &
                                       &              &
                                       &              &
                                   station_4
                                   station_9       station_3
                                   pcid=1001       pcid=1002
                                   eport=1001      eport=1002                                    
--]]
-- when nil ignored , otherwise test only specific device
local debug_specific_test_Name = nil--"test 2 : PE1,1001-->pe2,1002"

local debug_on = true -- until we have HW results ... need to 'debug'
local function _debug(...)
    if debug_on == true then
        printLog(...)
    end
end
-- print error , regardless to 'debug' mode
local function _debug_printError(...)
    printLog(...)
end
--'extern' _debug_to_string(...)

-- get port for index for br ports (BC2)
local function port_in_sip5_device(portIndex)
    local devNum = devEnv.dev
    if is_sip_5(devNum) then
        -- sip5 devices need to use actual 'tested' ports
        -- only non sip5 need to call getTestPortExplicitFamily("BOBCAT2",portIndex)
        return getTestPort(portIndex)
    end

    -- NOTE : for xcat3 those numbers will be updated via array : systemInfoArr_xcat3_ports
    return getTestPortExplicitFamily("BOBCAT2" , portIndex)
end

local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(devNum)
if "CPSS_PP_FAMILY_DXCH_AC3X_E" == devFamily then
    global_test_data_env.ePort1101 = 1006
    global_test_data_env.ePort1001 = 1001
    global_test_data_env.ePort1002 = 1002
    global_test_data_env.ePort1003 = 1003
    global_test_data_env.ePort1004 = 1004
    global_test_data_env.ePort1005 = 1005
else
    global_test_data_env.ePort1101 = 1101
    global_test_data_env.ePort1001 = 1001
    global_test_data_env.ePort1002 = 1002
    global_test_data_env.ePort1003 = 1003
    global_test_data_env.ePort1004 = 1004
    global_test_data_env.ePort1005 = 1005
end

 station_1 = "station_1"
 station_2 = "station_2"
 station_3 = "station_3"
 station_4 = "station_4"
 station_5 = "station_5"
 station_6 = "station_6"
 station_7 = "station_7"
 station_8 = "station_8"
 station_9 = "station_9"

-- describe stations
local stationInfoArr = {
     {name = station_1 , macAddr = "000123400011" }
    ,{name = station_2 , macAddr = "000123400022" }
    ,{name = station_3 , macAddr = "000123400033" }
    ,{name = station_4 , macAddr = "000123400044" }
    ,{name = station_5 , macAddr = "000123400055" }
    ,{name = station_6 , macAddr = "000123400066" }
    ,{name = station_7 , macAddr = "000123400077" }
    ,{name = station_8 , macAddr = "000123400088" }
    ,{name = station_9 , macAddr = "000123400099" }
}

 devicesName_CB = "CB"
 devicesName_PE1 = "PE1"
 devicesName_PE2 = "PE2"
 devicesName_PE3 = "PE3"

-- describe system info
 local systemInfoArr = {
     {name = devicesName_CB , -- the control bridge
         portsArr =
         {
             {port = port_in_sip5_device(4) ,
                 stationsArr = {station_1 }
             },
             {port = port_in_sip5_device(1),
                 stationsArr = {station_7}
             },
             {port = port_in_sip5_device(2) , connectedDevice = devicesName_PE2},
             {port = port_in_sip5_device(3) , connectedDevice = devicesName_PE3}
         }
     }
    ,{name = devicesName_PE3 , -- PE3 in the system
        portsArr =
        {
             {port = port_in_sip5_device(1) , pcid = 1001 , eport = global_test_data_env.ePort1101 ,
                stationsArr = {station_5 }
             },
             {port = port_in_sip5_device(2) , uplinkPort = true , connectedDevice = devicesName_CB}
        }
    }
    ,{name = devicesName_PE2, -- PE2 in the system
        portsArr =
        {
            {port = port_in_sip5_device(1) , pcid = 1004  , eport = global_test_data_env.ePort1004 ,
                stationsArr = {station_2 }
            },
            {port = port_in_sip5_device(2) , pcid = 1005  , eport = global_test_data_env.ePort1005 , cascadePort = true, connectedDevice = devicesName_PE1,
                stationsArr = {station_6}
            },
            {port = port_in_sip5_device(3) , uplinkPort = true , connectedDevice = devicesName_CB}

        }
    }
    ,{name = devicesName_PE1, -- PE1 in the system
        portsArr =
        {
            {port = port_in_sip5_device(3) , pcid = 1003  , eport = global_test_data_env.ePort1003 ,
                stationsArr = {station_8 }
            },
            {port = port_in_sip5_device(2) , pcid = 1002  , eport = global_test_data_env.ePort1002 ,
                stationsArr = {station_3 }
            },
            {port = port_in_sip5_device(1) , pcid = 1001  , eport = global_test_data_env.ePort1001 ,
                stationsArr = {station_4,station_9 }        -- 2 station on the PCID
            },
            {port = port_in_sip5_device(4) , uplinkPort = true , connectedDevice = devicesName_PE2}
        }
    }
}

-- xcat3 device need to replace ports in systemInfoArr
local  systemInfoArr_xcat3_ports = {
     [devicesName_PE3] = {orig          = {port_in_sip5_device(1),  port_in_sip5_device(2)} ,
                          replacement   = {getTestPort(1),          getTestPort(3)}}
    ,[devicesName_PE2] = {orig          = {port_in_sip5_device(1),  port_in_sip5_device(2), port_in_sip5_device(3)}, 
                          replacement   = {getTestPort(1),          getTestPort(3),         getTestPort(2)}}
    ,[devicesName_PE1] = {orig          = {port_in_sip5_device(3),  port_in_sip5_device(2), port_in_sip5_device(1),port_in_sip5_device(4)}, 
                          replacement   = {getTestPort(2),          getTestPort(3),         getTestPort(1),        getTestPort(4)}}
}
-- indication that systemInfoArr_xcat3_ports replacement was done
local systemInfoArr_xcat3_ports_replacement_done = false

-- tested cases
local testedCases_test_1 = {
    doCb = true ,
    downstream = {devicesName_PE2}
}
local testedCases_test_2 = {
    doCb = true,
    downstream = {devicesName_PE2,devicesName_PE1},
    upstream   = {devicesName_PE2,devicesName_PE1}
}
local testedCases_test_3 = {
    doCb = true,
    upstream = {devicesName_PE3}
}
local testedCases_test_4 = {
    upstream = {devicesName_PE2},
    downstream = {devicesName_PE3}
}
local testedCases_test_5 = {
    downstream = {devicesName_PE2}
}
local testedCases_test_6 = {
    doCb = true
}
local testedCases_test_7 = {
    doCb = true
}

local testedCases_test_10 = {
    doCb = true,
    downstream = {devicesName_PE2,devicesName_PE1,devicesName_PE3}
}
local testedCases_test_11 = {
    doCb = true,
    downstream = {devicesName_PE2,devicesName_PE1,devicesName_PE3}
}
local testedCases_test_12 = {
    doCb = true,
    downstream = {devicesName_PE2,devicesName_PE1,devicesName_PE3}
}
-- registered multicast tests - with CB and all downstream devices
local testedCases_test_CB_and_all_downstream = {
    doCb = true,
    -- PE support
    downstream = {devicesName_PE2,devicesName_PE1,devicesName_PE3}
}

-- array of unicast test scenarios
-- those are entries with next info:
-- testName : the test name (short unique description)
-- srcStation - the src station that packet ingress the system from
-- trgStation - the trg station that packet egress the system to it
-- cases - table of tested cases
local ucSystemTestScenarios = {
     {testName = "test 1 : CB-->PE2,1004"      , cases = testedCases_test_1 ,
         srcStation = station_1, trgStation = station_2}
    ,{testName = "test 2 : PE1,1001-->pe2,1002" , cases = testedCases_test_2 ,
        srcStation = station_4, trgStation = station_3}
    ,{testName = "test 3 : PE3,1001-->pe1,1001 (same PCID different CB ports)" , cases = testedCases_test_3 ,
        srcStation = station_5, trgStation = station_4}
    ,{testName = "test 4 : PE2,1005-->PE3,1001 (1005 is on 'cascade port')" , cases = testedCases_test_4 ,
        srcStation = station_6, trgStation = station_5}
    ,{testName = "test 5 : PE2,1004-->PE2,1005 (1005 is on 'cascade port')" , cases = testedCases_test_5 ,
        srcStation = station_2, trgStation = station_6}
    ,{testName = "test 6 : PE1,1001-->PE1,1001 (terminated at CB)" , cases = testedCases_test_6 ,
        srcStation = station_4, trgStation = station_9}
    ,{testName = "test 7 : PE2,1004-->CB" , cases = testedCases_test_7 ,
        srcStation = station_2, trgStation = station_1}
}

-- function returns the 'UC/flood' tests scenarios that 'relevant' to the 'deviceName'
-- the info returned is table with each entry in next format:
--  1. testName     - the test name
--  2. srcStation   - src station
--  3. trgStation   - trg station
--  4. isUpstream   - is it need to be tested as 'traffic to upstream'
--                  not relevant for CB
--  5. isDownstream - is it need to be tested as 'traffic to downstream'
--                  not relevant for CB
--  6. multiTrgStations - list of 'registered multi destination' stations
--                  supporting 'registered multi cast groups'
local function getTestScenarios(deviceName,fullTestsTable)
    local testTable = {}
    local isCb
    local currentIndex = 1
    local doUpstream , doDownstream , doCb

    if deviceName == devicesName_CB then
        isCb = true
    else
        isCb = false
    end

    for index1 , testEntry in pairs(fullTestsTable) do
        --[[ testEntry.cases in format of :
            doCb = true,
            downstream = {devicesName_PE2,devicesName_PE1},
            upstream   = {devicesName_PE2,devicesName_PE1}
        --]]
        doCb = false
        doUpstream = false
        doDownstream = false

        if isCb == true then
            if testEntry.cases.doCb == true then
                doCb = true
            end
        else -- not CB

            if testEntry.cases.upstream then

                for index2 , upstream_deviceName in pairs(testEntry.cases.upstream) do
                    if upstream_deviceName == deviceName then
                        --_debug ("upstream_deviceName : "..upstream_deviceName)
                        doUpstream = true
                        break
                    end
                end
            end -- upstream

            if testEntry.cases.downstream then

                for index2 , downstream_deviceName in pairs(testEntry.cases.downstream) do
                    --_debug ("downstream_deviceName : "..downstream_deviceName)
                    if downstream_deviceName == deviceName then
                        doDownstream = true
                        break
                    end
                end
            end -- downstream
        end -- not CB

        -- add entry to the table of tests
        if doUpstream == true or doDownstream == true or doCb == true then

            if debug_specific_test_Name ~= nil and debug_specific_test_Name ~= testEntry.testName then
                -- ignore other tests when we debug only specific one
            else
                testTable[currentIndex] = {}
                testTable[currentIndex].testName   = testEntry.testName
                testTable[currentIndex].srcStation = testEntry.srcStation
                testTable[currentIndex].trgStation = testEntry.trgStation
                testTable[currentIndex].isUpstream = doUpstream
                testTable[currentIndex].isDownstream = doDownstream
                testTable[currentIndex].multiTrgStations = testEntry.multiTrgStations
                testTable[currentIndex].multicastCase = testEntry.multicastCase
                currentIndex = currentIndex + 1
            end


            --_debug("testName : "..testEntry.testName .. " relevant to " .. deviceName )
        end
    end -- loop on fullTestsTable

    return testTable
end

-- function returns the 'UC' tests scenarios that 'relevant' to the 'deviceName'
-- the info returned is table with each entry in next format:
--  1. testName     - the test name
--  2. srcStation   - src station
--  3. trgStation   - trg station
--  4. isUpstream   - is it need to be tested as 'traffic to upstream'
--                  not relevant for CB
--  5. isDownstream - is it need to be tested as 'traffic to downstream'
--                  not relevant for CB
--
function util_802_1_br_getUcTestScenarios(deviceName)
    return getTestScenarios(deviceName,ucSystemTestScenarios)
end

-- target ports for test 20 (registered multicast)
--// the eports are : 1001(station_4) , 1003 (station_8) , 1004 (station_2) , 1101 (station_5) , 54 (station_1)
local multiTrgStations_vlan5_mc_55_55 = {
             station_1 -- on cb
            ,station_2 -- on pe2
            --,station_3  -- on pe1
            ,station_4  -- on pe1 (station_9 on same port)
            ,station_5  -- on pe3
            --,station_6  -- on pe2 (on cascade) --> NOTE: cascade always get 'flood' also of 'registered MC'
            --,station_7  -- on cb
            ,station_8  -- on pe1
            }
            
            
-- target ports for test 30 (registered multicast)
--// the eports are :  1003 (station_8) , 1004 (station_2) , 1101 (station_5) , 54 (station_1)
local multiTrgStations_vlan6_mc_55_55 = {
             station_1 -- on cb
            ,station_2 -- on pe2
            --,station_3  -- on pe1
            --,station_4  -- on pe1 (station_9 on same port)
            ,station_5  -- on pe3
            --,station_6  -- on pe2 (on cascade) --> NOTE: cascade always get 'flood' also of 'registered MC'
            --,station_7  -- on cb
            ,station_8  -- on pe1
            }

-- target ports for test 40 (registered multicast)
--// the eports are :  1002 (station_3) , 1101 (station_5)
local multiTrgStations_vlan6_mc_66_66 = {
            --station_1 -- on cb
            --,station_2 -- on pe2
            station_3  -- on pe1
            --,station_4  -- on pe1 (station_9 on same port)
            ,station_5  -- on pe3
            --,station_6  -- on pe2 (on cascade) --> NOTE: cascade always get 'flood' also of 'registered MC'
            --,station_7  -- on cb
            --,station_8  -- on pe1
            }
            
            
-- array of flood test scenarios
-- those are entries with next info:
-- testName : the test name (short unique description)
-- srcStation - the src station that packet ingress the system from
local floodSystemTestScenarios = {
    {testName = "test 10 : From CB"      ,cases = testedCases_test_10 ,
        srcStation = station_1}
    ,{testName = "test 11 : From PE1,1001"      ,cases = testedCases_test_11 ,
        srcStation = station_4}
    ,{testName = "test 12 : From PE2,1005 (1005 is on 'cascade port')"    ,cases = testedCases_test_12 ,
        srcStation = station_6}
        
    ,{testName = "test 20 : registered multicast from CB (vlan 5 mc 55:55)"      ,cases = testedCases_test_CB_and_all_downstream ,
        srcStation = station_1 , trgStation = "multicast" ,
        -- target ports
        multiTrgStations = multiTrgStations_vlan5_mc_55_55,
        multicastCase = 1 -- vlan 5 : bind 01:02:03:55:55:55 to <GRP> = 3 , <ecid-base> = 0x550
    }

    ,{testName = "test 21 : registered multicast from PE1,1001"      ,cases = testedCases_test_CB_and_all_downstream ,
        srcStation = station_4 , trgStation = "multicast" ,
        -- target ports
        multiTrgStations = multiTrgStations_vlan5_mc_55_55,
        multicastCase = 1 -- vlan 5 : bind 01:02:03:55:55:55 to <GRP> = 3 , <ecid-base> = 0x550
    }
    
    ,{testName = "test 22 : registered multicast from PE2,1005 (1005 is on 'cascade port')"      ,cases = testedCases_test_CB_and_all_downstream ,
        srcStation = station_6 , trgStation = "multicast" ,
        -- target ports
        multiTrgStations = multiTrgStations_vlan5_mc_55_55,
        multicastCase = 1 -- vlan 5 : bind 01:02:03:55:55:55 to <GRP> = 3 , <ecid-base> = 0x550
    }
    
    ,{testName = "test 30 : registered multicast from CB (vlan 6 mc 55:55)"      ,cases = testedCases_test_CB_and_all_downstream ,
        srcStation = station_1 , trgStation = "multicast" ,
        -- target ports
        multiTrgStations = multiTrgStations_vlan6_mc_55_55,
        multicastCase = 2 -- vlan 6 : bind 01:02:03:55:55:55 to <GRP> = 2 , <ecid-base> = 0x660
    }

    ,{testName = "test 40 : registered multicast from CB (vlan 6 mc 66:66)"      ,cases = testedCases_test_CB_and_all_downstream ,
        srcStation = station_1 , trgStation = "multicast" ,
        -- target ports
        multiTrgStations = multiTrgStations_vlan6_mc_66_66,
        multicastCase = 3 -- vlan 6 : bind 01:02:03:66:66:66 to <GRP> = 3 , <ecid-base> = 0x661
    }
    
}
-- function returns the 'flood' tests scenarios that 'relevant' to the 'deviceName'
-- the info returned is table with each entry in next format:
--  1. testName     - the test name
--  2. srcStation   - src station
--  3. isUpstream   - is it need to be tested as 'traffic to upstream'
--                  not relevant for CB
--  4. isDownstream - is it need to be tested as 'traffic to downstream'
--                  not relevant for CB
--
function util_802_1_br_getFloodTestScenarios(deviceName)
    return getTestScenarios(deviceName,floodSystemTestScenarios)
end

-- get path inside the list of port of specific device to the control bridge ('the uplink')
local function internal__getLocalPortToControlBridge(deviceEntry)

    -- do the 'upstream' direction
    -- look for the 'ingressStation'
    for index2 , portEntry in pairs(deviceEntry.portsArr) do
        if portEntry.uplinkPort == true then
            return portEntry.port , portEntry.connectedDevice
        end
    end

    return -- nil,nil
end

-- return local port on the 'next device' that connected to the current device
-- note that the 2 device must be 'direct' connected
local function getLocalPortOnNextDevice(currentDeviceName,nextDeviceName)
    for index1 , deviceEntry in pairs(systemInfoArr) do
        if deviceEntry.name == nextDeviceName then
            -- first look for 'direct path'
            for index2 , portEntry in pairs(deviceEntry.portsArr) do
                if portEntry.connectedDevice == currentDeviceName then
                    return portEntry.port
                end
            end

            -- if 'direct path' not found look for 'path' via others
            for index2 , portEntry in pairs(deviceEntry.portsArr) do
                -- check if the device is on one of the 'connected device'
                portOnConnectedDevice = getLocalPortOnNextDevice(portEntry.connectedDevice,nextDeviceName)
                if portOnConnectedDevice ~= nil then
                    return portOnConnectedDevice
                end
            end
        end
    end

    return nil
end

-- get the upstream port on the 'peDeviceName'
function util_802_1_br_getUptreamPortOnPe(peDeviceName)
    for index1 , deviceEntry in pairs(systemInfoArr) do
        if deviceEntry.name == peDeviceName then
            -- first look for 'uplink'
            for index2 , portEntry in pairs(deviceEntry.portsArr) do
                if portEntry.uplinkPort == true then
                    return portEntry.port
                end
            end
        end
    end

    return nil
end

--[[ function returns the port number of the CB on which a device direct/indirect connected to the CB
INPUT :
      deviceName - the device name which we look on what port connected on CB

 function returns : port number of the CB
--]]
function util_802_1_br_getLocalPortNumOnControlBridge(deviceName)
    -- special handle of the CB device
    if deviceName == devicesName_CB then
        _debug_printError("ERROR : no meaning for local port from CB to CB ")
        return nil
    end

    -- look for 'deviceName'
    for index1 , deviceEntry in pairs(systemInfoArr) do
        if deviceName == deviceEntry.name then
            for index2 , portEntry in pairs(deviceEntry.portsArr) do
                if devicesName_CB == portEntry.connectedDevice then

                    -- we need port on the CB , so lookinto the CB DB to find this 'connected device'
                    for index3 , deviceEntry2 in pairs(systemInfoArr) do
                        if devicesName_CB == deviceEntry2.name then
                            for index4 , portEntry2 in pairs(deviceEntry2.portsArr) do
                                if portEntry2.connectedDevice == deviceName then
                                    -- found port on the CB
                                    return portEntry2.port
                                end
                            end
                        end
                    end

                    -- not found the device connected to CB ?!
                    return nil

                end -- if connectedDevice match the CB
            end -- portEntry

            -- not found the 'CB' as directly connected ... try on devices that connected to 'uplinkPort'
            for index2 , portEntry in pairs(deviceEntry.portsArr) do
                if true == portEntry.uplinkPort then
                    -- call the uplink device to 'help'
                    return util_802_1_br_getLocalPortNumOnControlBridge(portEntry.connectedDevice)
                end -- if connectedDevice match the CB
            end -- portEntry

        end -- if deviceName match
    end --deviceEntry

    return nil
end

--[[ function returns full path to control bridge (including the port on the CB)
INPUT :
      deviceName - the device name that on it the 'ingress port' to the system
      localPortOnDevice - the 'ingress port' to the system
 function returns : full path to control bridge (including the port on the CB)
      this is 'link list' :
      deviceName - the device
      ingressPort - the ingress port
      targetPort  -- when nil --> termination of link list
      nextDeviceInfo --> info to the next device -- when nil --> termination of link list
--]]
local function getFullPathToControlBridge(deviceName,localPortOnDevice)
    local currentDeviceInfo
    local found = false
    local targetDeviceName, nextDeviceLocalPort

    if deviceName == nil or localPortOnDevice == nil then
        -- error
        return nil
    end

    -- special handle of the CB device
    if deviceName == devicesName_CB then
        currentDeviceInfo = {}
        currentDeviceInfo.deviceName = deviceName
        currentDeviceInfo.ingressPort = localPortOnDevice
        -- no targetPort , no nextDeviceInfo !
        return currentDeviceInfo
    end

    -- do the 'upstream' direction
    -- look for 'device,port'
    for index1 , deviceEntry in pairs(systemInfoArr) do
        if deviceName == deviceEntry.name then
            for index2 , portEntry in pairs(deviceEntry.portsArr) do
                if localPortOnDevice == portEntry.port then
                    currentDeviceInfo = {}
                    currentDeviceInfo.deviceName = deviceEntry.name
                    currentDeviceInfo.ingressPort = portEntry.port

                    -- find the uplink port (the direction to the next device .. towards the CB)
                    currentDeviceInfo.targetPort , targetDeviceName = internal__getLocalPortToControlBridge(deviceEntry)

                    -- get the port of the ingress port on next device
                    nextDeviceLocalPort =
                        getLocalPortOnNextDevice(currentDeviceInfo.deviceName,
                                    targetDeviceName)

                    -- the next device is not the CB need to continue the 'path'
                    currentDeviceInfo.nextDeviceInfo = getFullPathToControlBridge(targetDeviceName,nextDeviceLocalPort)

                    return currentDeviceInfo
                end -- if localPortOnDevice match
            end -- portEntry
        end -- if deviceName match
    end --deviceEntry

    return currentDeviceInfo
end
-- mac address FF:FF:FF:FF:FF:FF
local broadcast_macAddr = "ff".."ff".."ff".."ff".."ff".."ff"
-- mac address 01:02:03:55:55:55
local multicast_macAddr = "01".."02".."03".."55".."55".."55"
-- set the MC address to be used
function util_802_1_br_setRegisteredMcAddr(mcAddr)
    multicast_macAddr = mcAddr
end

-- get station info
-- if 'myStationName' is nil --> considered as 'dummy station for broadcast address'
-- the return info hold next:
--      deviceName - the device name
--      port - the port number
--      pcid - the PCID number
--      eport - the eport number
--      macAddr - the mac address of the station (when 'myStationName' is nil --> "FFFFFFFFFFFF")

function util_802_1_br_getStationInfo(myStationName)
    local stationInfo = nil

    if myStationName == nil or 
       myStationName == "multicast" then
        stationInfo = {}
        stationInfo.deviceName = "unknown"
        stationInfo.port = 0
        stationInfo.pcid = 0
        stationInfo.eport = 0
        if myStationName == "multicast" then
            stationInfo.macAddr = multicast_macAddr
            _debug_to_string("util_802_1_br_getStationInfo" , multicast_macAddr)
        else
            stationInfo.macAddr = broadcast_macAddr
        end
        
        --_debug_to_string("util_802_1_br_getStationInfo" , stationInfo.macAddr)

        return stationInfo
    end

    for index1 , deviceEntry in pairs(systemInfoArr) do
        --_debug(deviceEntry.name)
        for index2 , portEntry in pairs(deviceEntry.portsArr) do
            --_debug("port" .. portEntry.port)
            if portEntry.stationsArr ~= nil then
                for index3 , station in pairs(portEntry.stationsArr) do
                    if myStationName == station then
                        -- this is my station .. gather the info
                        stationInfo = {}
                        stationInfo.deviceName = deviceEntry.name
                        stationInfo.port = portEntry.port
                        stationInfo.pcid = portEntry.pcid
                        stationInfo.eport = portEntry.eport

                        for index4 , entry in pairs(stationInfoArr) do
                            if entry.name == myStationName then
                                stationInfo.macAddr = entry.macAddr
                            end
                        end

                        return stationInfo
                    end -- myStationName == station
                end
            end --  if portEntry.stationsArr ~= nil
        end
    end

    _debug_printError("ERROR : unknown remote device : " .. myStationName)

end


-- get port info
-- the return info hold next:
--      pcid - the PCID number
--      eport - the eport number
--      isCascade - (true/false) is it cascade port (on PE --> downstream port , on CB - connection to PE)
--      isUptreamPort - (true/false) is it upstream port (on PE --> upstream port , always 'false' for CB)
--
-- NOTE: returns nil if port not found in DB
--
 function util_802_1_br_getPortInfo(deviceName,localPort)
    local portInfo = nil
    for index1 , deviceEntry in pairs(systemInfoArr) do

        if deviceEntry.name == deviceName then

            for index2 , portEntry in pairs(deviceEntry.portsArr) do

                if portEntry.port == localPort then
                    -- this is my port .. gather the info
                    portInfo = {}
                    portInfo.pcid = portEntry.pcid
                    portInfo.eport = portEntry.eport
                    if portEntry.cascadePort ~= true then
                        portInfo.isCascade = false
                    else
                        portInfo.isCascade = true
                    end

                    if portEntry.uplinkPort ~= true then
                        portInfo.isUptreamPort = false
                    else
                        portInfo.isUptreamPort = true
                    end

                    if deviceEntry.name == devicesName_CB then
                        -- check implicit cascade ports in the CB (connected to PEs)
                        if portEntry.connectedDevice ~= nil then
                            -- on the CB those are 'cascade ports'
                            portInfo.isUptreamPort = false
                            portInfo.isCascade = true
                        end
                    end

                    if portEntry.isUptreamPort == true then
                        -- can't be both 'cascade and upstream'
                        portInfo.isCascade = false
                    end

                    return portInfo

                end -- if portEntry.port == localPort
            end -- loop on portEntry
        end --  if deviceEntry.name == deviceName
    end -- loop on deviceEntry


    _debug("NOTE : not found 'port info' on device : " .. to_string(deviceName).. " for port number " .. to_string(localPort))
    return nil

end


-- function get 'upstream path' : 'path' from the 'target' to the 'CB'
-- function returns : 'downstream path' --> meaning converted it as from the 'CB' to the 'target'
local function correctInfoFromUpstreamToDownstream(upstreamPath)
    local currentElement , nextElement , prevElement
--[[
    the upstreamPath of type :
      deviceName - the device
      ingressPort - the ingress port
      targetPort  -- when nil --> termination of link list
      nextDeviceInfo --> info to the next device -- when nil --> termination of link list
 --]]

    currentElement = {}
    -- take info from first element
    currentElement.deviceName  = upstreamPath.deviceName
    currentElement.ingressPort = upstreamPath.targetPort -- swap ingress and target
    currentElement.targetPort  = upstreamPath.ingressPort -- swap ingress and target
    -- point to second element
    nextElement = upstreamPath.nextDeviceInfo
    -- point to 'zero' element
    prevElement = nil

    while true do
        -- point from current to previous
        currentElement.nextDeviceInfo = prevElement

        if nextElement == nil then
            break
        end

        -- take info from current to 'previous'
        prevElement = currentElement
        prevElement.nextDeviceInfo = nil

        -- take info from next element into current
        currentElement.deviceName  = nextElement.deviceName
        currentElement.ingressPort = nextElement.targetPort -- swap ingress and target
        currentElement.targetPort  = nextElement.ingressPort -- swap ingress and target
        -- make the 'next element' to point 'current element'
        nextElement.nextDeviceInfo = currentElement
        -- jump to next element
        nextElement = upstreamPath.nextDeviceInfo
    end

    -- at this stage the 'current element' should be the 'downstream path'

    return currentElement
end

-- function print 'path'
local function debug_printPath(path,stationName)
    local currentElement

    currentElement = path

    if stationName == nil then
        stationName = "unknown station"
    end


    _debug("Start print of 'path' for: " .. stationName)

    while currentElement ~= nil do

        if currentElement.ingressPort == nil then currentElement.ingressPort = "--" end
        if currentElement.targetPort == nil  then currentElement.targetPort = "--" end

        _debug("\t On device : "   .. currentElement.deviceName ..
                 " ingressPort : " .. currentElement.ingressPort ..
                 " targetPort : "  .. currentElement.targetPort)

        -- jump to next element
        currentElement = currentElement.nextDeviceInfo
    end

    _debug("End print of 'path'")

end


--[[ for known UC packet ingress the system targeted to egress port , what will be it's ingress port and egress port
    on the specific device
    function return : info with next format:
    returnInfo:
        upstreamPath is 'link list'
                deviceName - the device
                ingressPort - the ingress port
                targetPort  -- when nil --> termination of link list
                nextDeviceInfo --> info to the next device -- when nil --> termination of link list

        downstreamPath - same format as upstreamPath

        ingressStationInfo - info about the ingress station :
                deviceName
                port
                pcid
                eport

        egressStationInfo - info about the egress station (like ingressStationInfo)

--]]
 function util_802_1_br_forUcGetIngressPortAndEgressPortOnDev(ingressStationName , egressStationName)
    local returnInfo = {}

    -- loop on the systemInfoArr to find how packet will switch in the device(s) getting into needed device.
    local found = false

    -- get full path to control bridge (from source)
    -- get device,port for the ingress station
    local ingressStationInfo = util_802_1_br_getStationInfo(ingressStationName)
    local upstreamPath       = getFullPathToControlBridge(ingressStationInfo.deviceName,ingressStationInfo.port)
    debug_printPath(upstreamPath,ingressStationName)

    if egressStationName then
        -- get full path to control bridge (from target)
        -- get device,port for the egress station
        local egressStationInfo = util_802_1_br_getStationInfo(egressStationName)
        local downstreamPath    = getFullPathToControlBridge(egressStationInfo.deviceName,egressStationInfo.port)
        debug_printPath(downstreamPath,egressStationName)

        -- we got 'path' from the 'target' to the 'CB'
        -- but need to convert it as from the 'CB' to the 'target'
        local updatedDownstreamPath    = correctInfoFromUpstreamToDownstream(downstreamPath)
        downstreamPath = nil -- not used any more
        debug_printPath(updatedDownstreamPath,egressStationName)
    end

    -- build full info to return
    returnInfo.ingressStationInfo = ingressStationInfo
    returnInfo.egressStationInfo = egressStationInfo

    -- uptream info
    returnInfo.upstreamPath = upstreamPath
    -- downstream info
    returnInfo.downstreamPath = updatedDownstreamPath

    return returnInfo
 end

-- get the 'upstream port' number on the PE
local function getUpstreamPortOnPe(myDeviceName)
    for index1 , deviceEntry in pairs(systemInfoArr) do
        if deviceEntry.name == myDeviceName then
            for index2 , portEntry in pairs(deviceEntry.portsArr) do
                if portEntry.uplinkPort == true then
                    return portEntry.port
                end
            end
        end -- deviceEntry.name == myDeviceName
    end

    return nil -- not found --> error
end

-- get 'local port' number on 'my device' that 'connect my device' to 'remote station'
-- function returns: the 'local' portNum
-- to get the 'info' caller should also call util_802_1_br_getPortInfo(myDeviceName,localPort)
function util_802_1_br_getLocalPortNum(myDeviceName,remoteStationName)
    -- loop on the systemInfoArr to find how packet will switch in the device(s) getting into needed device.
    local found = false
    -- get full path to control bridge (from source)
    -- get device,port for the ingress station
    local ingressStationInfo = util_802_1_br_getStationInfo(remoteStationName)
    local upstreamPath       = getFullPathToControlBridge(ingressStationInfo.deviceName,ingressStationInfo.port)
    debug_printPath(upstreamPath,remoteStationName)

    local entry = upstreamPath
    while entry do

        if entry.deviceName == myDeviceName then
            found = true
            break
        end

        entry = entry.nextDeviceInfo
    end

    -- this should supports 'upstream' and 'on CB'
    if found == true then
        return entry.ingressPort
    end

    -- not found 'my device' in the list of 'from station to CB' ...
    -- so assume that we need the 'upstream port' of the current PE
    local upstreamPortNum = getUpstreamPortOnPe(myDeviceName)
    return upstreamPortNum
end


local etagEthertype = "893f"
--function build ETag as string and return it
--input parameters define the ETag fields
--ingress parameters are all 'numerics' or 'nil' (when 'nil' considered as 0)
--the ETag is 8 bytes (include TPID = 0x893f -- according to IEEE802.1BR)
 function util_802_1_br_buildETag(e_pcp,e_dei,ingress_e_cid_base,grp,e_cid_base)
    if e_pcp == nil then e_pcp = 0 end
    if e_dei == nil then e_dei = 0 end
    if ingress_e_cid_base == nil then ingress_e_cid_base = 0 end
    if grp == nil then grp = 0 end
    if e_cid_base == nil then e_cid_base = 0 end

    -- WA to make generic test between BC2 and Bobk
    if grp > 0 then
        grp = bpe_grp(grp)
    end

    local value = bit_shl(e_pcp,13) + bit_shl(e_dei,12) + ingress_e_cid_base

    --bytes 0,1 are the TPID --> etagEthertype
    --bytes 2,3 are : E-PCP , E-DEI , ingress_e_cid_base
    --bytes 4,5 are : Re-,GRP, e_cid_base
    --bytes 6,7 are : 0

    local bytes_0_1 = etagEthertype

    value = bit_shl(e_pcp,13) + bit_shl(e_dei,12) + ingress_e_cid_base
    local bytes_2_3 = string.format("%4.4x",value)

    value = bit_shl(grp,12) + e_cid_base
    local bytes_4_5 = string.format("%4.4x",value)

    local bytes_6_7 = "00".."00"

    local etagString = bytes_0_1 .. bytes_2_3 .. bytes_4_5 .. bytes_6_7

    --_debug("Built ETag = " .. packetAsStringToPrintableFormat(etagString))

    return etagString
end
-- macSa end at offset is 12 bytes --> times 2 for the 'chars'
local afterMacSaOffsetInChars = 12 * 2
-- ETag length is 8 bytes --> times 2 for the 'chars'
local ETagLengthInChars = 8 * 2
-- insert Tag into the packet after mac SA
-- function return new packet
 function util_802_1_br_insertTagToPacketAfterMacs(packet , ETag)

    -- start of the packet
    local part1 = string.sub(packet , 1 , afterMacSaOffsetInChars)
    -- rest of the packet
    local part2 = string.sub(packet , afterMacSaOffsetInChars + 1)

    --_debug("Added Tag = " .. packetAsStringToPrintableFormat(ETag))

    -- insert the Tag in proper place
    return part1 .. ETag .. part2
end

-- remove Tag with x bytes from the packet after mac SA
-- function return :{
--      newPacket - the packet after the removal of the tag
--      removedTag - the removed tag
 function util_802_1_br_removeTagFromPacketAfterMacs(packet,tagSizeInBytes)
    local part1 = string.sub(packet , 1 , afterMacSaOffsetInChars)
    -- the etag 8 bytes
    local Tag  = string.sub(packet , afterMacSaOffsetInChars + 1, (afterMacSaOffsetInChars + 1) + (tagSizeInBytes - 1) )
    -- rest of the packet
    local part2 = string.sub(packet , afterMacSaOffsetInChars + tagSizeInBytes + 1)

    --_debug("Removed Tag = " .. packetAsStringToPrintableFormat(Tag))

    -- return the new packet and the ETag
    return part1..part2 , Tag
end

-- function to state that the bpe run on xcat3 devices
function util_802_1_br_system_xcat3_device()
    if systemInfoArr_xcat3_ports_replacement_done == true then
        -- the replacement was already done
        return
    end

    for deviceName , xcat3Info in pairs(systemInfoArr_xcat3_ports) do
        for index1 , deviceEntry in pairs(systemInfoArr) do
            if deviceEntry.name == deviceName then
                -- replace ports
                for index2 , systemPortEntry in pairs(deviceEntry.portsArr) do
                    for ii = 1,#xcat3Info.orig do
                        if systemPortEntry.port == xcat3Info.orig[ii] then
                            -- replace orig port with new port
                            systemPortEntry.port = xcat3Info.replacement[ii]
                            break
                        end
                    end
                end
            end
        end
    end

    -- indicate that the replacement was already done
    systemInfoArr_xcat3_ports_replacement_done = true
end

-- allow the test to set it !
--[[extern]] util_vTag_6bytes_tpid = 0x893f

util_vTag_6bytes_direction_upstream   = 0
util_vTag_6bytes_direction_downstream = 1
--function build 6 bytes V-Tag as string and return it
--input parameters define the V-Tag fields
--ingress parameters are all 'numerics' or 'nil' (when 'nil' considered as 0)
--the V-Tag is 6 bytes (include TPID)
-- direction - util_vTag_6bytes_direction_upstream/util_vTag_6bytes_direction_downstream 
-- isMulticast  - (bool) is trgInterface multicast ? 
-- trgInterface - the trg-EPort or the trg-vidx 
-- srcInterface - the src interface . for 'learning' and for 'src-mc-filter'
-- isMcFilter - (bool) is multicast filter needed 
function util_vTag_6bytes_buildTag(direction,isMulticast,trgInterface,srcInterface,isMcFilter)

    -- convert bool to integer
    if isMulticast == true then 
        isMulticast = 1 
    else
        isMulticast = 0
    end

    -- convert bool to integer
    if isMcFilter == true then 
        isMcFilter = 1 
    else
        isMcFilter = 0
    end
    
    if isMulticast == 1 and isMcFilter == 0 then
        -- reserved and set to 0 by the device
        srcInterface = 0
    end
    
    if srcInterface == nil then
        srcInterface = 0
    end
    if trgInterface == nil then
        trgInterface = 0
    end
    if direction == nil then
        direction = 0
    end
    
    printLog("direction",to_string(direction))
    printLog("isMulticast",to_string(isMulticast))
    printLog("trgInterface",to_string(trgInterface))
    printLog("srcInterface",to_string(srcInterface))
    printLog("isMcFilter",to_string(isMcFilter))


    local bytes_0_1 = string.format("%4.4x",util_vTag_6bytes_tpid)
    
    local value = bit_shl(direction,15) + bit_shl(isMulticast,14) + trgInterface
    local bytes_2_3 = string.format("%4.4x",value)

    local value = bit_shl(isMcFilter,15) + srcInterface
    local bytes_4_5 = string.format("%4.4x",value)

    local tagString = bytes_0_1 .. bytes_2_3 .. bytes_4_5

    printLog("Built 6-bytes-V-Tag = " .. packetAsStringToPrintableFormat(tagString))

    return tagString
end

-- return indication if eArch device not support registered MC on PE :
-- true  -> not implemented
-- false -> implemented
function bpe_802_1_br_is_eArch_Device_not_support_registered_mc_on_PE()
    -- we started implementation of registered MC for BC2 as PE !
    return false -- false meaning 'implemented'
end
