-- Incoming and egressed packets used in SPBM routing test (VPN2).
-- Used tunnel headers are MAC-in-MAC headers (IEEE802.1ah)

local rxEthernetHeader = ""
   .."00000000debb"             -- MAC DA
   .."000000000123"             -- MAC SA
   .."810003e8"                 -- vlan tag (vid = 0x3e8 = 1000)
   .."0800"                     -- EtherType = IPv4

local payload = ""
   .."fffefdfcfbfa000000000000" -- 48 bytes
   .."000000000000000000000000000000000000000000000000000000000000000000000000"

   
-- Three IP-parts of incoming packets (i.e. packets without Ethernet header)
local ipHeader1 = ""
   .."450000440000000028003898" -- IPv4 header: Ver=4, HdrLen =5, TotalLen=68, TTL=40
   .."ac100001"                 -- SIP = 172.16.0.1
   .."ac100201"                 -- DIP = 172.16.2.1

local ipHeader2 = ""
   .."450000440000000028003798" -- IPv4 header: Ver=4, HdrLen =5, TotalLen=68, TTL=40
   .."ac100001"                 -- SIP = 172.16.0.1
   .."ac100301"                 -- DIP = 172.16.3.1

local ipHeader3 = ""
   .."450000440000000028003698" -- IPv4 header: Ver=4, HdrLen=5, TotalLen=68, TTL=40
   .."ac100001"                 -- SIP = 172.16.0.1
   .."ac100401"                 -- DIP = 172.16.4.1


-------------------- PACKETS ---------------------------------------------------
-- incoming packets (device B, local customer port)
local rxPacket_B1 = rxEthernetHeader..ipHeader1..payload
local rxPacket_B2 = rxEthernetHeader..ipHeader2..payload
local rxPacket_B3 = rxEthernetHeader..ipHeader3..payload

-- outgoing packet #1 (device B, local customer port)
local txPacket_B1 = ""
   .."000000000221" -- MAC DA
   .."00000000debb" -- MAC SA
   .."810007d0"     -- vlan tag (vid = 0x7d0 = 2000)
   .."0800"         -- EtherTYpe = IPv4
   ..ipHeader1..payload

-- outgoing packet #2(device B, backbone port, intended to device A)
local txPacket_B2 = ""
   .."00000000deaa" -- B-DA MAC
   .."00000000debb" -- B-SA MAC
   .."88a801ff"     -- B-VLAN TAG: TPID=0x88a8, vid=0x1ff=511
   .."88e7"         -- TPID=0x88e7
   .."00020000"     -- I-SID = 0x020000
   .."00000000deaa" -- MAC DA
   .."00000000debb" -- MAC SA
   .."0800"         -- EtherType = IPv4
   ..ipHeader2..payload


-- outgoing packet #3(device B, backbone port, intended to device C)
local txPacket_B3 = ""
   .."00000000decc" -- B-DA MAC
   .."00000000debb" -- B-SA MAC
   .."88a801ff"     -- B-VLAN TAG: TPID=0x88a8, vid=0x1ff=511
   .."88e7"         -- TPID=0x88e7
   .."00020000"     -- I-SID = 0x020000
   .."00000000decc" -- MAC DA
   .."00000000debb" -- MAC SA
   .."0800"         -- EtherType = IPv4
   ..ipHeader3..payload

-- the packet sent by device B, should be egressed on the device A
local txPacket_A = ""
   .."000000000231" -- MAC DA
   .."00000000deaa" -- MAC SA
   ..""             -- without vlan tag
   .."0800"         -- EtherType = IPv4
   .. ipHeader2..payload

-- the packet sent by device B, should be egressed on the device C
local txPacket_C = ""
   .."000000000241" -- MAC DA
   .."00000000decc" -- MAC SA
   ..""             -- without vlan tag
   .."0800"         -- EtherType = IPv4
   ..ipHeader3..payload

return { 
   rxB = {rxPacket_B1, rxPacket_B2, rxPacket_B3},
   txB = {txPacket_B1, txPacket_B2, txPacket_B3},
   txA = txPacket_A,
   txC = txPacket_C
}
