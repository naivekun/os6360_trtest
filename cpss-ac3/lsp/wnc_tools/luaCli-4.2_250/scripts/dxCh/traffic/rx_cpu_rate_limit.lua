--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* rx_cpu_rate_limit.lua
--*
--* DESCRIPTION:
--*       RX CPU rate limit configurations and show commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

-- constants
local CPSS_NET_ALL_CPU_OPCODES_E                  = 0x7FFFFFFF

-- Window Granularity in seconds
local globalWindowGranularity  = 0.0001

-- 16 bits in entry
local maxRateEntryWindow  = 4095
local maxRateEntryPacketLimit  = 65535

-- device does not work good with small packet limit value and high (10G rate and more)
-- rates. Use this minimal value for Packet Limit in HW.
local minPacketLimit = 15

local diffThreshould = 0.001 -- in percent / 100, 0.01 means 1%

-- maximal rate in PPS
local maxRateInPps = math.floor(maxRateEntryPacketLimit / globalWindowGranularity)

--DB that holds pair {"rateLimit", index}
local rateLimitDb = {}

-- Rate Limiters Free Entries DB 
-- entries 1-31 initially free
local rateLimitFreeMemoDB = {{base = 1, size = 31}};

--
--  dsa_cpu_code_to_api_convert
--     function converts DSA CPU Code number to value from CPSS_NET_RX_CPU_CODE_ENT
--
function dsa_cpu_code_to_api_convert(dsa_cpu_code)
    -- CPU Code CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + given HW value
    return 1024 + dsa_cpu_code
end

--
-- cpu_rate_limit_from_entry_calc
--    function calculates rate in PPS from rate limit entry
--
function cpu_rate_limit_from_entry_calc(entry_window, entry_packets_limit)
    return (entry_packets_limit/(entry_window * globalWindowGranularity))
end

--
-- calc_window
--   function calculates limit entry window for given rate limit and packet limit
--
local function calc_window(rateLimit, packetLimit)
    local window = math.floor(packetLimit/(rateLimit * globalWindowGranularity))
    if window < 1 then
        window = 1
    elseif window > maxRateEntryWindow then
        window = maxRateEntryWindow
    end
    return window
end

--
-- calc_rate_diff
--   function calculates difference in given and current rate in % / 100 units
--
local function calc_rate_diff(rateLimit, window, packetLimit)
    local rate = cpu_rate_limit_from_entry_calc(window, packetLimit)
    local diff = math.abs(rate - rateLimit)

    return diff / rateLimit
end
-- ************************************************************************
---
--  toCpuRateLimitEnrtyValuesGet
--        @description  Calculate window and packet limit for rate limit entry 
--
--        @param rateLimit   - value of rate limit in PPS
--
--        @return       entry window size, entry packets limit
--
function toCpuRateLimitEnrtyValuesGet(rateLimit)
    local outPutPacketLimit, window, outPutWindow

    if rateLimit > maxRateInPps then
        outPutWindow = 1
        outPutPacketLimit = maxRateEntryPacketLimit
    elseif rateLimit == 0 then
        outPutWindow = 1
        outPutPacketLimit = 0
    else
        local minWindow = calc_window(rateLimit, minPacketLimit)
        local maxWindow = calc_window(rateLimit, maxRateEntryPacketLimit)
        local rateGran, rateDiff, minDiff, bestWindow, bestPacketLimit

        minDiff = maxRateInPps + 1 -- big number
        bestWindow = 1 -- 1 is best

        window = minWindow

        repeat
            -- find minimal window that comply with exit criteria defined by exitThreshould
            -- store window value for output
            outPutWindow = window

            -- calculate granularity of output rate for given window and one packet
            rateGran = cpu_rate_limit_from_entry_calc(window, 1)

            -- calculate packet limit for current window
            outPutPacketLimit = math.floor(window * globalWindowGranularity * (rateLimit + rateGran/2))

            -- calculate 
            rateDiff = calc_rate_diff(rateLimit, outPutWindow, outPutPacketLimit)
            if rateDiff < diffThreshould then
                return outPutWindow, outPutPacketLimit
            end
            
            if rateDiff < minDiff then
                minDiff = rateDiff
                bestWindow = outPutWindow
                bestPacketLimit = outPutPacketLimit
            end

            -- goto next window
            window = window + 1
        until outPutPacketLimit > maxRateEntryPacketLimit or window > maxRateEntryWindow

        outPutWindow = bestWindow
        outPutPacketLimit = bestPacketLimit
    end

    return outPutWindow, outPutPacketLimit
end

-- ************************************************************************
---
--  toCpuRateLimitIndexGet
--        @description  Allocare rate limiter for given rate limit
--                      Return already allocated entry if rate limit already exist
--                      Or allocates free entry 
--
--        @param devNum      - device number
--        @param rateLimit   - value of rate limit
--
--        @return       index if there was no error otherwise nil
--
function toCpuRateLimitIndexGet(devNum, rateLimit)
    local index
    local key

    key = tostring(rateLimit)
    if rateLimitDb[key] ~= nil then
        -- rate limit already exist -  use it
        index = rateLimitDb[key]
    else
        -- get free index
        index = DbMemoAlloc(rateLimitFreeMemoDB, 1)
        if index == nil then
            command_data:setFailDeviceAndLocalStatus()
            command_data:addError("Error at Rate Limiter allocatetion" ..
                                  " on device %d rateLimit %d", devNum, rateLimit)            
        end
        rateLimitDb[key] = index
    end
    return index
end
-- ************************************************************************
---
--  func_cpu_rx_rate_limit
--        @description  Configure CPU code table and rate limiter 
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
function func_cpu_rx_rate_limit(params)
    local result, values
    local devNum, rateLimit
    local cpuCode
    local command_data = Command_Data()
    local entry
    local rateLimiterIndex
    local entryWindow, entryPacketLimit
    local i
    
    -- Common variables initialization
    command_data:initAllDeviceRange(params)

    --print(to_string(params))
    rateLimit = params["rate_limit"]
        
    entry = {
        tc = 0,
        dp = 0, -- Green
        truncate = 0,
        cpuRateLimitMode = 0, --CPSS_NET_CPU_CODE_RATE_LIMIT_LOCAL_E
        cpuCodeRateLimiterIndex = 0,
        cpuCodeStatRateLimitIndex = 0,
        designatedDevNumIndex = 0
    }

    local iterator
    for iterator, devNum in command_data:getDevicesIterator() do
        command_data:clearLocalStatus()

        -- set "disable" value to support NO command
        rateLimiterIndex = 0

        -- Mac address adding condition getting.
        if true == command_data["local_status"] then
            if params.flagNo == nil then
                -- allocate rate limiter for given rate
                rateLimiterIndex = toCpuRateLimitIndexGet(devNum, rateLimit)                

                --print("Rate Limit index:", entry.cpuCodeRateLimiterIndex)
                if (rateLimiterIndex ~= nil) then

                    -- configure Rate Limit Window resolution
                    result = myGenWrapper("cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet",{
                                {"IN","GT_U8","devNum",devNum},
                                {"IN","GT_U32","windowResolution",(globalWindowGranularity * 1000000000)}
                            })

                    if (result ~= 0) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at Window Resolution set" ..
                                          " on device %d: %s",
                                          devNum, returnCodes[result])
                    end	

                    -- calculate window and packet limit
                    entryWindow, entryPacketLimit = toCpuRateLimitEnrtyValuesGet(rateLimit)
                    --print("entryWindow: ", entryWindow, "entryPacketLimit", entryPacketLimit, "rate", (entryPacketLimit/(entryWindow * globalWindowGranularity)) )
                     
                    -- configure Rate Limit Entry
                    result = myGenWrapper("cpssDxChNetIfCpuCodeRateLimiterTableSet",{
                                {"IN","GT_U8","devNum",devNum},
                                {"IN","GT_U32","rateLimiterIndex",rateLimiterIndex},
                                {"IN","GT_U32","windowSize",entryWindow},
                                {"IN","GT_U32","pktLimit",entryPacketLimit}
                            })
                    if (result ~= 0) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at Rate Limiter set" ..
                                          " on device %d index %d window %d pktLimit %d: %s",
                                          devNum, rateLimiterIndex, entryWindow, entryPacketLimit, returnCodes[result])
                    end	
                end
            end
            if (rateLimiterIndex ~= nil) then
                -- update rate limiter index in CPU code entry
                entry.cpuCodeRateLimiterIndex = rateLimiterIndex

                if (params.cpu_code_range ~= nil) then
                    for i,cpu_code in pairs(params.cpu_code_range) do                
                        -- CPU Code CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + given HW value
                        cpuCode = dsa_cpu_code_to_api_convert(cpu_code)
                        --print(cpu_code, cpuCode)
                        result = myGenWrapper("cpssDxChNetIfCpuCodeTableSet",{
                                    {"IN","GT_U8","devNum",devNum},
                                    {"IN",TYPE["ENUM"],"cpuCode",cpuCode},
                                    {"IN","CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC","entryInfoPtr",entry}
                                })

                        if (result ~= 0) then
                            command_data:setFailDeviceAndLocalStatus()
                            command_data:addError("Error at CPU Code table set" ..
                                              "on device %d CPU Code %d: %s",
                                              devNum, cpuCode, returnCodes[result])
                        end	
                    end
                else
                    cpuCode = CPSS_NET_ALL_CPU_OPCODES_E
                    --print("ALL CPU Codes")
                    result = myGenWrapper("cpssDxChNetIfCpuCodeTableSet",{
                                {"IN","GT_U8","devNum",devNum},
                                {"IN",TYPE["ENUM"],"cpuCode",cpuCode},
                                {"IN","CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC","entryInfoPtr",entry}
                            })

                    if (result ~= 0) then
                        command_data:setFailDeviceAndLocalStatus()
                        command_data:addError("Error at CPU Code table set" ..
                                          " on device %d CPU Code %d: %s",
                                          devNum, cpuCode, returnCodes[result])
                    end
                end
            end
        command_data:updateStatus()
        end
    end
    
    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

	return command_data:getCommandExecutionResults()
end

CLI_type_dict["rate_limit"] = {
    checker = CLI_check_param_number,
    min=0,
    max=maxRateInPps,
    complete = CLI_complete_param_number,
    help = "Rate limit value in packets per second"
}

--[[
Command cpu rx rate-limit.
The command configures CPU Code table and rate limiter table.
]]
CLI_addHelp("traffic", "cpu rx rate-limit", "limit RX CPU packets' rate") 
CLI_addCommand("traffic", "cpu rx rate-limit", {
    func = func_cpu_rx_rate_limit,
    help="limit RX CPU packets' rate",
    params={
        {   type="values", "%rate_limit" },
        {   type="named",
            "#all_device",
            { format="cpu-code %cpu_code_range", name="cpu_code_range", help = "Range of CPU Code numbers from functional specification"},
            mandatory={"all_device"}
        }
    }}        
)

--[[
Command no cpu rx rate-limit.
The command configures CPU Code table.
]]
CLI_addHelp("traffic", "no cpu rx rate-limit", "disable limit RX CPU packets' rate") 
CLI_addCommand("traffic", "no cpu rx rate-limit", {
    func=function(params)
        params.flagNo=true
        return func_cpu_rx_rate_limit(params)
    end,
    help="disable limit RX CPU packets' rate",
    params={
        {   type="named",
            "#all_device",
            { format="cpu-code %cpu_code_range", name="cpu_code_range", help = "Range of CPU Code numbers from functional specification"},
            mandatory={"all_device"}
        }
    }}        
)

