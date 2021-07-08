--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* mac_address.lua
--*
--* DESCRIPTION:
--*       setting of a MAC address for the system
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

-- define the entry name that will hold the 'MAC TO ME adress of the system'
local mac2meEntryName = "<MacToMe address of the system>"

-- ************************************************************************
---
--  mac_address_func
--        @description  set's a MAC address for the system
--
--        @param params         - params["mac-address"]: mac-address
--                                params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function mac_address_func(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum
    local command_data = Command_Data()
    -- Command specific variables declaration.
    local mac_address, mac_address_removing
    local lsb_router_mac_sa

    -- Command specific variables initialization.
    mac_address             = params["mac-address"]
    mac_address_removing    = command_data:getTrueIfFlagNoIsNil(params)
    lsb_router_mac_sa       = tonumber(mac_address[6], 16)

    -- Common variables initialization.
    command_data:initAllAvailableDevicesRange()
    command_data:initAllAvaiblesPortIterator()

    -- Setting of system MAC address lau environment variable.
    if true == command_data["status"] then
        setGlobal("myMac", mac_address)
        storeDeviceMac(mac_address)
    end
    command_data["eArch_glbl_index"] = {}

    -- Main device handling cycle
    if true == command_data["status"] then
        command_data:iterateOverDevices(
            function(c, devNum, mac_address)
                local i, result, values
                if not is_sip_5(devNum) then
                    result = myGenWrapper("cpssDxChIpRouterMacSaBaseSet", {
                                { "IN", "GT_U8",        "devNum", devNum  },
                                { "IN", "GT_ETHERADDR", "mac",    mac_address }})
                    c:handleCpssErrorDevice(result,
                                "setting of Router MAC SA Base address")
                else
                    -- Bobcat2 and above special case
                    -- scan cpssDxChIpRouterGlobalMacSa for empty slot
                    for i = 0, 255 do
                        result, values = myGenWrapper("cpssDxChIpRouterGlobalMacSaGet",
                                             {{ "IN", "GT_U8",        "devNum", devNum  },
                                              { "IN", "GT_U32", "routerMacSaIndex", i},
                                              { "OUT", "GT_ETHERADDR", "mac" }})
                        c:handleCpssErrorDevice(result,
                                    "RouterGlobalMacSaGet scan for empty slot")
                        if result ~= 0 then
                            return
                        end
                        if values.mac == "00:00:00:00:00:00" then
                            c["eArch_glbl_index"][devNum] = i
                            break
                        end
                        if string.upper(values.mac) == string.upper(mac_address.string) then
                            -- entry with required mac found, just set index
                            c["eArch_glbl_index"][devNum] = i
                            -- Mac found, but maybe other was added without removing it
                            -- Needed to configure ports to its index any case
                            break
                        end
                    end
                    if c["eArch_glbl_index"][devNum] == nil then
                        c:setFailDeviceAndLocalStatus()
                        c:addError("No empty slots for cpssDxChIpRouterGlobalMacSaGet()")
                        return
                    end
                    result = myGenWrapper("cpssDxChIpRouterGlobalMacSaSet",
                                 {{ "IN", "GT_U8",        "devNum", devNum  },
                                  {"IN", "GT_U32", "routerMacSaIndex",
                                            c["eArch_glbl_index"][devNum] },
                                  { "IN", "GT_ETHERADDR", "mac", mac_address }})
                    c:handleCpssErrorDevice(result,
                                "RouterGlobalMacSaSet index "..tostring(c["eArch_glbl_index"][devNum]))
                end
            end,
            mac_address)

        command_data:addWarningIfNoSuccessDevices(
            "Can not set Router MAC SA Base address on all processed devices.")
    end

        -- Main port handling cycle
        if true == command_data["status"] then
            command_data:iterateOverPorts(
                function(c,devNum,portNum, lsb_router_mac_sa)
                    local devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum);
                    if devFamily == "CPSS_PP_FAMILY_CHEETAH_E" or
                        devFamily == "CPSS_PP_FAMILY_CHEETAH2_E"
                    then
                        return
                    end
                    local result
                    local mode = "CPSS_SA_LSB_PER_PORT_E"
                    if is_sip_5(devNum) then
                        mode = "CPSS_SA_LSB_FULL_48_BIT_GLOBAL"
                    end
                    result =
                        cpssPerPortParamSet("cpssDxChIpPortRouterMacSaLsbModeSet",
                                            devNum, portNum,
                                            mode, "saLsbMode",
                                            "CPSS_MAC_SA_LSB_MODE_ENT")
                    c:handleCpssErrorDevPort(result,
                                "setting mode for MAC SA least significant byte")
                    if result ~= 0 then
                        return
                    end
                    if not is_sip_5(devNum) then
                        result =
                            cpssPerPortParamSet("cpssDxChIpRouterPortMacSaLsbSet",
                                                devNum, portNum, lsb_router_mac_sa,
                                                "saMac", "GT_U8")
                        c:handleCpssErrorDevPort(result,
                                    "setting the 8 LSB bits of Router MAC SA")
                    elseif c["eArch_glbl_index"][devNum] ~= nil then
                        result =
                            cpssPerPortParamSet("cpssDxChIpRouterPortGlobalMacSaIndexSet",
                                                devNum, portNum, c["eArch_glbl_index"][devNum],
                                                "index", "GT_U32")
                        c:handleCpssErrorDevPort(result,
                                    "setting Router Global MAC SA index")

                    end
                end,
                lsb_router_mac_sa)

            command_data:addWarningIfNoSuccessPorts(
                "Can not set MAC SA least significant byte for all processed ports.")
        end

    -- set a TTI MacToMe entry for mac address
    if true == command_data["status"] then
        local mac2meValuePtr = {}
        local mac2meMaskPtr = {}
        local interfaceInfo = {}
        local macToMeEntryIndex

        macToMeEntryIndex = sharedResourceNamedMac2meMemoAlloc(
            mac2meEntryName, 1 --[[size--]], false --[[fromEnd--]]);

        if macToMeEntryIndex == nil then
            return command_data:addErrorAndPrint("macToMeEntryIndex allocation failed");
        end

        -- given mac and any VLAN
        mac2meValuePtr.mac = mac_address
        mac2meValuePtr.vlanId = 0
        mac2meMaskPtr.mac = convertMacAddrStringTo_GT_ETHERADDR("ff:ff:ff:ff:ff:ff")
        mac2meMaskPtr.vlanId  = 0;

        interfaceInfo.includeSrcInterface = "CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E"

        command_data:iterateOverDevices(
            function(c, devNum, mac_address)
                local devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum);
                if devFamily == "CPSS_PP_FAMILY_CHEETAH_E" or
                    devFamily == "CPSS_PP_FAMILY_CHEETAH2_E"
                then
                    return
                end

                local result, values
                result, values = myGenWrapper("cpssDxChTtiMacToMeSet", {
                    { "IN",     "GT_U8",                        "devNum",       devNum },
                    { "IN",     "GT_U32",                       "entryIndex",   macToMeEntryIndex },
                    { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "valuePtr",     mac2meValuePtr },
                    { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "maskPtr",      mac2meMaskPtr },
                    { "IN",     "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC", "interfaceInfoPtr", interfaceInfo }
                })
                command_data:handleCpssErrorDevPort(result, "cpssDxChTtiMacToMeSet")
            end,
            mac_address)

        command_data:addWarningIfNoSuccessDevices(
            "Can not set MacToMe address on all processed devices.")
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


-- ************************************************************************
---
--  no_mac_address_func
--        @description  removes a MAC address definition
--
--        @param params         - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function no_mac_address_func(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum, portNum
    local command_data = Command_Data()

    local mac_address = getGlobal("myMac")

    local mac2meValuePtr = {}
    local mac2meMaskPtr = {}
    local interfaceInfo = {}

    if mac_address ~= nil then
        command_data:initAllAvailableDevicesRange()
        command_data:initAllAvaiblesPortIterator()
        command_data:iterateOverDevices(
            function(c, devNum, mac_address)
                local i, result, values
                if not is_sip_5(devNum) then
                    result = myGenWrapper("cpssDxChIpRouterMacSaBaseSet", {
                                { "IN", "GT_U8",        "devNum", devNum  },
                                { "IN", "GT_ETHERADDR", "mac",    "00:00:00:00:00:00" }})
                    c:handleCpssErrorDevice(result,
                                "resetting of Router MAC SA Base address")
                else
                    -- Bobcat2 and above special case
                    -- scan cpssDxChIpRouterGlobalMacSa for empty slot
                    for i = 0, 255 do
                        result, values = myGenWrapper("cpssDxChIpRouterGlobalMacSaGet",{
                                              { "IN", "GT_U8",        "devNum", devNum  },
                                              { "IN", "GT_U32", "routerMacSaIndex", i},
                                              { "OUT", "GT_ETHERADDR", "mac" }})
                        c:handleCpssErrorDevice(result,
                                    "RouterGlobalMacSaGet scan for empty slot")
                        if result ~= 0 then
                            return
                        end
                        if string.upper(values.mac) == string.upper(mac_address.string) then
                            local devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum);
                            if devFamily ~= "CPSS_PP_FAMILY_CHEETAH_E" and
                                devFamily ~= "CPSS_PP_FAMILY_CHEETAH2_E"
                            then
                                result = myGenWrapper("cpssDxChIpRouterGlobalMacSaSet",{
                                                      { "IN", "GT_U8",        "devNum", devNum  },
                                                      { "IN", "GT_U32", "routerMacSaIndex", i},
                                                      { "IN", "GT_ETHERADDR", "mac", "00:00:00:00:00:00" }})
                                c:handleCpssErrorDevice(result, "Reset RouterGlobalMacSa")
                                break
                            end
                        end
                    end
                end

                -- unset a TTI MacToMe entry for mac address

                -- never match
                mac2meValuePtr.mac = 0
                mac2meValuePtr.vlanId = 0xFFF
                mac2meMaskPtr.mac = convertMacAddrStringTo_GT_ETHERADDR("ff:ff:ff:ff:ff:ff")
                mac2meMaskPtr.vlanId  = 0xFFF

                interfaceInfo.includeSrcInterface = "CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E"

                result, values = myGenWrapper("cpssDxChTtiMacToMeSet", {
                                    { "IN",     "GT_U8",                        "devNum",       devNum },
                                    { "IN",     "GT_U32",                       "entryIndex",   macToMeEntryIndex },
                                    { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "valuePtr",     mac2meValuePtr },
                                    { "IN",     "CPSS_DXCH_TTI_MAC_VLAN_STC",   "maskPtr",      mac2meMaskPtr },
                                    { "IN",     "CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC", "interfaceInfoPtr", interfaceInfo }
                })
                command_data:handleCpssErrorDevPort(result, "cpssDxChTtiMacToMeSet")

                sharedResourceNamedMac2meMemoFree(mac2meEntryName);

            end,
            mac_address)
    end

    -- Setting to default of system MAC address lau environment variable.
    if true == command_data["status"] then
        setGlobal("myMac", nil)
    end

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: mac address
--------------------------------------------------------------------------------
CLI_addCommand("config", "mac address", {
  func   = mac_address_func,
  help   = "Setting of a MAC address for the system",
  params = {
      { type = "values",
           "%mac-address"
      }
  }
})

--------------------------------------------------------------------------------
-- command registration: no mac address
--------------------------------------------------------------------------------
CLI_addCommand("config", "no mac address", {
  func   = function(params)
               params.flagNo    = true
               no_mac_address_func(params)
           end,
  help   = "Removing of a MAC address definition"
})
