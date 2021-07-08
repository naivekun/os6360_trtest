--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_traffic.lua
--*
--* DESCRIPTION:
--*       show for the 'traffic' context
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("prvLuaTgfPortInWsModeInfoGet")
cmdLuaCLI_registerCfunction("prvLuaTgfPxPortsRateGet")

--includes
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

--constants
--##################################
--##################################
local function per_port__show_traffic_traffic_generator_emulate(command_data, devNum, portNum, params)
    local isError,isInWsMode,mirrorIndex = traffic_generator_emulate_port_info_get(command_data, devNum, portNum)
    if isError == true or isInWsMode == false then
        -- the port not in WS mode ... skip it
        return
    end

    -- Resulting string formatting and adding.
    local devPortStr=tostring(devNum).."/"..tostring(portNum)
    command_data["result"] =
        string.format("%-11s", devPortStr)

    command_data:addResultToResultArray()
end


local function show_traffic_traffic_generator_emulate(params)
    local header_string, footer_string
    params.all_interfaces="ethernet"

    header_string =
        "the ports that are currently in wirespeed mode \n" ..
        "Dev/Port   \n" ..
        "---------  \n"
    footer_string = "\n"

    params.header_string = header_string
    params.footer_string = footer_string

    return generic_all_ports_show_func(per_port__show_traffic_traffic_generator_emulate,params)
end
local help_string_show_traffic_traffic_generator_emulate = "show the ports that currently generated wirespeed traffic"
CLI_addHelp("exec", "show traffic traffic-generator emulate", help_string_show_traffic_traffic_generator_emulate)
CLI_addCommand("exec", "show traffic traffic-generator emulate", {
  func   = show_traffic_traffic_generator_emulate,
  help   = help_string_show_traffic_traffic_generator_emulate,
  params = {
      { type = "named",
         { format = "port %port-range", name="ethernet",
           help = "Ethernet interface to configure"},
          mandatory = { "ethernet" }
      }
  }
})
-- function to calculate percentage
function get_percent(partialValue,maxValue)
    if type(maxValue) ~= "number" then
        return 0
    end
    if maxValue == 0 then
        -- avoid divide by 0
        return 0
    end
    local value = (100*partialValue) / maxValue
    --[[
    printLog(string.format("returned from get_percent(%d,%d)",
                partialValue,maxValue),
            value)
    ]]
    return value
end

local function show_traffic_rates(params)
    local command_data = Command_Data()
    params.command_data = command_data
    params.bypass_final_return = true
    params.all_interfaces="ethernet"

    local packetSize = params.type_traffic_packet_size_with_crc
    if packetSize == nil then
        -- default for this optional parameter
        packetSize = 64
    end

    local header_string, footer_string

    header_string =
        "max-rate - kbps that the port allow with packet size=" .. packetSize .. " \n" ..
        "actual-tx-rate - kbps that egress the port (+ % from 'max')\n" ..
        "actual-rx-rate - kbps that ingress the port(+ % from 'max')\n" ..
        "Dev/Port   max-rate  actual-tx-rate    actual-rx-rate     \n" ..
        "---------  --------  ---------------   --------------- "
    footer_string = "\n"

    params.header_string = header_string
    params.footer_string = footer_string

    -- time between checks (milisec)
    local interval = 1000

    local luaWrapperParams = {}
    local index = 1
    local total_ports = 0

    local port_max = {}
    local port_tx = {}
    local port_rx = {}
    local rc, ii
    local cpu_exist = false

    local function print_specific_entry(command_data,total_ports)
        for ii = 1,total_ports do
            local devNum  = luaWrapperParams[(ii-1)*3 + 1]
            local portNum = luaWrapperParams[(ii-1)*3 + 2]

            if port_max[ii] == 0xFFFFFFFF then -- some kind of error
                port_max[ii] = "error"
            end

            local rx_rate = port_rx[ii]
            if rx_rate == 0 then rx_rate = "---" end

            local tx_rate = port_tx[ii]
            if tx_rate == 0 then tx_rate = "---" end

            local devPortStr=tostring(devNum).."/"..tostring(portNum)
            command_data["result"] =
                string.format("%-11s %-10s (%d%%)%-13s (%d%%)%-13s", devPortStr,
                    tostring(port_max[ii]),
                    get_percent(port_tx[ii],port_max[ii]),tostring(tx_rate),
                    get_percent(port_rx[ii],port_max[ii]),tostring(rx_rate) )

            command_data:addResultToResultArray()
        end
    end


    -- aggregate the ports
    local function per_port__show_traffic_rates(command_data, devNum, portNum, params)
        --printFromShow("portNum",portNum)

        if portNum == 63 then
            cpu_exist = true
            return
        end

        luaWrapperParams[index] = devNum
        index = index + 1

        luaWrapperParams[index] = portNum
        index = index + 1

        luaWrapperParams[index] = packetSize
        index = index + 1

        total_ports = total_ports + 1

        if((total_ports % 4) == 0) then
            rc,port_max[1],port_rx[1],port_tx[1],
               port_max[2],port_rx[2],port_tx[2],
               port_max[3],port_rx[3],port_tx[3],
               port_max[4],port_rx[4],port_tx[4] =
            prvLuaTgfPxPortsRateGet(total_ports , interval ,
                luaWrapperParams[0*3 + 1],
                luaWrapperParams[0*3 + 2],
                luaWrapperParams[0*3 + 3],

                luaWrapperParams[1*3 + 1],
                luaWrapperParams[1*3 + 2],
                luaWrapperParams[1*3 + 3],

                luaWrapperParams[2*3 + 1],
                luaWrapperParams[2*3 + 2],
                luaWrapperParams[2*3 + 3],

                luaWrapperParams[3*3 + 1],
                luaWrapperParams[3*3 + 2],
                luaWrapperParams[3*3 + 3]
                )


            index = 1
            total_ports = 0

            if rc ~= 0 then
                command_data:addError("ERROR : prvLuaTgfPxPortsRateGet failed ." .. returnCodes[rc])
                command_data:setFailStatus()
                return
            end

            -- print the info
            print_specific_entry(command_data,4)
        end
    end

    generic_all_ports_show_func(per_port__show_traffic_rates,params)

    --printFromShow("after generic_all_ports_show_func")


    if((total_ports % 4) ~= 0) then
        rc,port_max[1],port_rx[1],port_tx[1],
           port_max[2],port_rx[2],port_tx[2],
           port_max[3],port_rx[3],port_tx[3],
           port_max[4],port_rx[4],port_tx[4] =
        prvLuaTgfPxPortsRateGet(total_ports , interval ,
            luaWrapperParams[0*3 + 1],
            luaWrapperParams[0*3 + 2],
            luaWrapperParams[0*3 + 3],

            luaWrapperParams[1*3 + 1],
            luaWrapperParams[1*3 + 2],
            luaWrapperParams[1*3 + 3],

            luaWrapperParams[2*3 + 1],
            luaWrapperParams[2*3 + 2],
            luaWrapperParams[2*3 + 3]
            )

        --printFromShow("total_ports",total_ports)

        -- print the info
        print_specific_entry(command_data,total_ports)
    end

    if rc ~= 0 then
        command_data:addError("ERROR : prvLuaTgfPxPortsRateGet failed ." .. returnCodes[rc])
        command_data:setFailStatus()
    end

    local function calculate_cpu_rate(command_data,devNum)
        local rxQueue = 0
        local delayTimeInSec = 1
        local counter2 = 0
        -- reset counters
        local rc1, rxInPkts1
        for rxQueue=0,7,1 do
            rc1, rxInPkts1 = rx_sdma_counters_get(devNum, rxQueue)
        end

        delay(1000 * delayTimeInSec)

        local rc2, rxInPkts2
        for rxQueue=0,7,1 do
            rc2, rxInPkts2 = rx_sdma_counters_get(devNum, rxQueue)
            counter2 = counter2 + rxInPkts2
        end
        local diff = counter2 / delayTimeInSec
        -- the SDMA CPU port hold no counter from CPU to device , only from device to CPU
        port_tx[1] = diff
        port_rx[1] = 0
        port_max[1] = "N/A" -- we have no know 'max rate' --> will print '0' as max rate
        luaWrapperParams[2] = "CPU(63)"
        print_specific_entry(command_data,1)
    end

    if cpu_exist == true then
        calculate_cpu_rate(command_data,devNum)
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end

local help_type_traffic_packet_size_with_crc = "The packet size (including CRC !) for rate calculations"
CLI_type_dict["type_traffic_packet_size_with_crc"] = {
    checker = CLI_check_param_number,
    min=64,
    max=10*1024,
    complete = CLI_complete_param_number,
    help=help_type_traffic_packet_size_with_crc
}

local help_string_show_traffic_rates = "show the traffic rates of ports"
CLI_addHelp("exec", "show traffic rates", help_string_show_traffic_rates)
CLI_addCommand("exec", "show traffic rates", {
  func   = show_traffic_rates,
  help   = help_string_show_traffic_rates,
  params = {
      { type = "named",
            { format = "port %port-range", name="ethernet",
                help = "Ethernet interface to configure"},
          mandatory = { "ethernet" }
      }
  }
})

-- rx_sdma_counters_get
--  get values RX SDMA counters for queue
-- returns result, rxInPkts, rxInOctets
--
function rx_sdma_counters_get(devNum,queue)
    local result, values =  myGenWrapper("cpssPxNetIfSdmaRxCountersGet", {
                                            { "IN",  "GT_U8",    "devNum", devNum },
                                            { "IN",  "GT_U8",    "queueIdx", queue },
                                            { "OUT", "CPSS_PX_NET_SDMA_RX_COUNTERS_STC", "rxCountersPtr"}
                                        })

    local rxInPkts, rxInOctets
    if result ~= 0 then
        rxInPkts = 0
        rxInOctets = 0
    else
        rxInPkts = values.rxCountersPtr.rxInPkts
        rxInOctets = values.rxCountersPtr.rxInOctets
    end

    return result, rxInPkts, rxInOctets
end

-- rx_sdma_counters_get_and_print
--  create output string for RX SDMA counters for queue
--
local function rx_sdma_counters_get_and_print(command_data, devNum, queue)
    local result, rxInPkts, rxInOctets = rx_sdma_counters_get(devNum, queue)

    if result ~= 0 then
        command_data:setFailDeviceAndLocalStatus()
        command_data:addError("Error at RX SDMA Counters get" ..
                          " on device %d queue %d: %s",
                          devNum, queue, returnCodes[result])
        return
    end

    -- Resulting string formatting and adding.
    local prefix=tostring(devNum).."/"..tostring(queue)

    command_data["result"] =
        string.format("%-14s%-11s%-11s", prefix, tostring(rxInPkts),tostring(rxInOctets))

    command_data:addResultToResultArray()
    command_data:updateEntries()
end

-- per_device__show_cpu_rx_stat
--  per device iterator for RX SDMA info print
--
local function per_device__show_cpu_rx_stat(command_data,devNum,params)
    local queue
    for queue = 0,7 do
        rx_sdma_counters_get_and_print(command_data,devNum,queue)
    end
end

local function show_cpu_rx_stat(params)
    params.header_string =
        "\n" ..
        "Dev/RX queue  packets    bytes\n" ..
        "------------  ---------- ----------\n"
    params.footer_string = "\n"

    return generic_all_device_show_func(per_device__show_cpu_rx_stat,params)
end

--[[
Command show traffic cpu rx statistic
The command shows RX SDMA counter.
]]
CLI_addHelp("exec", "show traffic cpu rx statistic", "show traffic CPU RX statistic")
CLI_addCommand("exec", "show traffic cpu rx statistic", {
    func = show_cpu_rx_stat,
    help="show traffic CPU RX statistic",
    params={
        {   type="named",
            "#all_device",
            mandatory={"all_device"}
        }
    }}
)

