--********************************************************************************
--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* counters.lua
--*
--* DESCRIPTION:
--*       Centralized Counters (CNC) commands
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("luaCLI_convertPhysicalPortsToTxq")

--constants

local function dec2hex(IN)
    local B, K, OUT, I, D = 16, "0123456789ABCDEF", "", 0
    while IN > 0 do
        I = I + 1
        IN, D = math.floor(IN / B), math.mod(IN, B) + 1
        OUT = string.sub(K, D, D) .. OUT
    end
    return OUT
end

local function hex2dec(hex)
    local ans = 0
    local i, d
    d = 0
    for i = string.len(hex), 1, -1 do
        local char = string.sub(hex, i, i)
        local charAsNumber = tonumber(char, 16)
        ans = ans + math.pow(16, d) * charAsNumber
        d = d + 1
    end
    return ans
end

local function dec2bin(dec,length)
    dec = dec * 2
    local bin = ""
    if(length == nil) then length = 8 end
    for i = 0, (length-1) do
        bin = bin .. tostring(math.ceil(math.floor(dec / 2) % 2))
        dec = math.floor(dec / 2)
    end
    return string.reverse(bin)
end


-- ************************************************************************
--  showCountersACL
--
--  @description show CNC counters bound to PCL rules
--
--  @param params - params["devID"]     - device number.
--                  params["ruleIdx"]   - Range of the pclIds.
--
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************


function showCountersACL(params)
    local ret, val
    local i, j
    local blocksList, header, body, block
    local devNum = params["devID"]
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E = 0
    local maxRuleId = 1023
    local ruleIdx = params["ruleIdx"]
    local allFlag = false
    local oneRuleId = false
    local retTable = {}

    header = "\n"
    header = header .. " devID   rule-id   Packet-Counter     Byte-count counter         \n"
    header = header .. "---------------------------------------------------------------------------\n"
    body = ""
    if (ruleIdx == "all") then
        allFlag = true
        local ruleIdxTmp = {}
        for i = 0, maxRuleId do
            table.insert(ruleIdxTmp, i)
        end
        ruleIdx = ruleIdxTmp
    else
        ruleIdx = {ruleIdx}
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    if #ruleIdx == 1 and allFlag == false then
        oneRuleId = true
    end
    blocksList = getCNCBlock("IPCL_0")
    if blocksList == -1 then
      return false, "Wrong client name for selected device"
    end
    block = blocksList[1]

    local IPCL0_table = luaGlobalGet("IPCL_0")

    if (IPCL0_table ~= nil) then
        retTable["pass"] = {}
        for j = 1, #devices do -- main devices loop

            for i = 1, #ruleIdx do
              local counterPtr
              local index = ruleIdx[i]

              if IPCL0_table[index] ~= nil then
                ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                    { "IN", "GT_U8", "devNum", devices[j] },
                    { "IN", "GT_U32", "blockNum", block },
                    { "IN", "GT_U32", "index", index },
                    { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
                    { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                })

                if ret ~= 0 then
                    print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                    return false, "Error, cannot show counters acl"
                end

                counterPtr = val["counterPtr"]
                local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
                local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

                local byteCounterLSB = counterPtr["byteCount"]["l"][0]
                local byteCounterMSB = counterPtr["byteCount"]["l"][1]

                local packetsCounter, byteCounter

                -- converting the packet counter and the byte counter to 64 bit number
                packetsCounterLSB = dec2hex(packetsCounterLSB)
                packetsCounterMSB = dec2hex(packetsCounterMSB)
                packetsCounter = packetsCounterMSB .. packetsCounterLSB
                packetsCounter = hex2dec(packetsCounter)

                byteCounterLSB = dec2hex(byteCounterLSB)
                byteCounterMSB = dec2hex(byteCounterMSB)
                byteCounter = byteCounterMSB .. byteCounterLSB
                byteCounter = hex2dec(byteCounter)

                if (byteCounter ~= 0) then
                    local tuple = {
                        ["devID"] = devices[j],
                        ["rule-id"] = index,
                        ["Packet-Counter"] = packetsCounter,
                        ["Byte-count counter"] = byteCounter
                    }
                    table.insert(retTable["pass"], tuple)
                end

                if (oneRuleId == true) then
                    body = body .. "device:              " .. devices[j] .. "\n"
                    body = body .. "rule id:             " .. index .. "\n"
                    body = body .. "Packets counter:     " .. packetsCounter .. "\n"
                    body = body .. "Byte-count counter:  " .. byteCounter .. "\n"
                else
                    if (byteCounter ~= 0 or allFlag == false) then
                        body = body .. string.format("   %-6s %-7s %-20s %-20s\n",
                            devices[j],
                            index,
                            packetsCounter,
                            byteCounter)
                    end
                end
              end  -- if
            end -- for
        end -- for

        print("IPCL counters: Enabled")
        if (body == "") then
            print("There are no Counters to show")
        else
            if oneRuleId == false then
                print(header)
            end
            print(body)
        end
    else
        print("IPCL counters: Disabled")
    end

    return true, retTable
end

--------------------------------------------
-- command registration: show counters acl
--------------------------------------------

CLI_addCommand("exec", "show counters acl", {
    func = showCountersACL,
    help = "show CNC counters bound to pcl rules",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            { format = "%ruleIDorAll", name = "ruleIdx", help = "rule-id (0-16895)" },
            requirements = {
                ["ruleIdx"] = { "devID" }
            },
            mandatory = { "ruleIdx" }
        }
    }
})


-- ************************************************************************
--  showCountersEgressQueue
--
--  @description show CNC counters bound to Egress Queue client
--
--  @param params - params["devID"]     - device number.
--                  params["ruleIdx"]   - Range of the pclIds.
--
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************


function showCountersEgressQueue(params)
    local ret, val
    local i, j, k, d
    local blocksList, header, body
    local devNum
    local allFlag = false
    local port, dp, tc
    local dpBits, tcBits, portBits
    local index, blockSize, block
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E = 0
    local listOfDP = {}
    local listOfTC = {}
    local listOfPorts = {}
    local passAndDrop = {}
    local devices
    local retTable = {}
    local mapPhysicalPortToTxqPort
    local currPort

    local numberToColor = {
        [0] = "green",
        [1] = "yellow",
        [2] = "red",
        [3] = "DROP-any dp"-- sip_5_20
    }
    blocksList = getCNCBlock("EgressQueuePassAndDrop")
    if blocksList == -1 then
      return false, "Wrong client name for selected device"
    end
    body = ""
    header = "\n"
    header = header .. " port   TC   DP    Packet-Counter     Byte-count counter         \n"
    header = header .. "---------------------------------------------------------------------------\n"

    if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_LION2_E"] then
      blockSize = 512
    else
      blockSize = 1024 -- each block is 1k bytes
    end

    if (params["all"] == true) then
        allFlag = true
        devNum = params["devID"]

        for i = 0, 2 do
          table.insert(listOfDP, i)
        end

        for i = 0, 7 do
          table.insert(listOfTC, i)
        end

        if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_LION2_E"] then
          -- Only 6 bit's for Lion2
          for i = 0, 64 do
            table.insert(listOfPorts, i)
          end
        else
          for i = 0, 127 do
            table.insert(listOfPorts, i)
          end
        end

    else
        devNum = params["dev_port"]["devId"]
        port = params["dev_port"]["portNum"]
        tc = params["tc"]
        dp = params["dp"]

        table.insert(listOfDP, dp)
        table.insert(listOfTC, tc)
        table.insert(listOfPorts, port)
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    if (luaGlobalGet("EgressQueue") ~= nil) then
        table.insert(passAndDrop, "pass")
        retTable["pass"] = {}
        if (luaGlobalGet("EgressQueue") == "PassAndDrop") then
            table.insert(passAndDrop, "drop")
            retTable["drop"] = {}
        end


        for d = 1, #devices do -- main devices loop
            devNum = devices[d]

            if (is_sip_5(devNum)) then
                -- get TXQ iterator
                mapPhysicalPortToTxqPort = luaCLI_convertPhysicalPortsToTxq(devNum)
                --print("mapPhysicalPortToTxqPort",to_string(mapPhysicalPortToTxqPort))
                --[[ example of values in mapPhysicalPortToTxqPort -->
                    format is --> [phy_port]=txqPort
                    mapPhysicalPortToTxqPort        {
                      [0]=0,
                      [1]=1,
                      [2]=2,
                      [3]=3,
                      [65]=10,
                      [59]=8,
                      [63]=12,
                      [18]=4,
                      [80]=11,
                      [54]=6,
                      [64]=9,
                      [58]=7,
                      [36]=5
                    }
                ]]--
            end

            for j, passDrop in pairs(passAndDrop) do
                body = ""
                for i = 1, #listOfPorts do -- loop over the ports
                    local txPortNum = nil
                    if (is_sip_5(devNum)) then
                        currPort = listOfPorts[i]
                        txPortNum = mapPhysicalPortToTxqPort[currPort]
                        if txPortNum == nil then
                            txPortNum = 0
                        end
                    end

                    for j = 1, #listOfTC do -- loop over the traffic class
                        for k = 1, #listOfDP do -- loop over the drop precedence
                            local currTc = string.format("%03d", dec2bin(listOfTC[j]))
                            local currDp
                            local drop_cnc_offset = 4

                            if boardEnv.FamiliesList["CPSS_PP_FAMILY_DXCH_LION2_E"] then
                                currPort = string.format("%06d", dec2bin(listOfPorts[i]))
                                -- currDp = string.format("%02d", dec2bin(listOfDP[k]))

                                local dpLow = "0"
                                local dpHigh = "0"

                                if listOfDP[k]%2 > 0 then
                                  dpLow = "1"
                                end

                                if listOfDP[k]/2 > 0 then
                                  dpHigh = "1"
                                end
                                currDp = dpHigh .. dpLow

                                if (passDrop == "pass") then
                                  index = tonumber( dpHigh .. "0" .. currPort .. currTc .. dpLow, 2)
                                else
                                  index = tonumber( dpHigh .. "1" .. currPort .. currTc .. dpLow, 2)
                                end
                                block = math.floor(index / blockSize)
                                block = blocksList[block + 1]
                                index = index % blockSize
                            else
                              if (is_sip_5(devNum)) then
                                  currDp = string.format("%02d", dec2bin(listOfDP[k]))
                                  local local_dp = listOfDP[k]
                                  local sip5CurrDp;

                                  if (passDrop ~= "pass") then
                                      local_dp = 3
                                  end

                                  currPort = string.format("%10d", dec2bin(listOfPorts[i],10))
                                  
                                  local txPortNum_binary

                                  if (is_sip_6(devNum)) then
                                    -- support queues of ports from high tiles
                                    txPortNum = txPortNum  % 4096
                                  
                                    --  currTc used inside txPortNum_binary
                                    txPortNum_binary =
                                      string.format("%13s", dec2bin(txPortNum + currTc, 13))
                                  else
                                  -- base to concatenate TC
                                    txPortNum_binary =
                                      string.format("%10s", dec2bin(txPortNum, 10))
                                  end
                                  
                                  --print("txPortNum_binary",to_string(txPortNum_binary))

                                  if (is_sip_5_20(devNum)) then
                                    -- used Tail-Drop Reduced Counting Mode
                                    -- using single bit pass == 0 or drop == 1 instead of of DP
                                      local_dp = 0
                                      if (passDrop ~= "pass") then
                                          local_dp = 1
                                      end
                                      sip5CurrDp = string.format("%01d", dec2bin(local_dp))
                                  else
                                      sip5CurrDp = string.format("%02d", dec2bin(local_dp))
                                  end

                                  if (is_sip_6(devNum)) then
                                    --  currTc already used inside txPortNum_binary
                                    index = tonumber(txPortNum_binary ..  sip5CurrDp, 2)
                                  else
                                    index = tonumber(txPortNum_binary .. currTc .. sip5CurrDp, 2)
                                  end

                                  drop_cnc_offset = 0

                              else
                                  currPort = string.format("%07d", dec2bin(listOfPorts[i]))
                                  currDp = string.format("%02d", dec2bin(listOfDP[k]))
                                  index = tonumber(currPort .. currTc .. currDp, 2)
                              end
                              block = math.floor(index / blockSize)
                              --print("block",to_string(block))
                              if (passDrop == "pass") then
                                  block = blocksList[block + 1]
                              else
                                  block = blocksList[block + 1 + drop_cnc_offset]
                              end
                              index = index % blockSize
                            end

                            local counterPtr
                            ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                                { "IN", "GT_U8", "devNum", devNum },
                                { "IN", "GT_U32", "blockNum", block },
                                { "IN", "GT_U32", "index", index },
                                { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
                                { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                            })

                            if ret ~= 0 then
                                --print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret] .. "     idx= " .. tostring(index) .. "  block = " .. tostring(block))
                                --print("aIdx: " .. tostring(aIdx) .. " blockSize:  " .. tostring(blockSize) .. " currPort=" .. tostring(currPort) .. " currDp=" .. tostring(currDp) .. " currTc=" .. tostring(currTc))

                                print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                                return false, "Error, cannot show counters acl"
                            end

                            counterPtr = val["counterPtr"]
                            local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
                            local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

                            local byteCounterLSB = counterPtr["byteCount"]["l"][0]
                            local byteCounterMSB = counterPtr["byteCount"]["l"][1]

                            local packetsCounter, byteCounter

                            -- converting the packet counter and the byte counter to 64 bit number
                            packetsCounterLSB = dec2hex(packetsCounterLSB)
                            packetsCounterMSB = dec2hex(packetsCounterMSB)
                            packetsCounter = packetsCounterMSB .. packetsCounterLSB
                            packetsCounter = hex2dec(packetsCounter)

                            byteCounterLSB = dec2hex(byteCounterLSB)
                            byteCounterMSB = dec2hex(byteCounterMSB)
                            byteCounter = byteCounterMSB .. byteCounterLSB
                            byteCounter = hex2dec(byteCounter)

                            local portDev = devNum .. "/" .. tonumber(currPort, 2)

                            if (byteCounter ~= 0) then
                                local tuple = {
                                    ["port"] = portDev,
                                    ["TC"] = tonumber(currTc, 2),
                                    ["DP"] = numberToColor[tonumber(currDp, 2)],
                                    ["Packet-Counter"] = packetsCounter,
                                    ["Byte-count counter"] = byteCounter
                                }

                                table.insert(retTable[passDrop], tuple)
                            end

                            if (allFlag == false) then
                                body = body .. "port:                " .. portDev .. "\n"
                                body = body .. "TC:                  " .. tonumber(currTc, 2) .. "\n"
                                body = body .. "DP:                  " .. numberToColor[tonumber(currDp, 2)] .. "\n"
                                body = body .. "Packets counter:     " .. packetsCounter .. "\n"
                                body = body .. "Byte-count counter:  " .. byteCounter .. "\n"
                                body = body .. "CNC index:           " .. string.format("0x%X", index) .. "\n"
                            else
                                if (byteCounter ~= 0) then
                                    body = body .. string.format(" %-6s %-3s %-6s %-20s %-20s\n",
                                        portDev,
                                        tonumber(currTc, 2),
                                        numberToColor[tonumber(currDp, 2)],
                                        packetsCounter,
                                        byteCounter)
                                end
                            end
                        end
                    end
                end
                print(string.upper(passDrop) .. " counters: Enabled")
                if (body == "") then
                    print("There are no Counters to show")
                else
                    if allFlag == true then
                        print(header)
                    end
                    print(body)
                end
            end
        end
    end

    if (#passAndDrop < 2) then -- pass or drop (or both) not configured
        if (#passAndDrop == 0) then -- both
            print("PASS counters: Disabled")
        end
        print("DROP counters: Disabled")
    end

    return true, retTable
end

--------------------------------------------
-- command registration: show counters egress-queue all
--------------------------------------------

CLI_addCommand("exec", "show counters egress-queue all", {
    func = function(params)
        params["all"] = true
        return showCountersEgressQueue(params)
    end,
    help = "show all CNC counters bound to Egress Queue client",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            mandatory = { "devID" }
        }
    }
})

--------------------------------------------
-- command registration: show counters egress-queue ethernet
--------------------------------------------
CLI_addHelp("exec", "show counters egress-queue", "show CNC counters bound to Egress Queue client")
CLI_addCommand("exec", "show counters egress-queue ethernet", {
    func = showCountersEgressQueue,
    help = "show CNC counters for selected interface",
    params = {
        {
            type = "named",
            { format = "interface %dev_port", name = "dev_port", help = "Specific ethernet dev/port" },
            { format = "tc %queue_id_no_all", name = "tc", help = "Traffic Class associated with this set of Drop Parameters" },
            { format = "dp %drop_precedence_no_all", name = "dp", help = "Drop Profile Parameters to associate with the Traffic Class" },
            requirements = {
                ["dp"] = { "tc" },
                ["tc"] = { "dev_port" }
            },
            mandatory = { "dp" }
        }
    }
})


-- ************************************************************************
--  showCountersIngressVlan
--
--  @description show CNC counters bound to Ingress Vlan client
--
--  @param params - params["devID"]     - device number.
--                  params["vlanIdOrAll"]   - Range of the pclIds.
--
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************


function showCountersIngressVlan(params)

    local ret, val
    local i, j, passDrop, d
    local blocksList, header, body
    local devNum
    local allFlag = false
    local port, dp, tc
    local dpBits, tcBits, portBits
    local index, blockSize, block, vlanId
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E = 0
    local listOfVlans = {}
    local passAndDrop = {}
    local devices
    local retTable = {}

    blockSize = 1024 -- each block is 1k bytes
    blocksList = getCNCBlock("IngressVlanPassAndDrop")
    if blocksList == -1 then
      return false, "Wrong client name for selected device"
    end
    body = ""
    devNum = params["devID"]
    header = "\n"
    header = header .. " devID   VlanId     Packet-Counter     Byte-count counter         \n"
    header = header .. "---------------------------------------------------------------------------\n"

    if (params["vlanIdOrAll"] == "all") then
        allFlag = true
        for i = 1, 4095 do
            table.insert(listOfVlans, i)
        end
    else
        table.insert(listOfVlans, params["vlanIdOrAll"])
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    if (luaGlobalGet("IngressVlan") ~= nil) then
        table.insert(passAndDrop, "pass")
        retTable["pass"] = {}
        if (luaGlobalGet("IngressVlan") == "PassAndDrop") then
            table.insert(passAndDrop, "drop")
            retTable["drop"] = {}
        end
        for d = 1, #devices do -- main devices loop
            devNum = devices[d]
            for j, passDrop in pairs(passAndDrop) do
                body = ""
                for i = 1, #listOfVlans do -- loop over the vlans

                    vlanId = listOfVlans[i]
                    block = math.floor(vlanId / blockSize)
                    if (passDrop == "pass") then
                        block = blocksList[block + 1]
                    else
                        block = blocksList[block + 5]
                    end
                    index = vlanId % blockSize
                    local counterPtr
                    ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                        { "IN", "GT_U8", "devNum", devNum },
                        { "IN", "GT_U32", "blockNum", block },
                        { "IN", "GT_U32", "index", index },
                        { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
                        { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                    })

                    if ret ~= 0 then
                        print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                        return false, "Error, cannot show counters acl"
                    end
                    counterPtr = val["counterPtr"]
                    local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
                    local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

                    local byteCounterLSB = counterPtr["byteCount"]["l"][0]
                    local byteCounterMSB = counterPtr["byteCount"]["l"][1]

                    local packetsCounter, byteCounter

                    -- converting the packet counter and the byte counter to 64 bit number
                    packetsCounterLSB = dec2hex(packetsCounterLSB)
                    packetsCounterMSB = dec2hex(packetsCounterMSB)
                    packetsCounter = packetsCounterMSB .. packetsCounterLSB
                    packetsCounter = hex2dec(packetsCounter)

                    byteCounterLSB = dec2hex(byteCounterLSB)
                    byteCounterMSB = dec2hex(byteCounterMSB)
                    byteCounter = byteCounterMSB .. byteCounterLSB
                    byteCounter = hex2dec(byteCounter)

                    if (byteCounter ~= 0) then
                        local tuple = {
                            ["devID"] = devNum,
                            ["Vlan"] = vlanId,
                            ["Packet-Counter"] = packetsCounter,
                            ["Byte-count counter"] = byteCounter
                        }

                        table.insert(retTable[passDrop], tuple)
                    end
                    if (allFlag == false) then
                        body = body .. "devID:               " .. devNum .. "\n"
                        body = body .. "Vlan:                " .. vlanId .. "\n"
                        body = body .. "Packets counter:     " .. packetsCounter .. "\n"
                        body = body .. "Byte-count counter:  " .. byteCounter .. "\n"
                    else
                        if (byteCounter ~= 0) then
                            body = body .. string.format("   %-6s %-7s %-20s %-20s\n",
                                devNum,
                                vlanId,
                                packetsCounter,
                                byteCounter)
                        end
                    end
                end
                print(string.upper(passDrop) .. " counters: Enabled")
                if (body == "") then
                    print("There are no Counters to show")
                else
                    if allFlag == true then
                        print(header)
                    end
                    print(body)
                end
            end
        end
    end
    if (#passAndDrop < 2) then -- pass or drop (or both) not configured
        if (#passAndDrop == 0) then -- both
            print("PASS counters: Disabled")
        end
        print("DROP counters: Disabled")
    end
    return true, retTable
end

--------------------------------------------
-- command registration: show counters ingress-vlan
--------------------------------------------

CLI_addCommand("exec", "show counters ingress-vlan", {
    func = showCountersIngressVlan,
    help = "show all CNC counters bound to Ingress Vlan client",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            { format = "%vlanIdOrAll", name = "vlanIdOrAll", help = "The device number" },
            requirements = {
                ["vlanIdOrAll"] = { "devID" }
            },
            mandatory = { "vlanIdOrAll" }
        }
    }
})

-- ************************************************************************
--  showCountersArpNat
--
--  @description show CNC counters bound to ARP/NAT client
--
--  @param params - params["devID"]         - device number.
--
--  @return  true and struct on success, otherwise false and error message
--
-- ************************************************************************

function showCountersArpNat(params)
    local ret, val
    local i, j
    local blocksList, header, body, block
    local devNum = params["devID"]
    local CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E = 0
    local maxArpNatId=1023
    local allFlag = false
    local oneArpNatId = false
    local retTable = {}
    local arpNatIdx = params["arpNatCncIdx"]
    local arpNatIdxTmp

    header = "\n"
    header = header .. " devID   arp-nat-cnc-id   Packet-Counter     Byte-count counter         \n"
    header = header .. "---------------------------------------------------------------------------\n"
    body = ""
    if (arpNatIdx == "all") then
        allFlag = true
        arpNatIdxTmp = {}
        for i = 0, maxArpNatId do
            table.insert(arpNatIdxTmp, i)
        end
        arpNatIdx = arpNatIdxTmp
    else
        arpNatIdxTmp = {}
        table.insert(arpNatIdxTmp, tonumber(arpNatIdx))
        arpNatIdx = arpNatIdxTmp
    end

    if (devNum == "all") then
        devices = wrlDevList()
        allFlag = true
    else
        devices = { devNum }
    end

    if #arpNatIdx == 1 and allFlag == false then
        oneArpNatId = true
    end
    blocksList = getCNCBlock("ArpNatTableAccess")
    if blocksList == -1 then
      return false, "Wrong client name for selected device"
    end
    block = blocksList[1]

    if (luaGlobalGet("ArpNat") ~= nil) then

        retTable["pass"] = {}
        for j = 1, #devices do -- main devices loop

            for i = 1, #arpNatIdx do
                local counterPtr
                local index = arpNatIdx[i]
                ret, val = myGenWrapper("cpssDxChCncCounterGet", {
                    { "IN", "GT_U8", "devNum", devices[j] },
                    { "IN", "GT_U32", "blockNum", block },
                    { "IN", "GT_U32", "index", index },
                    { "IN", "CPSS_DXCH_CNC_COUNTER_FORMAT_ENT", "format", CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E },
                    { "OUT", "CPSS_DXCH_CNC_COUNTER_STC", "counterPtr" }
                })

                if ret ~= 0 then
                    print("")
                    print("cpssDxChCncCounterGet() failed: " .. returnCodes[ret])
                    return false, "Error, cannot show counters arp/nat"
                end

                counterPtr = val["counterPtr"]
                local packetsCounterLSB = counterPtr["packetCount"]["l"][0]
                local packetsCounterMSB = counterPtr["packetCount"]["l"][1]

                local byteCounterLSB = counterPtr["byteCount"]["l"][0]
                local byteCounterMSB = counterPtr["byteCount"]["l"][1]

                local packetsCounter, byteCounter

                -- converting the packet counter and the byte counter to 64 bit number
                packetsCounterLSB = dec2hex(packetsCounterLSB)
                packetsCounterMSB = dec2hex(packetsCounterMSB)
                packetsCounter = packetsCounterMSB .. packetsCounterLSB
                packetsCounter = hex2dec(packetsCounter)

                byteCounterLSB = dec2hex(byteCounterLSB)
                byteCounterMSB = dec2hex(byteCounterMSB)
                byteCounter = byteCounterMSB .. byteCounterLSB
                byteCounter = hex2dec(byteCounter)

                if (byteCounter ~= 0) then
                    local tuple = {
                        ["devID"] = devices[j],
                        ["arp-nat-id"] = index,
                        ["Packet-Counter"] = packetsCounter,
                        ["Byte-count counter"] = byteCounter
                    }
                    table.insert(retTable["pass"], tuple)
                end
                if (oneArpNatId == true) then
                    body = body .. "device              :" .. devices[j] .. "\n"
                    body = body .. "arp/nat cnc id      :" .. index .. "\n"
                    body = body .. "Packets counter     :" .. packetsCounter .. "\n"
                    body = body .. "Byte-count counter  :" .. byteCounter .. "\n"
                else
                    if (byteCounter ~= 0 or allFlag == false) then
                        body = body .. string.format("   %-6s %-7s %-20s %-20s\n",
                            devices[j],
                            index,
                            packetsCounter,
                            byteCounter)
                    end
                end
            end
        end

        print("ARP/NAT counters: Enabled")
        if (body == "") then
            print("There are no Counters to show")
        else
            if oneArpNatId == false then
                print(header)
            end
            print(body)
        end
    else
        print("ARP/NAT counters: Disabled")
    end

    return true, retTable
end

--------------------------------------------
-- command registration: show counters ARP_NAT
--------------------------------------------
CLI_addCommand("exec", "show counters ARP_NAT", {
    func = showCountersArpNat,
    help = "show all CNC counters bound to ARP/NAT client",
    params = {
        {
            type = "named",
            { format = "device %devID_all", name = "devID", help = "The device number" },
            { format = "arpNatCncIdx %arpNatCncIDorAll", name = "arpNatCncIdx", help = "arp-nat-cnc-id (0-1023)" },
            requirements = {
                ["arpNatCncIdx"] = {"devID"}
            },
            mandatory = {"arpNatCncIdx"}
        }
    }
})