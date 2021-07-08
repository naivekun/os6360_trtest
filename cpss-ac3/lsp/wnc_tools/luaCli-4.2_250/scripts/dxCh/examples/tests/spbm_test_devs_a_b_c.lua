--********************************************************************************
--*              (C), Copyright 2001, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* spbm_test_devs_a_b_c.lua
--*
--* DESCRIPTION:
--*       SPBM test (a "shortest path bridging MAC" feature).
--*       Check an unknown UC and known UC packets processing inside the core network.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[
    SPBM tests - 3 devices configuration below
--]]

-- =========================================================================================
--          00:00:00:00:0D:EA          00:00:00:00:0D:EB         00:00:00:00:0D:EC
--          |---------------|          |---------------|         |---------------|
--          |               |          |               |         |               |
--          |               |4 ------ 4|               |5 ----- 4|               |
--          |     DEV-A     |          |     DEV-B     |         |     DEV-C     |
--          |               |          |               |         |               |
--          |               |          |               |         |               |
--          |               |          |               |         |               |
--          |---------------|          |---------------|         |---------------|
--             1    2    3                1    2    3               1    2    3
--             |    |    |                |    |    |               |    |    |
--
--          B-VID = 511
--          MC-GROUPS (local FDB only)
--          A1:00:0A:01:00:00  ports A/4 B/5
--          A1:00:0A:02:00:00  ports A/4 B/5
--          A1:00:0B:01:00:00  ports B/4,5
--          A1:00:0B:02:00:00  ports B/4,5
--          A1:00:0C:01:00:00  ports C/4 B/4
--          A1:00:0C:02:00:00  ports C/4 B/4
--          C-VIDs, I-SIDs and ports
--          VID 1111 (0x457) SID 0x10000 ports A/1 (untagged), A/3 (tagged)
--          VID 1112 (0x458) SID 0x20000 ports A/2 (untagged), A/3 (tagged)
--          VID 2221 (0x8AD) SID 0x10000 ports B/1 (untagged), B/3 (tagged)
--          VID 2222 (0x8AE) SID 0x20000 ports B/2 (untagged), B/3 (tagged)
--          VID 3331 (0xD03) SID 0x10000 ports C/1 (untagged), C/3 (tagged)
--          VID 3332 (0xD04) SID 0x20000 ports C/2 (untagged), C/3 (tagged)
--          PVIDs
--          ports A/1 PVID 1111 (0x457)
--          ports A/2 PVID 1112 (0x458)
--          ports B/1 PVID 2221 (0x8AD)
--          ports B/2 PVID 2222 (0x8AE)
--          ports C/1 PVID 3331 (0xD03)
--          ports C/2 PVID 3332 (0xD04)
--          ePorts (egress + tunneling):
--          ePort A/1002 port A/4 mac-uc 00:00:00:00:0D:EB
--          ePort A/1003 port A/4 mac-uc 00:00:00:00:0D:EC
--          ePort A/1111 port A/4 mac-mc A1:00:0A:XX:XX:XX
--          ePort B/1001 port B/4 mac-uc 00:00:00:00:0D:EA
--          ePort B/1003 port B/5 mac-uc 00:00:00:00:0D:EC
--          ePort B/1111 port B/4 mac-mc A1:00:0B:XX:XX:XX
--          ePort B/2222 port B/5 mac-mc A1:00:0B:XX:XX:XX
--          ePort C/1001 port C/4 mac-uc 00:00:00:00:0D:EA
--          ePort C/1002 port C/4 mac-uc 00:00:00:00:0D:EB
--          ePort C/1111 port C/4 mac-mc A1:00:0C:XX:XX:XX
--          Registered Mac MC
--          Mac 91:01:00:11:22:33 sid 0x10000 ports A/1, B/3, C/3
--          on A: Mac 91:01:00:11:22:33 vid 1111 ports A/1,1111
--          on B: Mac 91:01:00:11:22:33 vid 2221 ports B/3,1111,2222
--          on C: Mac 91:01:00:11:22:33 vid 3331 ports C/3,1111
--
-- =========================================================================================
-- The test simulates passing Uncknown UC packet from A/1 to C/3 via device B and
-- passing another UC packet from C/3 to A/1 via device B. The secont packet has the same
-- (swapped) mac addresses that the first one and so it passes as Known UC (it's MAC_DA
-- is the MAC_SA of the first packet and already learned.
-- Tested scenarios below:
-- 1. Unknown UC packet flooding
-- 1.1. Packet1 (untagged) through Device A (send to A/1)
-- 1.2. Mc tunneled Packet1 through Device B (send to B/4)
-- 1.3. Mc tunneled Packet1 through Device C (send to C/4)
-- 2. Known UC packet forwarding (aggress learned from step 1.)
-- 2.1. Packet2 (tagged) through Device C (send to C/3)
-- 2.2. Uc tunneled Packet2 through Device B (send to B/5)
-- 2.3. Uc tunneled Packet2 through Device A (send to A/4)
-- 3. Known UC packet forwarding with port moving
--    all expected changes only on device "B", only it tested
-- 3.1. Packet2 (untagged, macs as in 2.1) through Device B (send to B/1)
-- 3.2. Uc tunneled Packet1(passenger macs as in 1.2) through Device B (send to B/4)
-- 4. Registered Mc Packet forwarding (from B via A and via C)
-- 4.1. Mc packet3 (registered Mc Mac DA, untagged) (send to B/1)
-- 4.2. Mc tunneled Packet3 through Device A (send to A/4)
-- 4.3. Mc tunneled Packet3 through Device C (send to C/4)
-- In order to optimize number of configurations/deconfigurations
-- used such scenario:
-- 1. Configure as A. Sends 1.1., 2.3., 4.2 Deconfigure.
-- 2. Configure as B. Sends 1.2., 2.2., 3.1., 3.2., 4.1. Deconfigure.
-- 3. Configure as C. Sends 1.3., 2.1., 4.3. Deconfigure.
-- =========================================================================================
local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local port5   = devEnv.port[5]
local port6   = devEnv.port[6]

local skip = true

-- --this test is temporary skipped
if skip == true then
    setTestStateSkipped();
    return;
end 

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_5") 

local strConfigFileName_A = "dxCh/examples/configurations/spbm_test_dev_a.txt"
local strDeconfigFileName_A =  "dxCh/examples/configurations/spbm_test_dev_a_deconfig.txt"
local strConfigFileName_B = "dxCh/examples/configurations/spbm_test_dev_b.txt"
local strDeconfigFileName_B =  "dxCh/examples/configurations/spbm_test_dev_b_deconfig.txt"
local strConfigFileName_C = "dxCh/examples/configurations/spbm_test_dev_c.txt"
local strDeconfigFileName_C =  "dxCh/examples/configurations/spbm_test_dev_c_deconfig.txt"

-- ability to debug only specific device(s)
local allowTestDevice_A = true
local allowTestDevice_B = true
local allowTestDevice_C = true

-- flush the FDB (use LUA CLI command !)
local function flushFdbDynamic()
    local fileName = "flush_fdb_only_dynamic"
    executeLocalConfig(luaTgfBuildConfigFileName(fileName))
end

local dev_own_mac = {
        A = "000000000DEA", B = "000000000DEB", C = "000000000DEC"}
local sid_str = {[1] = "010000", [2] = "020000"};
local sid_stamp = {[1] = "01", [2] = "02"};
local cust_port_str = {[1] = "01", [2] = "02", [3] = "03"};
local sid_idx_to_tag_vid = {
    A = {[1] = 1111, [2] = 1112},
    B = {[1] = 2221, [2] = 2222},
    C = {[1] = 3331, [2] = 3332}
};

-- device - "A", "B" or "C"
-- type - "own_mac", "erg_mc", "cust_node"
-- sid_idx - 1 or 2 (for 2 used SIDs)
-- cust_port - 1,2 or 3 (for 3 used customer ports)
local function pktMacAddr(device, type, sid_idx, cust_port)
    if not dev_own_mac[device] then
        print("pktMacAddr - invalid device");
        return nil;
    end
    if type == "own_mac" then
        return dev_own_mac[device];
    elseif type == "erg_mc" then
        if not sid_str[sid_idx] then
            print("pktMacAddr - invalid sid_idx");
            return nil;
        end
        return ("A1000" .. device .. sid_str[sid_idx]);
    elseif type == "cust_node" then
        if not sid_stamp[sid_idx] then
            print("pktMacAddr - invalid sid_idx");
            return nil;
        end
        if not cust_port_str[cust_port] then
            print("pktMacAddr - invalid cust_port");
            return nil;
        end
        return ("EEEEEE" .. sid_stamp[sid_idx]
                .. "0".. device  .. cust_port_str[cust_port]);
    else
        print("pktMacAddr - invalid type");
        return nil;
    end
end

-- tagged - nil or "src" (for sent packets) or "dst" (for received packets) or device ("A", "B", "C")
local function pktCustomPacketWithDstMac(
    src_dev, dst_dev, src_port, dst_port, sid_idx, tagged, dst_mac)
    local vid = nil;
    local dev = nil;
    local vlan_tag = "";
    local destination_mac;
    if tagged == "src" then
        dev = src_dev;
    end
    if tagged == "dst" then
        dev = dst_dev;
    end
    if sid_idx_to_tag_vid[tagged] then
        dev = tagged;
    end
    if dev then
        if not sid_idx_to_tag_vid[dev] then
            print("pktCustomPacket - invalid dev");
            return nil;
        end
        local vid = sid_idx_to_tag_vid[dev][sid_idx];
        if not vid then
            print("pktCustomPacket - invalid sid_idx");
            return nil;
        end
        vlan_tag = "81000" .. string.format("%03X", vid);
    end
    if dst_mac then
        destination_mac = dst_mac;
    else
        destination_mac = pktMacAddr(dst_dev, "cust_node", sid_idx, dst_port);
    end
    return (
        destination_mac ..
        pktMacAddr(src_dev, "cust_node", sid_idx, src_port) ..
        vlan_tag ..
        "3333" --[[ethertype--]] ..
        --[[payload--]]
        "0102030405060708090A0B0C0D0E0F" ..
        "0102030405060708090A0B0C0D0E0F" ..
        "0102030405060708090A0B0C0D0E0F" ..
        "FEDCBA9876543210"
        );
end

local function pktCustomPacket(src_dev, dst_dev, src_port, dst_port, sid_idx, tagged)
    return pktCustomPacketWithDstMac(
        src_dev, dst_dev, src_port, dst_port, sid_idx, tagged, nil --[[dst_mac--]]);
end

local function pktPbbUcPacket(src_dev, dst_dev, src_port, dst_port, sid_idx)
    return (
        pktMacAddr(dst_dev, "own_mac", nil --[[sid_idx--]], nil --[[cust_port--]]) ..
        pktMacAddr(src_dev, "own_mac", nil --[[sid_idx--]], nil --[[cust_port--]]) ..
        "88A801FF" .. --[[b-vid tag, b-vid = 0x1FF--]]
        "88E7" .. "00" .. sid_str[sid_idx] .. --[[SID - tag--]]
        pktCustomPacket(src_dev, dst_dev, src_port, dst_port, sid_idx, nil --[[tagged--]]));
end

local function pktPbbUcPacketWithHdrDevs(
    src_dev, dst_dev, src_port, dst_port, sid_idx, hdr_src_dev, hdr_dst_dev)
    return (
        pktMacAddr(hdr_dst_dev, "own_mac", nil --[[sid_idx--]], nil --[[cust_port--]]) ..
        pktMacAddr(hdr_src_dev, "own_mac", nil --[[sid_idx--]], nil --[[cust_port--]]) ..
        "88A801FF" .. --[[b-vid tag, b-vid = 0x1FF--]]
        "88E7" .. "00" .. sid_str[sid_idx] .. --[[SID - tag--]]
        pktCustomPacket(src_dev, dst_dev, src_port, dst_port, sid_idx, nil --[[tagged--]]));
end

local function pktPbbMcPacket(src_dev, dst_dev, src_port, dst_port, sid_idx)
    return (
        pktMacAddr(src_dev, "erg_mc", sid_idx, nil --[[cust_port--]]) ..
        pktMacAddr(src_dev, "own_mac", nil --[[sid_idx--]], nil --[[cust_port--]]) ..
        "88A801FF" .. --[[b-vid tag, b-vid = 0x1FF--]]
        "88E7" .. "00" .. sid_str[sid_idx] .. --[[SID - tag--]]
        pktCustomPacket(src_dev, dst_dev, src_port, dst_port, sid_idx, nil --[[tagged--]]));
end

local function pktPbbMcPacketWithPassengerDstMac(
    src_dev, dst_dev, src_port, dst_port, sid_idx, passenger_dst_mac)
    return (
        pktMacAddr(src_dev, "erg_mc", sid_idx, nil --[[cust_port--]]) ..
        pktMacAddr(src_dev, "own_mac", nil --[[sid_idx--]], nil --[[cust_port--]]) ..
        "88A801FF" .. --[[b-vid tag, b-vid = 0x1FF--]]
        "88E7" .. "00" .. sid_str[sid_idx] .. --[[SID - tag--]]
        pktCustomPacketWithDstMac(
            src_dev, dst_dev, src_port, dst_port, sid_idx,
            nil --[[tagged--]], passenger_dst_mac));
end

local function keep_error(msg)
    -- to keep error without termination
    -- "error" function terminates the caller
    pcall(error, msg);
end

local sentPacket, receivedPacket1, receivedPacket2;
local rc;
local customerMaskedBytes = {
    {startByte = 14, endByte = 14, startByteCompareMask = 0x0F} -- exclude Vlan UP and CFI
};
local pbbMaskedBytes = {
    {startByte = 14, endByte = 14, startByteCompareMask = 0x0F}, -- exclude B-Vlan UP and CFI
    {startByte = 18, endByte = 18} -- exclusde SID flags
};

if allowTestDevice_A then
print("-------------------------------------------------")
print("SPBM test device A")

-- run a configuration
if not executeLocalConfig(strConfigFileName_A) then
   keep_error("executeLocalConfig " .. strConfigFileName_A)
end

-- 1.1. Packet1 (untagged) through Device A (send to A/1)
-- Unknown Uc from Customer edge
-- sent from A/1 to C/3 (and learn mac) [egress tunneled as MC]
-- expected flooding (i.e, pbb MC from A/4)

sentPacket = pktCustomPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]],  nil --[[tagged--]]);
receivedPacket1 = pktPbbMcPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]]);
receivedPacket2 = pktCustomPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]],  "src" --[[tagged--]]);

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port2, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
      {portNum = port3, packetCount = 0},
      {portNum = port4, pktInfo =
          {fullPacket = receivedPacket2, maskedBytesTable = customerMaskedBytes},
          packetCount = 1},
      -- a tunneled packet sent to device B.
      {portNum = port5, pktInfo =
          {fullPacket = receivedPacket1, maskedBytesTable = pbbMaskedBytes},
          packetCount = 1}
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- 2.2. Uc tunneled Packet2 through Device A (send to A/4)
-- Known Uc from Provider edge
-- sent to A/4 packet [tunneled as UC] from C/3 to A/1 (and learn mac)
-- expected reaching A/1 only (learned at previous send)

sentPacket = pktPbbUcPacket(
    "C" --[[src_dev--]], "A"--[[dst_dev--]],
    3   --[[src_port--]], 1 --[[dst_port--]],
    1   --[[sid_idx--]]);
receivedPacket1 = pktCustomPacket(
    "C" --[[src_dev--]], "A"--[[dst_dev--]],
    3   --[[src_port--]], 1 --[[dst_port--]],
    1   --[[sid_idx--]],  nil --[[tagged--]]);
receivedPacket2 = nil;

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port5, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port2, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = customerMaskedBytes},
           packetCount = 1},
      {portNum = port3, packetCount = 0},
      {portNum = port4, packetCount = 0},
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- 4.2. Mc tunneled Packet3 through Device A (send to A/4)
-- Known Mc from Provider edge
-- sent to A/4 packet [tunneled as MC] from B/1 to 91:01:00:11:22:33 (and learn mac)
-- expected reaching A/1 only (learned by static configuration)

sentPacket = pktPbbMcPacketWithPassengerDstMac(
    "B" --[[src_dev--]], "A"--[[dst_dev--]],
    1   --[[src_port--]], 0xFF --[[dst_port--]],
    1   --[[sid_idx--]],  "910100112233" --[[passenger_dst_mac--]]);
receivedPacket1 = pktCustomPacketWithDstMac(
    "B" --[[src_dev--]], "A"--[[dst_dev--]],
    1   --[[src_port--]], 0xFF --[[dst_port--]],
    1   --[[sid_idx--]],  nil --[[tagged--]], "910100112233" --[[dst_mac--]]);
receivedPacket2 = nil;

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port5, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port2, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = customerMaskedBytes},
           packetCount = 1},
      {portNum = port3, packetCount = 0},
      {portNum = port4, packetCount = 0},
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- clear configuration
if not executeLocalConfig(strDeconfigFileName_A) then
   keep_error("executeLocalConfig " .. strDeconfigFileName_A)
end

flushFdbDynamic();
end --allowTestDevice_A

if allowTestDevice_B then
print("-------------------------------------------------")
print("SPBM test device B")

-- run a configuration
if not executeLocalConfig(strConfigFileName_B) then
   keep_error("executeLocalConfig " .. strConfigFileName_B)
end

-- 1.2. Mc tunneled Packet1 through Device B (send to B/4)
-- Pbb Mc tunneled from Provider edge
-- sent from B/4
-- expected pass to B/5 (as is) and flooding (B/1 untagged, B/3 tagged)

sentPacket = pktPbbMcPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]]);
receivedPacket1 = pktCustomPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]],  "B" --[[tagged--]]);
receivedPacket2 = pktCustomPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]],  nil --[[tagged--]]);

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
       devNum = devNum,
       portNum = port5, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port2, pktInfo =
           {fullPacket = receivedPacket2, maskedBytesTable = customerMaskedBytes},
           packetCount = 1},
       {portNum = port3, packetCount = 0},
       {portNum = port4, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = customerMaskedBytes},
           packetCount = 1},
       {portNum = port6, pktInfo =
           {fullPacket = sentPacket, maskedBytesTable = pbbMaskedBytes},
           packetCount = 1}
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- 2.2. Uc tunneled Packet2 through Device B (send to B/5)
-- Pbb Uc tunneled from Provider edge
-- sent from B/5
-- expected pass to B/4 (as is) only)

sentPacket = pktPbbUcPacket(
    "C" --[[src_dev--]], "A"--[[dst_dev--]],
    3   --[[src_port--]], 1 --[[dst_port--]],
    1   --[[sid_idx--]]);
receivedPacket1 = nil;
receivedPacket2 = nil;

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port6, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port2, packetCount = 0},
       {portNum = port3, packetCount = 0},
       {portNum = port4, packetCount = 0},
       {portNum = port5, pktInfo =
           {fullPacket = sentPacket, maskedBytesTable = pbbMaskedBytes},
           packetCount = 1}
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- 3.1. Packet2 (tagged vid as in device B) through Device B (send to B/3)
-- Known Uc from Customer edge
-- send tagged packet from B/3 to A/1
-- expected  (pbb UC tunneled) packet from B/4 only (learned by FDB from previous send)

sentPacket = pktCustomPacket(
    "C" --[[src_dev--]], "A"--[[dst_dev--]],
    3   --[[src_port--]], 1 --[[dst_port--]],
    1   --[[sid_idx--]],  "B" --[[tagged--]]);
receivedPacket1 = pktPbbUcPacketWithHdrDevs(
    "C" --[[src_dev--]], "A"--[[dst_dev--]],
    3   --[[src_port--]], 1 --[[dst_port--]],
    1   --[[sid_idx--]],
    "B" --[[hdr_src_dev--]], "A" --[[hdr_dst_dev--]]);
receivedPacket2 = nil;

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port4, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port2, packetCount = 0},
       {portNum = port3, packetCount = 0},
       {portNum = port5, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = pbbMaskedBytes},
           packetCount = 1},
       {portNum = port6, packetCount = 0},
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- 3.2. Uc tunneled Packet1 through Device B (send to B/4)
-- Pbb Uc tunneled from Provider edge
-- sent from B/4
-- expected to B/1 only, untuneled, untagged

sentPacket = pktPbbUcPacketWithHdrDevs(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]],
    "A" --[[hdr_src_dev--]], "B" --[[hdr_dst_dev--]]);
receivedPacket1 = pktCustomPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]],  "B" --[[tagged--]]);
receivedPacket2 = nil;

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port5, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port2, packetCount = 0},
       {portNum = port3, packetCount = 0},
       {portNum = port4, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = customerMaskedBytes},
           packetCount = 1},
       {portNum = port6, packetCount = 0},
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- 4.1. Mc packet3 (registered Mc Mac DA, untagged) (send to B/1)
-- expected to B/4 only, Mc-tuneled

sentPacket = pktCustomPacketWithDstMac(
    "B" --[[src_dev--]], "A"--[[dst_dev--]],
    1   --[[src_port--]], 0xFF --[[dst_port--]],
    1   --[[sid_idx--]],  nil --[[tagged--]], "910100112233" --[[dst_mac--]]);
receivedPacket1 = pktPbbMcPacketWithPassengerDstMac(
    "B" --[[src_dev--]], "A"--[[dst_dev--]],
    1   --[[src_port--]], 0xFF --[[dst_port--]],
    1   --[[sid_idx--]],  "910100112233" --[[passenger_dst_mac--]]);
receivedPacket2 = pktCustomPacketWithDstMac(
    "B" --[[src_dev--]], "A"--[[dst_dev--]],
    1   --[[src_port--]], 0xFF --[[dst_port--]],
    1   --[[sid_idx--]],  "B" --[[tagged--]], "910100112233" --[[dst_mac--]]);

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port2, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port3, packetCount = 0},
       {portNum = port4, pktInfo =
           {fullPacket = receivedPacket2, maskedBytesTable = customerMaskedBytes},
           packetCount = 1},
       {portNum = port5, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = pbbMaskedBytes},
           packetCount = 1},
       {portNum = port6, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = pbbMaskedBytes},
           packetCount = 1},
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- clear configuration
if not executeLocalConfig(strDeconfigFileName_B) then
   keep_error("executeLocalConfig " .. strDeconfigFileName_B)
end

flushFdbDynamic();
end -- allowTestDevice_A

if allowTestDevice_C then
print("-------------------------------------------------")
print("SPBM test device C")

-- run a configuration
if not executeLocalConfig(strConfigFileName_C) then
   keep_error("executeLocalConfig " .. strConfigFileName_C)
end

-- 1.3. Mc tunneled Packet1 through Device C (send to C/4)
-- Unknown Uc from Provider edge
-- send to port 4 packet from A/1 to C/3 (MC tunneled)
-- expected floodind C/1 and C/3

sentPacket = pktPbbMcPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]]);
receivedPacket1 = pktCustomPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]],  "dst" --[[tagged--]]);
receivedPacket2 = pktCustomPacket(
    "A" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 3 --[[dst_port--]],
    1   --[[sid_idx--]],  nil --[[tagged--]]);

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port5, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port2, pktInfo =
           {fullPacket = receivedPacket2, maskedBytesTable = customerMaskedBytes},
           packetCount = 1},
       {portNum = port3, packetCount = 0},
       {portNum = port4, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = customerMaskedBytes},
           packetCount = 1},
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- 2.1. Packet2 (tagged) through Device C (send to C/3)
-- Known Uc from Customer edge
-- send tagged packet from C/3 to A/1
-- expected  (pbb UC tunneled) packet from C/4 only (learned by FDB from previous send)

sentPacket = pktCustomPacket(
    "C" --[[src_dev--]], "A"--[[dst_dev--]],
    3   --[[src_port--]], 1 --[[dst_port--]],
    1   --[[sid_idx--]],  "src" --[[tagged--]]);
receivedPacket1 = pktPbbUcPacket(
    "C" --[[src_dev--]], "A"--[[dst_dev--]],
    3   --[[src_port--]], 1 --[[dst_port--]],
    1   --[[sid_idx--]]);
receivedPacket2 = nil;

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port4, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port2, packetCount = 0},
       {portNum = port3, packetCount = 0},
       {portNum = port5, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = pbbMaskedBytes},
           packetCount = 1},
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- 4.3. Mc tunneled Packet3 through Device C (send to C/4)
-- Known Mc from Provider edge
-- sent to A/4 packet [tunneled as MC] from B/1 to 91:01:00:11:22:33 (and learn mac)
-- expected reaching A/1 only (learned by static configuration)

sentPacket = pktPbbMcPacketWithPassengerDstMac(
    "B" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 0xFF --[[dst_port--]],
    1   --[[sid_idx--]],  "910100112233" --[[passenger_dst_mac--]]);
receivedPacket1 = pktCustomPacketWithDstMac(
    "B" --[[src_dev--]], "C"--[[dst_dev--]],
    1   --[[src_port--]], 0xFF --[[dst_port--]],
    1   --[[sid_idx--]],  "C" --[[tagged--]], "910100112233" --[[dst_mac--]]);
receivedPacket2 = nil;

-- send the packet and check captured packets
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
   {  -- transmitInfo
      devNum = devNum,
      portNum = port5, pktInfo = {fullPacket = sentPacket}
   },
   {  -- expected packets
       {portNum = port2, packetCount = 0},
       {portNum = port3, packetCount = 0},
       {portNum = port4, pktInfo =
           {fullPacket = receivedPacket1, maskedBytesTable = customerMaskedBytes},
           packetCount = 1},
   }
);

if rc ~= 0 then
   keep_error(
       string.format("luaTgfTransmitPacketsWithExpectedEgressInfo failed. Error:"..
       "#%d, \" %s \" ", rc, returnCodes[rc]))
end

-- clear configuration
if not executeLocalConfig(strDeconfigFileName_C) then
   keep_error("executeLocalConfig " .. strDeconfigFileName_C)
end

flushFdbDynamic();

print("-------------------------------------------------")
end --allowTestDevice_C

if not allowTestDevice_A then
    print("NOT tested device A")
end
if not allowTestDevice_B then
    print("NOT tested device B")
end
if not allowTestDevice_C then
    print("NOT tested device C")
end
