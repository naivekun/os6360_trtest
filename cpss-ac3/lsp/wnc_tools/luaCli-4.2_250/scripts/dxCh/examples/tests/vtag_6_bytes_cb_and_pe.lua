--[[

    test the BPE system

--]]

local devNum  = devEnv.dev 
-- this test is relevant for BobK only. sip_5_15 only (without sip_5_16)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5_15",true--[[restrictions]],"SIP_5_16") 
SUPPORTED_FEATURE_DECLARE(devNum, "BPE_802_1_BR_CONTROL_BRIDGE" , "BPE_802_1_BR_PORT_EXTENDER")
cmdLuaCLI_registerCfunction("prvLuaTgfIngressTaggingForCapturedPortsSet")

-- get extra packet's util functions
-- use dofile instead of require .. to allow quick load of test 
dofile("dxCh/examples/common/bpe_802_1br_utils.lua")

-- when nil ignored , otherwise test only specific device
local debug_specific_device =  nil--devicesName_PE2
-- when nil ignored , otherwise test skip UC tests
local debug_Skip_Uc_tests = false
-- when nil ignored , otherwise test skip Flood tests
--local debug_Skip_Flood_tests = true

local totalError = false
local isError = false
local purpose = nil -- purpose of the 'sending traffic'
local testedDevices = {
    devicesName_CB
    ,devicesName_PE1
    ,devicesName_PE2
    ,devicesName_PE3
}

local currentTestedDevice = nil -- current tested device (one of testedDevices)
local isCbCurrent = false -- indication if current device is the 'CB'
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

-- 'extern' function _debug_to_string(...)


local portsInTest = nil

local function load_portsInTest()
    portsInTest = {getTestPort(1),getTestPort(2),getTestPort(3),getTestPort(4)}
    
 --   print("portsInTest",to_string(portsInTest))
end

-- table that hold all the tests that failed
local summaryOfTests = {
    -- entry format is :
     [devicesName_CB]  = {device = devicesName_CB  , index = 1 ,tests = {},pass = {}}
    ,[devicesName_PE1] = {device = devicesName_PE1 , index = 1 ,tests = {},pass = {}}
    ,[devicesName_PE2] = {device = devicesName_PE2 , index = 1 ,tests = {},pass = {}}
    ,[devicesName_PE3] = {device = devicesName_PE3 , index = 1 ,tests = {},pass = {}}
}

-- state that the 'from cpu' should go to EPort that will not add ETag .
local eportWithoutAddingETag = 255

--
-- define parts of packet to be used for 'send from port' and 'expected to receive on port(s)'
-- the parts are :
-- macDa/macSa - mac addresses that are associated with 'stations' (like PC)
-- outer vlan tag - the outer vlan tag in the packet (may be ommited)
-- inner vlan tag - the inner vlan tag in the packet (may be ommited)
-- ETag - the 8 bytes 'ETag' that hold information about source/target according to it's directions (upstream/downstream)
--        NOTE: when the ETag exists on the packet it will be the most outer
-- payload
--
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"

local vlanWithTag5 = "8100".."0005"
local vlanWithTag6 = "8100".."0006"
local vlanWithTag7 = "8100".."0007"

local tag0 = vlanWithTag5
local tag1 = vlanWithTag6
local tag2 = vlanWithTag7

-- index for vlan5 (outer tag) in vlanInfo[]
local vlan5Index = 2
-- index for vlan6 (outer tag) in vlanInfo[]
local vlan6Index = 3
-- current tested vlan index (index for vlan 5 or vlan 6)
local currentVlanIndex = vlan5Index
-- info needed per vlan
local vlanInfo = {
     { vlanId = 1 , GRP = 3 , ecid = 0xf01 }
    ,{ vlanId = 5 , GRP = 1 , ecid = 0xf05 }
    ,{ vlanId = 6 , GRP = 2 , ecid = 0xf06 }
}

-- used as index into registeredMcMacInfo[]
-- get value from 'multicastCase' in 'floodSystemTestScenarios'
local testedRegisteredMcMacIndex = 1

--vlan 5 mac 01:02:03:55:55:55 --> grp 3 e-cid-base 0x550
local registeredMcMacInfo = {
--// in vlan 5 : bind 01:02:03:55:55:55 to <GRP> = 3 , <ecid-base> = 0x550
    { vlanId = vlanInfo[vlan5Index].vlanId , GRP = 3 , ecid = 0x550 , mcAddr = "01".."02".."03".."55".."55".."55"},
--// in vlan 6 : bind 01:02:03:55:55:55 to <GRP> = 2 , <ecid-base> = 0x660
    { vlanId = vlanInfo[vlan6Index].vlanId , GRP = 2 , ecid = 0x660 , mcAddr = "01".."02".."03".."55".."55".."55"},
--// in vlan 6 : bind 01:02:03:66:66:66 to <GRP> = 3 , <ecid-base> = 0x661    
    { vlanId = vlanInfo[vlan6Index].vlanId , GRP = 3 , ecid = 0x661 , mcAddr = "01".."02".."03".."66".."66".."66"}
}

-- build packet (string) from parts of packet (strings)
-- parts can be omitted (nil)
local function buildPacket( ...)
    part1,part2,part3,part4,part5,part6,part7,part8,part9,part10 = ...
    if part1 == nil then part1 = "" end
    if part2 == nil then part2 = "" end
    if part3 == nil then part3 = "" end
    if part4 == nil then part4 = "" end
    if part5 == nil then part5 = "" end
    if part6 == nil then part6 = "" end
    if part7 == nil then part7 = "" end
    if part8 == nil then part8 = "" end
    if part9 == nil then part9 = "" end
    if part10 == nil then part10 = "" end

    return part1 .. part2 .. part3 .. part4 .. part5 .. part6 .. part7 .. part8 .. part9 .. part10
end

-- build packet with macDa,macSa,tag0,tag1,payload
local function buildPacketVid0Vid1(srcStation,trgStation)
    --[[
        stationInfo.deviceName
        stationInfo.port
        stationInfo.pcid
        stationInfo.eport
        stationInfo.macAddr
    --]]
    local srcInfo = util_802_1_br_getStationInfo(srcStation)
    local trgInfo = util_802_1_br_getStationInfo(trgStation)

    local packet = buildPacket(trgInfo.macAddr,srcInfo.macAddr,tag0,tag1,packetPayload)

    return packet
end

-- send traffic that will cause 'mac SA learning' in the CB device
local function doMacSaLearningOnCb(stationName)
    local ETag
    local localPort     = util_802_1_br_getLocalPortNum(currentTestedDevice,stationName)
    local localPortInfo = util_802_1_br_getPortInfo(currentTestedDevice,localPort)
    local stationInfo   = util_802_1_br_getStationInfo(stationName)
    local srcEPort

    local packetVid0Vid1_learning     = buildPacketVid0Vid1(stationName,nil)

    if localPortInfo.isCascade == true then
        ETag = util_vTag_6bytes_buildTag(util_vTag_6bytes_direction_upstream , --direction
                    false,--isMulticast
                    0,--trgInterface,
                    stationInfo.pcid,--srcInterface,
                    false)--isMcFilter
        --ETag = util_802_1_br_buildETag(0,0,0,0,stationInfo.pcid)
        packetVid0Vid1_learning = util_802_1_br_insertTagToPacketAfterMacs(packetVid0Vid1_learning,ETag)

        -- state the the 'from cpu' should go to EPort that will not add ETag .
        -- not that the physical port is still the port that we state in 'transmitInfo.portNum'
        luaTgfStateTrgEPortInsteadPhyPort(eportWithoutAddingETag)

    end

    local transmitInfo = {portNum = localPort , pktInfo = {fullPacket = packetVid0Vid1_learning} }

    if stationInfo.eport == nil then
        srcEPort = localPort
    else
        srcEPort = stationInfo.eport
    end

    purpose = "mac learning of " .. stationInfo.macAddr .. " from local port :" .. transmitInfo.portNum .. " , associated with eport : " .. srcEPort
    _debug("Start " .. purpose)

    -- send packet from the remote port for 'mac learning'
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)
    if rc == 0 then
        _debug ("Passed : " .. purpose .. "\n")
    else
        _debug_printError ("Failed : " .. purpose .. "\n")
        isError = true
    end

    -- restore configuration
    luaTgfStateTrgEPortInsteadPhyPort(nil)
end

-- send traffic that expected to be 'known uc' ... from srcStation to trgStation
-- onPeToUpstreamDirection - parameter relevant on 'PE' to know if this is upstream or downstream direction
-- function check that traffic egress properlly
local function sendUcFromSrcToTrg(srcStation,trgStation,onPeToUpstreamDirection)
    local ETag
    local e_pcp,e_dei,ingress_e_cid_base,grp,e_cid_base
    --src
    local srcStationInfo    = util_802_1_br_getStationInfo(srcStation)
    local src_localPort

    if isCbCurrent == true or onPeToUpstreamDirection == true then
        src_localPort = util_802_1_br_getLocalPortNum(currentTestedDevice,srcStation)
    else
        -- the src port MUST be the from the 'upstream port' regardless if the 'srcStation' is 'local on current device' or on downstram device !!!
        src_localPort = util_802_1_br_getUptreamPortOnPe(currentTestedDevice)
    end

    local src_localPortInfo = util_802_1_br_getPortInfo(currentTestedDevice,src_localPort)
    --trg
    local trgStationInfo    = util_802_1_br_getStationInfo(trgStation)
    local trg_localPort
    if isCbCurrent == true or onPeToUpstreamDirection == false then
        trg_localPort = util_802_1_br_getLocalPortNum(currentTestedDevice,trgStation)
    else
        -- the trg port MUST be the 'upstream port' regardless if the 'trgStation' is 'local on current device' or on downstram device !!!
        trg_localPort = util_802_1_br_getUptreamPortOnPe(currentTestedDevice)
    end
    local trg_localPortInfo = util_802_1_br_getPortInfo(currentTestedDevice,trg_localPort)

    local packetVid0Vid1     = buildPacketVid0Vid1(srcStation,trgStation)

    local ingressPacket = packetVid0Vid1
    local egressPacket  = packetVid0Vid1

    ingress_e_cid_base = 0
    grp = 0
    up_onETag = 6
    
    local direction , trgInterface , srcInterface 

    if src_localPortInfo.isCascade == true or
       src_localPortInfo.isUptreamPort == true then
       
        if isCbCurrent == true or onPeToUpstreamDirection == true then
            -- e-cid indicate the 'src port'
            e_cid_base = srcStationInfo.pcid
            direction = util_vTag_6bytes_direction_upstream
            trgInterface = 0
            srcInterface = srcStationInfo.pcid
        else
            -- e-cid indicate the 'trg port'
            e_cid_base = trgStationInfo.pcid
            direction = util_vTag_6bytes_direction_downstream
            trgInterface = trgStationInfo.pcid
            srcInterface = srcStationInfo.pcid
        end

        ETag = util_vTag_6bytes_buildTag(direction , --direction
                    false,--isMulticast
                    trgInterface,--trgInterface,
                    srcInterface,--srcInterface,
                    false)--isMcFilter

--        ETag = util_802_1_br_buildETag(up_onETag,0,ingress_e_cid_base,grp,e_cid_base)
        ingressPacket = util_802_1_br_insertTagToPacketAfterMacs(ingressPacket,ETag)

        -- state that the 'from cpu' should go to EPort that will not add ETag .
        -- note that the physical port is still the port that we state in 'transmitInfo.portNum'
        luaTgfStateTrgEPortInsteadPhyPort(eportWithoutAddingETag)
    end

    local keepIngressETag -- indication that ingress ETag continue 'as is' to egress ETag
    if isCbCurrent == false                     and
       onPeToUpstreamDirection == true          and
       src_localPortInfo.isCascade == true      and
       trg_localPortInfo.isUptreamPort == true
    then
        keepIngressETag = true
    else
        keepIngressETag = false
    end

    if trg_localPortInfo.isCascade == true  or
       trg_localPortInfo.isUptreamPort == true then

        if isCbCurrent == true or onPeToUpstreamDirection == false then
            -- e-cid indicate the 'trg port'
            e_cid_base = trgStationInfo.pcid
            
            direction = util_vTag_6bytes_direction_downstream
            trgInterface = trgStationInfo.pcid
            srcInterface = srcStationInfo.pcid
            
        else
            -- e-cid indicate the 'src port'
            e_cid_base = srcStationInfo.pcid
            
            direction = util_vTag_6bytes_direction_upstream
            trgInterface = 0
            srcInterface = srcStationInfo.pcid
            
        end

        if keepIngressETag == false then 
            if bpe_802_1_br_is_eArch_Device() then
                up_onETag = 0 -- the 'rebuilt ETag' not have 'up' same as 'ingress' ... need extra QOS classifications for such support
            else
                if not src_localPortInfo.isCascade and not src_localPortInfo.isUptreamPort then
                    up_onETag = 0 -- the ingress packet have 'up1 = 0'
                else
                    -- keep the UP from the ingress E-Tag
                end
            end
        end

        if trg_localPortInfo.isCascade and 
            isCbCurrent == false and -- not bpe_802_1_br_is_eArch_Device() and
            e_cid_base == trg_localPortInfo.eport
        then
            -- no etag expected for packets egress the PCID of the cascade port
        else
            ETag = util_vTag_6bytes_buildTag(direction , --direction
                        false,--isMulticast
                        trgInterface,--trgInterface,
                        srcInterface,--srcInterface,
                        false)--isMcFilter
--            ETag = util_802_1_br_buildETag(up_onETag,0,ingress_e_cid_base,grp,e_cid_base)
            egressPacket = util_802_1_br_insertTagToPacketAfterMacs(egressPacket,ETag)
        end
    end

    local transmitInfo = {portNum = src_localPort , pktInfo = {fullPacket = ingressPacket} }

    local egressInfoTable = {
         -- this port need to get traffic
         {portNum = trg_localPort , pktInfo = {fullPacket = egressPacket} }
         -- other port should not
    }

    local ii = 2 -- index 1 used by 'trg_localPort'
    -- add entries into egressInfoTable
    for index,value in pairs(portsInTest) do

        if value ~= trg_localPort and value ~= src_localPort then
            egressInfoTable[ii] = {}
            egressInfoTable[ii].portNum = value
            egressInfoTable[ii].packetCount = 0
            ii = ii + 1
        end
    end

    if srcStationInfo.eport == nil then
        srcEPort = src_localPort
    else
        srcEPort = srcStationInfo.eport
    end
    if trgStationInfo.eport == nil then
        trgEPort = trg_localPort
    else
        trgEPort = trgStationInfo.eport
    end

    purpose = "Send known UC from " .. srcStationInfo.macAddr .. " to  " .. trgStationInfo.macAddr ..
              " from eport " .. srcEPort .. " to eport " .. trgEPort
    _debug("Start " .. purpose)

    --************* our system for testings (loopback port(s)) limited to cases where ingress packet got to egress port ,
    --************* which is the same as ingress port
    if trg_localPort == src_localPort then
        if srcStationInfo.pcid ~= trgStationInfo.pcid then
            _debug("NOTE: Test can't check 'captured' egress packet , because on 'local device' ingress port is also the egress port " .. src_localPort)
            _debug("\t Test will check that not 'flooding' to any other port ")
            -- remove the need to 'capture' the egress port (as it is also the ingress port)
            egressInfoTable[1].pktInfo = nil
            -- state that storming should happen
            transmitInfo.loopbackStorming = true
        else
            -- the packet should be filtered by the CB ... as this is the same eport !!!
            egressInfoTable[1] = nil
        end
    end

    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        _debug ("Passed : " .. purpose .. "\n")
    else
        _debug_printError ("Failed : " .. purpose .. "\n")
        isError = true
    end

    -- restore configuration
    luaTgfStateTrgEPortInsteadPhyPort(nil)
end

-- send traffic that expected to be 'flooding' ... from srcStation to trgStation
-- onPeToUpstreamDirection - parameter relevant on 'PE' to know if this is upstream or downstream direction
-- function check that traffic egress properly
local function sendFloodFromSrcToTrg(srcStation,trgStation,onPeToUpstreamDirection,multiTrgStations,multicastCase)
    local ETag
    local e_pcp,e_dei,ingress_e_cid_base,grp,e_cid_base
    --src
    local srcStationInfo    = util_802_1_br_getStationInfo(srcStation)
    local src_localPort
    -- if isRegisteredMulticast == true --> meaning that 'flood' in vidx domain (subset of vlan)
    local isRegisteredMulticast = false

    --_debug_to_string("sendFloodFromSrcToTrg",srcStation,trgStation,onPeToUpstreamDirection,multiTrgStations)
    
    if multiTrgStations ~= nil then
        isRegisteredMulticast = true
        testedRegisteredMcMacIndex = multicastCase
    end

    if isCbCurrent == true or onPeToUpstreamDirection == true then
        src_localPort = util_802_1_br_getLocalPortNum(currentTestedDevice,srcStation)
    else
        -- the src port MUST be the from the 'upstream port' regardless if the 'srcStation' is 'local on current device' or on downstram device !!!
        src_localPort = util_802_1_br_getUptreamPortOnPe(currentTestedDevice)
    end

    local src_localPortInfo = util_802_1_br_getPortInfo(currentTestedDevice,src_localPort)

    local packetVid0Vid1
    
    if isRegisteredMulticast == true then
        util_802_1_br_setRegisteredMcAddr(registeredMcMacInfo[testedRegisteredMcMacIndex].mcAddr)
    end

    if isRegisteredMulticast == false or 
       registeredMcMacInfo[testedRegisteredMcMacIndex].vlanId == 5 
    then
        packetVid0Vid1 = buildPacketVid0Vid1(srcStation,trgStation)
    else
        local srcInfo = util_802_1_br_getStationInfo(srcStation)
        local trgInfo = util_802_1_br_getStationInfo(trgStation)

        packetVid0Vid1 = buildPacket(trgInfo.macAddr,srcInfo.macAddr,tag1,tag2,packetPayload)
    end
    
    local ingressPacket = packetVid0Vid1
    -- local port on CB that the 'src station' connected to
    local src_localPortOnCbOfStation = util_802_1_br_getLocalPortNum(devicesName_CB,srcStation)
    -- local port on CB that current tested device connected to
    local currentDevice_localPortOnCb

    if isCbCurrent == true then
        -- on CB each egress port need to do the logic
    else
        currentDevice_localPortOnCb = util_802_1_br_getLocalPortNumOnControlBridge(currentTestedDevice)
    end


    up_onETag = 6

    local direction , trgInterface , srcInterface , isMcFilter  , isMulticast
   
    if src_localPortInfo.isCascade == true or
       src_localPortInfo.isUptreamPort == true then

        srcInterface = srcStationInfo.pcid

        if isCbCurrent == true or onPeToUpstreamDirection == true then
            -- e-cid indicate the 'src port'
            e_cid_base = srcStationInfo.pcid
            grp = 0
            ingress_e_cid_base = 0

            direction = util_vTag_6bytes_direction_upstream
            trgInterface = 0
            isMcFilter = false
            isMulticast = false
        else
        
            -- grp,e-cid indicate the 'flood domain'
            if isRegisteredMulticast == true then
                e_cid_base = registeredMcMacInfo[testedRegisteredMcMacIndex].ecid
                grp        = registeredMcMacInfo[testedRegisteredMcMacIndex].GRP
            else
                e_cid_base  = vlanInfo[currentVlanIndex].ecid
                grp         = vlanInfo[currentVlanIndex].GRP
            end
            
            grp = bpe_grp(grp)

            --_debug_to_string("grp,e-cid indicate the 'flood domain'",grp,e_cid_base)
           
            if src_localPortOnCbOfStation ~= currentDevice_localPortOnCb then
                -- the CB sent packet with indication to NOT do 'src filtering'
                ingress_e_cid_base = 0 -- this is indication for 'no src filtering' !!!
                isMcFilter = false
            else
                ingress_e_cid_base = srcStationInfo.pcid
                isMcFilter = true
            end

            direction = util_vTag_6bytes_direction_downstream
            trgInterface = bit_shl(grp,12) + e_cid_base
            isMulticast = true


        end

        ETag = util_vTag_6bytes_buildTag(direction , --direction
                    isMulticast,--isMulticast
                    trgInterface,--trgInterface,
                    srcInterface,--srcInterface,
                    isMcFilter)--isMcFilter
--        ETag = util_802_1_br_buildETag(up_onETag,0,ingress_e_cid_base,grp,e_cid_base)
        ingressPacket = util_802_1_br_insertTagToPacketAfterMacs(ingressPacket,ETag)

        -- state the the 'from cpu' should go to EPort that will not add ETag .
        -- not that the physical port is still the port that we state in 'transmitInfo.portNum'
        luaTgfStateTrgEPortInsteadPhyPort(eportWithoutAddingETag)
    end

    local transmitInfo = {portNum = src_localPort , pktInfo = {fullPacket = ingressPacket} }

    local keepIngressETag -- indication that ingress ETag continue 'as is' to egress ETag
    local expectFloodBackToSrcLocalPort -- indication that the src port expected to get flood
    local trg_singleDestinationLocalPort = nil -- when not nil indicate that traffic should egress only this single port

    if isCbCurrent == true and src_localPortInfo.isCascade == true then
        -- came from downstream .. need to be flood back to all downstream ports (including the local src port)
        expectFloodBackToSrcLocalPort = true
    elseif isCbCurrent == false and onPeToUpstreamDirection == true then
        -- on upstream always single destination to 'upstream port'
        expectFloodBackToSrcLocalPort = false
        trg_singleDestinationLocalPort = util_802_1_br_getUptreamPortOnPe(currentTestedDevice)
    else
        -- on CB from non cscd port
        -- on PE on downstream direction
        expectFloodBackToSrcLocalPort = false
    end

    local egressInfoTable = {
        -- dynamically add ports that should/should not get the traffic
    }

    local ii = 1 -- index 1 is first index in LUA

    for index,trg_localPort in pairs(portsInTest) do

        --_debug_to_string("trg_localPort = ",trg_localPort)
        
        local portExcluded -- indication that port excluded from the 'flood'

        if expectFloodBackToSrcLocalPort == false and trg_localPort == src_localPort then
            -- not expect flood back to local src port
            portExcluded = true
        elseif trg_singleDestinationLocalPort ~= nil and trg_singleDestinationLocalPort ~= trg_localPort then
            -- not expect this egress port to get flood
            portExcluded = true
        else
            portExcluded = false
        end

        if trg_localPort ~= src_localPort then

            local trg_localPortInfo = util_802_1_br_getPortInfo(currentTestedDevice,trg_localPort)
            local trg_portToBeFiltered_onSrcFiltering = false -- is trg port to be filtered on 'src filtering'

            if trg_localPortInfo ~= nil then
                -- do src filtering logic
                if trg_localPortInfo.eport ~= nil and
                   trg_localPortInfo.eport == srcStationInfo.eport and
                   trg_localPortInfo.isCascade ~= true then
                    trg_portToBeFiltered_onSrcFiltering = true
                else
                    trg_portToBeFiltered_onSrcFiltering = false
                end

            end

            --_debug_to_string(portExcluded,trg_localPortInfo,isRegisteredMulticast,trg_portToBeFiltered_onSrcFiltering)
           
            if portExcluded == false and 
               trg_localPortInfo ~= nil and 
               isRegisteredMulticast == true and 
               trg_portToBeFiltered_onSrcFiltering == false and
               trg_localPortInfo.isCascade ~= true
            then
                local multicast_trg_allowed = false
                -- filter ports that are not in the 'multicast group'
                for index,trgStation in pairs(multiTrgStations) do

                    --_debug_to_string("trgStation",trgStation)
    
                    -- get local port on this device if have registered port
                    multicast_trg_localPort = util_802_1_br_getLocalPortNum(currentTestedDevice,trgStation)
                    
                    --_debug_to_string("multicast_trg_localPort = ",multicast_trg_localPort,"trg_localPort=",trg_localPort)

                    if trg_localPort == multicast_trg_localPort then
                        multicast_trg_allowed = true
                        break
                    end
                end

                if multicast_trg_allowed == false then
                    -- this port should not get this registered multicast traffic
                    portExcluded = true
                end
            end
            
            -- add entry into egressInfoTable
            egressInfoTable[ii] = {}
            egressInfoTable[ii].portNum = trg_localPort

            if portExcluded == false                and
               trg_localPortInfo ~= nil             and
               trg_portToBeFiltered_onSrcFiltering == false
            then

                if isCbCurrent == false                     and
                   onPeToUpstreamDirection == true          and
                   src_localPortInfo.isCascade == true      and
                   trg_localPortInfo.isUptreamPort == true
                then
                    keepIngressETag = true
                else
                    keepIngressETag = false
                end

                local egressPacket  = packetVid0Vid1

                if trg_localPortInfo.isCascade == true  or
                   trg_localPortInfo.isUptreamPort == true then

                    if isCbCurrent == true or onPeToUpstreamDirection == false then
                        if isRegisteredMulticast == true then
                            e_cid_base = registeredMcMacInfo[testedRegisteredMcMacIndex].ecid
                            grp        = registeredMcMacInfo[testedRegisteredMcMacIndex].GRP
                        else
                            -- e-cid indicate the 'ecid of vid'
                            e_cid_base = vlanInfo[currentVlanIndex].ecid
                            grp = vlanInfo[currentVlanIndex].GRP
                        end
                        
                        grp = bpe_grp(grp)
                            

                        if isCbCurrent == true then
                            currentDevice_localPortOnCb = trg_localPort
                        end

                        isMcFilter = true
                        if currentDevice_localPortOnCb ~= src_localPortOnCbOfStation then
                            -- the CB reset the ingress_e_cid_base when orig came from different physical port (on CB)
                            -- then the cascade port that gets the flood
                            ingress_e_cid_base = 0 -- this is indication for 'no src filtering' !!!
                            isMcFilter = false
                        else
                            ingress_e_cid_base = srcStationInfo.pcid
                        end

                        isMulticast = true
                        direction = util_vTag_6bytes_direction_downstream
                        trgInterface = bit_shl(grp,12) + e_cid_base
                        srcInterface = srcStationInfo.pcid
                        
                        
                    else -- upstream on PE
                        -- e-cid indicate the 'src port'
                        e_cid_base = srcStationInfo.pcid
                        grp = 0
                        ingress_e_cid_base = 0
                        
                        direction = util_vTag_6bytes_direction_upstream
                        isMulticast = false
                        trgInterface = 0
                        srcInterface = srcStationInfo.pcid
                        isMcFilter = false
                        
                    end

                    local origUpOnETag = up_onETag

                    if keepIngressETag == false and bpe_802_1_br_is_eArch_Device() then
                        up_onETag = 0 -- the 'rebuilt ETag' not have 'up' same as 'ingress' ... need extra QOS classifications for such support
                    end

                    ETag = util_vTag_6bytes_buildTag(direction , --direction
                                isMulticast,--isMulticast
                                trgInterface,--trgInterface,
                                srcInterface,--srcInterface,
                                isMcFilter)--isMcFilter

--                    ETag = util_802_1_br_buildETag(up_onETag,0,ingress_e_cid_base,grp,e_cid_base)
                    egressPacket = util_802_1_br_insertTagToPacketAfterMacs(egressPacket,ETag)

                    -- restore value
                    up_onETag = origUpOnETag
                end -- trg -- isCascade or isUptreamPort

                egressInfoTable[ii].pktInfo = {fullPacket = egressPacket }

            else -- portExcluded == true
                -- port not expect to get the traffic
                egressInfoTable[ii].packetCount = 0
            end

            ii = ii + 1

        end -- trg_localPort ~= src_localPort

    end -- loop pairs(portsInTest)

    if srcStationInfo.eport == nil then
        srcEPort = src_localPort
    else
        srcEPort = srcStationInfo.eport
    end

    purpose = "Send flood from " .. srcStationInfo.macAddr .. "from eport " .. srcEPort
    _debug("Start " .. purpose)

    --************* our system for testings (loopback port(s)) limited to cases where ingress packet got to egress port ,
    --************* which is the same as ingress port
    if expectFloodBackToSrcLocalPort == true then
        _debug("NOTE: 'storming' may occur due to flood back to local src port")
        -- state that storming should happen
        transmitInfo.loopbackStorming = true
    end

    -- transmit packet and check that egress as expected
    rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    if rc == 0 then
        _debug ("Passed : " .. purpose .. "\n")
    else
        _debug_printError ("Failed : " .. purpose .. "\n")
        isError = true
    end

    -- restore configuration
    luaTgfStateTrgEPortInsteadPhyPort(nil)
end

-- the 'prefix' of the file name of 'config' and 'deconfig'
local configFileName = nil
--**********************************************************************************
-- load the proper config
local function loadConfig()
    local deviceName = currentTestedDevice
    -- load the config info
    _debug("load the config file for : " .. deviceName)
    if isCbCurrent == true then
        configFileName = "vtag_6_bytes_control_bridge"
    elseif deviceName == devicesName_PE1 then
        configFileName = "vtag_6_bytes_port_extender_dev_1"
    elseif deviceName == devicesName_PE2 then
        configFileName = "vtag_6_bytes_port_extender_dev_2"
    else --deviceName == devicesName_PE3
        configFileName = "vtag_6_bytes_port_extender_dev_3"
    end

    if not bpe_802_1_br_is_eArch_Device() then
        -- the files for xcat3 includes 'xcat3' in the name
        configFileName = configFileName .. "_xcat3"
    end
    
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName))
end

-- restore the proper config
local function restoreConfig()
    local deviceName = currentTestedDevice
    _debug("load the deconfig file for : " .. deviceName)
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName,true))
end

--**********************************************************************************
-- run all flood/UC tests of specifc device (currentTestedDevice)
-- loop on all test cases that relevant to UC/flood
-- for UC:
--  1. run traffic to learn mac SA (will be used as mac DA)
--  2. run traffic with the DA
--      check if need to run on CB , PE upstream , PE downstream
-- for flood:
--  1. run traffic with the DA = FF:FF:FF:FF:FF:FF
--      check if need to run on CB , PE upstream , PE downstream
--
local function runAllRelevantCasesOfDevice(isKnownUc)
    local myTotalError = false
    local origIsError = isError
    local prefixName
    local sendTrafficFunction

    isError = false
    -- get the list of testcases from the 'system manager'
    --[[
        --  1. testName     - the test name
        --  2. srcStation   - src station
        --  3. trgStation   - trg station
        --  4. isUpstream   - is it need to be tested as 'traffic to upstream'
        --                  not relevant for CB
        --  5. isDownstream - is it need to be tested as 'traffic to downstream'
        --                  not relevant for CB
    --]]

    local testsTable

    if isKnownUc == true then
        testsTable = util_802_1_br_getUcTestScenarios(currentTestedDevice)
        prefixName = " known UC "
        sendTrafficFunction = sendUcFromSrcToTrg
    else
        testsTable = util_802_1_br_getFloodTestScenarios(currentTestedDevice)
        prefixName = " flooding "
        sendTrafficFunction = sendFloodFromSrcToTrg
    end

    -- loop on all tests
    for index,testEntry in pairs(testsTable) do
        _debug("\n\n \t\t***********")
        _debug("Start" .. prefixName .. ": " .. testEntry.testName)

        local skipTest = false

        if isCbCurrent == true then
            if isKnownUc == true then
                -- do SA learning before we can send UC to it
                doMacSaLearningOnCb(testEntry.trgStation)
            end

            --_debug_to_string("On CB :",testEntry.multiTrgStations)
            -- send the traffic
            sendTrafficFunction(testEntry.srcStation,testEntry.trgStation,nil,testEntry.multiTrgStations, testEntry.multicastCase)

        else -- on PE
            if testEntry.isUpstream then
                _debug("PE start" .. prefixName .. "on 'upstream path'")

                -- send the traffic to 'upstream' (twords the 'CB')
                sendTrafficFunction(testEntry.srcStation,testEntry.trgStation , true , testEntry.multiTrgStations, testEntry.multicastCase)

                _debug("PE ended" .. prefixName .. "on 'upstream path'")
            end
            
            if bpe_802_1_br_is_eArch_Device() and testEntry.multiTrgStations and 
                bpe_802_1_br_is_eArch_Device_not_support_registered_mc_on_PE() then
                
                -- registered MC was not implemented for BC2 device ... only for xcat3
                skipTest = true
            end

            
            if testEntry.isDownstream and skipTest == false then
                _debug("PE start" .. prefixName .. "on 'downstream path'")

                -- send the traffic to 'downstream' (from the 'CB')
                sendTrafficFunction(testEntry.srcStation,testEntry.trgStation , false, testEntry.multiTrgStations, testEntry.multicastCase)

                _debug("PE ended" .. prefixName .. "on 'downstream path'")
            end
        end

        local ii = summaryOfTests[currentTestedDevice].index
        summaryOfTests[currentTestedDevice].tests[ii] = testEntry
        if isError == true then
            summaryOfTests[currentTestedDevice].pass[ii] = false
        else
            summaryOfTests[currentTestedDevice].pass[ii] = true
        end
        summaryOfTests[currentTestedDevice].index = ii + 1


        if skipTest == true then
            _debug("SKIPPED :" .. prefixName .. "Test : " .. testEntry.testName)
        elseif isError == true then
            isError = false
            myTotalError = true
            _debug_printError("FAILED :" .. prefixName .. "Test : " .. testEntry.testName)
        else
            _debug("PASSES :" .. prefixName .. "Test : " .. testEntry.testName)
        end

    end

    isError = origIsError or myTotalError

end

-- run all UC tests of specifc device (currentTestedDevice)
local function runAllRelevantCasesOfDevice_knownUc()
    -- run tests that relevant for known UC
    runAllRelevantCasesOfDevice(true)
end

-- run all flood tests of specifc device (currentTestedDevice)
local function runAllRelevantCasesOfDevice_flood()
    -- run tests that relevant for flooding
    runAllRelevantCasesOfDevice(false)
end

-- flush the FDB (use LUA CLI command !)
local function flushFdb()

    local fileName = "flush_fdb_only_dynamic"
    _debug("call to Flush the FDB")
    executeLocalConfig(luaTgfBuildConfigFileName(fileName))
   
end

-- run all the tests of specifc device (currentTestedDevice)
-- 1. run UC tests
-- 2. flush FDB
-- 3. run flood tests
-- 4. flush FDB
local function runAllRelevantCasesOfDevice()
    if debug_Skip_Uc_tests ~= true then
        _debug("\n\n start tests for 'known UC' : \t\t***********")
        runAllRelevantCasesOfDevice_knownUc()
        _debug("\n\n ended tests for 'known UC' : \t\t***********")
    end

    -- flush the FDB (before flood)
    flushFdb()

    if debug_Skip_Flood_tests ~= true then
        _debug("\n\n start tests for 'flooding' : \t\t***********")
        runAllRelevantCasesOfDevice_flood()
        _debug("\n\n ended tests for 'flooding' : \t\t***********")
        -- flush the FDB
        flushFdb()
    end

end
-- test all the devices
-- loop on all devices in the system
-- for each device do :
--  1.'config'
--  2. run all uc,flood cases relate to device
--  3. 'deconfig'
local function testAllDevices()
    local role
    for index,deviceName in pairs(testedDevices) do

        if debug_specific_device ~= nil then
            deviceName = debug_specific_device
        end

        if deviceName == devicesName_CB then
            isCbCurrent = true
            role = "control_bridge"
        else
            isCbCurrent = false
            role = "port_extender"
        end

        currentTestedDevice = deviceName
        --check if the device supports this role
        if true == bpe_802_1_br_check_device_for_role(role) then

            if (is_xCat3x_in_system()) then
                xcat3x_start(true)

                xCat3x_restore_working_ports(--[[no devNum--]])

                if (deviceName == devicesName_CB) then
                    -- all ports
                    xCat3x_replace_working_ports_with_aldrin_only({getTestPort(2)--[[downlink]],getTestPort(3)--[[downlink]]})
                elseif (deviceName == devicesName_PE1) then
                    xCat3x_replace_working_ports_with_aldrin_only({getTestPort(4)--[[uplink]]})
                elseif (deviceName == devicesName_PE2) then
                    xCat3x_replace_working_ports_with_aldrin_only({getTestPort(3)--[[uplink]] , getTestPort(2)--[[downlink]]})
                else
                    xCat3x_replace_working_ports_with_aldrin_only({getTestPort(2)--[[uplink]] })
                end
                --reload the ports
                load_portsInTest()
 
                -- reload ports
                luaTgfConfigTestedPorts(devNum,portsInTest,true)
 
                -- reload also the utils file
                dofile("dxCh/examples/common/bpe_802_1br_utils.lua")
            end

            currentTestedDevice = deviceName
            _debug("BPE 802.1 br : start Testing on " .. currentTestedDevice)

            -- load configuration
            loadConfig()
            -- run the tests
            runAllRelevantCasesOfDevice()
            -- restore configurations
            restoreConfig()

            if (is_xCat3x_in_system()) then
                -- restore values (after the deconfig)
                xcat3x_start(false)
            end
           
            
            if isError == true then
                totalError = true
                _debug_printError("FAILED : BPE 802.1 br : on " .. currentTestedDevice)
            else
                _debug("PASSED : BPE 802.1 br : on " .. currentTestedDevice)
            end

            if debug_specific_device ~= nil then
                -- allow debug only specific device
                break
            end
        else
            _debug("SKIPPED (the running device not support it) : BPE 802.1 br : on " .. currentTestedDevice)
        end
    end

end

-- print the passed/failed tests
local function  printFinishedTests(passed)
    local numTestsCounted = 0
    local numTestsTotal = 0
    local myString
    _debug("************************")
    if passed == false then
        myString = "Failed"
    else
        myString = "Passed"
    end

    _debug("BPE " .. myString .." tests:")


    for index1 , entry in pairs(summaryOfTests) do
        _debug("On device : " .. entry.device)

        for index2, test in pairs(entry.tests) do
            if (entry.pass[index2] == false and passed == false) or
               (entry.pass[index2] == true  and passed == true)
            then
                --[[
                        --  1. testName     - the test name
                        --  2. srcStation   - src station
                        --  3. trgStation   - trg station
                        --  4. isUpstream   - is it need to be tested as 'traffic to upstream'
                        --                  not relevant for CB
                        --  5. isDownstream - is it need to be tested as 'traffic to downstream'
                        --                  not relevant for CB
                --]]

                if test.trgStation ~= nil then
                    trgStation = test.trgStation
                else
                    trgStation = " flooding "
                end

                 _debug("\t test : " .. test.testName .. " srcStation " .. test.srcStation ..
                             " trgStation " .. trgStation .. " isUpstream " .. tostring(test.isUpstream) ..
                             " isDownstream " .. tostring(test.isDownstream))

                 numTestsCounted = numTestsCounted + 1
            end

            numTestsTotal = numTestsTotal + 1
        end

    end

    _debug("BPE "..myString.." in " .. numTestsCounted .. " tests out of " .. numTestsTotal)
    _debug("************************")
end

--**********************************************************************************
--**********************************************************************************
--**********************************************************************************
--**********************************************************************************
--**********************************************************************************
local function mainFunction()
    -- start ALL the tests on all the devices (CB,PE1,PE2,PE3)
    testAllDevices()

    origDebugOn = debug_on
    -- open debug for summary prints
    debug_on = true

    _debug("******** BPE : summary of results : ****************")

    -- print success tests
    printFinishedTests(true)
    -- check for summary of errors
    if totalError == true then
        -- print failed tests
        printFinishedTests(false)

        setFailState()
    end

    debug_on = origDebugOn
end

-- need to be called before calling luaTgfConfigTestedPorts(...)
if not bpe_802_1_br_is_eArch_Device() then
--[[ not needed as the getTestPort() already sets portsInTest[] with proper values    
    -- replace the ports to be tested
    portsInTest = {0,8,18,23}
]]
    -- state the 'util' that xcat3 is running
    util_802_1_br_system_xcat3_device()
end

load_portsInTest()
-- init tested ports : force link up + reset mac counters
luaTgfConfigTestedPorts(devNum,portsInTest,true)
mainFunction()
-- end tested ports : unforce link up + reset mac counters
luaTgfConfigTestedPorts(devNum,portsInTest,false)

