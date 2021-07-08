--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* traffic_generator_emulate.lua
--*
--* DESCRIPTION:
--*       'emulate' external 'traffic generator' such as : smartBits / IXIA,
--*       to inject packet(s) into specific ingress port of the device.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
cmdLuaCLI_registerCfunction("prvLuaTgfPortForceLinkUpEnableSet")

function traffic_generator_emulate_port_info_get(command_data, devNum, portNum)
    local rc,mirrorIndex = prvLuaTgfPortInWsModeInfoGet(devNum, portNum)
    local isInWsMode

    if rc ~= 0 or mirrorIndex == nil then
        -- the port not in WS mode
        isInWsMode = false
    else
        isInWsMode = true
    end

    local isError = false

    return isError,isInWsMode,mirrorIndex
end

-- rate of 64 Byets packets in simulation per second (pps)
local wirespeed_rate_on_simulation = nil
function get_wirespeed_rate_on_simulation(devNum,portNum)
    if wirespeed_rate_on_simulation ~= nil and
       wirespeed_rate_on_simulation ~= 0 then
        -- already calculated WS rate
        return wirespeed_rate_on_simulation
    end
    local interval = 1000
    local rc,port_max,port_rx,port_tx =
        prvLuaTgfPortsRateGet(1 , 1000 , devNum,portNum,64);

    wirespeed_rate_on_simulation = port_tx

    if wirespeed_rate_on_simulation == 0 then
        -- traffic is currently not generated on the port
        local transmitInfo = {
            devNum = devNum,
            portNum = portNum,
            pktInfo = {fullPacket = "00000000000e00000000000d"} ,
            burstCount = transmit_continuous_wire_speed
            }

        printLog("ATTENTION: start to generate 'wirespeed' traffic to calculate simulation rate of wirespeed")

        local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)
        if rc == 0 then
            local rc,port_max,port_rx,port_tx =
                prvLuaTgfPortsRateGet(1 , 1000 , devNum,portNum,64);

            wirespeed_rate_on_simulation = port_tx

            -- stop the traffic
            transmitInfo.burstCount = stop_transmit_continuous_wire_speed
            local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)

            printLog("ATTENTION: stopped generate 'wirespeed' traffic to calculate simulation rate of wirespeed")
        else
            wirespeed_rate_on_simulation = 0
        end
    end

    -- print the calculated value
    printLog("wirespeed_rate_on_simulation (64B PPS)",to_string(wirespeed_rate_on_simulation))

    return wirespeed_rate_on_simulation
end

-- return padded packet if needed
function pad_packet_format(packet,minLen)
    local minimalPacketSizeNoCrc = 60
    packet = packet:gsub("\"", "")  -- removing quotes from data param
    --logic copied from sendPacket_41()
    if (minLen==nil) then
        minLen = minimalPacketSizeNoCrc
    end

    if (minLen < minimalPacketSizeNoCrc) then
        minLen = minimalPacketSizeNoCrc
    end

    -- add padding to minLen
    if string.len(packet) < (minLen*2) then
        packet = packet .. string.rep("0",minLen*2-string.len(packet))
    end

    if 1 == (string.len(packet) % 2) then
        -- add '0' nibble
        packet = packet .. "0"
    end

    return packet
end

--
-- on error - return true , error string
-- on success return packet_data
--
local function func_get_packet_format(params,minLen)
    local ret, status

    -- logic copied from function cmdSendPacket()
    if nil~=params.file then
        local filename=params.file
        local fd,e=fs.open(filename)

        if fd == nil then
            -- error
            return true , "failed to open file " .. filename .. ": "..e
        end

        local data=""
        local str, e = fs.gets(fd)
        while (str~=nil) do
            str=string.gsub(str,"\n","")
            data=data..str
            str, e = fs.gets(fd)
        end
        fs.close(fd)
        if e ~= "<EOF>" then
            -- error
            return true, "error reading file: " .. e
        else
            return pad_packet_format(data,minLen)
        end
    end

    if nil==params.data then
        -- error
        return true, "not valid data and no valid file ?!"
    end

    return pad_packet_format(params.data,minLen)
end
-- send the packet 'per port'
local function func_traffic_generator_emulate__per_port(command_data, devNum, portNum, params)
    local transmitInfo = {
        devNum = devNum,
        portNum = portNum,
        pktInfo = {fullPacket = params.packet_data} ,
        burstCount = params.count -- nil / number / "continuous-wire-speed"
        }

    -- force link up on port that is link down
    local apiName = "cpssPxPortLinkStatusGet"
    local isError , result, values =
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",      "devNum",     devNum },
        { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum },
        { "OUT",    "GT_BOOL",    "isLinkUpPtr"}
    })
    command_data.dontStateErrorOnCpssFail = nil
    if isError then
        return
    end

    if values.isLinkUpPtr == false then
        -- force link up on port that is link down
        -- this function will also set proper speed for the port
        prvLuaTgfPortForceLinkUpEnableSet(devNum,portNum,true)
    end

    -- transmit packet
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)
    if rc ~= 0 then
        print(string.format("ERROR : failed (dev/port[%d/%d])",devNum,portNum))

        command_data:addError("ERROR : the transmit of packet(s) failed  . (dev/port[%d/%d])" .. returnCodes[rc],
            devNum,portNum)
        command_data:setFailStatus()
    end
end

-- send the packet to needed port(s)
local function func_traffic_generator_emulate(params)
    local command_data = Command_Data()

    -- next needed for the iterators
    params.all_interfaces = "all"
    params.ethernet = params.port

    --print(to_string(params))

    local val1,val2 = func_get_packet_format(params)

    if val1 == true then
        -- error
        command_data:addError("ERROR:" .. val2)
        command_data:setFailStatus()
    else
        params.packet_data = val1
        params.command_data = command_data
        params.bypass_final_return = true

        generic_port_range_func(func_traffic_generator_emulate__per_port,params)
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function CLI_check_param_FWS(param,name,desc,varray,params)
    --print("CLI_check_param_FWS")
    if param == transmit_continuous_wire_speed then
        --filled by command
        return true,transmit_continuous_wire_speed
    end
    return false,""
end

local function CLI_complete_param_FWS(param,name,desc,varray,params)
    --print("CLI_complete_param_FWS")
    return transmit_continuous_wire_speed , "- option to generate continuous wire-speed"
end

local help_count_or_ws = "the number of packets to send or 'continuous-wire-speed'. (default is 1)."

local function CLI_check_param_number__count_or_ws(param,name,desc,varray,params)
    --print("CLI_check_param_number__count_or_ws")
    local my_dictionary = {min = 1 , max = 10000}
    return CLI_check_param_number(param,name,my_dictionary,varray,params)
end
local function CLI_complete_param_number__count_or_ws(param,name,desc,varray,params)
    --print("CLI_complete_param_number__count_or_ws")
    local my_dictionary = {min = 1 , max = 10000}
    local ret1,ret2 =  CLI_complete_param_number(param,name,my_dictionary,varray,params)
    return ret1,ret2
end
CLI_type_dict["count_or_ws"] = {
    checker  = {CLI_check_param_number__count_or_ws      , CLI_check_param_FWS},
    complete = {CLI_complete_param_number__count_or_ws   , CLI_complete_param_FWS},
    help=help_count_or_ws --[[actually set in runtime by proper 'complete' function]]
}

--[[
Command "traffic-generator emulate"
Command to 'emulate' external 'traffic generator' such as : smartBits / IXIA, to inject packet(s) into specific ingress port of the device.

Generate traffic by sending packet(s) from CPU to egress from specific port.
Since the port is set into 'mac loopback' mode , the traffic will 'ingress' this port ,
and start 'packet processing' in the device.

NOTE: the command allow to send explicit number of packets or 'continuous-wire-speed'.
a.	When 'explicit number of packets' the commands ended when all packets are sent.
b.	When 'continuous-wire-speed' the command return after setting proper configurations and generating traffic for this mode.
In this case the traffic continue to be 'injected' to the ingress port using SRC port mapping mechanism.
In order to 'stop' this continuous-wire-speed traffic , must call the 'no traffic-generator emulate'  on the port

Format
traffic-generator emulate {port <devPort>} {data <packet-data> | file <file-name>} [count <count | continuous-wire-speed>]
]]
CLI_addHelp("traffic", "traffic-generator", "allow to generate traffic without external 'traffic generator' such as : smartBits / IXIA.")
CLI_addCommand("traffic", "traffic-generator emulate", {
    func = func_traffic_generator_emulate,
    help="Emulate external 'traffic generator' such as : smartBits / IXIA, to inject packet(s) into specific ingress port of the device.",
    params={ { type="named",
        { format="port %port-range",name = "port" ,help="The device and port number" },
        "#dataOrFilename",
        { format="count %count_or_ws",name="count", help="(optional) " .. help_count_or_ws },
        mandatory={"inputString"--[[come from #dataOrFilename]]},--port is also mandatory , comes from 'requirements'
        requirements={["file"]={"port"},["data"]={"port"}}
      }}}

)

local function func_traffic_generator_emulate_stop__per_port(command_data, devNum, portNum, params)
    local isError,isInWsMode,mirrorIndex = traffic_generator_emulate_port_info_get(command_data, devNum, portNum)
    if isError == true or isInWsMode == false then
        -- the port not in WS mode ... skip it
        return
    end

    local transmitInfo = {
        devNum = devNum,
        portNum = portNum,
        burstCount = stop_transmit_continuous_wire_speed
        }
    -- stop the transmit ... see stop_transmit_continuous_wire_speed
    local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,nil)
    if rc ~= 0 then
        command_data:addError("ERROR : stopping the send failed . (dev/port[%d/%d])" .. returnCodes[rc],
            devNum,portNum)
        command_data:setFailStatus()
    end
end

local function func_traffic_generator_emulate_stop(params)
    -- next needed for the iterators
    params.all_interfaces = "all"
    params.ethernet = params.port

    return generic_port_range_func(func_traffic_generator_emulate_stop__per_port,params)
end

--[[
'no' command
The command needed to be called only in order to 'stop' the 'continuous-wire-speed' sending.
(and ignored for ports that are not under 'continuous-wire-speed')

Format:
no traffic-generator emulate {port <devPort>}

Parameters:
  port                 The device and port number
]]
CLI_addHelp("traffic", "no", "options to restore previous settings")
CLI_addHelp("traffic", "no traffic-generator", "stop the continuous wire-speed traffic ingress a port")
CLI_addCommand("traffic", "no traffic-generator emulate", {
    func = func_traffic_generator_emulate_stop,
    help="stop the continuous wire-speed traffic ingress a port",
    params={ { type="named",
        { format="port %port-range",name = "port" ,help="The device and port number" },
        mandatory={"port"}
      }}}
)
