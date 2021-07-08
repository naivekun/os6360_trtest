--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* RxTx.lua
--*
--* DESCRIPTION:
--*       sending/receiving of a packet
--*       receiving of a packet and sending it back
--*       ping of a certain ip/
--*       activating of a specific protocol
--*
--* FILE REVISION NUMBER:
--*       $Revision: 4 $
--*
--********************************************************************************

--includes

--constants

require("dxCh/traffic/protocols_control")

--registering c functions

cmdLuaCLI_registerCfunction("wrlGetSystemTime")
cmdLuaCLI_registerCfunction("prvLuaTgfTrafficGeneratorPortTxEthCaptureSet")

--[[
Globals defined (or used):
    myIP - The ip address of the device
    myMac - The mac address of the device
]]

-- ************************************************************************
---
--  cmdSendPacket
--        @description  sending of packets
--
--        @param params         - packet command parameters
--
function cmdSendPacket(params)

    local ret, status
    local fd
    local data
    local burstcnt
    if nil~=params.pktnum then
        burstcnt = params.pktnum
    else
        burstcnt = 1
    end
    if nil~=params.file then
        filename=params.file

        fd,e=fs.open(filename)

        if fd == nil then
            print("failed to open file " .. filename .. ": "..e)
            return false
        end

        data=""
        str, e = fs.gets(fd)
        while (str~=nil) do
            str=string.gsub(str,"\n","")
            data=data..str
            str, e = fs.gets(fd)
        end
        fs.close(fd)
        if e ~= "<EOF>" then
            print("error reading file: " .. e)
        else
            data = stripHexDataSequence(data);
            if data == nil then
                print("error: data for sending is not hexadecimal")
                return;
            end
            ret,status=sendPacket(params.port, data, 60, burstcnt)
            if (ret~=0) then
                print(status)
            end
        end
    end


    if nil~=params.data then
        data = stripHexDataSequence(params.data);
        if data == nil then
            print("error: data for sending is not hexadecimal")
            return;
        end
        ret,status=sendPacket(params.port, data, 60, burstcnt)
        if (ret~=0) then
            print(status)
        end
    end
end

-- do not use this value unless you know what you want !!!
send_packet_to_all_devices_all_ports = "send packet to all devices all ports"

-- ************************************************************************
---
--  sendPacket
--        @description  This function receives a device number, port
--                      number/s as an array and data to send, if
--                      successful return 0, if not returns ret~=0 and a
--                      status
--
--        @param devPortTable   - interface
--        @param packetData     - packet data
--        @param minLen         - minimum packet length
--        @param burstCount     - packet count to send
--
function sendPacket_40(devPortTable,packetData,minLen,burstCount)
    local ret, txTaskQueue,replyQ,status,message,i

    -- convert send_packet_to_all_devices_all_ports to nil ..
    -- to keep legacy behaviour for CPSS 3.4
    if (devPortTable==send_packet_to_all_devices_all_ports) then devPortTable = nil end
    if (devPortTable==nil) then devPortTable=luaCLI_getDevInfo() end

    if (burstCount==nil) then
        burstCount = 1
    end

    if (minLen==nil) then minLen=64 end
    if (minLen < 64) then minLen=64 end
    --activates the TX queue task if its not activated and receives the tasks id
    ret, txTaskQueue = cpssGenWrapper("transmitTaskQueueIdGet", { { "OUT", "GT_U32", "msgqId" } })
    if ret ~= 0 then
        return ret, "Error creating tx queue"
    else
        transmitTaskQ = luaMsgQAttach("txTaskQ", "TX_PARAMS_STC", txTaskQueue.msgqId)
    end

    -- create reply queue
    replyQ = luaMsgQCreate("replyQueue", "GT_U32", 10);
    if type(replyQ) ~= "table" then
        print("sendPacket_40(): luaMsgQCreate() returned "..to_string(replyQ))
        return replyQ, "luaMsgQCreate() failed"
    end

    for dev,portTable in pairs(devPortTable) do
        for i=1,#portTable do
            -- transmit packet
            CLI_execution_unlock()
            status = luaMsgQSend(transmitTaskQ,
                {replyQ = replyQ.msgqId,srcInterface = {type = "port",devPort = { devNum = dev, portNum = portTable[i] } },
                packet = { len=math.max(minLen,string.len(packetData)/2), data= packetData }  })

            if (type(status)~="string") then
                -- recv status
                status = luaMsgQRecv(replyQ)
            end
            CLI_execution_lock()

            if (type(status)=="string") then
                status="Error sending packet through port device "..dev .. " port ".. portTable[i] .." : " .. status
                ret=1
                break
            end
        end
    end

    luaMsgQDelete(replyQ)
    return ret,status
end
sendPacket = sendPacket_40

-- if ok return 0. if failed return rc(~=0), status
function sendPacket_41(devPortTable,packetData,minLen,burstCount)
    local ret = 0
    local xTaskQueue,replyQ,status,message,i

    if (devPortTable==nil) then
        print(" the nil port is not supported any more ! , use explicit list port de,port or us ["..send_packet_to_all_devices_all_ports.."]")
        -- stop_the_run() is an 'undefined function' to cause LUA catch this error ... 'fatal error'
        stop_the_run()
        return
    end

    if (burstCount==nil) then
        burstCount = 1
    end

    if (devPortTable==send_packet_to_all_devices_all_ports) then
        -- convert send_packet_to_all_devices_all_ports to actual 'all port on all device'
        devPortTable=luaCLI_getDevInfo(nil, true) -- only to ports which are up
        -- except CPU port
        for dev,portTable in pairs(devPortTable) do
            for i=1,#portTable do
                if portTable[i] == 63 then
                    table.remove(portTable, i)
                    break
                end
            end
        end

    end
    if (minLen==nil) then minLen=60 end
    if (minLen < 60) then minLen=60 end
    -- add padding to minLen
    if string.len(packetData) < minLen*2 then
        packetData = packetData .. string.rep("0",minLen*2-string.len(packetData))
    end
    packetInfo = { partsArray= {
        { type="TGF_PACKET_PART_PAYLOAD_E", partPtr = packetData }
    }}


    for dev,portTable in pairs(devPortTable) do
        for i=1,#portTable do
            -- transmit packet
            CLI_execution_unlock()
            ret = prvLuaTgfTransmitPackets(dev, portTable[i], packetInfo, burstCount)
            CLI_execution_lock()

            if ret ~= 0 then
                status="Error sending packet through port device "..dev .. " port ".. portTable[i] .." : " .. tostring(status)
                break
            end
        end
    end
    return ret,status
end

sendPacket = sendPacket_41


-- ************************************************************************
---
--  rcvPacket
--        @description  receiving of packets
--
--        @param params         - packet command parameters
--
function rcvPacket(params)
    local packetContent

    setRcv(nil,false)
    setRcv(params.port,true)
    packetContent = rcvSinglePacket(params.timeout)
    setRcv(nil,false)

    if nil~=params.file and packetContent ~= nil then
        filename=params.file

        --receiving data
        fd,e=fs.open(filename)

        if fd == nil then
            print("failed to open file " .. filename .. ": "..e)
            return false
        end

        data=""
        str, e = fs.gets(fd)
        while (str~=nil) do
            str=string.gsub(str,"\n","")
            data=data..str
            str, e = fs.gets(fd)
        end
        fs.close(fd)
        if e ~= "<EOF>" then
            print("error reading file: " .. e)
        else
            ret,status=sendPacket(params.port, data)
            if (ret~=0) then
                print(status)
            end
        end

        --comparing file and packet data
        if data ~= packetContent.packet.data then
            print("Matching fails. The packet doesn't match to the file.")
        else
            print("Matching was successful. The packet matches to the file.")
        end
    end
end

local function hex2MacString(mac)
    return string.format("%s:%s:%s:%s:%s:%s",
            string.sub(mac,1,2),string.sub(mac,3,4),
            string.sub(mac,5,6),string.sub(mac,7,8),
            string.sub(mac,9,10),string.sub(mac,11,12))
end
local function hex2IpString(ip)
    return string.format("%d.%d.%d.%d",
        tonumber(string.sub(ip,1,2),16),
        tonumber(string.sub(ip,3,4),16),
        tonumber(string.sub(ip,5,6),16),
        tonumber(string.sub(ip,7,8),16))
end


-- ************************************************************************
---
-- filterDevPorts
-- @description  delete items from list of devices/ports that sutisfy
--               the filter
--
-- @param devPortList - list of devices/ports. Looks like:
--                   {dev1 ={por11,port12...}, dev2 = {port21, port22...}}
-- @param filterFunc  - a boolean filter-function. Accepts two arguments:
--                      devNum, portNum. If false a port should be
--                      excluded from the resulting list.
-- @return
--   devPortList (first argument)
--
local function filterDevPorts(devPortList, filterFunc)
    for devNum, portList in pairs(devPortList) do
        -- After deleting item #i indexes of all following items will
        -- be reduced to one. So use backward order to ignore this.
        for i=#portList,1,-1 do
            if not filterFunc(devNum, portList[i]) then
                table.remove(portList,i)
            end
        end
    end
    return devPortList
end

-- ************************************************************************
---
-- isNetworkTxPort
-- @description  check if a tx port is network (not cascade)
--
-- @param dev  - device number
-- @param port - port number
-- @return
--   true - if a port is network port
--   false - otherwise
--

local function isNetworkTxPort(dev, port)
    local rc,values = myGenWrapper(
        "cpssDxChCscdPortTypeGet", {
            {"IN",  "GT_U8", "devNum", dev},
            {"IN",  "GT_PHYSICAL_PORT_NUM", "portNum", port},
            {"IN",  "CPSS_PORT_DIRECTION_ENT", "portDirection", "CPSS_PORT_DIRECTION_TX_E"},
            {"OUT", "CPSS_CSCD_PORT_TYPE_ENT", "portTypePtr"}})
    if rc ~= 0 then
        print(string.format("Error of reading cascading state of a tx port %d/%d: %s",
                            dev, port, returnCodes[rc]))
    end
    return rc == 0 and values.portTypePtr == "CPSS_CSCD_PORT_NETWORK_E"
end

-- ************************************************************************
---
--  ping
--        @description  ping command
--
function ping(params)
    local rxQ,ret,status,rxdata,destIP,destMac,timeOut

    is_ip_defined(myIP)
    is_mac_defined(myMac)

    --broadcast arp request
    setRcv(nil,true)  --receive from all ports (for arp reply)
    destIP=string.format("%02X%02X%02X%02X",params.ip[1],params.ip[2],params.ip[3],params.ip[4])

    -- will wait arp reply
    rxQ = registerRx("PingArpQuery", {
        noCpuPort = true,
        level=2,
        ieee8021q = {
            dst=myMac,
            etherType=0x0806
        },
        payload={
            -- htype=1,ptype=0800,hlen=6,plen=4,oper=2
            {offset=0,len=8,val="0001080006040002" },
            {offset=14,len=4,val=destIP },
            {offset=18,len=6,val=myMac },
            {offset=24,len=4,val=myIP }
        }
    })

    local devPortList=luaCLI_getDevInfo(nil, true) -- only to ports which are up
    filterDevPorts(devPortList, isNetworkTxPort)
    sendPacket(devPortList,createArpPacket(destIP))

    print("Pinging "..params.ip.string
        .." using src mac: "..hex2MacString(myMac)
        .."  src ip:"..hex2IpString(myIP).."\n")

    status, rxdata = luaMsgQRecv(rxQ, 1000*3) -- 3 sec
    unregisterRx("PingArpQuery", rxQ)
    if rxdata == nil then
        print("Arp request timed out")
        setRcv(nil,false)
        return false
    end
    --received arp reply, send ping
    local parsed = {}
    --resolve to skip vlan tags
    parse_ieee8021q(rxdata.packet.data,parsed)
    destMac = proto_get_bytes(rxdata.packet.data,parsed.ieee8021q.payloadOffset+8,6)

    --print("ARP reply received from "..hex2MacString(destMac).." port="..to_string(rxdata.portNum).." data="..rxdata.packet.data)
    -- will wait icmp reply
    rxQ = registerRx("PingReply", {
        level=3,
        ieee8021q = {
            dst = myMac,
        },
        ipv4 = {
            src = destIP,
            dst = myIP,
            protocol = 1
        },
        payload = {
            -- type == 0, code == 0  echo request
            { offset=0,len=2,val="0000" }
        }
    })

    setRcv(nil,false)

    if params.noPortRestrict then
        setRcv(nil,true)    -- receive from all ports (for icmp reply) in case of external cable loop
                            -- actially, we need to listen rxdata.portNum plus second port to get external cable loop working
    else
        setRcv({[rxdata.devNum]={rxdata.portNum}},true)
    end

    sendPacket({[rxdata.devNum]={rxdata.portNum}},createICMPPacket(destMac,destIP))

    status, rxdata = luaMsgQRecv(rxQ,1000*4) -- 3 seconds

    unregisterRx("PingReply", rxQ)
    setRcv(nil,false)
    if rxdata == nil then
        print("ping timeout")
        return false
    end

    --print("ICMP reply received data="..rxdata.packet.data)
    parsed = {}
    parse_ipv4(rxdata.packet.data,parsed)

    local icmpLen = parsed.ipv4.length-parsed.ipv4.IHL*4
    local icmpOffset = parsed.ipv4.payloadOffset
    if crc16(rxdata.packet.data,icmpLen,icmpOffset) ~= 0 then
        -- bad ICMP crc
        print(string.format("bad ICMP crc: 0x%04x",crc16(rxdata.packet.data,icmpLen,icmpOffset)))
        return false
    end

    print("Ping to ip "..params.ip["string"].." successful.")
    return true
end


-- ************************************************************************
---
--  createArpPacket
--        @description  This function creates an arp request packet
--
--        @param targetIP       - target ip
--
--        @return       packet data
--
function createArpPacket(targetIP)
    is_ip_defined(myIP)
    is_mac_defined(myMac)

    return "FFFFFFFFFFFF" .. myMac .. "0806" ..
            proto_create_struct("ARP",{oper=1,
                senderHA=myMac, senderPA=myIP, targetPA=targetIP})
end


-- ************************************************************************
---
--  createICMPPacket
--        @description  This function creates an ICMP packet (ping
--                      request), including the checksum
--
--        @param targetMac      - target mac-address
--        @param targetIP       - target ip
--
--        @return       string of target mac addr/icmp header
--
function createICMPPacket(targetMac,targetIP)
    local payload="000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
    local icmp = proto_create_struct_icmp({
        icmptype=8,seq=1
    }, payload)
    local ipPkt = proto_create_struct_ip({
        protocol=1,src=myIP,dst=targetIP
    }, icmp)

    return targetMac .. myMac .. "0800"..ipPkt
end

local function setRcv_dev_ingress_mirror_to_cpu(dev, enable)
    if (enable==true) then
        local mystruct = {
            interface = {
                type = 0, devPort = {devNum = dev ,portNum = 63}
        }}

        --Enable mirroring
        local family   = wrlCpssDeviceFamilyGet(dev)
        local frwdMode = is_sip_5(dev)
            and "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E"
            or  "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E"

        if family == "CPSS_PP_FAMILY_CHEETAH_E"
            or family == "CPSS_PP_FAMILY_CHEETAH2_E"
            or family == "CPSS_PP_FAMILY_CHEETAH3_E"
        then
            --configure the analyzer index
            local hwDevNum = dev;
            ret,val = myGenWrapper("cpssDxChCfgHwDevNumGet",{
                {"IN","GT_U8","devNum",dev},
                {"OUT","GT_HW_DEV_NUM","hwDevNumPtr"}
            })
            if ret ~= 0 then
                print ("Error in cpssDxChCfgHwDevNumGet for device #".. dev)
            end
            hwDevNum = val["hwDevNumPtr"];
            ret,val = myGenWrapper("cpssDxChMirrorRxAnalyzerPortSet",{
                {"IN","GT_U8","devNum",dev},
                {"IN","GT_PORT_NUM","analyzerPort",63}, --CPU port to be analyzer port
                {"IN","GT_HW_DEV_NUM","analyzerHwDev",hwDevNum}
            })
            if ret ~= 0 then
                print ("Error setting CPU port to be RX Analyzer for device #".. dev)
            end
        else
            if not is_sip_6(devNum) then
                ret,val = myGenWrapper(
                "cpssDxChMirrorToAnalyzerForwardingModeSet", {
                    {"IN","GT_U8", "devNum", dev},
                    {"IN","CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT", "mode", frwdMode}
                })
                if ret ~= 0 then
                    print ("Error setting forwarding mode for device #".. dev)
                end
                --print("Forwarding mode returned:"..ret)--.." and val is:"..val[next(val)])
            end

            --configure the analyzer index
            ret,val = myGenWrapper("cpssDxChMirrorAnalyzerInterfaceSet",{
                {"IN","GT_U8","devNum",dev},
                {"IN","GT_U32","index",6}, --index of analyzer port
                {"IN","CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC","interfacePtr",mystruct}  --the interface struct
            })
            if ret ~= 0 then
                print (string.format("Error setting analyzer interface %d/%d for device #%d ",
                                     mystruct.interface.devPort.devNum,
                                     mystruct.interface.devPort.portNum,
                                     dev))
            end
        end

        --print("Analyzer interface set returned:"..ret) --.." and val is:"..val[next(val)])
    elseif is_sip_6(devNum) then
        -- do nothing 'keep' the single supported mode : 'end-to-end'
    else
         ret,val = myGenWrapper(
                "cpssDxChMirrorToAnalyzerForwardingModeSet", {
                    {"IN","GT_U8", "devNum", dev},
                    {"IN","CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT", "mode", "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E"}
            })
            if ret ~= 0 then
                print ("Error setting forwarding mode for device #".. dev)
            end
    end
end

local function setRcv_port_ingress_mirror_to_cpu(dev, port, enable)
            --configure a specific port to send to a specific index
    local ret,val = myGenWrapper("cpssDxChMirrorRxPortSet",{
    {"IN","GT_U8","devNum",dev},
        {"IN","GT_U8","mirrPort",port},
        {"IN","GT_BOOL","isPhysicalPort",true},  --is port physical
        {"IN","GT_BOOL","enable",enable},  --enable/disable mirroring
        {"IN","GT_U32","index",6}  --Analyzer port index
    })
    if (ret~=0) then print ("Error setting analyzer port for port number:"..portTable[i]) end
end

-- ************************************************************************
---
--  setRcv
--        @description  This function enables/disables receiving of packets
--                      through the specified ports if the rcv is set to
--                      false the function will check for existing rxtx
--                      queue and ENABLE all the ports back to this queue
--
--        @param devPortTable   - interface
--        @param enable         - enabling property
--
--        @usage __global       - __global["ifRange"]: iterface range
--
--        @return       true on success, otherwise false and error message
--
function setRcv(devPortTable, enable, capture_type)
    local ret,rxQ,val,status,rxdata,mystruct,i,dev
    if capture_type == nil then
        capture_type = "mirror_ingress_to_cpu";
    end
    if (devPortTable==nil) then devPortTable=luaCLI_getDevInfo() end  --nil means all

    if (enable==false) and (luaGlobalGet("RxTxTaskStat")==1) then  -- "RxTxTaskStat" -  parameter for the rcv task status
        devPortTable=luaCLI_getDevInfo()
        enable=true
    end

    if capture_type == "mirror_ingress_to_cpu" then
        for dev,portTable in pairs(devPortTable) do
            setRcv_dev_ingress_mirror_to_cpu(dev, enable);
            for i=1,#portTable do
                setRcv_port_ingress_mirror_to_cpu(dev, portTable[i], enable);
            end
        end
    end
    if capture_type == "mirror_egress_to_cpu" then
        for dev,portTable in pairs(devPortTable) do
            for i=1,#portTable do
                prvLuaTgfTrafficGeneratorPortTxEthCaptureSet(
                    dev, portTable[i], 1 --[[TGF_CAPTURE_MODE_MIRRORING_E--]], enable);
            end
        end
    end
    if capture_type == "pcl_trap_egress_to_cpu" then
        for dev,portTable in pairs(devPortTable) do
            for i=1,#portTable do
                prvLuaTgfTrafficGeneratorPortTxEthCaptureSet(
                    dev, portTable[i], 0 --[[TGF_CAPTURE_MODE_PCL_E--]], enable);
            end
        end
    end

    signalReceiverTask()
end


-- ************************************************************************
---
--  rcvSinglePacket
--        @description  This function returns the first packet that
--                      arrives in the specified timeout (after setRcv was
--                      called)
--
--        @param timeout        - time-out
--
--        @return       received data
--
function rcvSinglePacket(timeout)
    local ret,rxQ,status,rxdata,mystruct,origQueue

    rxQ = registerRx("rcvSinglePacket",{})

    status, rxdata = luaMsgQRecv(rxQ,timeout * 1000)    -- recv status
    unregisterRx("rcvSinglePacket",rxQ)

    if (rxdata==nil) then
        print("Receive timed out, No packet was received.")
    else
        print("\nPacket received:\n" ..to_string(rxdata).."\n")
    end

    return rxdata
end

--
-- ************************************************************************
---
--  activateProtocol
--        @description  active protocol command
--
--        @param params         - command params
--
function activateProtocol(params)
    local func
    if params.flagNo==nil then
        func = _G["proto_"..params.enable.."_enable"]
    else
        func = _G["proto_"..params.enable.."_disable"]
    end
    if type(func) == "function" then
        func()
    end
end


-- ************************************************************************
---
--  activateProtocolWrapper
--        @description  wrapper for activate protocol
--        @param params
--        @return
--
function activateProtocolWrapper(params)
    if params.enable~="all" then    --if should activate only one protocol
        activateProtocol(params)

    else    -- enable protocol all choosed
        for _, protocolInfo in pairs(CLI_type_dict["protocol_names"].enum) do
            if "all"~=protocolInfo.value then
                params.enable=protocolInfo.value
                activateProtocol(params)
            end
        end

    end
end


CLI_addCommand("traffic", "send", {
    func=cmdSendPacket,
    help="Send a packet",
    params={ { type="named",
        { format="port %port-range",name="port", help="The device and port number" },
        "#dataOrFilename",
        {format="pktnum %GT_U32", name="pktnum", help="Packet number to send, default one packet"},
        mandatory={"inputString"},
        requirements={["file"]={"port"},["data"]={"port"}}
      }}}

)


CLI_addCommand("traffic", "receive", {
    func=rcvPacket,
    help="Receive a packet",
    params={
    { type="named", { format="port %port-range",name="port", help="ports to receive message from" },
                    { format="timeout %GT_32",name="timeout", help="timeout in seconds to wait for packet" },
                    { format="match %string", name="file", help="Enter file name to match with the packet" },
                    mandatory={"timeout","port"}}
    }

})





CLI_addCommand("traffic", "ping", {
    func=ping,
    help="ping a certain ip",
    params={
        {   type="values", { format="%ipv4", name="ip", help="ip to ping" },
            mandatory={"ip"},
        } ,
        {   type="named",
                           { format="noPortRestrict", name="noPortRestrict", help="Receive ICMP from any port, not port where ARP reply received from" },
        }
    }
})



--------------------------------------------
-- type registration: protocol_names
--------------------------------------------
CLI_type_dict["protocol_names"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "Enter protocol name",
    enum = {
        ["arp"] =    { value="arp_reply",
                                        help="arp protocol"      },
        ["ping"] =   { value="ping_reply",
                                        help="ping protocol"     },
        ["rxtx"] =  { value="rxtx_reply",
                                        help="rxtx protocol"       },
        ["all"] =  { value="all",
                                        help="all protocols"       }
    }
}


CLI_addCommand("traffic", "protocol", {
    func=activateProtocolWrapper,
    help="activate a specific protocol",
    params={ { type="named", { format="enable %protocol_names",name="enable", help="enable a protocol" },mandatory={"enable"} }}
})

CLI_addCommand("traffic", "no protocol", {
    func=function(params)
        params.flagNo=true
        return activateProtocolWrapper(params)
    end,
    help="activate a specific protocol",
    params={ { type="named", { format="enable %protocol_names",name="enable", help="disable a protocol" },mandatory={"enable"} }}
})



-- ************************************************************************
---
--  traficRate
--        @description  The function prints traffic rate
--        @param device       - device number
--        @param port         - port number
--        @param interval     - interval
--        @param iterations   - number of iterations
--        @return   error message in case of error
--
function traficRate(params)
    local iterNum
    local ret, values

    if params.iter ~= nil then
        iterNum = params.iter
    else
        iterNum = 10 --default value
    end

    local function get_c(dev,port,cntr)
        local ret, values = myGenWrapper("cpssDxChMacCounterGet",{
            {"IN", "GT_U8",                      "devNum", dev},
            {"IN", "GT_PHYSICAL_PORT_NUM",       "portNum",port},
            {"IN", "CPSS_PORT_MAC_COUNTERS_ENT", "counter",cntr},
            {"OUT","GT_U64",                     "cntrValue"}})
        if ret ~= 0 then
            return "Error in cpssDxChMacCounterGet calling"
        end
        return wrlCpssGtU64StrGet(values.cntrValue)
    end

    for i=1,iterNum,1 do
        myGenWrapper("osTimerWkAfter",{
            {"IN", "GT_U32", "mils", 1000*params.interval}})

        print("rx=",get_c(params.devID,params.port,"CPSS_GOOD_UC_PKTS_RCV_E")..
            "  tx=",get_c(params.devID,params.port,"CPSS_GOOD_UC_PKTS_SENT_E"))
    end
end

--The command displays traffic rate
CLI_addCommand("traffic", "rate", {
    func=traficRate,
    help="Display traffic rate for certain port",
    params={
        {type="named",
        { format="device %devID", name="device", help="ID of the device needed to count"},
        { format="port %port", name="port", help="Port number" },
        { format="interval %number", name="interval", help="Interval in seconds" },
        { format="iterations %number", name="iter", help="Number of iterations. Default 10" },
        mandatory = {"device", "port", "interval"}}
    }
})

-- ************************************************************************
---
--  CPU_rx_dump
--        @description  The function enables packate dumps for rx cpu
--
--        @return   true on success
--
function CPU_rx_dump(params)
    local enable = true
    local ret, val
    local enableDumpRxPacket
    if params["noFlag"] ~= nil then
        enable = false
    end

    ret,val = myGenWrapper("dxChNetIfRxPacketParse_DebugDumpEnable",{
        {"IN","GT_BOOL","enableDumpRxPacket",enable}
    })

    return true
end

--The command enables CPU rx packets dump
CLI_addHelp("traffic", "cpu", "allow print (dump) of the Rx Packet include DSA tag.")
CLI_addHelp("traffic", "cpu rx", "allow print (dump) of the Rx Packet include DSA tag.")
CLI_addCommand("traffic", "cpu rx dump", {
    func=CPU_rx_dump,
    help="allow print (dump) of the Rx Packet include DSA tag.",
    params={}
})

--The command disable cpu rx packets dump
CLI_addCommand("traffic", "no cpu rx dump", {
    func=function(params)
            params["noFlag"] = true
            return CPU_rx_dump(params)
        end,
    help="disable print (dump) of the Rx Packet include DSA tag.",
    params={}
})
