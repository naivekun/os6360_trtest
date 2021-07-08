--********************************************************************************
--*              (c), Copyright 2014, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* protocols.lua
--*
--* DESCRIPTION:
--*       Packet analyzing and protocol implementations tools
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cmdLuaCLI_registerCfunction("luaMsgQCreate")
cmdLuaCLI_registerCfunction("luaMsgQAttach")
cmdLuaCLI_registerCfunction("luaMsgQDelete")
cmdLuaCLI_registerCfunction("luaMsgQSend")
cmdLuaCLI_registerCfunction("luaMsgQRecv")
cmdLuaCLI_callCfunction("registerBitwizeFunctions")

-- ************************************************************************
---
--  protocols_log
--        @description  print message to console using osPrintf()
--                      This function can be used in packet receiver task
--
--        @param msg            - message to print
--
function protocols_log(msg)
    if not protocols_log_enabled then
        return
    end
    if log_console ~= nil then
        log_console(msg.."\r")
    else
        print(msg)
    end
end

-- ************************************************************************
---
--  proto_get_bytes
--        @description  Read bytes from packet (hex string)
--
--        @param data       - data
--        @param offset     - offset in _bytes_, starting 0
--        @param len        - number of bytes to read
--        @return           - bytes (hex string) or empty line
--
function proto_get_bytes(data,offset,len)
    return string.sub(data,offset*2+1,offset*2+len*2)
end
-- ************************************************************************
---
--  proto_get_bits
--        @description  Read bitfield from packet (hex string)
--
--        @param data       - data
--        @param offset     - offset in _bytes_, starting 0
--        @param bitoffset  - offset in bits from data+offset, starting 0
--        @param bits       - number of bits
--        @return           - value (number)
--
function proto_get_bits(data,offset,bitoffset,bits)
    local o = offset
    local value = 0
    while bitoffset >= 8 do
        o = o + 1
        bitoffset = bitoffset - 8
    end
    while bits > 0 do
        local byte = tonumber("0x"..proto_get_bytes(data, o, 1))
        local cbits
        if bitoffset + bits > 8 then
            cbits = 8 - bitoffset
        else
            cbits = bits
        end
        if bitoffset + cbits < 8 then
            byte = bit_shr(byte, 8-bitoffset-cbits)
        end
        byte = bit_and(byte, bit_shl(1,cbits)-1)
        value = bit_shl(value, cbits) + byte
        bits = bits - cbits
        bitoffset = 0
        o = o + 1
    end
    return value
end

-- ************************************************************************
---
--  parse_field
--        @description  Read structure member from packet
--
--        @param fdescr     - field description:
--                            .off    - byte offset in structure
--                            .bytes  - length of data in bytes
--                            .bits      - length of data in bits
--                            .bitoffset - offset in bits of numeric data
--        @param data       - data
--        @param offset     - structure offset in _bytes_, starting 0
--        @return           - value (number or hex string)
--
local function parse_field(fdescr, data, offset)
    if type(fdescr) ~= "table" then
        return fdescr
    end
    if fdescr.off == nil then return nil end
    if fdescr.bytes ~= nil then
        return proto_get_bytes(data, offset + fdescr.off, fdescr.bytes)
    end
    if fdescr.bits == nil then
        return nil
    end
    local bitoffset = fdescr.bitoffset
    local bits = fdescr.bits
    if bitoffset == nil then bitoffset = 0 end
    if bits == 8 and bitoffset == 0 then
        return tonumber("0x"..proto_get_bytes(data, offset + fdescr.off, 1))
    elseif bits == 16 and bitoffset == 0 then
        return tonumber("0x"..proto_get_bytes(data, offset + fdescr.off, 2))
    elseif bits == 32 and bitoffset == 0 then
        return tonumber("0x"..proto_get_bytes(data, offset + fdescr.off, 4))
    elseif bits <= 32 then
        return proto_get_bits(data, offset + fdescr.off, bitoffset, bits)
    end
end
-- ************************************************************************
---
--  parse_packet_structure
--        @description  parse structure in data
--
--        @param struct     - data structure description:
--                            fieldname => fdescr (see parse_field)
--                            LENGTH => struct length in bytes (optional)
--        @param data       - data
--        @param offset     - structure offset in _bytes_, starting 0
--        @return           - parsed structure
--
function parse_packet_structure(struct, data, offset)
    local fname, fdescr
    local value
    local out = {}
    if offset == nil then offset = 0 end
    for fname, fdescr in pairs(struct) do
        out[fname] = parse_field(fdescr, data, offset)
    end
    return out
end

proto_ieee8021qHeader={
    dst       = { off=0, bytes=6 },
    src       = { off=6, bytes=6 },
    etherType = { off=12, bits=16 }
}
proto_ieee8021qVlanTag={
    TPID      = { off=0, bits=16 },
    PCP       = { off=2, bitoffset=0, bits=3 },
    DEI       = { off=2, bitoffset=3, bits=1 },
    vid       = { off=2, bitoffset=4, bits=12 }
}
-- ************************************************************************
---
--  parse_ieee8021q
--        @description  parse ieee8021q header:
--              src mac, dst mac, etherType, vlan tags
--
--        @param packet     - packet data (hex string)
--        @param parsed     - table object for parsed data
--
function parse_ieee8021q(packet,parsed)
    if parsed.ieee8021q ~= nil then return end
    local ieee8021q = parse_packet_structure(proto_ieee8021qHeader,packet)
    ieee8021q.payloadOffset = 14
    parsed.ieee8021q = ieee8021q
    -- handle vlan tags
    while ieee8021q.etherType == 0x8100 or ieee8021q.etherType == 0x9100 do
        if ieee8021q.vlan == nil then
            ieee8021q.vlan = {}
        end
        table.insert(ieee8021q.vlan, parse_packet_structure(
            proto_ieee8021qVlanTag, packet, ieee8021q.payloadOffset-2))
        ieee8021q.etherType = proto_get_bits(packet,ieee8021q.payloadOffset+2,0,16)
        ieee8021q.payloadOffset = ieee8021q.payloadOffset + 4
    end
end

proto_struct_IPv4={
    LENGTH  = 20,
    version = { off=0, bitoffset=0, bits=4 },
    IHL     = { off=0, bitoffset=4, bits=4 },
    DSCP    = { off=0, bitoffset=8, bits=6 },
    ECN     = { off=0, bitoffset=16,bits=2 },
    length  = { off=2, bits=16 },
    ident   = { off=4, bits=16 },
    flags   = { off=6, bits=3 },
    fragOff = { off=6, bitoffset=3, bits=13 },
    ttl     = { off=8, bits=8 },
    protocol= { off=9, bits=8 },
    crc     = { off=10,bits=16 },
    src     = { off=12,bytes=4 },
    dst     = { off=16,bytes=4 }
}
proto_struct_IPv4_def={
    version=4, IHL=5, length=20, ttl=40
}

proto_struct_IPv6={
    LENGTH        = 40,
    version       = { off=0, bitoffset=0, bits=4 },
    trafficClass  = { off=0, bitoffset=4, bits=8 },
    flowLabel     = { off=1, bitoffset=4, bits=20 },
    payloadLength = { off=4, bits=16 },
    nextHeader    = { off=6, bits=8 },
    hopLimit      = { off=7, bits=8 },
    src           = { off=8, bytes=16 },
    dst           = { off=24,bytes=16 }
}
proto_struct_IPv6_def={
    version=6, TrafficClass=0, flowLabel=0, payloadLength = 0,
    nextHeader=59, hopLimit=40
}

-- ************************************************************************
---
--  parse_ipv4
--        @description  parse IPv4 header
--              see proto_struct_IPv4
--              also check if crc is correct
--
--        @param packet     - packet data (hex string)
--        @param parsed     - table object for parsed data
--
function parse_ipv4(packet,parsed)
    parse_ieee8021q(packet,parsed)
    if parsed.ieee8021q.etherType ~= 0x0800 then
        parsed.ipv4 = {
            payloadOffset=parsed.ieee8021q.payloadOffset
        }
        return
    end
    local ipoffset = parsed.ieee8021q.payloadOffset
    local ipv4 = parse_packet_structure(proto_struct_IPv4, packet, ipoffset)
    -- check header crc
    if crc16(packet,ipv4.IHL*4,ipoffset) ~= 0 then
        -- crc failed
        protocols_log(string.format("    IPv4 header crc error, crc16=0x%04x",crc16(packet,ipv4.IHL*4,ipoffset)))
        parsed.ipv4 = {
            payloadOffset = parsed.ieee8021q.payloadOffset
        }
        return
    end
    ipv4.payloadOffset = ipoffset + ipv4.IHL*4
    parsed.ipv4 = ipv4
end

-- ************************************************************************
---
--  do_match_p
--        @description  check match
--              for all keys
--                  if key defined in proto then
--                      it must match in parsed data
--
--        @param proto      - protocol match
--        @param parsed     - table object of parsed data
--        @param keys       - keys to check
--
local function do_match_p(proto,parsed,keys)
    local i,key
    for i,key in pairs(keys) do
        if proto[key] ~= nil and proto[key] ~= parsed[key] then
            return false
        end
    end
    return true
end
-- ************************************************************************
---
--  match_ieee8021q
--        @description  check if packet match L2(ieee8021q) rules
--
--        @param packet     - packet to match
--        @param parsed     - table object of parsed data
--        @param proto      - protocol definition table, check
--                            proto.ieee8021q
--
local function match_ieee8021q(packet,parsed,proto)
    parse_ieee8021q(packet,parsed)
    local i = proto.ieee8021q
    if i == nil then
        return true
    end
    local p = parsed.ieee8021q
    if not do_match_p(i,p,{"src","dst","etherType"}) then
        return false
    end
    if i.vlan ~= nil then
        local vi,vl
        local found = false
        for vi,vl in pairs(p.vlan) do
            if vl.vlan == i.vlan then
                found = true
            end
        end
        if not found then
            return false
        end
    end
    protocols_log("    ieee8021q rules matched")
    return true
end

-- ************************************************************************
---
--  match_ipv4
--        @description  check if packet match L3(IPv4) rules
--
--        @param packet     - packet to match
--        @param parsed     - table object of parsed data
--        @param proto      - protocol definition table, check
--                            proto.ipv4
--
local function match_ipv4(packet,parsed,proto)
    local i = proto.ipv4
    if i == nil then
        return true
    end
    parse_ipv4(packet,parsed)
    local p = parsed.ipv4
    if p == nil then
        return false
    end
    if p.version == nil then
        return false
    end
    return do_match_p(i,p,{"src","dst","protocol","ttl","version","length","IHL"})
end

-- ************************************************************************
---
--  crc16
--        @description  calculate crc16 of data
--
--        @param data       - data
--        @param len        - number of bytes to calculate crc for
--        @param offset     - offset in data _bytes_, starting 0, default 0
--
--        @return           - crc16 (number)
--
function crc16(packet,len,offset)
    local i = 0
    local crc = 0
    local w
    if offset == nil then offset = 0 end
    while i + 1 < len do
        crc = crc + tonumber("0x0"..proto_get_bytes(packet, offset + i, 2))
        if crc > 0xffff then
            crc = crc - 0xffff
        end
        i = i + 2
    end
    if i < len then
        crc = crc + tonumber("0x0"..proto_get_bytes(packet, offset + i, 1))
        if crc > 0xffff then
            crc = crc - 0xffff
        end
    end
    return bit_xor(crc, 0xffff)
end

-- ************************************************************************
---
--  set_crc16
--        @description  set crc16 value in data
--
--        @param packet     - packet data (hex string)
--        @param len        - data length in bytes
--        @param crc_offset - offset of crc16 in data
--        @param offset     - offset of data in packet
--        @return           - data (hex string) with crc16 set
--
function set_crc16(packet,len,crc_offset,offset)
    if offset == nil then offset = 0 end
    packet = proto_set_bytes(packet,offset+crc_offset,2,"0000")
    local crc = crc16(packet,len,offset)
    return proto_set_bytes(packet,offset+crc_offset,2,string.format("%04x",crc))
end

-- ************************************************************************
---
--  proto_set_bytes
--        @description  set bytes in packet (hex string)
--
--        @param data       - data
--        @param offset     - offset in _bytes_, starting 0
--        @param len        - number of bytes to read
--        @param value      - bytes to set (hex string)
--        @return           - data (hex string) with bytes set
--
function proto_set_bytes(data,offset,len,value)
    if string.len(data) < (offset+len)*2 then
        data = data .. string.rep("0",(offset+len)*2-string.len(data))
    end
    return  string.sub(data,1,offset*2) ..
            string.sub(value,1,len*2) ..
            string.sub(data,offset*2+len*2+1)
end
-- ************************************************************************
---
--  proto_set_bits
--        @description  set bitfield in packet (hex string)
--
--        @param data       - data
--        @param offset     - offset in _bytes_, starting 0
--        @param bitoffset  - offset in bits from data+offset, starting 0
--        @param bits       - number of bits
--        @param value      - value to set (number)
--        @return           - data (hex string) with bits set
--
function proto_set_bits(data,offset,bitoffset,bits,value)
    local o = offset
    while bitoffset >= 8 do
        o = o + 1
        bitoffset = bitoffset - 8
    end
    while bits > 0 do
        local byte = tonumber("0x"..proto_get_bytes(data, o, 1))
        if byte == nil then byte = 0 end
        local cbits
        if bitoffset + bits > 8 then
            cbits = 8 - bitoffset
        else
            cbits = bits
        end
        local mask = bit_shl(1,cbits)-1
        local v = value
        if bits > cbits then
            v = bit_shr(v,bits-cbits)
        end
        v = bit_and(v, mask)
        if bitoffset + cbits < 8 then
            v = bit_shl(v, 8-bitoffset-cbits)
            mask = bit_shl(v, 8-bitoffset-cbits)
        end
        byte = bit_or(bit_and(byte, bit_not(mask)),v)
        data = proto_set_bytes(data,o,1,string.format("%02x",bit_and(byte,0xff)))
        bits = bits - cbits
        bitoffset = 0
        o = o + 1
    end
    return data
end

-- ************************************************************************
---
--  set_field
--        @description  Write structure member to packet
--
--        @param fdescr     - field description:
--                            .off    - byte offset in structure
--                            .bytes  - length of data in bytes
--                            .bits      - length of data in bits
--                            .bitoffset - offset in bits of numeric data
--        @param data       - data
--        @param offset     - structure offset in _bytes_, starting 0
--        @param value      - value (number or hex string)
--        @return           - data (hex string) with struct member set
--
local function set_field(fdescr, data, offset, value)
    if type(fdescr) ~= "table" then return data end
    if fdescr.off == nil then return data end
    if fdescr.bytes ~= nil then
        return proto_set_bytes(data, offset + fdescr.off, fdescr.bytes, value)
    end
    if fdescr.bits == nil then
        return nil
    end
    local bitoffset = fdescr.bitoffset
    local bits = fdescr.bits
    if bitoffset == nil then bitoffset = 0 end
    if bits == 8 and bitoffset == 0 then
        return proto_set_bytes(data,offset+fdescr.off,1,string.format("%02x", bit_and(value, 0xff)))
    elseif bits == 16 and bitoffset == 0 then
        return proto_set_bytes(data,offset+fdescr.off,2,string.format("%04x", bit_and(value, 0xffff)))
    elseif bits == 32 and bitoffset == 0 then
        return proto_set_bytes(data,offset+fdescr.off,4,string.format("%08x", bit_and(value, 0xffffffff)))
    elseif bits <= 32 then
        return proto_set_bits(data, offset + fdescr.off, bitoffset, bits, value)
    end
end

-- ************************************************************************
---
--  fill_packet_structure
--        @description  fill structure in packet
--
--        @param struct     - data structure description:
--                            fieldname => fdescr (see parse_field)
--                            LENGTH => struct length in bytes (optional)
--        @param values     - values to set (table)
--        @param data       - data
--        @param offset     - structure offset in _bytes_, starting 0
--        @return           - data (hex string) with struct members set
--
function fill_packet_structure(struct, values, data, offset)
    local fname, fdescr
    local value
    if offset == nil then offset = 0 end
    if data == nil then data = "" end
    if values == nil then values = {} end
    if struct.LENGTH ~= nil then
        if string.len(data) < (offset*2 + struct.LENGTH*2) then
            data = data .. string.rep("0",(offset*2 + struct.LENGTH*2)-struct.LENGTH)
        end
    end
    for fname, fdescr in pairs(struct) do
        if values[fname] ~= nil then
            data = set_field(fdescr,data,offset,values[fname])
        end
    end
    return data
end

function proto_create_struct(protoName,values,pkt)
    local t = _G["proto_struct_"..protoName]
    local def = _G["proto_struct_"..protoName.."_def"]
    if type(t) ~= "table" then
        return pkt
    end
    if pkt == nil and def ~= nil then
        pkt = fill_packet_structure(t,def)
    end
    if values ~= nil then
        pkt = fill_packet_structure(t,values,pkt)
    end
    return pkt
end

function proto_create_struct_ip(values,content)
    local pkt
    values.length = (string.len(content)/2)+20
    pkt = proto_create_struct("IPv4",values) .. content
    pkt = set_crc16(pkt,20,10)
    return pkt
end

-- ************************************************************************
---
--  checkProtocol
--        @description  This function checks the protocol array for
--                      protocol matches.
--                      For every matched protocol it activates function or
--                      send packet to associated queue
--
--        @param protocolArray  - protocol array
--        @param rxdata         - received data
--
function checkProtocol(protocolArray, rxdata)
    local i,proto,ident
    local parsed = {}
    local payloadOffset
    local matched
    local packet = rxdata.packet.data
    protocols_log(string.format("checkProtocol() port=%d/%d data=%s",
            rxdata.devNum,rxdata.portNum,rxdata.packet.data))
    for i,proto in pairs(protocolArray) do
        matched = true
        if proto.disabled then
            -- skip
            matched = false
        else
            protocols_log("  proto="..i)
            ident = proto.ident
            matched = do_match_p(ident,rxdata,{"devNum","portNum","vlanId"})
            if rxdata.portNum == 63 and ident.noCpuPort == true then
                matched = false
            end
        end
        if matched then
            if ident.level == 2 then
                matched = match_ieee8021q(packet,parsed,ident)
                payloadOffset = parsed.ieee8021q.payloadOffset
            elseif ident.level == 3 then
                -- other L3 proto (ipv6)
                matched = match_ipv4(packet,parsed,ident)
                protocols_log("    L3.matched="..tostring(matched))
                payloadOffset = parsed.ipv4.payloadOffset
            else
                payloadOffset = 0
            end
        end
        if matched then
            if type(ident.payload) == "table" then
                local ii,p
                for ii,p in pairs(ident.payload) do
                    -- check match
                    if proto_get_bytes(packet,payloadOffset+p.offset,p.len) ~= p.val then
                        matched = false
                    end
                end
            end
        end
        protocols_log("    protocol "..i.." matched="..tostring(matched))
        if matched then
            local func = proto.func
            if type(func) == "string" then
                func = _G[func]
            end
            if type(func) == "function" then
                func(rxdata,parsed,payloadOffset)
            end
            if type(func) == "number" then
                local q = luaMsgQAttach(proto.name.."Queue", luaGlobalGet("rxDataFormat"), func)
                luaMsgQSend(q, rxdata, 0) --zero timeout meaning drop packets if queue full (we got deadlock when the queue was full with many packets)
            end
        end
    end
end
