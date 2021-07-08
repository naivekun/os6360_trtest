--[[
-- main purpose of this file is 'utilities for packets' .
-- NOTE: lua test that need to transmit packets / capture packets should use use the powerful function :
--      luaTgfTransmitPacketsWithExpectedEgressInfo
--============================================================================================================
-- list of functions that are 'extern' (not local) that can be used by other lua files.

    1. packetAsStringToPrintableFormat(packetAsString) -
        function return 'printable string' for inpout of 'packet as string'
    2. luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,local_egressInfoTable) -
        function to transmit packet(s) and check egress ports for : counters (packets,bytes) and actual packets.
--============================================================================================================
--]]

cmdLuaCLI_registerCfunction("prvLuaTgfPxTransmitPackets")

-- function to force the LUA 'garbage collector' to act.
-- in order for the operations to be less expensive in memory
local function free_lua_mem(line,noDebug)
    if not noDebug then
        -- check lua usage before the 'free'
        local count = collectgarbage("count")
        -- free what ever you can ...
        collectgarbage()
        -- check lua usage after the 'free'
        local count1 = collectgarbage("count")

        -- do indication if free more than 100KB
        if((count - count1) > 100--[[KB]]) then
            local lineStr = to_string(line)
            printLog("index ["..lineStr.."] collectgarbage() --> did free of : " .. (count - count1) .. " Kbytes")
        end
    else
        -- no debug ...
        -- just free what ever you can ...
        collectgarbage()
    end
end

--[[ function return 'printable string' for inpout of 'packet as string' :
      *****************************
for input:
"887711115566983221191806810000c8"..
"08004500007400000000211193720202"..
"020201010103fad4147f006000000010"..
"02000000000000010203340200040506"..
"07118100000a00010203040506070809"..
"0a0b0c0d0e0f10111213141516171819"..
"1a1b1c1d1e1f20212223242526272829"..
"2a2b2c2d2e2f30313233343536373839"..
"3a3b3c3d3e3f"
      *****************************
OUTPUTS:
return a new string in format of:
"0x0000 : 88 77 11 11 55 66 98 32 21 19 18 06 81 00 00 c8"..
"0x0010 : 08 00 45 00 00 74 00 00 00 00 21 11 93 72 02 02"..
"0x0020 : 02 02 01 01 01 03 fa d4 14 7f 00 60 00 00 00 10"..
"0x0030 : 02 00 00 00 00 00 00 01 02 03 34 02 00 04 05 06"..
"0x0040 : 07 11 81 00 00 0a 00 01 02 03 04 05 06 07 08 09"..
"0x0050 : 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19"..
"0x0060 : 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27 28 29"..
"0x0070 : 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39"..
"0x0080 : 3a 3b 3c 3d 3e 3f"
]]--
function packetAsStringToPrintableFormat(packetAsString)
    local numChars = string.len(packetAsString)
    local numBytes = math.floor((numChars + 1) / 2)
    local printableFormat = ""--start with empty string
    local currentChar
    local maxPrintSize = 256
    local didTruncation = false

    if packetAsString == nil then return nil end

    if numBytes > maxPrintSize then
        numBytes = maxPrintSize
        didTruncation = true
    end

    for currentByteIndex = 0,(numBytes - 1) do

        if 0 == (currentByteIndex % 0x10) and
            numBytes > 16 then

            if currentByteIndex ~= 0 then
                -- add 'new line' when new line is needed
                printableFormat =  printableFormat .. "\n"
            end

            printableFormat =  printableFormat .. string.format("0x%4.4x : " , currentByteIndex)

        end

        if 0xff == (currentByteIndex % 0x100) then
            free_lua_mem(nil,true--[[no debug]])
        end

        currentChar = (currentByteIndex * 2) + 1
        -- get 2 characters from the string that represents a 'byte' in the packet
        currentByte = string.sub(packetAsString, currentChar,currentChar+1)

        -- add the byte to printable string
        printableFormat = printableFormat .. " " .. currentByte
    end

    if didTruncation then
        printableFormat = printableFormat .. "\n .. truncated to first [" .. maxPrintSize .. "] bytes"
    end

    free_lua_mem(nil,true--[[no debug]])

    return printableFormat
end

-- calc packet length
local function calcPacketLength(pktInfo)
    if pktInfo == nil then return nil end

    --support 'fullPacket' by lua test .. convert it to 'payload' before sent the 'tgf'
    if pktInfo.fullPacket ~= nil then
        return math.floor((string.len(pktInfo.fullPacket) + 1) / 2)
    end

    -- not support packet format !!!
    -- need TGF support !!!
    return nil
end

-- add crc 4 bytes to calculations
local function considerCalcOfCrc4Bytes(numPackets,numBytesPerPacket)
    local local_numPackets = 1

    if numPackets == stormingExpectedIndication then
        return stormingExpectedIndication
    end

    if numPackets then
        local_numPackets = numPackets
    end

    if numBytesPerPacket then
        return (4 + numBytesPerPacket) * local_numPackets
    end

    return nil

end

-- indication for field 'transmitInfo.burstCount' to start generate 'full wirespeed'
transmit_continuous_wire_speed = "continuous-wire-speed"
-- indication for field 'transmitInfo.burstCount' to stop generate 'full wirespeed'
stop_transmit_continuous_wire_speed = "stop continuous-wire-speed"

luaTgfTransmitPacket_px_trace = false

--[[
-- function to transmit packet(s) and check egress ports for : counters (packets,bytes) and actual packets.
-- parameters are:
INPUTS :
      *****************************
          transmitInfo hold next fields :
          devNum - the ingress device
                  -- when nil --> considered = devEnv.dev
          portNum - the ingress port
                  -- when nil --> error
          pktInfo - the ingress packet info
                  -- when nil --> error
                      pktInfo.fullPacket ~= nil --> support 'fullPacket' by lua test
          burstCount - the ingress burst count
              -- when nil --> considered 1
              -- when "continuous-wire-speed" --> meaning we need to generate 'full wirespeed'
              -- when "stop continuous-wire-speed" -->meaning we need to stop the 'full wirespeed'

          egress info holds next fields:
          numEntries  - number of egress ports info in array of egressPortsArr[]
          egressPortsArr[] - array of egress ports to check for counters and packets

          function return 'GT_STATUS' that explain 'C' type errors from 'DUT'
--]]
function luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    local GT_OK = 0
    local GT_FAIL = 1
    local GT_BAD_PARAM = 4
    local GT_BAD_PTR = 5
    local rc -- return code from TGF
    local numEgressPorts
    local local_transmitInfo = deepcopy(transmitInfo);
    local local_egressInfoTable = deepcopy(egressInfoTable);
    local numOfErrorsToReport = 0

    if local_transmitInfo == nil then
        -- special case for 'test egress rates' only
        return checkExpectedEgressInfo(local_egressInfoTable)
    end


    transmitInfo    = nil -- make sure not to use it in this function
    egressInfoTable = nil -- make sure not to use it in this function

    local devNum;

    -- support omit of many parameters ... set 'defaults'
    if local_transmitInfo.devNum == nil           then local_transmitInfo.devNum = devEnv.dev end
    devNum = local_transmitInfo.devNum;
    if local_transmitInfo.portNum == nil          then
        printLog("ERROR : local_transmitInfo.portNum --> nil ")
        setFailState()
        return GT_BAD_PARAM
    end
    if local_transmitInfo.burstCount == nil       then local_transmitInfo.burstCount = 1 end

    if local_transmitInfo.burstCount ~= stop_transmit_continuous_wire_speed then
        if local_transmitInfo.pktInfo == nil          then
            printLog("ERROR : local_transmitInfo.pktInfo --> nil ")
            setFailState()
            return GT_BAD_PARAM
        end

        if local_transmitInfo.pktInfo.partsArray == nil and
           local_transmitInfo.pktInfo.fullPacket == nil then

           -- error of the test ... not initialized packet properly
            printLog("ERROR : local_transmitInfo.pktInfo.partsArray --> nil and local_transmitInfo.pktInfo.fullPacket --> nil ")
            setFailState()
            return GT_BAD_PTR

        end

        rc = GT_OK

        if luaTgfTransmitPacket_px_trace then
            printLog("\n\n ===============")
            printLog(
                "Inject Packet into"
                .. " Dev "  .. local_transmitInfo.devNum
                .. " Port " .. local_transmitInfo.portNum
                .. " :")
        end


        --support 'fullPacket' by lua test .. convert it to 'payload' before sent the 'tgf'
        if local_transmitInfo.pktInfo.fullPacket ~= nil then
            local_transmitInfo.pktInfo.fullPacket = string.lower(local_transmitInfo.pktInfo.fullPacket)

            -- pad the packet with zeros (if needed) ... minimal size is 60 bytes (without the CRC)
            local_transmitInfo.pktInfo.fullPacket =
                pad_packet_format(local_transmitInfo.pktInfo.fullPacket)

            local_transmitInfo.pktInfo.partsArray={
                {type="TGF_PACKET_PART_PAYLOAD_E", partPtr= local_transmitInfo.pktInfo.fullPacket } }

            if luaTgfTransmit_px_Packet_trace then
                local printable_ingressPacket = packetAsStringToPrintableFormat(
                    local_transmitInfo.pktInfo.fullPacket)
                printLog(printable_ingressPacket)
            end
        end
    else
        printLog("\n\n ===============")
        printLog(
            "Stop WS traffic from "
            .. " Dev "  .. local_transmitInfo.devNum
            .. " Port " .. local_transmitInfo.portNum)
    end

    didError_GT_BAD_PARAM = false
    didError_general_rc = false

    local senderPacketLen = calcPacketLength(local_transmitInfo.pktInfo)
    local pktInfoListArr = {};
    local numEntries = 0
    local withLoop = false

    -- sum all the ports that we need to send traffic to
    if local_egressInfoTable == nil then
        local burstCount,pktInfo
        local pktInfo = local_transmitInfo.pktInfo

        if local_transmitInfo.burstCount == transmit_continuous_wire_speed then
            burstCount = 0xFFFFFFFF -- start WS
            withLoop = true
        elseif local_transmitInfo.burstCount == stop_transmit_continuous_wire_speed then
            burstCount = 0 -- stop WS
            -- dummy
            pktInfo = {fullPacket = "3333333333333333333333333333333"} 
        else
            burstCount = local_transmitInfo.burstCount -- regular burst
        end
    
        -- send packet without check egress ports
        rc = prvLuaTgfPxTransmitPackets(
            local_transmitInfo.devNum,
            local_transmitInfo.portNum,
            pktInfo,
            burstCount,
            withLoop
        )

        if rc ~= GT_OK then
            printLog (
                "ERROR : prvLuaTgfTransmitPackets failed with code " .. rc
                .. " dev " .. local_transmitInfo.devNum
                .. " port " .. local_transmitInfo.portNum)
        end

        -- finished processing
        printLog(
            "\n\n Finished handling Packet from Port "
            .. local_transmitInfo.devNum .. "/"
            .. local_transmitInfo.portNum
            .. " (send packet without check egress ports)")

        return rc;
    else
        for index, egressEntry in pairs(local_egressInfoTable) do
                -- support omit of many parameters ... set 'defaults'
            if egressEntry.devNum == nil           then egressEntry.devNum = local_transmitInfo.devNum end
            if egressEntry.portNum == nil          then
                didError_GT_BAD_PARAM = "ERROR : egressEntry.portNum == nil"
                break
            end

            if egressEntry.packetCount == nil then
                egressEntry.packetCount = 0
            end

            if egressEntry.packetCount ~= 0 then
                if egressEntry.numBytes == nil then
                    egressEntry.numBytes = considerCalcOfCrc4Bytes(egressEntry.packetCount, senderPacketLen);
                end
                if egressEntry.burstCount == nil then
                    egressEntry.burstCount = local_transmitInfo.burstCount
                end
            end
            if egressEntry.pktInfo ~= nil          then
                if egressEntry.pktInfo.partsArray == nil and
                egressEntry.pktInfo.fullPacket == nil then

                -- error of the test ... not initialized packet properly
                    printLog("ERROR : egressEntry.pktInfo.partsArray --> nil and egressEntry.pktInfo.fullPacket --> nil ")
                    setFailState()
                    return GT_BAD_PTR
                end

                egressEntry.pktInfo.fullPacket = pad_packet_format(egressEntry.pktInfo.fullPacket)
                egressEntry.pktInfo.partsArray={
                    {type="TGF_PACKET_PART_PAYLOAD_E", partPtr= egressEntry.pktInfo.fullPacket } }

                local printable_egressPacket = packetAsStringToPrintableFormat(egressEntry.pktInfo.fullPacket)
                printLog(printable_egressPacket)
            end

            -- save lists
            pktInfoListArr[index] = egressEntry;
            -- increment number of entries
            numEntries = numEntries + 1
        end
    end

    -- transmit packet and check results
    rc = prvLuaTgfPxTransmitPacketsWithCapture(
         local_transmitInfo.devNum,
         local_transmitInfo.portNum,
         local_transmitInfo.pktInfo,
         local_transmitInfo.burstCount,
         numEntries,
         pktInfoListArr
    )
    if rc ~= GT_OK then
        didError_general_rc = { my_string = "ERROR : prvLuaTgfTransmitPacketsWithCapture failed with code " .. rc , my_rc = rc}
    end
    if didError_general_rc then
        printLog(didError_general_rc.my_string)
        setFailState()
        return didError_general_rc.my_rc
    end

    if didError_GT_BAD_PARAM then
        printLog(didError_GT_BAD_PARAM)
        setFailState()
        return GT_BAD_PARAM
    end

    -- finished processing
    printLog(
        "\n\n Finished handling Packet from Port "
         .. local_transmitInfo.devNum .. "/" .. local_transmitInfo.portNum)

    return rc
end

