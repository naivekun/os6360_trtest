--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* tuntap.lua
--*
--* DESCRIPTION:
--*       setting a management interface (OS)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
local function configure_management_interface(params)
    local dev_vlan_range = getGlobal("ifRange")
    local devId, vlanId
    local d, v, i
    for d,v in pairs(dev_vlan_range) do
        if devId ~= nil then
            print("Can be used for only one vlan")
            return false
        end
        devId = d
        for i = 1,#v do
            if vlanId ~= nil then
                print("Can be used for only one vlan")
                return false
            end
            vlanId = v[i]
        end
    end
    if vlanId == nil then
        print("no device/vlan choosen")
        return false
    end

    local ret
    ret = myGenWrapper("osNetworkStackInit",{})
    if ret ~= 0 then
        print("osNetworkStackInit() failed, rc="..to_string(ret))
        return false
    end
    ret = myGenWrapper("osNetworkStackConnect",{
        {"IN","GT_U8","devId",devId},
        {"IN","GT_U16","vlanId",vlanId},
        {"IN","GT_ETHERADDR","ether",params.mac.string}
    })
    if ret ~= 0 then
        print("osNetworkStackConnect() failed, rc="..to_string(ret))
        return false
    end
    local ifconfig
    -- linux only
    ifconfig = "up inet "..params.ip.string
    if params.mask ~= nil then
        ifconfig = ifconfig .. " netmask "

        local msk = 0xffffffff
        if params.mask < 32 then
            msk = bit_shl(msk,32-params.mask)
        end
        ifconfig = ifconfig .. to_string(bit_and(bit_shr(msk,24),0xff)).."."
        ifconfig = ifconfig .. to_string(bit_and(bit_shr(msk,16),0xff)).."."
        ifconfig = ifconfig .. to_string(bit_and(bit_shr(msk,8),0xff)).."."
        ifconfig = ifconfig .. to_string(bit_and(msk,0xff))
    end
    -- end linux only
    ret = myGenWrapper("osNetworkStackIfconfig",{
      {"IN","string","config",ifconfig}})
    if ret ~= 0 then
        print("osNetworkStackIfconfig() failed, rc="..to_string(ret))
        return false
    end
    --TODO: params.default_gateway.string
    print("configured")
    return true
end

local function disable_management_interface(params)
    local ret
    ret = myGenWrapper("osNetworkStackDisconnect",{})
    if ret ~= 0 then
        print("osNetworkStackDisconnect() failed, rc="..to_string(ret))
        return false
    end
    return true
end

CLI_addHelp("vlan_configuration", "management", "Configure management interface")
CLI_addCommand("vlan_configuration", "management interface", {
    func   = configure_management_interface,
    help   = "Configure management interface",
    params = {
      { type = "named",
          { format="mac %mac-address-uc" },
          { format="ip %ipv4_uc" },
          { format="mask %default_ipv4-mask" },
          -- "#default_gateway",
          mandatory={"mac", "ip" }
      }    
  }
})

CLI_addHelp("vlan_configuration", "no management", "Disable management interface")
CLI_addCommand("vlan_configuration", "no management interface", {
    func   = disable_management_interface,
    help   = "Disable management interface"
})
