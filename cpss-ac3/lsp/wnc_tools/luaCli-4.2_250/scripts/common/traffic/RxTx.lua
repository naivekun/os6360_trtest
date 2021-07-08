--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* RxTx.lua
--*
--* DESCRIPTION:
--*       Functions to work with packets
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

require("common/traffic/protocols")

-- ************************************************************************
---
--  genPktL2
--        @description  The function generated L2 packet by input data
--        @param dstMAC               - destination MAC address
--        @param srcMAC               - source MAC address
--        @param ieee802_1q_Tag       - optional
--        @param ieee802_1q_OuterTag  - optional
--        @param ieee802_1q_InnerTag  - optional
--        @param etherType            - Type/MTU tag
--        @param payload              - packets payload
--        @return  L2 packet
--
function genPktL2(params)
  local t = { }

  t[#t+1] = params.dstMAC
  t[#t+1] = params.srcMAC

  if params.ieee802_1q_Tag ~= nil and
     params.ieee802_1q_OuterTag == nil and params.ieee802_1q_InnerTag == nil then
    t[#t+1] = params.ieee802_1q_Tag
  end

  if params.ieee802_1q_Tag == nil and
     params.ieee802_1q_OuterTag ~= nil and params.ieee802_1q_InnerTag ~= nil then
    t[#t+1] = params.ieee802_1q_OuterTag
    t[#t+1] = params.ieee802_1q_InnerTag
  end

  t[#t+1] = params.etherType
  t[#t+1] = params.payload

  return table.concat(t)
end

local function stringIpToHex(str)
    local ip = ""
    local w
    for w in string.gmatch(str, "%d+") do
        ip = ip..string.format("%02X", tonumber(w))
    end
    return string.sub(ip.."00000000",1,8)
end

-- ************************************************************************
--  stringIpv6ToHex
--
--  @description  Transform IPv6 address (string) to a hexadecimal
--                view (string).
--
--  @param str - string with an IPv6 address
--
--  @return
--      string with hexadecimal vies. If str is ill-formed IPv6 address
--      return nil.
--
local function stringIpv6ToHex(str)
    local t = {} -- here a result will be collected
    local zix = 0 -- table index of "::". Will be replaced by zeroes
    local finishedAtOneColon = false

    -- check for leading "::".
    if (str:sub(1,2) == "::") then
        zix = 1
    elseif str:sub(1,1) == ":" then
        return nil
    end

    local ix = 0
    for ns, cs in str:gmatch("([^:]+)(:*)") do
        ix = ix + 1
        if (#ns > 4) or (#cs > 2) or (#cs == 2 and zix ~=0)
        then
            return nil
        end
        if #cs == 2 then
            zix = ix+1
            finishedAtOneColon = false
        else
            finishedAtOneColon = #cs > 0
        end
        table.insert(t, ix, string.sub("0000"..ns, -4))
    end

    if finishedAtOneColon         -- an address can't be end by a single colon
        or (zix == 0 and #t ~= 8) -- and IP address should consist of 8 groups
        or (#t == 0)              -- there is no address at all
        or (#t > 8)               -- too mach groups
    then
        return nil
    end
    if zix ~= 0 then            -- a "::" was found. Replace by zeroes
        table.insert(t, zix, string.rep("0000", 8 - #t))
    end
    return table.concat(t)
end


-- ************************************************************************
---
--  genPktIpV4
--        @description  The function generated IpV4 packet by input data
--        @param srcIP               - source IpV4 address
--        @param dstIP               - destination IpV4 address
--        @param data                - data
--        @param protocol            - ip protocol (optional)
--                                     ...
--        @return  packet
--
function genPktIpV4(params)
  params.src = stringIpToHex(params.srcIp)
  params.dst = stringIpToHex(params.dstIp)

  return proto_create_struct_ip(params,params.data)
end


-- ************************************************************************
--  genPktIpV6
--
--  @description  The function generated IPv6 packet by input data
--
--  @param srcIP      - source IPv6 address
--  @param dstIP      - destination IPv6 address
--  @param data             - data
--  @param protocol   - ip protocol (optional)
--  ... here can be any additional parameters that are present in the
--      structure proto_struct_IPv6 if you don't like defaults.
--  @return packet
--
function genPktIpV6(params)
    local payload = params.data
    params.src = stringIpv6ToHex(params.srcIp)
    params.dst = stringIpv6ToHex(params.dstIp)
    params.payloadLength = string.len(payload) / 2;
    return proto_create_struct("IPv6", params) .. payload
end
