--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* proto_arp_reply.lua
--*
--* DESCRIPTION:
--*       arp reply protocol
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

proto_struct_ARP={
    LENGTH=28,
    hdr={off=0,bytes=6}, -- htype(16) ptype(16) hlen(8) plen(8)
    oper={off=6,bits=16},
    senderHA={off=8,bytes=6},
    senderPA={off=14,bytes=4},
    targetHA={off=18,bytes=6},
    targetPA={off=24,bytes=4}
}
proto_struct_ARP_def={
    hdr="000108000604"
}

-- ************************************************************************
---
--  handleArp
--        @description  This function builds an arp reply from an existing 
--                      arp request 
--
--        @param rxdata         - receive data
--

function handleArp(rxdata,parsed,payloadOffset)
    if myIP == nil or myMac == nil then
        -- no IP/MAC defined, ignore
        return
    end
    local i = parse_packet_structure(proto_struct_ARP,rxdata.packet.data,payloadOffset)
    --L2
    reply = proto_get_bytes(rxdata.packet.data,0,payloadOffset)
    reply = proto_set_bytes(reply, 0, 6, i.senderHA)
    reply = proto_set_bytes(reply, 6, 6, myMac)
    --ARP reply
    reply = reply .. fill_packet_structure(proto_struct_ARP,{
        hdr="000108000604",
        oper=2,
        senderHA=myMac,
        senderPA=myIP,
        targetHA=i.senderHA,
        targetPA=i.senderPA
    })
    sendPacket_devport(rxdata.devNum, rxdata.portNum, reply)
end


protocolARP = {
    name="ARPReply",
    func = "handleArp",
    ident = {
        level = 2,
        ieee8021q = {
            dst = "FFFFFFFFFFFF",
            etherType = 0x0806
        },
        payload = {
            -- htype=1,ptype=0800,hlen=6,plen=4,oper=1
            { offset=0,len=8,val="0001080006040001" },
            -- myIP
            { offset=24,len=4,val="00000000" }
        }
    }
}

-- Enable ARP Reply protocol
function proto_arp_reply_enable()
    if myIP == nil then
        print("ARP enable failed: my IP address is not defined")
        return false
    end
    if myMac == nil then
        print("ARP enable failed: my MAC address is not defined")
        return false
    end
    print("Activating ARP reply proto")
    protocolARP.disabled = nil
    protocolARP.ident.payload[2].val = myIP
    protocolAdd("ARPReply", protocolARP)
end
function proto_arp_reply_disable()
    protocolEnable("ARPReply",false)
end
