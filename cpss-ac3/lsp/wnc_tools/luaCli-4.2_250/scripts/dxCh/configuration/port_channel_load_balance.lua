--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* port_channel_load_balance.lua
--*
--* DESCRIPTION:
--*       configuring the load-balancing policy of the port channeling"
--*
--* FILE REVISION NUMBER:
--*       $Revision: 5 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  port_channel_load_balance_func
--        @description  configures the load-balancing policy of the port
--                      channeling
--
--        @param params         - params["load_balance"]: load balance
--                                identifier;
--                                params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function port_channel_load_balance_func(params)
    -- Common variables declaration.
    local command_data = Command_Data()
    local hashUseMac
    local hashUseIp
    local hashPortMode
    local devNum

    -- Common variables initialization.
    command_data:initAllAvailableDevicesRange()

    -- Command specific variables initialization.
    local hashMode = params["load_balance"]

    if params["flagNo"] then
        -- reset to defaults
        hashUseMac   = false
        hashUseIp    = false
        hashPortMode = "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E"

    elseif (hashMode == "src_dst_mac") then
        -- hash based on MAC address
        hashUseMac   = true
        hashUseIp    = false
        hashPortMode = "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E"

    elseif (hashMode == "src_dst_ip") then
        -- hash based on IP address only
        hashUseMac   = false
        hashUseIp    = true
        hashPortMode = "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E"

    elseif (hashMode == "src_dst_mac_ip") then
        -- hash based on MAC and IP address
        hashUseMac   = true
        hashUseIp    = true
        hashPortMode = "CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E"

    elseif (hashMode == "src_dst_mac_ip_port") then
        -- hash based on MAC and IP address and TCP/UDP port
        hashUseMac   = true
        hashUseIp    = true
        hashPortMode = "CPSS_DXCH_TRUNK_L4_LBH_LONG_E"
    else
        Command_Data:setFailStatus()
        command_data:addError("trying to set unknown trunk hash mode: %s",
                              hashMode)
    end

    -- Main device handling cycle
    if true == command_data["status"] then
        for _, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()
            command_data:clearLocalStatus()

            -- hash mode is packet based in any case
            genericCpssApiWithErrorHandler(command_data,
                "cpssDxChTrunkHashGlobalModeSet", {
                    {"IN", "GT_U8",  "devNum", devNum},
                    {"IN", "CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT",  "hashMode", "CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E"}}
            )

            if command_data["local_status"]   then
                genericCpssApiWithErrorHandler(command_data,
                    "cpssDxChTrunkHashIpAddMacModeSet", {
                        {"IN", "GT_U8",  "devNum", devNum},
                        {"IN", "GT_BOOL",  "enable", hashUseMac}}
                )
                genericCpssApiWithErrorHandler(command_data,
                    "cpssDxChTrunkHashIpModeSet", {
                        {"IN", "GT_U8",  "devNum", devNum},
                        {"IN", "GT_BOOL",  "enable", hashUseIp}}
                )
                genericCpssApiWithErrorHandler(command_data,
                    "cpssDxChTrunkHashL4ModeSet", {
                        {"IN", "GT_U8",  "devNum", devNum},
                        {"IN", "CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT",  "hashMode", hashPortMode}}
                )
            end

            command_data:updateStatus()
            command_data:updateDevices()
        end

        command_data:addWarningIfNoSuccessDevices(
            "Can not configure load-balancing policy of the port channeling " ..
            "of all processed devices.")
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: port-channel load-balance
--------------------------------------------------------------------------------
CLI_addHelp("config",    "port-channel",
                                    "Configure port-channel")
CLI_addCommand("config", "port-channel load-balance", {
  func   = port_channel_load_balance_func,
  help   = "Configuring the load-balancing policy of the port channeling",
  params = {
      { type = "named",
          { format = "src-dst-mac",             name = "src_dst_mac",
                help = "Port channel load balancing is based on the source " ..
                       "and destination MAC address"                        },
          { format = "src-dst-ip",              name = "src_dst_ip",
                help = "Port channel load balancing is based on the source " ..
                       "and destination IP address"                         },
          { format = "src-dst-mac-ip",          name = "src_dst_mac_ip",
                help = "Port channel load balancing is based on the source " ..
                       "and destination of MAC and IP addresses"            },
          { format = "src-dst-mac-ip-port",     name = "src_dst_mac_ip_port",
                help = "Port channel load balancing is based on source and " ..
                       "destination of MAC and IP addresses and on source " ..
                       "and destination TCP/UDP port numbers"               },
            alt =   { load_balance  = { "src_dst_mac",
                                        "src_dst_ip",
                                        "src_dst_mac_ip",
                                        "src_dst_mac_ip_port"               }},
        mandatory = { "load_balance" }
    }
  }
})

--------------------------------------------------------------------------------
-- command registration: no port-channel load-balance
--------------------------------------------------------------------------------
CLI_addHelp("config",    "no port-channel", "Reset port-channel configuration to defaults")
CLI_addCommand("config", "no port-channel load-balance", {
  func   = function(params)
               params.flagNo = true
               return port_channel_load_balance_func(params)
           end,
  help   = "Returning to default of the load-balancing policy of the port channeling"
})
