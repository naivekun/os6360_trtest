--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* set_mac_and_ip.lua
--*
--* DESCRIPTION:
--*       setting of the device MAC or IP address
--*
--* FILE REVISION NUMBER:
--*       $Revision: 3 $
--*
--********************************************************************************

--includes

--constants

--These are the default MAC and IP address, if they were alerady defined they will be loaded

--Globals defined:
--   myIP - The ip address of the device
--  myMac - The mac address of the device

myMac=luaGlobalGet("myMac")
myIP=luaGlobalGet("myIP")

function storeDeviceMac(mac)
    if mac ~= nil then
        myMac = string.gsub(mac.string,":","")
    luaGlobalStore("myMac",myMac)
    end
end

function storeDeviceIp(ip)
    if ip ~= nil then
    myIP = string.format("%02X%02X%02X%02X",ip[1],ip[2],ip[3],ip[4])
    luaGlobalStore("myIP",myIP)
    end
end

function is_ip_defined(ip_address)
    if ip_address == nil then
        print("error: System ip address is not defined")
        return false
    end
    return true
end

function is_mac_defined(ip_address)
    if ip_address == nil then
        print("error: System mac address is not defined")
        return false
    end
    return true
end


--This function sets device MAC/IP, they addresses are saved in the global variables myMac and myIP.
--The addresses are also saved in the global storage which is global for all lua theards (myMac and myIP)

-- ************************************************************************
---
--  setDeviceAddr
--        @description  This function sets device MAC/IP, they addresses
--                      are saved in the global variables myMac and myIP.
--                      The addresses are also saved in the global storage
--                      which is global for all lua theards
--
local function setDeviceAddr(params)
    storeDeviceIp(params.ip)
end

CLI_addCommand("config", "set", {
    func=setDeviceAddr,
  help="Set the device MAC or IP address",
    params={ { type="named", { format="ip %ipv4",name="ip", help="IP to set as device IP, for example : \"1.1.1.1\"" },
      }}

})
