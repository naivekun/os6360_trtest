--[[
-- main purpose of this file is 'utilities for packets' .
-- NOTE: lua test that need to transmit packets / capture packets should use use the powerful function :
--      luaTgfTransmitPacketsWithExpectedEgressInfo
--      and NOT the 'C' functions : prvLuaTgfTransmitPackets,prvLuaTgfTransmitPacketsWithCapture,prvLuaTgfRxCapturedPacket
--============================================================================================================
-- list of functions that are 'extern' (not local) that can be used by other lua files.

    1. packetAsStringToPrintableFormat(packetAsString) -
        function return 'printable string' for inpout of 'packet as string'
    2. luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,local_egressInfoTable) -
        function to transmit packet(s) and check egress ports for : counters (packets,bytes) and actual packets.
    3. luaTgfStateTrgEPortInsteadPhyPort(newEPort) -
        Option to state that the cpu will send packets to newEPort that is not
        the 'physical port' (portNum) that used by prvLuaTgfTransmitPackets(..portNum..)
        prvLuaTgfTransmitPacketsWithCapture(..inPortNum..)
--============================================================================================================
--]]

if (dxCh_family == true) then
  cmdLuaCLI_registerCfunction("wrlDxChDsaToString")
  cmdLuaCLI_registerCfunction("wrlDxChDsaToStruct")
  cmdLuaCLI_registerCfunction("wrlDxChNetIfCpuToDsaCode")
end

-- mac address FF:FF:FF:FF:FF:FF
broadcast_macAddr = "ff".."ff".."ff".."ff".."ff".."ff"

-- array of pair of ports that connected to each other , per device
-- luaTgf_pairsOfPorts -
-- luaTgf_pairsOfPorts[devNum] = {{a,b},{c,d}}
local luaTgf_pairsOfPorts = {}

-- return nil or the 'peer' port
local function isPortHoldPeer(devNum,portNum)
    if luaTgf_pairsOfPorts[devNum] == nil then
        return nil
    end
    -- look for the port
    for index,entry in pairs(luaTgf_pairsOfPorts[devNum]) do
        if entry[1] == portNum then
            -- port is first in the pair ... the 'peer' is the second port
            return entry[2]
        end
        if entry[2] == portNum then
            -- port is second in the pair ... the 'peer' is the first port
            return entry[1]
        end
    end

    -- not found
    return nil
end

--[[
* luaTgfStateTrgEPortInsteadPhyPort
*
* DESCRIPTION:
*       Option to state that the cpu will send packets to newEPort that is not
*       the 'physical port' (portNum) that used by prvLuaTgfTransmitPackets(..portNum..)
*       prvLuaTgfTransmitPacketsWithCapture(..inPortNum..)
*
*       use newEPort != nil to state to start using newEPort
*       use newEPort = nil to state to start NOT using trgEPort (newEPort is ignored)
--]]
function luaTgfStateTrgEPortInsteadPhyPort(newEPort)
    if newEPort == nil then
        enable = false
        newEPort = 0-- don't care
    else
        enable = true
    end

    prvLuaTgfStateTrgEPortInsteadPhyPort(newEPort,enable)
end

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

--[[ returns mask to apply currentByteIndex according to info in maskedBytesTable[]
    maskedBytesTable - table of bytes to mask when 'compare' packets:
    each entry is of type:
        startByte - the byte that starts the not compared range
        endByte   - the byte that ends the not compared range
        startByteCompareMask - mask for start byte compared bits (default 0x00)
        endByteCompareMask - mask for end byte compared bits (default 0x00)

    return 'mask' - mask of 8 bits to compare in the byte
--]]
local full_byte_mask = 0xff
local non_byte_mask  = 0
local function getCurrentByteMask(maskedBytesTable,currentByteIndex)
    local startByte, endByte, startByteCompareMask, endByteCompareMask;

    if maskedBytesTable == nil then
        return full_byte_mask
    end

    for index,entry in pairs(maskedBytesTable) do

        startByte            = entry.startByte;
        endByte              = entry.endByte;
        startByteCompareMask = entry.startByteCompareMask;
        endByteCompareMask   = entry.endByteCompareMask;

        if startByte == nil then
            -- ignored
            startByte = 0xffffffff
        end

        if endByte == nil then
            endByte = entry.startByte
        end

        if startByteCompareMask == nil then
            startByteCompareMask = 0
        end

        if endByteCompareMask == nil then
            endByteCompareMask = 0
        end

        if currentByteIndex >= entry.startByte and currentByteIndex <= entry.endByte then

            --print ("need mask currentByte : " .. currentByteIndex)

            if currentByteIndex ~= entry.startByte and currentByteIndex ~= entry.endByte then
                --print ("non_byte_mask")
                return non_byte_mask
            end

            -- calc mask of bits to compare
            local compareBitsMask = 0;

            if currentByte == entry.startByte then
                compareBitsMask = bit_or(compareBitsMask, startByteCompareMask);
            end

            if currentByte == entry.endByte then
                compareBitsMask = bit_or(compareBitsMask, endByteCompareMask);
            end

            return compareBitsMask;
        end
    end

    return full_byte_mask;
end

local nibble_to_num =
{
    ["0"] = 0, ["1"] = 1, ["2"] = 2, ["3"] = 3, ["4"] = 4,
    ["5"] = 5, ["6"] = 6, ["7"] = 7, ["8"] = 8, ["9"] = 9,
    ["a"] = 10, ["b"] = 11, ["c"] = 12, ["d"] = 13, ["e"] = 14, ["f"] = 15,
    ["A"] = 10, ["B"] = 11, ["C"] = 12, ["D"] = 13, ["E"] = 14, ["F"] = 15
};

-- convert "f5" to 0xf5 (decimal = 245)
-- return 245 (0xf5)
function byteAs2NibblesToNumeric(byteAsString)
    bin_byte1_nib0 = nibble_to_num[string.sub(byteAsString, 1, 1)]
    bin_byte1_nib1 = nibble_to_num[string.sub(byteAsString, 2, 2)]

    bin_byte1 = ((16 * bin_byte1_nib0) + bin_byte1_nib1);

    return bin_byte1
end

--[[
    check if the 2 bytes compared are equal if applied proper mask
    parameters:
    byte1 - byte 1 value
    byte2 - byte 2 value
    byteIndex - byte index that the 2 compared bytes belongs to

    maskedBytesTable - table of bytes to mask when 'compare' packets:
    each entry is of type:
        startByte - the byte that starts the not compared range
        endByte   - the byte that ends the not compared range
        startByteCompareMask - mask for start byte compared bits (default 0x00)
        endByteCompareMask - mask for end byte compared bits (default 0x00)
--]]
local function isErrorCompareByteWithMaskTable(byte1,byte2,byteIndex,maskedBytesTable)
    local mask = getCurrentByteMask(maskedBytesTable,byteIndex);
    if string.len(byte1) ~= 2 or string.len(byte2) ~= 2 then
        -- packet format error
        return true
    end
    local bin_byte1_nib0 = nibble_to_num[string.sub(byte1, 1, 1)];
    local bin_byte1_nib1 = nibble_to_num[string.sub(byte1, 2, 2)];
    local bin_byte2_nib0 = nibble_to_num[string.sub(byte2, 1, 1)];
    local bin_byte2_nib1 = nibble_to_num[string.sub(byte2, 2, 2)];

    if bin_byte1_nib0 == nil then return true end -- packet format error
    if bin_byte1_nib1 == nil then return true end -- packet format error
    if bin_byte2_nib0 == nil then return true end -- packet format error
    if bin_byte2_nib1 == nil then return true end -- packet format error

    local bin_byte1 = ((16 * bin_byte1_nib0) + bin_byte1_nib1);
    local bin_byte2 = ((16 * bin_byte2_nib0) + bin_byte2_nib1);

    -- (full_byte_mask - mask)
    local masked_byte1 = bit_and(bin_byte1, mask);
    local masked_byte2 = bit_and(bin_byte2, mask);

    if masked_byte1 ~= masked_byte2 then
        -- error
        return true
    end

    --print ("2 bytes comared ["..byte1.."]["..byte2.."] ok after apply mask :]"..mask.."] in index ["..byteIndex.."]")

    -- not errors
    return false
end



--[[ compare 2 packets : fine compare .. which bytes not match
    expectedEgressPacket - is the 'expected'
    actualEgressPacket - is the 'actual'
    maskedBytesTable - table of bytes to mask when 'compare' packets:
    each entry is of type:
        startByte - the byte that starts the not compared range
        endByte   - the byte that ends the not compared range
        startByteCompareMask - mask for start byte compared bits (default 0x00)
        endByteCompareMask - mask for end byte compared bits (default 0x00)

    return 'is ok' - true --> no diff , otherwise false
--]]
local function packetsCompareFineTuning(expectedEgressPacket,actualEgressPacket,maskedBytesTable)
    local numChars1 = string.len(expectedEgressPacket)
    local numBytes1 = math.floor((numChars1 + 1) / 2)
    local numChars2 = string.len(actualEgressPacket)
    local numBytes2 = math.floor((numChars2 + 1) / 2)
    local currentChar
    local currentByte1 , currentByte2
    local numBytes
    local numOfErrors = 0
    local isOK = true

    if numBytes2 < numBytes1 then
        numBytes = numBytes2
    else
        numBytes = numBytes1
    end

    for currentByteIndex = 0,(numBytes - 1) do

        currentChar = (currentByteIndex * 2) + 1
        -- get 2 characters from the string that represents a 'byte' in the packet
        currentByte1 = string.sub(expectedEgressPacket, currentChar,currentChar+1)
        currentByte2 = string.sub(actualEgressPacket, currentChar,currentChar+1)
        if currentByte1 ~= currentByte2 then
            local isError = true

            if maskedBytesTable ~= nil then
                isError = isErrorCompareByteWithMaskTable(currentByte1,currentByte2,currentByteIndex,maskedBytesTable)
            end

            if isError == true then
                numOfErrors = numOfErrors + 1

                if numOfErrors == 1 then
                    printLog("Mismatch in Bytes:")
                end

                if numOfErrors <= 10 then
                    printLog("byte index ["..currentByteIndex.."] expected [0x"..currentByte1.."] got [0x"..currentByte2.."]")
                end
            end -- error == true
        end -- currentByte1 ~= currentByte2
    end -- for currentByteIndex = 0,(numBytes - 1)

    if numOfErrors > 0 then
        isOK = false
        printLog("Got "..numOfErrors.." mismatch errors")

        if numOfErrors > 10 then
            printLog("stopped showing differences after 10 mismatches")
        end
    end

    if numBytes2 ~= numBytes1 then
        isOK = false
        printLog("mismatch in packet Length : expected ["..numBytes1.."] got ["..numBytes2.."]")
    end

    return isOK
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



local function func_cpssDxChPclPortIngressPolicyEnable(devNum,portNum,enable)
    apiName = "cpssDxChPclPortIngressPolicyEnable"
    result = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",          "devNum",     devNum },
        { "IN",     "GT_PORT_NUM",    "portNum",    portNum},
        { "IN",     "GT_BOOL",        "enable",      enable  }
    })
    --command_data:handleCpssErrorDevPort(result, apiName.."()")
    --_debug(apiName .. "   result = " .. result)
end

-- return the 'pktInfo' of the 'dev/port' in the egress table
local function getPacketInfoOfDevPort(egressTable,devNum,portNum)
    -- format of local_egressInfoTable
    for index1,egressEntry in pairs(egressTable) do
        if egressEntry.devNum == devNum and
           egressEntry.portNum == portNum
        then
            return egressEntry.pktInfo
        end
    end

    return nil
end
-- db to hold info about restore values
local cscdInfoRestore =
{
    -- [string_dev_port] = {mcPhysicalSourceFilteringIgnore}
}

local cpuSdmaNumber = 63
--check if the port is 'SDMA' (hold no mac and limited counters)
function isCpuSdmaPort(devNum,portNum)
    isCpuSdma = (portNum == cpuSdmaNumber or string.upper(portNum) == "CPU" ) and DeviceCpuPortMode == "CPSS_NET_CPU_PORT_MODE_SDMA_E"
    return isCpuSdma
end
-- function to check if port is cascade.
-- if cascade then function changes the 'tx' direction (for 'sender port') or 'rx' direction (for 'egress port')
-- to be 'non cscd'.
--
-- NOTE: function should be called once with restoreValue = nil. this call will return false or 'value to restore'.
--      if returned value is NOT 'false' then function should be called again with restoreValue = 'value to restore'.
--
-- inputs:
--      devNum  - the device.
--      portNum - the port number.
--      isSender - indication that {devNum,portNum} are of 'sender port' or of 'egress port'
--                  true - 'sender'
--                  false - 'egress'
--      restoreValue - indication that we need to restore value to 'cscd' , because was changed when
--              function was called with restore = nil
--                  nil - indication that we may need to remove the cascade config from the port.
--                  'value to restore' - the value to restore to the port.
--  returned values:
--  return indication that port's mode was changed.
--  NOTE: when restoreValue is NOT 'false' the returned value has no meaning !!!
--
local function supportCscdPort(devNum,portNum,isSender,restoreValue)
    local string_dev_port = "devNum"..devNum.."portNum"..portNum

    if isCpuSdmaPort(devNum,portNum) then
        --cpu port must not be changed
        return
    end
    
    local peerPort = isPortHoldPeer(devNum,portNum)
    if peerPort ~= nil then
        -- this port hold peer port that will egress/ingress packets for our port.
        -- so we are not concerned about our port's cscd config
        return false
    end

    local old_portType = false
    local new_portType = false
    local apiName
    local ret,val
    local portDirection

    if isSender == true then
        portDirection = "CPSS_PORT_DIRECTION_TX_E"
    else
        portDirection = "CPSS_PORT_DIRECTION_RX_E"
    end

    if not restoreValue then -- restoreValue == false/nil
        apiName = "cpssDxChCscdPortTypeGet"
        ret,val = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",          "devNum",     devNum },
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum},
            { "IN",     "CPSS_PORT_DIRECTION_ENT",    "portDirection",    portDirection},
            { "OUT",     "CPSS_CSCD_PORT_TYPE_ENT",    "portType"}
        })

        if ret == 0 then
            old_portType = val.portType

            if is_device_eArch_enbled(devNum) then
                -- need to get value before calling cpssDxChCscdPortTypeSet(...)
                apiName = "cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet"
                ret,val = myGenWrapper(
                    apiName, {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    { "OUT", "GT_BOOL", "ignoreFilter"}
                })

                ignoreFilter = val.ignoreFilter

                if cscdInfoRestore[string_dev_port] == nil then
                    cscdInfoRestore[string_dev_port] = {}
                end

                cscdInfoRestore[string_dev_port].mcPhysicalSourceFilteringIgnore = ignoreFilter
            end
        end

        if old_portType == "CPSS_CSCD_PORT_NETWORK_E" then
            -- no need to modify port's mode
            old_portType = false
        end

        if old_portType ~= false then
            -- we need new mode 'NETWORK'
            new_portType = "CPSS_CSCD_PORT_NETWORK_E"
        end


    else
        -- restore the value
        new_portType = restoreValue
    end

    if new_portType ~= false then
        -- update the mode of the port
        apiName = "cpssDxChCscdPortTypeSet"
        ret = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",          "devNum",     devNum },
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum},
            { "IN",     "CPSS_PORT_DIRECTION_ENT",    "portDirection",    portDirection},
            { "IN",     "CPSS_CSCD_PORT_TYPE_ENT",    "portType" , new_portType}
        })
    end

    if restoreValue then
        if cscdInfoRestore[string_dev_port] then
            if cscdInfoRestore[string_dev_port].mcPhysicalSourceFilteringIgnore ~= nil then
                -- need to restore value after calling cpssDxChCscdPortTypeSet(...)
                apiName = "cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet"
                ret,val = myGenWrapper(
                    apiName, {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    { "IN", "GT_BOOL", "ignoreFilter",cscdInfoRestore[string_dev_port].mcPhysicalSourceFilteringIgnore}
                })
            end
            cscdInfoRestore[string_dev_port] = nil
        end
    end


    return old_portType
end

-- function to check if port is trunk member.
--  just print LOG info
--
-- inputs:
--      devNum  - the device.
--      portNum - the port number.
--  returned values:
--      boolean : port is trunk member
--
local function isPortInTrunk(devNum,portNum)
    local string_dev_port = "devNum[" .. devNum .. "] portNum[" .. portNum .. "]"

    local peerPort = isPortHoldPeer(devNum,portNum)
    if peerPort ~= nil then
        -- this port hold peer port that will egress/ingress packets for our port.
        -- so we are not concerned about our port's 'trunkId' config
        return false
    end

    local isMember , trunkId = check_port_is_trunk_member(devNum, portNum)

    if isMember and trunkId then
        printLog (string_dev_port .. " is member of trunk [".. trunkId .. "]")
    end
    -- no more to do
    return isMember
end

-- get 16 bytes of dsa and return indication that dsa is actually 8 or 16 bytes
local function getDsaTagLength(dsaTag)
    -- word 1 bit 31 is 'extended'
    -- this is byte 4 msNibble
    local nibbleOfByte4Bit7 = string.sub(dsaTag,1+(4*2),1+(4*2))
    local value = nibble_to_num[nibbleOfByte4Bit7]

    if value >= 8 then
        -- meaning bit 3 in the nibble is 1
        return 16
    else
        -- meaning bit 3 in the nibble is 0
        return 8
    end

end
-- get dsa tag from packet
local function  getDsa(fullPacket)
    local dsaTagBytes = string.sub(fullPacket, 1 + (12*2), 1 + (12*2) + (16*2) - 1) -- support 16 bytes eDsa
    local dsaTagLength =  getDsaTagLength(dsaTagBytes)
    -- update dsa according to actual length
    dsaTagBytes = string.sub(dsaTagBytes,1,(dsaTagLength*2))

    return dsaTagBytes
end

-- print DSA as STC from string
function luaTgfPrintDsaStcFromString(devNum,dsaString,myInfoString)
    local rc, val = wrlDxChDsaToStruct(devNum , dsaString)
    local dsaStructure = val
    if rc ~=0 then
        dsaStructure = {error_code = returnCodes[rc]}
        if val == nil then
            val = ""
        end
        printLog("Error: " .. myInfoString .. val)
    end

    -- compress the structure to hold only non-zero values
    dsaStructure = table_deep_copy_stripped(dsaStructure,
                    true, -- remove empty structures (got empty due to the compress)
                    nil)  -- default : remove '0','false' values

    if myInfoString then
        --print the DSA tag structure :
        printLog(myInfoString .. " = " , to_string(dsaStructure))
    else
        printLog(to_string(dsaStructure))
    end

end

--print the DSA tag structure
local function printDsaTagStructure(devNum,fullPacket,purposeName)
    local myString = purposeName .. " DSA tag structure "
    -- the ingress packet hold DSA tag .
    -- parse it !
    local dsaTagBytes = getDsa(fullPacket)

    luaTgfPrintDsaStcFromString(devNum,dsaTagBytes,myString)
end

-- DB of 'original' bmp of TPID per port befor changed by consider_ingress_as_untagged(...)
-- each entry hold info : tag0 = ??? , tag1 = ???
local orig_tpid_bmp_per_port = {}

-- build key for 'dev,port'
local function build_dev_port_key(devNum,portNum)
    return "devNum = "..to_string(devNum).." portNum = "..to_string(portNum)
end
-- allow the test to force ingress packets for 'captured traffic' on port to not recognize tags.
local function consider_ingress_as_untagged(devNum,portNum,unset)
    if bpe_802_1_br_is_eArch_Device() then
        -- tests currently works without it ... due to 'add edsa' for packets that go to CPU ...
        -- the 'ethertype' of BPE is not damaged.
        return
    end

    local command_data = Command_Data();
    local tpid0Bmp,tpid1Bmp
    local dev_port_key = build_dev_port_key(devNum,portNum)
    local tpidInfo

    if unset then
        -- restore
        tpidInfo = orig_tpid_bmp_per_port[dev_port_key]

        -- if tpidInfo = nil .. error !
        tpid0Bmp = tpidInfo.tpid0
        tpid1Bmp = tpidInfo.tpid1
    else
        -- set the port to not recognize tag1 (the BPE)
        tpid0Bmp = 0
        tpid1Bmp = 0

        -- get the original values

        apiName = "cpssDxChBrgVlanPortIngressTpidGet"
        local isError , result, values_0 = genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U32", "portNum", portNum},
            { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE0_E" },
            { "OUT", "GT_U32", "tpidBmp"}
        })
        local isError , result, values_1 = genericCpssApiWithErrorHandler(command_data,
            apiName, {
            { "IN", "GT_U8", "devNum", devNum},
            { "IN", "GT_U32", "portNum", portNum},
            { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
            { "OUT", "GT_U32", "tpidBmp"}
        })

        orig_tpid_bmp_per_port[dev_port_key] = {tpid0 = values_0.tpidBmp , tpid1 = values_1.tpidBmp }
    end

    apiName = "cpssDxChBrgVlanPortIngressTpidSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "portNum", portNum},
        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE0_E" },
        { "IN", "GT_U32", "tpidBmp", tpid0Bmp }
    })

    apiName = "cpssDxChBrgVlanPortIngressTpidSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN", "GT_U8", "devNum", devNum},
        { "IN", "GT_U32", "portNum", portNum},
        { "IN", "CPSS_ETHER_MODE_ENT", "ethMode", "CPSS_VLAN_ETHERTYPE1_E" },
        { "IN", "GT_U32", "tpidBmp", tpid1Bmp }
    })

    if unset then
        -- we are done with the entry
        orig_tpid_bmp_per_port[dev_port_key] = nil
    end

end

-- check egress rates
local function checkExpectedEgressInfo(local_egressInfoTable)
    local command_data = Command_Data();
    local GT_OK = 0
    local GT_FAIL = 1
    local GT_BAD_PARAM = 4
    local packetSize = 64 
    
    local function check_up_to_4_ports_traffic_rates(devNum, portInfoArr)
        --printLog("portInfoArr",to_string(portInfoArr))
    
        local luaWrapperParams = {}
        local total_ports
        local interval = 1000
        
        local index = 1
        for ii=1,#portInfoArr do
            luaWrapperParams[index] = devNum
            index = index + 1

            luaWrapperParams[index] = portInfoArr[ii].portNum
            index = index + 1

            luaWrapperParams[index] = packetSize
            index = index + 1
        end
        
        total_ports = #portInfoArr
        local port_max = {}
        local port_rx = {}
        local port_tx = {}
        
        rc,port_max[1],port_rx[1],port_tx[1],
           port_max[2],port_rx[2],port_tx[2],
           port_max[3],port_rx[3],port_tx[3],
           port_max[4],port_rx[4],port_tx[4] = 
        prvLuaTgfPortsRateGet(total_ports , interval , 
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
   
        --[[
        printLog("port_max",to_string(port_max))
        printLog("port_rx",to_string(port_rx))
        printLog("port_tx",to_string(port_tx))
        --]]
            
        -- entry is from checkedPorts[]
        local function func__check_rate(command_data, entry)
            local max_diff_allowed = 2
            local expected_rate_in_Kbps = entry.rate -- no convert needed !
            local actual_rate_in_Kbps = convertPpsToKbps(entry.port_tx , packetSize)
            local max_rate_in_Kbps    = portSpeedKbpsGet(devNum,entry.portNum)
            
            local description = string.format("compare expected rate with actual rate on port %d expected rate (%d)Kbps , got (%d)Kbps",
                entry.portNum,
                expected_rate_in_Kbps,
                actual_rate_in_Kbps)
            
            if entry.rateDiffInPercent ~= nil then
                max_diff_allowed = entry.rateDiffInPercent
            end
            
            if entry.rate ~= 0 and entry.port_tx == 0 then
                return true , nil,nil , description
            end

            -- XG ports in loopback can generate more then 100% WS ... we get 132% on Aldrin .
            if actual_rate_in_Kbps > max_rate_in_Kbps and 
               max_rate_in_Kbps >= 10*1000*1000 then
                max_diff_allowed = 40
            end
            --[[
            printLog("entry.rate",entry.rate)
            printLog("entry.rateDiffInPercent",entry.rateDiffInPercent)
            printLog("expected_rate_in_Kbps",expected_rate_in_Kbps)
            printLog("actual_rate_in_Kbps",actual_rate_in_Kbps)
            printLog("max_rate_in_Kbps"   ,max_rate_in_Kbps)
            printLog("max_diff_allowed"   ,max_diff_allowed)
            --]]
            if isSimulationUsed() then
                -- get the simulation WS
                local simMaxRate = get_wirespeed_rate_on_simulation(devNum,entry.portNum)
                
                max_rate_in_Kbps = convertPpsToKbps(simMaxRate,packetSize)

                --printLog("-->simulation --> max_rate_in_Kbps ="   ,max_rate_in_Kbps)
                
                -- the runs on linux simulation cause some time big diffs between 'simMaxRate' and current value
                max_diff_allowed = 0xFFFFFFFF
            end
            
            local percent_actual   = get_percent(actual_rate_in_Kbps  ,expected_rate_in_Kbps)
            local diff = math.abs (percent_actual - 100)
            --printLog("percent_actual",percent_actual)
            --printLog("diff"   ,diff)

            local isError = false
            local actualValue , expectedValue = true,true
            
            if diff > max_diff_allowed then
                local add_description = string.format("ERROR : got diff(%d)percent more than (%d)percent , ",
                    diff,
                    max_diff_allowed)
                description = add_description .. description 
                
                isError = true
                actualValue = false
            end
            
            return isError,true,true,description
        end
        
        for ii=1,#portInfoArr do
            portInfoArr[ii].port_tx = port_tx[ii]
        end
        
        testCheckExpectedValues(command_data,func__check_rate,portInfoArr) 

        return GT_OK
    end -- end of check_up_to_4_ports_traffic_rates
    
    local devNum = devEnv.dev
    local portInfoArr = {}
    for index,entry in pairs(local_egressInfoTable) do
        local port_index = #portInfoArr
        portInfoArr[port_index + 1] = {portNum = entry.portNum , rate = entry.rate, rateDiffInPercent = entry.rateDiffInPercent}
        
        if entry.rate == nil then
            printLog(string.format("ERROR : port (%d) - no rate value specified",
                entry.portNum))
            setFailState()
            return GT_BAD_PARAM
        end
        
        if 4 == #portInfoArr then
            -- we are with 4 ports now 
            local rc = check_up_to_4_ports_traffic_rates(devNum, portInfoArr)
            if rc ~= GT_OK then
                return rc
            end
            
            -- empty the array
            portInfoArr = {}
        end
    end

    if 1 <= #portInfoArr then
        -- we are with 1..3 ports now 
        local rc = check_up_to_4_ports_traffic_rates(devNum, portInfoArr)
        if rc ~= GT_OK then
            return rc
        end
        
        -- empty the array
        portInfoArr = {}
    end
    
    return GT_OK
end

-- currently our tests are with CPU uses 'SDMA'
-- TBD not SDMA CPU port !!!
local isCpuUsedSdma = true

-- indication for field 'transmitInfo.burstCount' to start generate 'full wirespeed' 
transmit_continuous_wire_speed = "continuous-wire-speed"
-- indication for field 'transmitInfo.burstCount' to stop generate 'full wirespeed' 
stop_transmit_continuous_wire_speed = "stop continuous-wire-speed"

--[[
-- function to transmit packet(s) and check egress ports for : counters (packets,bytes) and actual packets.
-- the function is very flexible in terms of optional parameters
-- parameters are:
INPUTS :
      *****************************
      INPUT1 : transmitInfo - Hold info about the transmission :
          (if transmitInfo == nil --> we not sending traffic !!!
            this is valid for cases where need to check egress traffic that may be in the device.
            in this case we look for 'rate' in egressInfoTable          
            )
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
          loopbackStorming - true/false indication that the 'ingress port' will be 'stormed' due to 'mac loopback' ... as one of
                the egress interfaces 'bridged' packet back to it.
                when 'true' :
                    1. the expected ingress counters checked only for 'not zero'
                    2. the expected egress counters on port checked as:
                        a. if expected '0' then keep expecting '0'
                        b. else expect 'non zero'
              -- when nil --> considered false
              -- NOTE: this is regardless to the number of burstCount.
          expectPacketFilteredBeforLoopback - true/false/nil indication that the transmission will not be able
                to reach the 'ingress port'  , and is about to be dropped on path from the 'cpu port' to the 'looped ingress port'
                when 'true' :
                    The counters of the 'ingress port' are checked for Rx,Tx to be ZERO , and we ignore the info in egressInfoTable !!!
              -- when nil --> considered false
          doNotModifyMacLoopback - indication that must not change the MAC loopback mode on the transmit port
              -- when nil --> considered false
          isPacketExpectedToBeFilteredByTheMacPhyLayer - optional parameter, indicates that ingress packet is expected 
                                                         to be filtered by the MAC/PHY layer
      *****************************
      INPUT2 : egressInfoTable - A 'table' that holds entries (index 1..max)
          (if egressInfoTable == nil --> egress not checked !!!
            this is valid for 'SA learning' purposes !!!!)
          each index in egressInfoTable hold next fields :
          devNum - the egress device
                  -- when nil --> considered = transmitInfo.devNum
          portNum - the egress port
                  -- when nil --> error
                  -- NOTE : the if transmitInfo.portNum == egressInfoTable[ii].portNum then the flag 'transmitInfo.loopbackStorming = true'
                            must be also set , because :
                            1. this 'egress port' is also the 'ingress port' that internally configured as 'loopback port'
                            2. this 'egress port' will have same value of counters as 'ingress port' --> 'never 0'
                  - the "CPU" string value can be used for packet trapped/mirrored to CPU

          pktInfo - the egress packet info
                  -- when nil --> egress packet is not compared to received one.
                      pktInfo.fullPacket ~= nil --> support 'fullPacket' by lua test
                  -- NOTE : the loopback port 'capturing' limit us from having transmitInfo.portNum == egressInfoTable[ii].portNum
                            because the 'ingress packet' will not be able to 'bridge' ... it will be directly sent to CPU (as mirror)
                  pktInfo.maskedBytesTable - table of bytes to mask when 'compare' packets:
                      each entry is of type:
                        startByte - the byte that starts the not compared range
                        endByte   - the byte that ends the not compared range
                        startByteCompareMask - mask for start byte compared bits (default 0x00)
                        endByteCompareMask - mask for end byte compared bits (default 0x00)
          pktInfoSet or pktInfoSequence - instead of pktInfo - both of them are lists of
              data in pktInfo format. The difference is that pktInfoSet requires the contents
              of packets in any order, but pktInfoSequence - in given order only
          packetCount - the egress packet count
              -- when nil --> considered transmitInfo.burstCount
          packetLength - the num of bytes that expected per packet transmitted packet.
              -- NOTE: used ONLY when pktInfo is nil.
          cpuCodeNumber - numeric value of CPU Code got with chaptered packet as
              described in Functional Specification of PP
          captureMode -  "mirroring" / "pcl".
              -- when nil --> considered "mirroring"
              -- actually any value that is not "pcl" is considered "mirroring"
              -- NOTE: used ONLY when pktInfo exists(not nil).
          packetCountIngress - allow to specify that the 'egress' port need to be checked for 'ingress counters' different then 0 !
          rate - the expected rate kbps (1000 bits per second) that egress the port.
          rateDiffInPercent - acceptable difference between actual rate and expected in percent. 
                              Default value is 2% if parameter is ommited.
      *****************************
 function return 'GT_STATUS' that explain 'C' type errors from 'DUT'
--]]
function luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
    local GT_OK = 0
    local GT_FAIL = 1
    local GT_BAD_PARAM = 4
    local GT_BAD_PTR = 5
    local rc -- return code from TGF
    local captureOnTime = 0
    local allEgressPorts = {} -- array of all egress ports
    local expectedEgressPacket , actualEgressPacket
    local needToCheckCounters = true
    local numOfErrorsToReport = 0
    local allEgressPortsWithoutCapture = true
    local firstIndexWithCapture = nil

    local local_transmitInfo = deepcopy(transmitInfo);
    local local_egressInfoTable = deepcopy(egressInfoTable);

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

        printLog("\n\n ===============")
        printLog(
            "Inject Packet into"
            .. " Dev "  .. local_transmitInfo.devNum
            .. " Port " .. local_transmitInfo.portNum
            .. " :")


        --support 'fullPacket' by lua test .. convert it to 'payload' before sent the 'tgf'
        if local_transmitInfo.pktInfo.fullPacket ~= nil then
            local_transmitInfo.pktInfo.fullPacket = string.lower(local_transmitInfo.pktInfo.fullPacket)
            
            -- pad the packet with zeros (if needed) ... minimal size is 60 bytes (without the CRC)
            local_transmitInfo.pktInfo.fullPacket = 
                pad_packet_format(local_transmitInfo.pktInfo.fullPacket)
            
            local_transmitInfo.pktInfo.partsArray={
                {type="TGF_PACKET_PART_PAYLOAD_E", partPtr= local_transmitInfo.pktInfo.fullPacket } }

            local printable_ingressPacket = packetAsStringToPrintableFormat(local_transmitInfo.pktInfo.fullPacket)

            printLog(printable_ingressPacket)
        end
    else
        printLog("\n\n ===============")
        printLog(
            "Stop WS traffic from "
            .. " Dev "  .. local_transmitInfo.devNum
            .. " Port " .. local_transmitInfo.portNum)
    end

    --[[
        check if 'sender port' is configured as 'cascade port' on 'egress direction'
        if 'yes' then we need temporarily to disable it ... so the 'from_cpu' of the
        mechanize can be 'stripped' from the packet before it loopbacks ... otherwise
        we will get endless loop (by the 'from_cpu' to 'sender port')
    --]]
    local senderCscdValue = supportCscdPort(local_transmitInfo.devNum,local_transmitInfo.portNum,true,false)

    -- check if ingress port have peer port
    local peerPort_transmit = isPortHoldPeer(local_transmitInfo.devNum,local_transmitInfo.portNum)
    
    isPortInTrunk(local_transmitInfo.devNum,local_transmitInfo.portNum)
    
    if senderCscdValue or peerPort_transmit then
        -- the ingress packet hold DSA tag .
        -- parse it !
        local purposeName = "ingress packet"
        printDsaTagStructure(local_transmitInfo.devNum,
            local_transmitInfo.pktInfo.fullPacket,
            purposeName)
    end

    local transmitPortNum
    local withLoopBack_transmit

    if peerPort_transmit then
        -- we actually send packet to 'peer' port .. that is connected to the 'ingress test port'
        -- by physical cable (or slan in simulation)
        transmitPortNum = peerPort_transmit
        withLoopBack_transmit = false
    else
        if(type(local_transmitInfo.portNum) == "string" and 
           isCpuSdmaPort(local_transmitInfo.devNum,local_transmitInfo.portNum)) 
        then
            transmitPortNum = cpuSdmaNumber
        else
            transmitPortNum = local_transmitInfo.portNum
        end
        withLoopBack_transmit = true
    end

    if local_transmitInfo.doNotModifyMacLoopback == true then
        -- the caller not want to modify the MAC loopback on the transmit port
        -- assumed that port already in loopback mode ... and must not undo it after sending
        withLoopBack_transmit = false
    end
    
    if isCpuSdmaPort(local_transmitInfo.devNum,local_transmitInfo.portNum) then
        --CPU port is SDMA and not support mac loopback
        withLoopBack_transmit = false
    end

    if local_egressInfoTable == nil or
       local_transmitInfo.expectPacketFilteredBeforLoopback
    then
        local burstCount,pktInfo
        local pktInfo = local_transmitInfo.pktInfo

        if local_transmitInfo.burstCount == transmit_continuous_wire_speed then
            burstCount = 0xFFFFFFFF -- start WS
        elseif local_transmitInfo.burstCount == stop_transmit_continuous_wire_speed then
            burstCount = 0 -- stop WS
            -- dummy
            pktInfo = {fullPacket = "3333333333333333333333333333333"} 
        else
            burstCount = local_transmitInfo.burstCount -- regular burst
        end
    
        -- send packet without check egress ports
        rc = prvLuaTgfTransmitPackets(
            local_transmitInfo.devNum ,
            transmitPortNum ,
            pktInfo ,
            burstCount ,
            withLoopBack_transmit ,--withLoopBack
            nil -- vfdArray
        )

        if rc ~= GT_OK then
            printLog (
                "ERROR : prvLuaTgfTransmitPackets failed with code " .. rc
                .. " dev " .. local_transmitInfo.devNum
                .. " port " .. transmitPortNum)
        end

        if senderCscdValue then
            -- restore cascade port mode
            supportCscdPort(local_transmitInfo.devNum,local_transmitInfo.portNum,true,senderCscdValue)
            senderCscdValue = nil
        end

        if local_transmitInfo.expectPacketFilteredBeforLoopback  then
            local devPortCounters = {}
            devPortCounters[local_transmitInfo.devNum] = {[transmitPortNum] = {Rx = 0 , Tx = 0}}

            --check expected counters:
            -- print the expected counters
            printExpectedCounters(devPortCounters)

            printLog("\n")
            printLog("EXPECT the packet from the CPU to be filtered on the way to the 'looped ingress port' ...")
            local isError = checkExpectedCounters(nil,devPortCounters)
            if  isError == true then
                printLog("ERROR: Counters check failed (packet was not filtered/dropped !)")
                numOfErrorsToReport = numOfErrorsToReport + 1
            else
                printLog("Counters check Passed ! (packet was filtered/dropped)")
            end
        end

        -- reset mac counters of 'sender port'
        resetPortCounters(local_transmitInfo.devNum,local_transmitInfo.portNum)
        if peerPort_transmit then
            -- reset mac counters of 'peer port'
            resetPortCounters(local_transmitInfo.devNum,peerPort_transmit)
        end

        -- finished processing
        printLog(
            "\n\n Finished handling Packet from Port "
            .. local_transmitInfo.devNum .. "/"
            .. local_transmitInfo.portNum
            .. " (send packet without check egress ports)")

        return rc;
    end

    didError_GT_BAD_PARAM = false

    -- sum all the ports that we need to send traffic to
    for index,egressEntry in pairs(local_egressInfoTable) do

        -- support omit of many parameters ... set 'defaults'
        if egressEntry.devNum == nil           then egressEntry.devNum = local_transmitInfo.devNum end
        if egressEntry.portNum == nil          then
            didError_GT_BAD_PARAM = "ERROR : egressEntry.portNum == nil"
            break
        end

        -- check that used only one way of packet contents checking
        local pkt_info_types_numof = 0;
        if egressEntry.pktInfo then
            pkt_info_types_numof = pkt_info_types_numof + 1
        end
        if egressEntry.pktInfoSet then
            pkt_info_types_numof = pkt_info_types_numof + 1
        end
        if egressEntry.pktInfoSequence then
            pkt_info_types_numof = pkt_info_types_numof + 1
        end
        if pkt_info_types_numof > 1 then
            didError_GT_BAD_PARAM =
               "ERROR : more then one of pktInfo, pktInfoSet, pktInfoSequence used";
            break
        end

        if local_transmitInfo.burstCount ~= 1 and
            (egressEntry.pktInfoSet ~= nil or
             egressEntry.pktInfoSequence ~= nil)
         then
             didError_GT_BAD_PARAM =
                "ERROR : burstCount > 1 and pktInfoSet or pktInfoSequence used";
             break
         end

        if egressEntry.pktInfo then
            if not egressEntry.packetCount then
                egressEntry.packetCount = local_transmitInfo.burstCount;
            end
            if not egressEntry.byteCount then
                local packetLength = egressEntry.pktInfo.packetLength;
                if not packetLength then
                    packetLength = calcPacketLength(egressEntry.pktInfo);
                end
                egressEntry.byteCount = considerCalcOfCrc4Bytes(
                    egressEntry.packetCount, packetLength);
            end
        end
        if egressEntry.pktInfoSet then
            if not egressEntry.packetCount then
                egressEntry.packetCount = #(egressEntry.pktInfoSet)
            end
            if not egressEntry.byteCount then
                local byteCount = 0;
                for _i = 1,#(egressEntry.pktInfoSet) do
                    local packetLength = egressEntry.pktInfoSet.packetLength;
                    if not packetLength then
                        packetLength = calcPacketLength(egressEntry.pktInfoSet[_i]);
                    end
                    byteCount = byteCount + 4 --[[CRC--]] + packetLength;
                end
                egressEntry.byteCount = byteCount;
            end
        end
        if egressEntry.pktInfoSequence then
            if not egressEntry.packetCount then
                egressEntry.packetCount = #(egressEntry.pktInfoSequence)
            end
            if not egressEntry.byteCount then
                local byteCount = 0;
                for _i = 1,#(egressEntry.pktInfoSequence) do
                    local packetLength = egressEntry.pktInfoSequence.packetLength;
                    if not packetLength then
                        packetLength = calcPacketLength(egressEntry.pktInfoSequence[_i]);
                    end
                    byteCount = byteCount + 4 --[[CRC--]] + packetLength;
                end
                egressEntry.byteCount = byteCount;
            end
        end

        if egressEntry.packetCount == nil      then
            egressEntry.packetCount = local_transmitInfo.burstCount
        end

        if egressEntry.devNum  == local_transmitInfo.devNum     and
           egressEntry.portNum == local_transmitInfo.portNum    and
           local_transmitInfo.loopbackStorming ~= true          and
           not isCpuSdmaPort(local_transmitInfo.devNum,local_transmitInfo.portNum)
        then
            didError_GT_BAD_PARAM = "ERROR : egressEntry.portNum == transmitInfo.portNum , but 'not expecting' storming ?!"
            break
        end

        if local_transmitInfo.loopbackStorming == true then
            -- state to the 'counters checkers' that the expect 'storming'
            if egressEntry.packetCount ~= 0 then
                egressEntry.packetCount = stormingExpectedIndication
            end
        end

        if egressEntry.devNum  == local_transmitInfo.devNum     and
           egressEntry.portNum == local_transmitInfo.portNum    and
           not isCpuSdmaPort(local_transmitInfo.devNum,local_transmitInfo.portNum) and
           (egressEntry.pktInfo ~= nil or
            egressEntry.pktInfoSet ~= nil or
            egressEntry.pktInfoSequence ~= nil)
        then
            -- NOTE : the loopback port 'capturing' limit us because the 'ingress packet' will not be able to 'bridge' ...
            --        it will be directly sent to CPU (as mirror)
            didError_GT_BAD_PARAM = "ERROR : egressEntry.portNum == transmitInfo.portNum , and egressEntry.pktInfo not nil "
            break
        end

        if egressEntry.devNum  == local_transmitInfo.devNum     and
           egressEntry.portNum == local_transmitInfo.portNum
        then
            -- do not add the ingress port to 'egress list' , because it effect the expected egress counters
        else
            if type(egressEntry.portNum) == "number" then
                allEgressPorts["devNum "..egressEntry.devNum.." portNum "..egressEntry.portNum] =
                    {devNum = egressEntry.devNum,
                     portNum = egressEntry.portNum ,
                     packetCount = egressEntry.packetCount,
                     byteCount =  egressEntry.byteCount,
                     packetCountIngress = egressEntry.packetCountIngress}
            
            isPortInTrunk(egressEntry.devNum,egressEntry.portNum)
            end
        end

        if (egressEntry.pktInfo ~= nil or
            egressEntry.pktInfoSet ~= nil or
            egressEntry.pktInfoSequence ~= nil)
        then
            -- indicate that at least one egress port needs 'capture'
            allEgressPortsWithoutCapture = false

            if firstIndexWithCapture == nil then
                -- svae the index of the 'first need to capture'
                firstIndexWithCapture = index
            end
        end
    end

    if didError_GT_BAD_PARAM then
        if senderCscdValue then
            -- restore cascade port mode
            supportCscdPort(local_transmitInfo.devNum,local_transmitInfo.portNum,true,senderCscdValue)
            senderCscdValue = nil
        end

        printLog(didError_GT_BAD_PARAM)
        setFailState()
        return GT_BAD_PARAM
    end


    -- reset mac counters of 'egress ports'
    for _index,_egressPortEntry in pairs(allEgressPorts) do
        if type(_egressPortEntry.portNum) == "number" then
            -- to bypass port == "CPU"
            resetPortCounters(_egressPortEntry.devNum,_egressPortEntry.portNum);
        end
    end

    -- reset mac counters of 'sender port'
    resetPortCounters(local_transmitInfo.devNum,local_transmitInfo.portNum)
    if peerPort_transmit ~= nil then
        -- reset mac counters of 'peer port'
        resetPortCounters(local_transmitInfo.devNum,peerPort_transmit)
    end

    -- check if non of the egress ports need 'capture' (for full compare)
    if allEgressPortsWithoutCapture == true then
        -- send packet without doing capture on any other port
        rc = prvLuaTgfTransmitPackets(
            local_transmitInfo.devNum ,
            transmitPortNum ,
            local_transmitInfo.pktInfo ,
            local_transmitInfo.burstCount ,
            withLoopBack_transmit ,--withLoopBack
            nil -- vfdArray
            )

        if senderCscdValue then
            -- restore cascade port mode
            supportCscdPort(local_transmitInfo.devNum,local_transmitInfo.portNum,true,senderCscdValue)
            senderCscdValue = nil
        end

        if rc ~= GT_OK then
            printLog ("ERROR : prvLuaTgfTransmitPackets failed with code " .. rc)
            -- finished processing
            printLog(
                "\n\n Finished handling Packet from Port "
                .. local_transmitInfo.devNum .. "/"
                .. local_transmitInfo.portNum
                .. " (send packet without capture)")

            return rc
        end
    else
        -- we make sure that the first element in the 'local_egressInfoTable' need 'capture'
        if firstIndexWithCapture ~= nil and firstIndexWithCapture ~= 1 then
            -- make the 'need a capture' as first on in the table
            -- swap places with the one in index [1]
            temp = local_egressInfoTable[firstIndexWithCapture]
            local_egressInfoTable[firstIndexWithCapture] = local_egressInfoTable[1]
            local_egressInfoTable[1] = temp
        end

    end

    local senderPacketLen = calcPacketLength(local_transmitInfo.pktInfo)

    didError_GT_BAD_PARAM = false
    didError_general_rc = false

    local pktInfoListArr = {};
    local pktIndexListArr = {};

    for index,egressEntry in pairs(local_egressInfoTable) do
        -- lists of expected packets and indexes of expected packet to check
        -- per received packet
        local pktInfoList = nil;
        local pktIndexList = nil;
        if egressEntry.pktInfo then
            pktInfoList = {egressEntry.pktInfo};
            pktIndexList = {{1}};
        end
        if egressEntry.pktInfoSequence then
            pktInfoList = egressEntry.pktInfoSequence;
            pktIndexList = {};
            for _i = 1, #(egressEntry.pktInfoSequence) do
                -- check _i-th received packet by _i-th expected only
                pktIndexList[_i] = {_i};
            end
        end
        if egressEntry.pktInfoSet then
            pktInfoList = egressEntry.pktInfoSet;
            -- common list for all received packets
            -- index of found pattern will be removed each time
            local tmp_pktIndexList = {};
            for _i = 1, #(egressEntry.pktInfoSet) do
                tmp_pktIndexList[_i] = _i;
            end
            pktIndexList = {};
            for _i = 1, #(egressEntry.pktInfoSet) do
                pktIndexList[_i] = tmp_pktIndexList;
            end
        end
        -- save lists
        pktInfoListArr[index] = pktInfoList;
        pktIndexListArr[index] = pktIndexList;
    end

    if didError_GT_BAD_PARAM then
        printLog(didError_GT_BAD_PARAM)
        setFailState()
        return GT_BAD_PARAM
    end

    --print("pktInfoListArr " .. to_string(pktInfoListArr));
    --print("pktIndexListArr " .. to_string(pktIndexListArr));

    -- Transmit Port Expected counters calculation
    local devPortCounters = {}
    local portCounters = {}

    -- state that the sender in looped back .. so rx = tx = burstCount
    portCounters[local_transmitInfo.portNum] = {}
    portCounters[local_transmitInfo.portNum].Tx = local_transmitInfo.burstCount
    if local_transmitInfo.loopbackStorming == true then
        -- state to the 'counters checkers' that the expect 'storming'
        portCounters[local_transmitInfo.portNum].isSrcOfStorming = true -- this port should get a least 2 packets and not just 1
        portCounters[local_transmitInfo.portNum].Tx = stormingExpectedIndication
    end

    if nil == local_transmitInfo.isPacketExpectedToBeFilteredByTheMacPhyLayer then
        portCounters[local_transmitInfo.portNum].Rx = portCounters[local_transmitInfo.portNum].Tx
        portCounters[local_transmitInfo.portNum].RxBytes = portCounters[local_transmitInfo.portNum].TxBytes
    end

    portCounters[local_transmitInfo.portNum].TxBytes =
        considerCalcOfCrc4Bytes(portCounters[local_transmitInfo.portNum].Tx,senderPacketLen)

    if peerPort_transmit ~= nil then
        -- split the counters of rx from the tx ...
        -- peer port hold 'tx' counters
        portCounters[peerPort_transmit] = {}
        portCounters[peerPort_transmit].TxBytes = portCounters[local_transmitInfo.portNum].TxBytes
        portCounters[peerPort_transmit].Tx  = portCounters[local_transmitInfo.portNum].Tx

        -- 'orig' port hold 'rx' counters
        portCounters[local_transmitInfo.portNum].TxBytes = 0
        portCounters[local_transmitInfo.portNum].Tx = 0
    end
    
    if isCpuSdmaPort(local_transmitInfo.devNum,local_transmitInfo.portNum) then
        -- 1. the CPU counters not hold 'Tx counter'
        -- 2. the test not mean to 'loopback' on this ports , but just to indicate that CPU is the 'real sender' of the test (not cscd/network port)
        portCounters[local_transmitInfo.portNum].TxBytes = nil
        portCounters[local_transmitInfo.portNum].Tx      = nil
        
        if (isGmUsed()) then
            -- the GM not supports the SDMA counters 
            portCounters[local_transmitInfo.portNum].RxBytes = nil
            portCounters[local_transmitInfo.portNum].Rx      = nil
        end
        
    end
    

    for index,egressEntry in pairs(local_egressInfoTable) do
        local egressCscdValue

        -- support omit of many parameters ... set 'defaults'
        if egressEntry.devNum == nil           then egressEntry.devNum = local_transmitInfo.devNum end
        if egressEntry.portNum == nil          then
            didError_GT_BAD_PARAM = "ERROR : egressEntry.portNum --> nil "
            break
        end

        -- lists of expected packets and indexes of expected packet to check
        -- per received packet
        local pktInfoList = pktInfoListArr[index];
        local pktIndexList = pktIndexListArr[index];
        --print("index " .. to_string(index));
        --print("pktIndexList " .. to_string(pktIndexList));
        --print("pktInfoList " .. to_string(pktInfoList));

        if pktInfoList then
            -- reset mac counters of 'egress ports'
            for _index,_egressPortEntry in pairs(allEgressPorts) do
                if type(_egressPortEntry.portNum) == "number" then
                    -- to bypass port == "CPU"
                    resetPortCounters(_egressPortEntry.devNum,_egressPortEntry.portNum)
                end
            end

            -- reset mac counters of 'sender port'
            resetPortCounters(local_transmitInfo.devNum,local_transmitInfo.portNum)
            if peerPort_transmit ~= nil then
                -- reset mac counters of 'peer port'
                resetPortCounters(local_transmitInfo.devNum,peerPort_transmit)
            end

            if egressEntry.captureMode == nil or egressEntry.captureMode ~= "pcl" then
                captureMode = "TGF_CAPTURE_MODE_MIRRORING_E"
            else
                captureMode = "TGF_CAPTURE_MODE_PCL_E"
            end

            -- check if egress port is cascade port
            if type(egressEntry.portNum) == "number" then
                egressCscdValue = supportCscdPort(egressEntry.devNum,egressEntry.portNum,false,false)
            end

            -- check if egress port have peer port
            peerPort_egress = isPortHoldPeer(egressEntry.devNum,egressEntry.portNum)
            if peerPort_egress then
                -- we actually capture packet on 'peer' port .. that is connected to the 'egress test port'
                -- by physical cable (or slan in simulation)
                egressPortNum = peerPort_egress
                withLoopBack_egress = false

                -- reset mac counters of 'peer port'
                resetPortCounters(egressEntry.devNum,peerPort_egress)
            else
                if string.upper(egressEntry.portNum) == "CPU" then
                    -- packet mirrored or trapped to CPU 
                    -- 0xFFFFFFFF means that all traffic trapped/mirrored to CPU will be captured 
                    -- due to special tests configuration without intervention of traffic generator
                    egressPortNum = 0xFFFFFFFF
                    withLoopBack_egress = false
                else
                    egressPortNum = egressEntry.portNum
                    withLoopBack_egress = true
                end
            end

            -- set the 'force ingress packets for 'captured traffic' on port to not recognize tags'
            if type(egressEntry.portNum) == "number" then
                consider_ingress_as_untagged(egressEntry.devNum,egressEntry.portNum,false)
            end

            
            
            printLog(
                "'capture' on port: "
                .. egressEntry.devNum .. "/"
                .. egressEntry.portNum)
            -- now we are ready to call the 'C' function
            rc = prvLuaTgfTransmitPacketsWithCapture(
                local_transmitInfo.devNum ,
                transmitPortNum ,
                local_transmitInfo.pktInfo ,
                local_transmitInfo.burstCount ,
                egressEntry.devNum ,
                egressPortNum ,
                captureMode,
                captureOnTime,
                withLoopBack_transmit,
                withLoopBack_egress
            )

            -- restore the 'force ingress packets for 'captured traffic' on port to not recognize tags'
            if type(egressEntry.portNum) == "number" then
                consider_ingress_as_untagged(egressEntry.devNum,egressEntry.portNum,true)

                if egressCscdValue then
                    -- restore cascade port mode
                    supportCscdPort(egressEntry.devNum,egressEntry.portNum,false,egressCscdValue)
                end
            end


            if rc ~= GT_OK then
                didError_general_rc = { my_string = "ERROR : prvLuaTgfTransmitPacketsWithCapture failed with code " .. rc , my_rc = rc}
                numOfErrorsToReport = numOfErrorsToReport + 1
                break
            end
        end

        -- only the first iteration should check counters
        if needToCheckCounters == true then
            -- print("allEgressPorts" .. to_string(allEgressPorts));
            for _index,_egressPortEntry in pairs(allEgressPorts) do

                -- state that the egress port should egress the expected number of packets
                portCounters[_egressPortEntry.portNum] = {}
                portCounters[_egressPortEntry.portNum].Tx = _egressPortEntry.packetCount
                portCounters[_egressPortEntry.portNum].TxBytes = nil -- not checked

                if egressEntry.portNum == _egressPortEntry.portNum and
                   egressEntry.devNum  == _egressPortEntry.devNum and
                   allEgressPortsWithoutCapture ~= true
                then
                    -- this egress port is also 'currently looped' .. so rx=tx=packetCount
                    portCounters[_egressPortEntry.portNum].Rx = portCounters[_egressPortEntry.portNum].Tx
                    portCounters[_egressPortEntry.portNum].RxBytes = portCounters[_egressPortEntry.portNum].TxBytes
                else
                    -- the port only egress packets and not supposed to get packets
                    if (_egressPortEntry.packetCountIngress) then
                        portCounters[_egressPortEntry.portNum].Rx = _egressPortEntry.packetCountIngress
                        portCounters[_egressPortEntry.portNum].RxBytes = nil -- not checked
                    else
                        portCounters[_egressPortEntry.portNum].Rx = 0
                        portCounters[_egressPortEntry.portNum].RxBytes = 0
                    end
                end

                -- check if egress port have peer port
                peerPort_egress = isPortHoldPeer(_egressPortEntry.devNum,_egressPortEntry.portNum)
                if peerPort_egress ~= nil then
                    -- split the counters of rx from the tx ...
                    -- peer port hold 'rx' counters
                    portCounters[peerPort_egress] = {}
                    portCounters[peerPort_egress].RxBytes = portCounters[_egressPortEntry.portNum].TxBytes
                    portCounters[peerPort_egress].Rx  = portCounters[_egressPortEntry.portNum].Tx

                    -- 'orig' egress port hold 'tx' counters
                    portCounters[_egressPortEntry.portNum].RxBytes = 0
                    portCounters[_egressPortEntry.portNum].Rx = 0
                end

                if portCounters[_egressPortEntry.portNum].Tx == 0 then
                    printLog(
                        "Expect port "
                        .._egressPortEntry.devNum.."/"
                        .._egressPortEntry.portNum.." not to egress packets")
                else
                    printLog(
                        "Expect port "
                        .._egressPortEntry.devNum.."/"
                        .._egressPortEntry.portNum.." to egress [" .. portCounters[_egressPortEntry.portNum].Tx .. "] packets")
                end

                if portCounters[_egressPortEntry.portNum].TxBytes ~= nil and
                   portCounters[_egressPortEntry.portNum].TxBytes ~= 0
                then
                    printLog(
                        "Expect port "
                        .._egressPortEntry.devNum.."/"
                        .._egressPortEntry.portNum.." to egress ["
                        .. portCounters[_egressPortEntry.portNum].TxBytes
                        .. "] bytes (include crc 4 bytes)")
                end

            end

            devPortCounters[egressEntry.devNum] = portCounters

            --check expected counters:
            -- print the expected counters
            printExpectedCounters(devPortCounters)

            printLog("\n check the counters ...")
            local isError = checkExpectedCounters(nil,devPortCounters)
            if  isError == true then
                printLog("ERROR: Counters check failed")
                numOfErrorsToReport = numOfErrorsToReport + 1
            else
                printLog("Counters check Passed !")
            end
        end

        needToCheckCounters = false

        if allEgressPortsWithoutCapture == true then
            -- we not need to capture any port so we are done here
            break
        end

        -- pktInfoList
        -- pktIndexList

        peerPort_egress = isPortHoldPeer(egressEntry.devNum,egressEntry.portNum)

        if pktIndexList then
            local getFirst = true;
            for _pktRcvIdx,_pktChkIdxList in pairs(pktIndexList) do --

                -- get captured packet
                if peerPort_egress ~= nil then
                    -- we actually capture packet on 'peer' port .. that is connected to the 'egress test port'
                    -- by physical cable (or slan in simulation)
                    egressPortNum = peerPort_egress
                    printLog ("we actually capture packet on 'peer' port : " .. to_string(egressPortNum))
                else
                    if string.upper(egressEntry.portNum) == "CPU" then
                        egressPortNum = transmitPortNum
                    else
                        egressPortNum = egressEntry.portNum
                    end

                end



                local egressPacketParts -- not used !!!
                local rxParam
                rc , egressPacketParts , actualEgressPacket, rxParam  =
                    prvLuaTgfRxCapturedPacket(egressEntry.devNum, egressPortNum, "TGF_PACKET_TYPE_ANY_E", getFirst);
                getFirst = false;

                local didError = false
                if rc ~= GT_OK or actualEgressPacket == nil then
                    if rc == GT_OK then rc = GT_BAD_PTR end

                    printLog (
                        "ERROR: prvLuaTgfRxCapturedPacket failed with code " .. rc
                        .. " port " .. devNum .. "/" .. egressPortNum)
                    didError = true
                    numOfErrorsToReport = numOfErrorsToReport + 1
                    break;
                end

                if didError == false and isCpuUsedSdma == true then
                    -- the 'CPU' when working in SDMA mode will get 4 bytes of '55' that replaces 'CRC' of packet. we need to trim it !
                    actualEgressPacket = string.sub(actualEgressPacket , 1 , -(8+1))
                    free_lua_mem("after remove 4 bytes of '55' bytes",true--[[no debug]])
                end

                --print("Checking received packet");
                --print(actualEgressPacket);
                --  matched packet index will be removed from _pktChkIdxList
                --  (to avoid matching next received packets to same pattern)
                --  but the loop will be immediately broken
                local compareSuccess = false;
                for _idxKey,_pktChkIdx in pairs(_pktChkIdxList) do

                    --print("_idxKey " .. to_string(_idxKey));
                    --print("_pktChkIdx " .. to_string(_pktChkIdx));
                    --print("_pktChkIdxList " .. to_string(_pktChkIdxList));

                    local expPktInfo = pktInfoList[_pktChkIdx];
                    expectedEgressPacket = nil

                    --support 'fullPacket' by lua test .. convert it to 'payload' before sent the 'tgf'
                    if expPktInfo ~= nil and
                        expPktInfo.fullPacket ~= nil
                    then
                        expectedEgressPacket = string.lower(expPktInfo.fullPacket)
                    else
                        printLog("ERROR: luaTgfTransmitPacketsWithExpectedEgressInfo not support expected egress packet in format of 'packet parts' ")
                        expectedEgressPacket = nil
                        numOfErrorsToReport = numOfErrorsToReport + 1
                    end

                    -- check if need to compare egress packet with the 'expected one'
                    if expectedEgressPacket ~= nil then
                        -- convet the expected packet to 'printable format'
                        local printable_expectedEgressPacket =
                            packetAsStringToPrintableFormat(expectedEgressPacket)
                        local expectedLength = string.len(expectedEgressPacket) / 2

                        printLog("\n\n Port ".. egressEntry.devNum .. "/" .. egressEntry.portNum ..
                                 " : Expected Packet to egress the port : (length "..expectedLength..")")
                        printLog(printable_expectedEgressPacket);
                        
                        printable_expectedEgressPacket = nil-- allow to free memory

                        printLog("\n Checking captured packet ...")

                        -- check if egress port have peer port
                        peerPort_egress = isPortHoldPeer(egressEntry.devNum,egressEntry.portNum)

                        if egressCscdValue or peerPort_egress then
                            -- the egress packet hold DSA tag .
                            -- parse it !
                            local purposeName = "(expected) egress packet on port " .. egressEntry.portNum
                            printDsaTagStructure(egressEntry.devNum,
                                expectedEgressPacket,
                                purposeName)
                        end


                        if didError == false then

                            local actualLength   = string.len(actualEgressPacket) / 2
                            if actualLength ~= expectedLength then
                                local diff
                                if expectedLength > actualLength then
                                    diff = expectedLength - actualLength
                                else
                                    diff = actualLength - expectedLength
                                end

                                printLog ("ERROR : expected " .. expectedLength ..
                                          " bytes , but got " .. actualLength ..
                                          " (diff of " .. diff ..")" )
                                numOfErrorsToReport = numOfErrorsToReport + 1
                            end


                            local isOk = true
                            -- compare packets
                            -- check if dsa tag problem
                            if egressCscdValue or peerPort_egress then
                                local expectedDsa = getDsa(expectedEgressPacket)
                                local actualDsa   = getDsa(actualEgressPacket)

                                if expectedDsa ~= actualDsa then
                                    -- the egress packet hold DSA tag .
                                    -- parse it !
                                    --[[
                                    local purposeName = "(actual) egress packet on port " ..
                                    egressEntry.portNum
                                    printDsaTagStructure(egressEntry.devNum,
                                        actualEgressPacket,
                                        purposeName)
                                    --]]
                                    -- print differences between the expected and actual
                                    local rc, expectedDsaSTC = wrlDxChDsaToStruct(devNum , expectedDsa)
                                    local rc, actualDsaSTC   = wrlDxChDsaToStruct(devNum , actualDsa)

                                    local diffStc = table_differences_get(expectedDsaSTC, actualDsaSTC, nil)
                                    printLog(
                                        "\n MISMATCH in the DSA : val1 is 'expected' , val2 is 'actual' :",
                                        to_string(diffStc))
                                end
                            end

                            -- do 'fine tuning' compare
                            isOk = packetsCompareFineTuning(
                                expectedEgressPacket,actualEgressPacket,
                                expPktInfo.maskedBytesTable)

                            if isOk ~= true then
                                -- convert the actual packet to 'printable format'
                                local printable_actualEgressPacket   =
                                    packetAsStringToPrintableFormat(actualEgressPacket)

                                if(isCpuSdmaPort(egressEntry.devNum,egressEntry.portNum)) then
                                    printLog("\n\n ERROR: Actual packet got to CPU but NOT 'as expected'. ('CPU' is the real Target of the packet!)")
                                else
                                    printLog("\n\n ERROR: Port ".. egressEntry.portNum ..
                                         " Actual packet that egress is : (not as expected - above)")
                                end
                                printLog(printable_actualEgressPacket);
                            end

                            if isOk == true then
                            
                                if(isCpuSdmaPort(egressEntry.devNum,egressEntry.portNum)) then
                                    printLog("Actual packet got to CPU 'as expected' ... good ! ('CPU' is the real Target of the packet!)")
                                else
                                    printLog("Actual packet egress 'as expected' ... good !")
                                end
                                -- remove matced packet index from compare list
                                -- to avoid comparisation next received packets with matched pattern
                                _pktChkIdxList[_idxKey] = nil;
                                -- save the good result
                                compareSuccess = true;
                                -- stop comparing and pass to next received packet

                                -- checking CPU code
                                if expPktInfo.cpuCodeNumber then
                                    local gotCpuCode =
                                        wrlDxChNetIfCpuToDsaCode(rxParam.cpuCodeNumber);
                                    printLog(
                                        "Received CPU code " .. tostring(gotCpuCode).. "\n");
                                    if expPktInfo.cpuCodeNumber ~= gotCpuCode then
                                        printLog(
                                            "\n\n ERROR: CPU code expected " ..
                                            tostring(expPktInfo.cpuCodeNumber) ..
                                            " not as received \n\n");
                                        numOfErrorsToReport = numOfErrorsToReport + 1;
                                    end
                                end
                                break;
                            end
                        end -- if didError == false
                    end -- if expectedEgressPacket ~= nil
                end -- pairs(_pktChkIdxList)
                if not compareSuccess then
                    printLog(
                        "\n\n ERROR: Port ".. egressEntry.devNum .. "/" .. egressEntry.portNum
                        .." Actual packet that egress is : (not as any of expected)")
                    numOfErrorsToReport = numOfErrorsToReport + 1
                end
            end -- for pairs (pktInfoList)
        end -- if pktIndexList then
    end-- for pairs(local_egressInfoTable)

    if senderCscdValue then
        -- restore cascade port mode
        supportCscdPort(local_transmitInfo.devNum,local_transmitInfo.portNum,true,senderCscdValue)
        senderCscdValue = nil
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


    -- reset mac counters of 'egress ports'
    for _index,_egressPortEntry in pairs(allEgressPorts) do
        resetPortCounters(_egressPortEntry.devNum,_egressPortEntry.portNum)
        peerPort_egress = isPortHoldPeer(_egressPortEntry.devNum,_egressPortEntry.portNum)
        if peerPort_egress then
            -- reset mac counters of 'peer port'
            resetPortCounters(_egressPortEntry.devNum,peerPort_egress)
        end
    end

    -- reset mac counters of 'sender port'
    resetPortCounters(local_transmitInfo.devNum,local_transmitInfo.portNum)
    if peerPort_transmit ~= nil then
        -- reset mac counters of 'peer port'
        resetPortCounters(local_transmitInfo.devNum,peerPort_transmit)
    end

    -- finished processing
    printLog(
        "\n\n Finished handling Packet from Port "
         .. local_transmitInfo.devNum .. "/" .. local_transmitInfo.portNum)

    if numOfErrorsToReport > 0 then
        printLog("luaTgfTransmitPacketsWithExpectedEgressInfo : Total num of errors : " .. numOfErrorsToReport)
        setFailState()

        rc = GT_FAIL
    end

    return rc
end

--function to init for the tested ports
-- devNum - the device woth ports
-- get array of ports to act on.
-- startTest - bool for indication for start/end test
--
-- for each port do:
-- if startTest == true then force link up else 'unforce' link up
-- reset mac counters
function luaTgfConfigTestedPorts(devNum,testedPorts,startTest)
    local GT_OK = 0
    local enable--must have true/false valuse (and NOT numeric 0/1 and NOT "GT_TRUE"/"GT_FALSE")
    local operationName
    local commonOperations = " force link UP + reset counters "
    if startTest == false then
        enable = false
        operationName = "Stop (unset " .. commonOperations .. " )"
    else
        enable = true
        operationName = "Start (set " .. commonOperations .. " )"
    end

    --state the ports that expected to be checked during the test
    printLog(operationName .. " test for ports : " .. to_string(testedPorts))

    for index,portNum in pairs(testedPorts) do
        --print (devNum,portNum,to_string(enable))
        -- force/unforce link up
        rc = prvLuaTgfPortForceLinkUpEnableSet(devNum,portNum,enable);
        if rc ~= GT_OK then
            printLog (
                "ERROR: prvLuaTgfPortForceLinkUpEnableSet failed with code " .. rc
                .. " for port " .. devNum .. "/" .. portNum)
        end

        -- reset mac counters on the port
        resetPortCounters(devNum,portNum)
    end
end

-- function to calculate 16-bit check sum
-- packet_parts - the list of packet parts
-- packet part may be hexadecimal nibble string or
-- a substring of nibble string in such format
-- {packet_string, [begin_nibble_index[, num_of_nibbles]]}
-- index starts from 1 as usually for LUA tables and strings
-- ommited begin_nibble_index means 1,
-- ommited num_of_nibbles means up to the end of the string
-- returns 16-bit number of CheckSum on success, nil on bad input data
function luaTgfCalculate16bitCheckSum(packet_parts)
    local part_key, part_val;
    local sum;
    local pkt_str, pkt_start, pkt_len
    local b16val, b16nibble_index, pkt_nibble_index, pkt_nibble_len;
    local nibble_str, nibble_val;

    -- accomulates the sums across all packet parts
    sum = 0;
    -- 16 bit treating
    b16val = 0;
    b16nibble_index = 0;

    part_key = next(packet_parts, nil);
    while (part_key ~= nil) do
        part_val = packet_parts[part_key];
        if (type(part_val) == "table") then
            pkt_str = part_val[1];
            pkt_start = part_val[2];
            pkt_len =  part_val[3];
        elseif (type(part_val) == "string") then
            pkt_str = part_val;
            pkt_start = nil;
            pkt_len =  nil;
        else
            return nil;
        end
        if type(pkt_str) ~= "string" then
            return nil;
        end
        if (pkt_start == nil) then pkt_start = 1; end
        if (pkt_start > string.len(pkt_str)) then
            return nil;
        end
        if (pkt_len == nil) then
            -- index starts from 1
            pkt_len = string.len(pkt_str) - pkt_start + 1;
        end
        if ((pkt_start + pkt_len - 1) > string.len(pkt_str)) then
            return nil;
        end
        pkt_nibble_len = pkt_len;
        pkt_nibble_index = pkt_start;
        while (pkt_nibble_len > 0) do
            nibble_str = string.sub(pkt_str, pkt_nibble_index, pkt_nibble_index);
            nibble_val = nibble_to_num[nibble_str];
            if (nibble_val == nil) then
                return nil;
            end
            b16val = (b16val * 16) + nibble_val;
            b16nibble_index = b16nibble_index + 1;
            if (b16nibble_index >=4) then
                sum = sum + b16val;
                b16val = 0;
                b16nibble_index = 0;
            end

            pkt_nibble_len = (pkt_nibble_len - 1);
            pkt_nibble_index = (pkt_nibble_index + 1);
        end

        part_key = next(packet_parts, part_key);
    end
    -- add last nibbles
    sum = sum + b16val;
    -- compress to 16 bits
    if (sum > 0xFFFF) then
        sum = (sum % 0xFFFF);
        if (sum == 0) then
            sum = 0xFFFF;
        end
    end
    -- sum ==> bitwise not of sum
    sum = 0xFFFF - sum;
    -- result
    return sum;
end

-- 1. removed from default vlan 1 (so not egress flooding in this vlan)
-- 2. set force pvid = 0 , so ingress packets (from 'main port') will be dropped by the 'bridge'
local function setHelperPort(devNum,helperPort,unset)
    portNum = helperPort
    defaultVlan = 1

    -- 1. removed from default vlan 1 (so not egress flooding in this vlan)
    if unset == true then
        apiName = "cpssDxChBrgVlanMemberAdd"
        ret = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",          "devNum",     devNum },
            { "IN",     "GT_U16",    "vlanId",    defaultVlan},
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum},
            { "IN",     "GT_BOOL",    "isTagged",    false},
            { "IN",     "CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT",    "portTaggingCmd",    "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E"}
        })
    else
        apiName = "cpssDxChBrgVlanPortDelete"
        ret = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",          "devNum",     devNum },
            { "IN",     "GT_U16",    "vlanId",    defaultVlan},
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum}
        })
    end

    if unset == true then
        enable = false
    else
        enable = true
    end
    -- 2. set force pvid = 0 , so ingress packets (from 'main port') will be dropped by the 'bridge'
    apiName = "cpssDxChBrgVlanForcePvidEnable"
    ret = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",          "devNum",     devNum },
        { "IN",     "GT_PORT_NUM",    "portNum",    portNum},
        { "IN",     "GT_BOOL",    "enable",    enable}
    })

    if unset == true then
        newPvid = defaultVlan
    else
        newPvid = 0
    end

    apiName = "cpssDxChBrgVlanPortVidSet"
    ret = myGenWrapper(
        apiName, {
        { "IN",     "GT_U8",          "devNum",     devNum },
        { "IN",     "GT_PORT_NUM",    "portNum",    portNum},
        { "IN",     "CPSS_DIRECTION_ENT",    "direction",    "CPSS_DIRECTION_BOTH_E"},
        { "IN",     "GT_U16",    "vlanId",    newPvid}

    })


end

local unbind_slan_mode = 1
local bind_slan_mode = 0
--bind 2 ports to each other by slan
local function bindSlanForPorts(devNum,port1,port2,unset)
    -- NOTE: the slan name length must be <= 8
    -- and since each port is up to 2 digits , total 4 digits
    -- we also need seperator between ports
    slanName = "ext"..port1.."_"..port2

    if unset == true then
        wrlSimulationSlanManipulations(unbind_slan_mode,devNum,port1,devNum,port2)
    else
        wrlSimulationSlanManipulations(bind_slan_mode,devNum,port1,devNum,port2,slanName)
    end
--[[
    apiName = "prvCpssDrvDevNumToSimDevIdConvert"
    ret,values = myGenWrapper(
        apiName, {
        { "IN",     "GT_U32",          "deviceId",     deviceId },
        { "IN",     "GT_U32",    "portGroupId",    0},
        { "OUT",     "GT_U32",    "simDeviceId"}
    })
    if ret == 0 then
        sim_deviceId = values.simDeviceId
    else
        sim_deviceId = 0
    end

    ports = {port1,port2}

    for index,portNum in pairs(ports) do
        if unset == true then
            apiName = "skernelUnbindDevPort2Slan"
            ret = myGenWrapper(
                apiName, {
                { "IN",     "GT_U32",          "deviceId",     sim_deviceId },
                { "IN",     "GT_U32",    "portNum",    portNum}
            })
        else
            apiName = "skernelBindDevPort2Slan"
            ret = myGenWrapper(
                apiName, {
                { "IN",     "GT_U32",          "deviceId",     sim_deviceId },
                { "IN",     "GT_U32",    "portNum",    portNum},
                { "IN",     "GT_CHAR",    "slanNamePtr",    slanName},
                { "IN",     "GT_BOOL",    "unbindOtherPortsOnThisSlan",    false} -- allow ports to be connected to each other
            })
        end
    end
--]]
end

--[[
 function to state list of pairs of ports.
 for each pair of ports : the 2 ports are 'connected by physical cable' (on simulation by 'slan') to each other.
                        NOTE: the first port is the 'primary' tested port , and the second port is the 'helper'
                              this function (try to) make sure that the 'helper' port will not generate loops of traffic:
                                -- 1. removed from default vlan 1 (so not egress flooding in this vlan)
                                -- 2. set force pvid = 0 , so ingress packets (from 'main port') will be dropped by the 'bridge'
 example for pairsOfPorts : {{15,36} ,{12,34}} --> meaning port 15 connected to 36 (and vise versa), port 12 connected to 34 (and vise versa)
 NOTE: this info is used by luaTgfTransmitPacketsWithExpectedEgressInfo(...) to allow DSA tag tests on xcat devices that not support
      separated config between rx,tx about 'dsa tag'

      so if test uses ports 0,8,18,23 as regular xcat3 ports , and need to 'ingress dsa' from port 18 ... we need to do:
      (port 19 assumed 'not cscd' , port 18 assumed as 'cscd')
      1. state that port 19 is connected to port 18.
      2. 'test' will state to 'inject' packet into 18 , but the engine(luaTgfTransmitPacketsWithExpectedEgressInfo) will 'see'
      that port 18 have 'peer' (port 19) , so it will send to packet egress from peer (port 19) ... and since 19 connected to 18
      the packet will ingress port 18.


      similar if we need to 'capture' egress packet with dsa that egress port 23 .. we need to do:
      (port 22 assumed 'not cscd' , port 23 assumed as 'cscd')
      1. state that port 22 is connected to port 23.
      2. 'test' will state to 'expect' packet with dsa on port 23 , but the engine(luaTgfTransmitPacketsWithExpectedEgressInfo) will 'see'
      that port 23 have 'peer' (port 22) , so it will set 'capture' on egress from peer (port 22) ... and since 23 connected to 22
      the packet will egress port 23 ... will ingress from port 22 ... will be captured to cpu.


 NOTE: for simulation the ports will be connected with slan in runtime ... allowing automated test
--]]
function luaTgfStatePhysicalConnectedPairs(devNum,pairsOfPorts)
    isSimulation = wrlCpssIsAsicSimulation()
    if devNum == nil then
        for index1,entry1 in pairs(luaTgf_pairsOfPorts) do
            pairsOfPorts = entry1
            devNum = index1
            for index,entry in pairs(pairsOfPorts) do
                setHelperPort(devNum,entry[2],true) -- restore config
                if isSimulation then
                    -- unbind slan from the ports
                    bindSlanForPorts(devNum,entry[1],entry[2],true)
                end
            end
        end

        -- clear DB fully
        luaTgf_pairsOfPorts = {}
        return
    end

    if pairsOfPorts == nil then
        pairsOfPorts = luaTgf_pairsOfPorts[devNum]
        if pairsOfPorts then
            for index,entry in pairs(pairsOfPorts) do
                setHelperPort(devNum,entry[2],true) -- restore config
                if isSimulation then
                    -- unbind slan from the ports
                    bindSlanForPorts(devNum,entry[1],entry[2],true)
                end
            end
        end
        -- remove from device info from the db
        luaTgf_pairsOfPorts[devNum] = nil
        return
    end

    luaTgf_pairsOfPorts[devNum] = {}

    for index,entry in pairs(pairsOfPorts) do
        luaTgf_pairsOfPorts[devNum][index] = entry

        -- set configuration on the 'helper' port
        setHelperPort(devNum,entry[2])
        if isSimulation then
            -- use slan to bind the ports
            bindSlanForPorts(devNum,entry[1],entry[2])
        end
    end

end

-- list of failed tests
local failed_tests = {}
local function add_fail(purposeOfTest)
    failed_tests[purposeOfTest] = true
end
local passed_tests = {}
local function add_pass(purposeOfTest)
    passed_tests[purposeOfTest] = true
end

-- print the failed tests
function luaTgfPrintFailedTestResults()
    number = 0

    for index,entry in pairs(failed_tests) do
        number = number + 1
    end

    if number == 0 then
        return
    end

    _debug_to_string("FAIL Summary: there are " .. number .. " failing tests: ")

    for index,entry in pairs(failed_tests) do
        _debug_to_string(index)
    end

    _debug_to_string("end of (" .. number .. ") failing tests")

end
-- print the passed tests
function luaTgfPrintPassedTestResults()
    number = 0

    for index,entry in pairs(passed_tests) do
        number = number + 1
    end

    if number == 0 then
        return
    end

    _debug_to_string("Pass Summary: there are " .. number .. " passing tests: ")

    for index,entry in pairs(passed_tests) do
        _debug_to_string(index)
    end

    _debug_to_string("end of (" .. number .. ") passing tests")

end

-- print the pass/fail tests
function luaTgfPrintTestResults()
    luaTgfPrintFailedTestResults()
    luaTgfPrintPassedTestResults()
end
-- clear the registration of the failed tests
function luaTgfClearTestResults()
    failed_tests = {}
    passed_tests = {}
end

-- check if test passed/failed , and the 'purpose' of the test
-- so the fail/pass can have purpose information
-- rc - cpss style of 'return code'
-- purposeOfTest - string of the purpose (can be nil)
-- when rc != 0 (GT_OK) then function sets : setFailState()
function luaTgfCheckTestResults(rc,purposeOfTest)
    local myString

    if rc == 0 then
        myString = "passed"
    else
        errorCodeString = returnCodes[rc]
        myString = "failed (rc="..rc.."->"..errorCodeString..")"
        setFailState()
    end

    if purposeOfTest then
        myString = myString .. " - " .. purposeOfTest
        if rc ~= 0 then
            add_fail(purposeOfTest)
        else
            add_pass(purposeOfTest)
        end
    end

    myString = myString .. "\n"

    printLog (myString)
end

-- build name of config/deconfig file name (full path name)
-- for example : examples\configurations\bpe_port_extender_dev_1_deconfig.txt
function luaTgfBuildConfigFileName(fileName,isDeconfig)
    local deconfigName
    local fullName =  ""
    if (px_family == true) then
      fullName = "px/examples/configurations/" .. fileName
    else
      fullName = "dxCh/examples/configurations/" .. fileName
    end
    
    if isDeconfig == true then
        deconfigName = "_deconfig"
    else
        deconfigName = ""
    end

    return fullName .. deconfigName .. ".txt"
end

-- replace dsa
function luaTgfDsaReplaceOnPacket(packetString,oldDsaString,newDsaString)
    return string.gsub(packetString,oldDsaString,newDsaString,1)
end

-- remove the dsa
function luaTgfDsaRemoveFromPacket(packetString,dsaString)
    return replaceDsaOnPacket(packetString,dsaString,"")
end

-- add dsa
function luaTgfDsaAddToPacket(packetString,dsaString)
    macSaDa = string.sub(packetString,1,12*2)
    payload = string.sub(packetString,12*2 + 1)

    return macSaDa.. dsaString ..payload
end


-- get DSA string from STC
function luaTgfDsaStringFromSTC(devNum,dsaSTC)
   local rc, val = wrlDxChDsaToString(devNum, dsaSTC)
   if (rc ~=0) then
      setFailState()
      return nil
   end

   return string.lower(val)
end


--*****************************************************************************
-- luaTgfSimpleTest
--
-- @description  simplify test writing. Use if you test scenario looks like:
--               1) execute a file with CLI configuration commands (config-file)
--               2) execute a user's function (usually that is a packets
--                 transmitting)
--               2) execute a file with CLI deconfiguration commands
--                 (deconfig-file).
--               The deconfig file will be executed regardless errors
--               occured during execution of the config file or the user's
--               function.
--
-- @param  configFile   - a configuration file name (string). Can be nil.
-- @param  deconfigFile - a deconfiguration file name (string). Can be nil.
-- @param  testBody     - a user's function
-- @param  ...          - any arguments that should be passed to the testBody()
--
-- @return true if ok. False - if any of config-file, deconfig-file, testBody
--                     returned false (i.e. something was failed)
function luaTgfSimpleTest(configFile, deconfigFile, testBody, ...)
    local isOk = true

    -- setup the configuration
    if configFile and not executeLocalConfig(configFile) then
        isOk = false
        setFailState()
        print("error running config " .. configFile)
    end

    -- execute test's body
    if isOk then
        isOk, errMsg  = pcall(testBody, ...) -- catch exceptions
        if not isOk then
            setFailState()
            print(errMsg)
        end
    end

    -- clear the configuration.
    if deconfigFile and not executeLocalConfig(deconfigFile) then
        isOk = false
        setFailState()
        print("error while running config " .. deconfigFile)
    end
    return isOk
end



--*****************************************************************************
-- enableSimulationLog
--
-- @description enable/disable writing of simulation log.
--
-- @param mode: 0 - stop
--              1 - start
--              2 - start with from_cpu
--              3 - start full
-- @param logFile - a log file name. Optional parameter. Can be nil.
--
-- @return nothing
--
local function enableSimulationLog(mode, logFile)
    if wrlCpssIsAsicSimulation() then
        wrlSimulationLog(mode,logFile)
    else
        printLog("the command is supported on simulation only")
    end
end

-- ability for tests to easy embed a call to start simulation log
-- start simulation log recording
function startSimulationLog()
    enableSimulationLog(1)
end

-- ability for tests to easy embed a call to start simulation log
-- stop simulation log recording
function stopSimulationLog()
    enableSimulationLog(0)
end

--*****************************************************************************
-- execWithCapturing
--
-- @description enable capturing of packets to CPU,  execute some
--             function func(), disable capturing. Capturing will be disabled
--             regardless of successfull of func() call.
--             if func() generate an exception (error()), it will be
--             regenerated after the capturing is disabled.
--
-- @param func  - a function to be executed.
-- @param ...   - rest of parameters will be passed to the func as arguments.
--
-- @return      - result of the func() call.
--
--
function execWithCapturing(func, ...)
    local rc = myGenWrapper("tgfTrafficTableRxPcktTblClear", {});
    if rc ~= 0 then
        -- print error but don't interrupt execution
        printLog("ERROR on clear TGF traffic table")
    end

    local rc = myGenWrapper( "tgfTrafficTableRxStartCapture", {
                                 { "IN", "GT_BOOL", "enable", true} })
    if rc ~= 0 then
        printLog("ERROR on enabling capturing")
    end

    -- call func(). Any exception will be suppressed.
    local results  = {pcall(func, ...)}
    local rc = myGenWrapper( "tgfTrafficTableRxStartCapture", {
                                 { "IN", "GT_BOOL", "enable", false} })
    if rc ~= 0 then
        printLog("ERROR on disabling capturing")
    end

    -- return results of func() call. Or re-generate an exception if
    -- it was occured furing func() execution.

    if results[1] then           -- pcall(func(),...) returned true
        table.remove(results, 1) -- delete "true" returned by pcall).
        return unpack(results)
    else                         -- pcall(func(),...) returned false
        error(results[2])
    end
end

--*****************************************************************************
-- checkPacketsWereCaptured
--
-- @description check a number of packets captured in CPU and ensure
--             what expected packets are captured.
--
-- @param packetNumber - a number of captured packets of type 'packetType'.
--                     Can be nil.
-- @param packetType - packet't type to be checked. TGF_PACKET_TYPE_ENT
--                     A TGF_PACKET_TYPE_ANY_E is used if not specified.
-- @param packetInfoList - a list of a checked packet's info. Looks like
--                     { payload: (string) - a packet's string in hex view.
--                       rxParam : (table) - appropriate to TGF_NET_DSA_STC.
--                     }
--                     If some field is not specified (nil) it will be ignored.
--
-- @return true  - if all expected packets from packetInfoList was found
--         false - otherwise
--
function checkPacketsWereCaptured(packetNumber, packetType, packetInfoList)
    local isOk = false

    packetType = packetType or "TGF_PACKET_TYPE_ANY_E"
    packetInfoList = packetInfoList or {}
    local timeToWait = (packetInfoList == {} ) and 100 or 500


    myGenWrapper("tgfTrafficGeneratorRxInCpuNumWait",{
                     {"IN",  "GT_U32", "numOfExpectedPackets", #packetInfoList},
                     {"IN",  "GT_U32", "maxTimeToWait", timeToWait},
                     {"OUT", "GT_U32", "timeActuallyWaitPtr"}}
    )
    local infoList = deepcopy(packetInfoList)
    local getFirst = true
    local rc = 0
    local captPktParts, captPayload, captRxParam
    local capturedPacketNumber = 0
    repeat
        -- get next captured packet
        rc, captPktParts, captPayload, captRxParam =
            prvLuaTgfRxCapturedPacket(nil, nil, packetType, getFirst)
        if rc ~= 0 then
            if rc == 0xC  then  -- GT_NO_MORE
                break
            else
                return false, returnCodes[rc]
            end
        end
        getFirst = false
        capturedPacketNumber = capturedPacketNumber + 1

        -- iterate through expected packets and compare them with captured packet
        local i = #infoList
        while i > 0 do -- iterate in back order  make an index decrement independent on item delition.
            local packetInfo = infoList[i]
            local isEqual = true
            -- compare packets
            -- compare payloads
            if packetInfo.payload and
                string.upper(packetInfo.payload) ~= string.upper(captPayload)
            then
                isEqual = false
            else
                -- payloads are equal. Compare RxParam
                for key, value in pairs(packetInfo.rxParam) do
                    if captRxParam[key] ~= value then
                        isEqual = false
                        break
                    end
                end
            end

            if isEqual then
                -- don't look for this item anymore.
                table.remove(infoList, i)
                break
            end
            i = i - 1
        end
    until rc ~= 0

    -- if all expected packets are found and a total number of captured packets is as expected?
    isOk = (#infoList == 0) and packetNumber == capturedPacketNumber
    if not isOk then       -- not all expected packets was found
        if packetNumber > 0 then
            printLog(string.format("A total number of captured packets(%s) is less than expected(%s)",
                                   to_string(capturedPacketNumber), to_string(packetNumber)))
        end
        if #infoList ~= 0 then
            print (" ---------- Expected packets that was not captured: ---------- ")
            for _, info in ipairs(infoList) do
                printLog(to_string(info.payload), to_string(info.rxParam))
            end
        end
        return false, "Either number of captured packets is unexpected or some expected packets was not captured!"
    end
    return isOk
end
