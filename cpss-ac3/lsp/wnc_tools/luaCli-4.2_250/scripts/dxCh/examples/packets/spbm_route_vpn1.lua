-- Incoming and egressed packets used in SPBM routing test (VPN1).
-- Used tunnel headers are MAC-in-MAC headers (IEEE802.1ah)

local rxEthernetHeader = ""
   .."00000000deaa"             -- MAC DA
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
-- incoming packets (device A, customer port)
local rxPacket_A1 = rxEthernetHeader..ipHeader1..payload
local rxPacket_A2 = rxEthernetHeader..ipHeader2..payload
local rxPacket_A3 = rxEthernetHeader..ipHeader3..payload

-- outgoing packet #1 (device A, local port)
local txPacket_A1 = ""
   .."000000000121" -- MAC DA
   .."00000000deaa" -- MAC SA
   .."810007d0"     -- vlan tag (vid = 0x7d0 = 2000)
   .."0800"         -- EtherTYpe = IPv4
   ..ipHeader1..payload

-- outgoing packet #2(device A, backbone port)
local txPacket_A2 = ""
   .."00000000debb" -- B-DA MAC
   .."00000000deaa" -- B-SA MAC
   .."88a801ff"     -- B-VLAN TAG: TPID=0x88a8, vid=0x1ff=511
   .."88e7"         -- TPID=0x88e7
   .."00010000"     -- I-SID = 0x010000
   .."00000000debb" -- MAC DA
   .."00000000deaa" -- MAC SA
   .."0800"         -- EtherType = IPv4
   ..ipHeader2..payload


-- outgoing packet #3(device A, backbone port)
local txPacket_A3 = ""
   .."00000000decc" -- B-DA MAC
   .."00000000deaa" -- B-SA MAC
   .."88a801ff"     -- B-VLAN TAG: TPID=0x88a8, vid=0x1ff=511
   .."88e7"         -- TPID=0x88e7
   .."00010000"     -- I-SID = 0x010000
   .."00000000decc" -- MAC DA
   .."00000000deaa" -- MAC SA
   .."0800"         -- EtherType = IPv4
   ..ipHeader3..payload

-- the packet sent by device A, should be egressed to device B port 3
local txPacket_B = ""
   .."000000000131" -- MAC DA
   .."00000000debb" -- MAC SA
   ..""             -- without vlan tag
   .."0800"         -- EtherType = IPv4
   .. ipHeader2..payload

-- the packet sent by device A, should be egressed to device B port 3
local txPacket_C = ""
   .."000000000141" -- MAC DA
   .."00000000decc" -- MAC SA
   ..""             -- without vlan tag
   .."0800"         -- EtherType = IPv4
   ..ipHeader3..payload

return { 
   rxA = {rxPacket_A1, rxPacket_A2, rxPacket_A3},
   txA = {txPacket_A1, txPacket_A2, txPacket_A3},
   txB = {txPacket_B},
   txC = {txPacket_C}
}
