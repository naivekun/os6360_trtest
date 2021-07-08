-- Incoming and egressed packets used in remote switched port analyzer test.
local rxEthernetHeader = ""
   .."000000000022"             -- MAC DA
   .."000000000011"             -- MAC SA
   
local remoteRxVlanTag = ""
    .."811001f4"                -- remote Rx analyaer VLAM tag

local remoteTxVlanTag = ""
    .."812003e8"                -- remote Tx analyaer VLAM tag

local payload = ""
   .."fffefdfcfbfa000000000000" -- 48 bytes
   .."000000000000000000000000000000000000000000000000000000000000000000000000"

   
-------------------- PACKETS ---------------------------------------------------
-- incoming packet
local rxPacket = rxEthernetHeader..payload

-- the packet sent by device should be egressed to analyzer port 3
local rxPacket_anlyzer = ""
    ..rxEthernetHeader
    ..remoteRxVlanTag
    ..payload

-- the packet sent by device should be egressed to analyzer port 4
local txPacket_anlyzer = ""
    ..rxEthernetHeader
    ..remoteTxVlanTag
    ..payload

-- the packet sent by device should be egressed to port 2
local txPacket = rxEthernetHeader..payload

return { 
   rx1 = rxPacket,
   tx2 = txPacket,
   tx3 = rxPacket_anlyzer,
   tx4 = txPacket_anlyzer
}
