--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ping_reply_protocol.lua
--*
--* DESCRIPTION:
--*       ping reply protocol
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

proto_struct_IcmpEcho={
    icmptype={off=0,bits=8},
    code={off=1,bits=8},
    crc={off=2,bits=16},
    id={off=4,bits=16},
    seq={off=6,bits=16}
}

function proto_create_struct_icmp(values,content)
    local pkt
    pkt = proto_create_struct("IcmpEcho",values) .. content
    pkt = set_crc16(pkt,8+string.len(content)/2,2)
    return pkt
end
-- ************************************************************************
---
--  handleArp
--        @description  This function builds an arp reply from an existing 
--                      arp request 
--
--        @param rxdata         - receive data
--
function handleICMP(rxdata,parsed,payloadOffset)
    if myIP == nil or myMac == nil then
        -- no IP/MAC defined, ignore
        return
    end

    local icmpLen = parsed.ipv4.length-parsed.ipv4.IHL*4
    if crc16(rxdata.packet.data,icmpLen,payloadOffset) ~= 0 then
        -- bad ICMP crc
        protocols_log(string.format("bad ICMP crc: 0x%04x",
                    crc16(rxdata.packet.data,icmpLen,payloadOffset) ))
        return
    end
    local i = parse_packet_structure(proto_struct_IcmpEcho,rxdata.packet.data,payloadOffset)

    --L2
    reply = rxdata.packet.data
    reply = proto_set_bytes(reply, 0, 6, parsed.ieee8021q.src)
    reply = proto_set_bytes(reply, 6, 6, myMac)
    --L3
    reply = fill_packet_structure(proto_struct_IPv4,{
            src = myIP,
            dst = parsed.ipv4.src,
            ttl = parsed.ipv4.ttl-1
        }, reply, parsed.ieee8021q.payloadOffset)
    -- set IP header crc
    reply = set_crc16(reply,parsed.ipv4.IHL*4,10,parsed.ieee8021q.payloadOffset)
    -- ICMP reply
    reply = fill_packet_structure(proto_struct_IcmpEcho, {
            icmptype = 0, -- echo reply
            code = 0 -- echo reply
        }, reply, payloadOffset)
    -- ICMP crc
    reply = set_crc16(reply,icmpLen,2,payloadOffset)

    sendPacket_devport(rxdata.devNum, rxdata.portNum, reply)
end


protocolICMP = {
    name="ICMPEchoReply",
    func="handleICMP",
    ident={
        level=3,
        ieee8021q = {
            -- myMac
            dst = "000000000000"
        },
        ipv4={
            -- myIP
            dst = "00000000",
            protocol = 1
        },
        payload = {
            -- type == 8, code == 0  echo request
            { offset=0,len=2,val="0800" }
        }
    }
}

-- Enable ICMP Echo Reply protocol
function proto_ping_reply_enable()
    if myIP == nil then
        print("ping_reply enable failed: my IP address is not defined")
        return false
    end
    if myMac == nil then
        print("ping_reply enable failed: my MAC address is not defined")
        return false
    end
    print("Activating ping reply proto")
    protocolICMP.disabled = nil
    protocolICMP.ident.ieee8021q.dst = myMac
    protocolICMP.ident.ipv4.dst = myIP
    protocolAdd("ICMPEchoReply", protocolICMP)
end
function proto_ping_reply_disable()
    protocolEnable("ICMPEchoReply",false)
end
