-- Incoming and egressed packets used in Inter I-SID SPBM routing test.
-- Used tunnel headers are MAC-in-MAC headers (IEEE802.1ah)
local rxEthernetHeader = ""
   .."00000000deaa"             -- MAC DA
   .."000000000123"             -- MAC SA
   ..""                         -- vlan untagged
   .."0800"                     -- EtherType = IPv4

local payload = ""
   .."fffefdfcfbfa000000000000" -- 48 bytes
   .."000000000000000000000000000000000000000000000000000000000000000000000000"

   
-- Three IP-parts of incoming packets (i.e. packets without Ethernet header)
local ipHeader = ""
   .."450000440000000028003898" -- IPv4 header: Ver=4, HdrLen =5, TotalLen=68, TTL=40
   .."ac100001"                 -- SIP = 172.16.0.1
   .."ac100201"                 -- DIP = 172.16.2.1


-------------------- PACKETS ---------------------------------------------------
-- incoming packets (device A, a customer port)
local rxPacket_A = rxEthernetHeader..ipHeader..payload

-- outgoing packet #1 (device A, a backbone port).
local txPacket_A = ""
   .."00000000decc" -- B-DA MAC
   .."00000000deaa" -- B-SA MAC
   .."88a801ff"     -- B-VLAN TAG: TPID=0x88a8, vid=0x1ff=511
   .."88e7"         -- TPID=0x88e7
   .."00010000"     -- I-SID = 0x010000
   .."00000000decc" -- MAC DA
   .."00000000deaa" -- MAC SA
   .."0800"         -- EtherType = IPv4
   ..ipHeader..payload

-- the packet sent by the device C back to device B (through the backbone port)
local txPacket_C = ""
   .."00000000debb" -- B-DA MAC
   .."00000000decc" -- B-SA MAC
   .."88a801ff"     -- B-VLAN TAG: TPID=0x88a8, vid=0x1ff=511
   .."88e7"         -- TPID=0x88e7
   .."00020000"     -- I-SID = 0x020000
   .."00000000debb" -- MAC DA
   .."00000000decc" -- MAC SA
   .."0800"         -- EtherType = IPv4
   ..ipHeader..payload

-- the packet sent by device A, should be egressed to device B port 3
local txPacket_B = ""
   .."000000000121" -- MAC DA
   .."00000000debb" -- MAC SA
   ..""             -- without vlan tag
   .."0800"         -- EtherType = IPv4
   .. ipHeader..payload

return { 
   rxA = rxPacket_A,
   txA = txPacket_A, -- dev A -> dev B (not changed) -> dev C
   txB = txPacket_B, -- packet  that came back to device B from device C
   txC = txPacket_C
}
