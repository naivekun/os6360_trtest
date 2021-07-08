--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* traffic_generator.lua
--*
--* DESCRIPTION:
--*       configure, connect, disconnect enable and disable the Traffic generator 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  trafficGeneratorConfigure
--        @description  configure traffic generator for the interface
--
--        @param params             - params["macDest"]: destination MAC
--                    params["macSource"]: source MAC
--                    params["ethertype"]: The etherType to be sent in the packet after the VLAN tag(if any).
--                                             (0..0xFFFF)
--                    params[vid]: if ~=nil- The VLAN ID of the VLAN tag added to packet. (1..4095), otherwise default configuration (no vlan)
--                    params[vpt]: if ~=nil The VPT of the VLAN tag added to packet. (0..7), otherwise default configuration (0)
--                    params[cfi]: if ~=nil The CFI of the VLAN tag added to packet. (0..1), otherwise default configuration (0)
--                    params["payload"]: if ~=nil- payload type, otherwise default configuration (cyclic with pattern=0) 
--                    params["payload-cyclic"]: if ~=nil- pattern for payload-cyclic
--                    params["payload-random"]: if ~=nil- payload is choosed to be random, otherwise-cycle
--                    params["increment"]: if ~=nil- Enables MAC DA incremental for every transmitted packet. otherwise default configuration (no MAC increment)
--                    params["length-random"]: if ~=nil- define packet length to be random. otherwise packet length is constant(user number or by default-64)
--                    params["undersize-enable"]: if ~=nil- Enables undersized packets transmission, otherwise default configuration (no undersized packets)
--                    params["length"]: if ~=nil- described the packet length, otherwise if params["length-random"]==nil then use default configuration (64)
--                    params["ipg"]: if ~=nil- Interpacket gap in bytes. (0..0xFFFF), otherwise default configuration (8)
--                    params["packet-count"]: if ~=nil- Packet count to transmit in single burst. (1..8191) otherwise default configuration (transmit mode is continues)

--        @return       true on success, otherwise false and error message
--

local function trafficGeneratorConfigure(params)
    local ret
    local connect,configPtr
    local all_ports, dev_ports, devNum
    all_ports = getGlobal("ifRange")--get table of ports
    local NA_NUMBER=30
    local sizeTable, totalPacketCount
    local macDaStr=params["macDest"]["string"]
    local macSaS=params["macSource"]["string"]
    --configPtr building
    configPtr=
    {
    macDa={[0]=tonumber(string.sub(macDaStr,1,2),16),
               tonumber(string.sub(macDaStr,4,5),16),
               tonumber(string.sub(macDaStr,7,8),16),
               tonumber(string.sub(macDaStr,10,11),16),
               tonumber(string.sub(macDaStr,13,14),16),
               tonumber(string.sub(macDaStr,16,17),16)
           },
    macSa={[0]=tonumber(string.sub(macSaS,1,2),16),
               tonumber(string.sub(macSaS,4,5),16),
               tonumber(string.sub(macSaS,7,8),16),
               tonumber(string.sub(macSaS,10,11),16),
               tonumber(string.sub(macSaS,13,14),16),
               tonumber(string.sub(macSaS,16,17),16)
           },
    etherType=params["ethertype"]
    }

    if nil~=params["vid"] then  --vlan is enabled
        configPtr["vlanTagEnable"]=true
        configPtr["vid"]=params["vid"]
        if nil~=params["vpt"] then
            configPtr["vpt"]=params["vpt"]
        end
        if nil~=params["cfi"] then
            configPtr["cfi"]=params["cfi"]
        end
    else  --default
        configPtr["vlanTagEnable"]=false
    end

    if nil~=params["payload"] then
        if "payload-random"==params["payload"] then
            configPtr["payloadType"]="CPSS_DIAG_PG_PACKET_PAYLOAD_RANDOM_E"
        else
            configPtr["payloadType"]="CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E"

            local s=params["payload-cyclic"]
            local strLen=string.len(s)
            if strLen >= 16 then
              s=string.format("%016s",s)  --padding the number to 16 digits
            else
              local str= string.rep("0",16-strLen)
              s=s..str --right pad 0s to number till 16 digits
            end
            --  cut the number
            configPtr["cyclicPatternArr"]=
            {
              [0]=tonumber(string.sub(s,1,2),16),
              tonumber(string.sub(s,3,4),16),
              tonumber(string.sub(s,5,6),16),
              tonumber(string.sub(s,7,8),16),
              tonumber(string.sub(s,9,10),16),
              tonumber(string.sub(s,11,12),16),
              tonumber(string.sub(s,13,14),16),
              tonumber(string.sub(s,15,16),16)
            }
        end
    else  --default
        configPtr["payloadType"]="CPSS_DIAG_PG_PACKET_PAYLOAD_CYCLIC_E"
        configPtr["cyclicPatternArr"]=
        {
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          [0]=0
        }
    end

    if nil~=params["increment"] then
        configPtr["macDaIncrementEnable"]=true
    else  --default
        configPtr["macDaIncrementEnable"]=false
    end

    if nil~=params["length-random"] then
        configPtr["packetLengthType"]="CPSS_DIAG_PG_PACKET_LENGTH_RANDOM_E"
        if nil~=params["undersize-enable"] then
            configPtr["undersizeEnable"]=true
        else
            configPtr["undersizeEnable"]=false
        end
    else
        configPtr["packetLengthType"]="CPSS_DIAG_PG_PACKET_LENGTH_CONSTANT_E"  --default
        if nil~=params["length"] then
            configPtr["packetLength"]=params["length"]
        else
            configPtr["packetLength"]=64
        end
    end

    if nil~=params["ipg"] then
        configPtr["ipg"]=params["ipg"]
    else  --default
        configPtr["ipg"]=8
    end

    if nil~=params["packet-count"] then
        configPtr["transmitMode"]="CPSS_DIAG_PG_TRANSMIT_SINGLE_BURST_E"

        totalPacketCount=params["packet-count"]
        sizeTable=
        {
        CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1_E=1,
        CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_256_E=256,
        CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_512_E=512,
        CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1K_E=1024,
        CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_4K_E=4096,
        CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_64K_E=65536,
        CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_1M_E=1048576,
        CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_16M_E=16777216
        }
        for enum, multiplier in pairs(sizeTable) do
            local temp=totalPacketCount/multiplier
            if temp>0 and temp<8192 then
                local integral, fractional=math.modf(temp)
                if 0==fractional then
                    configPtr["packetCount"]=integral
                    configPtr["packetCountMultiplier"]= enum
                    break
                end
            end
        end


    else  --default
        configPtr["transmitMode"]="CPSS_DIAG_PG_TRANSMIT_CONTINUES_E"
    end



    -- setting configPtr for all ports
    for devNum, dev_ports in pairs(all_ports) do
        for k, port in pairs(dev_ports) do
            connect=true
            ret = myGenWrapper("cpssDxChDiagPacketGeneratorConnectSet",{  --set configure:
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_BOOL","connectPtr",connect},
                {"IN","CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC","configPtr", configPtr}
                })
            if 0~=ret then
                if ret==NA_NUMBER then
                    print("Error at traffic generator configure setting: "..returnCodes[ret])
                    break
                end
                print(devNum.."/"..port..": Error at traffic generator configure setting: "..returnCodes[ret])
            end
        end
    end
end


-- ************************************************************************
---
--  trafficGeneratorConnect
--        @description  connect/disconnect the interface MAC to packet generator
--
--        @param params             - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function trafficGeneratorConnect(params)
    local ret, val
    local connect, configPtr
    local port,devNum, devPort
    local all_ports = getGlobal("ifRange")--get table of ports
    local NA_NUMBER=30

        -- setting connect for all ports
    for devNum, dev_ports in pairs(all_ports) do
        for k, port in pairs(dev_ports) do
            ret,val = myGenWrapper("cpssDxChDiagPacketGeneratorConnectGet",{  --get configure:
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"OUT","GT_BOOL","connectPtr"},
                {"OUT","CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC","configPtr"}
                })

            if 0~=ret then
                if ret==NA_NUMBER then
                    print("Error at traffic generator configure getting: "..returnCodes[ret])
                    break
                end
                print(devNum.."/"..port..": Error at traffic generator configure getting: "..returnCodes[ret])
            else  --configure
                configPtr=val["configPtr"]
                if nil==params["flagNo"] then
                    connect=true
                else
                    connect=false
                end

                if true==val["connectPtr"] and false==connect then  --stop Transmission first
                    trafficGeneratorTransmit({flagNo=true})
                    print("Transmittion was stopped in order to disconnect")  --*******************************************************************************

                end

                ret = myGenWrapper("cpssDxChDiagPacketGeneratorConnectSet",{  --set connection:
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                    {"IN","GT_BOOL","connectPtr",connect},
                    {"IN","CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC","configPtr", configPtr}
                    })
                if 0~=ret then
                    print(devNum.."/"..port..": Error at traffic generator configure setting: "..returnCodes[ret])
                end

            end
        end
    end
end






-- ************************************************************************
---
--  trafficGeneratorTransmit
--        @description  enable/disable transmission on the interface
--
--        @param params             - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
function trafficGeneratorTransmit(params)
    local ret, val
    local transmit
    local port,devNum, devPort
    local all_ports = getGlobal("ifRange")--get table of ports
    local NA_NUMBER=30
        -- setting TRANSMIT for all ports
    for devNum, dev_ports in pairs(all_ports) do
        for k, port in pairs(dev_ports) do
            ret,val = myGenWrapper("cpssDxChDiagPacketGeneratorConnectGet",{  --get configure:
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"OUT","GT_BOOL","connectPtr"},
                {"OUT","CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC","configPtr"}
                })

            if 0~=ret then
                if ret==NA_NUMBER then
                    print("Error at traffic generator configure getting: "..returnCodes[ret])
                    break
                end
                print(devNum.."/"..port..": Error at traffic generator configure getting: "..returnCodes[ret])
            else  --configure
                if false==val["connectPtr"]  then
                    print(devNum.."/"..port..": Not connected. only one of every four adjacent ports can be connected")
                else
                    if nil==params["flagNo"] then
                        transmit=true
                    else
                        transmit=false
                    end

                    ret = myGenWrapper("cpssDxChDiagPacketGeneratorTransmitEnable",{  --set configure:
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                    {"IN","GT_BOOL","connectPtr",transmit}
                    })
                    if 0~=ret then
                        print(devNum.."/"..port..": Error at traffic generator transmit setting: "..returnCodes[ret])
                    end
                end
            end
        end
    end
end


--------------------------------------------------------------------------------
-- command registration: traffic-generator configure
--------------------------------------------------------------------------------

CLI_addHelp("interface", "traffic-generator", "Handle traffic generator")
CLI_addCommand("interface", "traffic-generator configure", {
  func=trafficGeneratorConfigure,
  help = "Configure traffic generator and make connection ",
  params={
    {type="named",
      { format="mac-dest %mac-address", name="macDest", help="Destination MAC" },
      { format="increment %GT_U32", name="increment", help="Increment destination MAC for every packet" },
      { format="mac-source %mac-address", name="macSource" , help="Source MAC"},
      { format="vid %vlan", name="vid", help="Configure VLAN" },
      { format="cfi %cfi", name="cfi", help="Configure CFI to the VLAN tag " },
      { format="vpt %vpt", name="vpt", help="Configure VPT to the VLAN tag" },
      { format="ethertype %GT_U16", name="ethertype", help="Configure the ethertype to be sent" },
      { format="payload-cyclic %payload-cyclic", name="payload", help="Enter payload in numeric format" },
      { format="payload-random", name="payload", help="payload in random format"},
      "#trafficGenLengthType",
      { format="undersize-enable", name="undersize-enable", help="Enable undersized packets transmission" },
      { format="packet-count %packetNumberTransmit", name="packet-count", help="Packet count to transmit in single burst" },
      { format="ipg %GT_U16", name="ipg", help="Interpacket gap in bytes" },

  requirements = {
  ["cfi"] = {"vid"},
  ["vpt"] = {"vid"},
  ["undersize-enable"]={"length-random"},
        },
  mandatory = {"macDest", "macSource", "ethertype"}
    }
  }
})


--------------------------------------------------------------------------------
-- command registration: traffic-generator connect
--------------------------------------------------------------------------------

CLI_addCommand("interface", "traffic-generator connect", {
  func=trafficGeneratorConnect,
  help = "Connect the interface to traffic generator "
})
--------------------------------------------------------------------------------
-- command registration: no traffic-generator connect
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no traffic-generator connect", {
    func=function(params)
        params.flagNo=true
        return trafficGeneratorConnect(params)
    end,
  help   = "Disconnect the interface from traffic generator ",
})


--------------------------------------------------------------------------------
-- command registration: traffic-generator transmit
--------------------------------------------------------------------------------

CLI_addCommand("interface", "traffic-generator transmit", {
  func=trafficGeneratorTransmit,
  help = "Transmit traffic on the interface"
})
--------------------------------------------------------------------------------
-- command registration: no traffic-generator transmit
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no traffic-generator transmit", {
    func=function(params)
        params.flagNo=true
        return trafficGeneratorTransmit(params)
    end,
  help   = "Stop transmit traffic on the interface",
})
