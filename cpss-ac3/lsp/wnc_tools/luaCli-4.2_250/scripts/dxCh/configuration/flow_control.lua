--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* flow_control.lua
--*
--* DESCRIPTION:
--*       configuration of the flow control of a given interface running at
--*       full-duplex
--*
--* FILE REVISION NUMBER:
--*       $Revision: 9 $
--*
--[[
---+++ CLI EXECUTION TRACE START +++---
show flow-control device ${dev}
---+++ CLI EXECUTION TRACE END +++---
]]--

--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlDxChPortFlowControlConfiguringConditionChecking")

--constants
local default_flow_control_negotiation_enable = false
local default_flow_control_state              = "CPSS_PORT_FLOW_CONTROL_DISABLE_E"


-- ************************************************************************
---
--  flow_control_func
--        @description  sets port flow control if port in full-duplex mode
--
--        @param params         - params["flow_state"]: flow state
--                                string: auto/on/off/rx/tx
--
--        @usage __global       - __global["ifRange"]: iterface range
--
--        @return       true on success, otherwise false and error message
--
local function flow_control_func(params)
    -- Common variables declaration
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration
    local full_duplex, port_link
    local auto_negotiation_enable
    local flow_control_configuring_condition
    local flow_control_configuring_warning_string
    local flow_control
    local maxPort



    -- Command specific variables initialization
    if nil == params.flagNo then
        local all_ports, dev_ports, devNum
        all_ports = getGlobal("ifRange")--get table of ports
        for devNum, dev_ports in pairs(all_ports) do
            result, values = myGenWrapper("cpssDxChPortFcHolSysModeSet",--set FC mode
                                    {{ "IN", "GT_U8",      "devNum",   devNum  },
                                    { "IN", "CPSS_DXCH_PORT_HOL_FC_ENT",      "modeFcHol",  "CPSS_DXCH_PORT_FC_E" }})
            if  0 ~= result then
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error at set device mode " ..
                                      "to FC: device %d: %s",
                                      devNum, returnCodes[result])
            end
        end


        if "auto" == params["flow_state"] then
            auto_negotiation_enable = true
            flow_control            = "CPSS_PORT_FLOW_CONTROL_RX_TX_E"
        else
            auto_negotiation_enable = false
            if     "on"  == params["flow_state"] then
                flow_control = "CPSS_PORT_FLOW_CONTROL_RX_TX_E"
            elseif "off" == params["flow_state"] then
                flow_control = "CPSS_PORT_FLOW_CONTROL_DISABLE_E"
            elseif "rx"  == params["flow_state"] then
                flow_control = "CPSS_PORT_FLOW_CONTROL_RX_ONLY_E"
            elseif "tx"  == params["flow_state"] then
                flow_control = "CPSS_PORT_FLOW_CONTROL_TX_ONLY_E"
            end
        end
    else
        auto_negotiation_enable = default_flow_control_negotiation_enable
        flow_control            = default_flow_control_state
    end

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in
                                    command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()

            command_data:clearLocalStatus()
            -- get link status of port
            port_link = false
            if true == command_data["local_status"] then
                result, values = cpssPerPortParamGet("cpssDxChPortLinkStatusGet",
                                                     devNum, portNum, "link",
                                                     "GT_BOOL")
                if        0 == result then
                    port_link = values["link"]
                elseif 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("Port link getting is not allowed " ..
                                            "on device %d port %d.", devNum,
                                            portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port link getting on " ..
                                          "device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end
            end

            if true == port_link then
                -- Port duplex mode getting.
                if true == command_data["local_status"] then
                    result, values =
                        cpssPerPortParamGet("cpssDxChPortDuplexModeGet", devNum,
                                            portNum, "mode", "CPSS_PORT_DUPLEX_ENT")
                    if     0 == result then
                        full_duplex = values["mode"]
                    elseif 0x10 == result then
                        command_data:setFailPortStatus()
                            command_data:addWarning("It is not allowed to get " ..
                                                    "port duplex mode %d " ..
                                                    "on device %d port %d.",
                                                    full_duplex, devNum, portNum)
                    elseif 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at port duplex mode " ..
                                              "getting of device %d port %d: %s",
                                              devNum, portNum, returnCodes[result])
                    end
                end
            else
                -- assume that all link down as FULL DUPLEX
                full_duplex = "CPSS_PORT_FULL_DUPLEX_E"
            end

            -- Port full duplex mode checking.
            if (true == command_data["local_status"])       and
               ("CPSS_PORT_FULL_DUPLEX_E" ~= full_duplex)   then
                    command_data:setFailPortStatus()
                command_data:addWarning("Port %d of device %d is not in " ..
                                        "full-duplex mode", portNum, devNum)
            end

            -- Flow control enabling condition checking.
            if (true == command_data["local_status"])     and
               ("CPSS_PORT_FULL_DUPLEX_E" == full_duplex) then
                result, values, flow_control_configuring_warning_string =
                    wrLogWrapper("wrlDxChPortFlowControlConfiguringConditionChecking", 
                                 "(devNum, portNum, flow_control)", devNum, portNum, flow_control)
                if        0 == result                   then
                    flow_control_configuring_condition   = values
                elseif 0x10 == result                   then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addWarning("It is not allowed to check " ..
                                            "flow control enabling " ..
                                            "condition on device %d port %d.",
                                            devNum, portNum)
                elseif    0 ~= result                   then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at checking of flow control" ..
                                          "enabling on device %d port %d: %s.",
                                          trunkId, devNum, portNum)
                end
            end

            -- Port flow-control autonegatiation mode setting.
            if (true == command_data["local_status"])       and
               ("CPSS_PORT_FULL_DUPLEX_E" == full_duplex)   and
               (true == flow_control_configuring_condition) then
                result, values =
                    myGenWrapper("cpssDxChPortFlowCntrlAutoNegEnableSet",
                                 {{ "IN", "GT_U8",      "devNum",   devNum  },
                                  { "IN", "GT_PHYSICAL_PORT_NUM",      "portNum",  portNum },
                                  { "IN", "GT_BOOL",    "state",
                                                    auto_negotiation_enable },
                                  { "IN", "GT_BOOL",    "pauseAdvertise",
                                                                      false }})
                if 0x10 == result then
                    command_data:setFailPortStatus()
                    command_data:addWarning("It is not allowed to set " ..
                                            "flow-contol autonegatiation " ..
                                            "on device %d port %d.", devNum,
                                            portNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at port flow-contol " ..
                                          "autonegatiation mode setting " ..
                                          "of device %d port %d: %s", devNum,
                                          portNum, returnCodes[result])
                end
            end

            -- Port flow-control setting.
            if true == command_data["local_status"]         and
               ("CPSS_PORT_FULL_DUPLEX_E" == full_duplex)   and
               (true == flow_control_configuring_condition) then
                    result, values =
                        myGenWrapper("cpssDxChPortFlowControlEnableSet",
                                     {{ "IN", "GT_U8",  "devNum",  devNum       },
                                      { "IN", "GT_PHYSICAL_PORT_NUM",
                                                        "portNum", portNum      },
                                      { "IN", "CPSS_PORT_FLOW_CONTROL_ENT",
                                                        "state",   flow_control }})
                    if 0x10 == result then
                        command_data:setFailPortStatus()
                        command_data:addWarning("It is not allowed to set port " ..
                                            "flow-contol %d on device %d " ..
                                                "port %d.", flow_control, devNum,
                                                portNum)
                    elseif 0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at port flow-contol mode " ..
                                          "setting on device %d port %d: %s",
                                              devNum, portNum,
                                              returnCodes[result])
                    end
                end

            if (true == command_data["local_status"])       and
               ("CPSS_PORT_FULL_DUPLEX_E" == full_duplex)   and
               (false == flow_control_configuring_condition)   then
                command_data:setFailPortStatus()
                command_data:addWarning("It is not allowed to configure " ..
                                        "flow control on device %d port %d: %s.",
                                        devNum, portNum,
                                        flow_control_configuring_warning_string)
            end

            command_data:updateStatus()
            command_data:updatePorts()
        end

        command_data:addWarningIfNoSuccessPorts("Can not set flow-control " ..
                                                "for all processed ports.")
    end


    --check if there is no ports in flow control mode and if don't set mode to CPSS_DXCH_PORT_HOL_FC_ENT
        local all_ports = getGlobal("ifRange")--get table of ports

        for devNum, dev_ports in pairs(all_ports) do
            --getting max port for device
            result, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
            if 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at getting device max " ..
                                          "port: device %d: %s",
                                          devNum, returnCodes[result])
                    maxPort=-1
            end

            local noFC=true
            for portNum = 0,maxPort do
                --[[ skip not existed ports ]]
                if portNum~= 63 and does_port_exist(devNum, portNum) then --not CPU port
                    result, values = myGenWrapper("cpssDxChPortFlowControlEnableGet",--set FC mode
                                    {{ "IN", "GT_U8",      "devNum",   devNum  },
                                    { "IN", "GT_PHYSICAL_PORT_NUM",      "portNum",  portNum },
                                    { "OUT", "CPSS_PORT_FLOW_CONTROL_ENT",      "statePtr"}})
                    if 0~=result then
                        noFC=false
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at get FC " ..
                                              "status: device %d port: %d%s: ",
                                              devNum, portNum,returnCodes[result])
                        break
                    elseif "CPSS_PORT_FLOW_CONTROL_DISABLE_E"~=values["statePtr"] then
                        noFC=false
                        break


                    end
                end
            end

            if noFC then
                local devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)
                result, values = myGenWrapper("cpssDxChPortFcHolSysModeSet",--set FC mode
                                {{ "IN", "GT_U8",      "devNum",   devNum  },
                                { "IN", "CPSS_DXCH_PORT_HOL_FC_ENT",      "modeFcHol",  "CPSS_DXCH_PORT_HOL_E" }})
                if  0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at set device mode " ..
                                          "to FC: device %d: %s",
                                          devNum, returnCodes[result])
                end

                -- xCat and xCat3 device has enabled sharing of descriptors by default.
                -- this configuration disturbs to features like shaper and scheduler for flooded/MC traffic.
                -- Disable the sharing to make tests working as expected. 
                if ("CPSS_PP_FAMILY_DXCH_XCAT_E"  == devFamily) or 
                   ("CPSS_PP_FAMILY_DXCH_XCAT3_E"  == devFamily) or 
                   ("CPSS_PP_FAMILY_DXCH_XCAT2_E"  == devFamily) then
                    result, values = myGenWrapper("cpssDxChPortTxSharingGlobalResourceEnableSet",
                                    {{ "IN", "GT_U8",      "devNum",   devNum  },
                                     { "IN", "GT_BOOL",      "enable",  false }})
                    if  0 ~= result then
                        command_data:setFailPortAndLocalStatus()
                        command_data:addError("Error at disable sharing " ..
                                              ": device %d: %s",
                                              devNum, returnCodes[result])
                    end
                end
            end
        end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

--------------------------------------------
-- command registration: flow-control
--------------------------------------------
CLI_addCommand("interface", "flow-control", {
  func=flow_control_func,
  help="Configure flow-control mode",
  params={
    { type="named",
      { format = "auto",  help = "Enable AUTO flow-control configuration"   },
      { format = "on",    help = "Force flow-control as enabled"            },
      { format = "off",   help = "Force flow-control as disabled"           },
      { format = "rx",    help = "Enable only Rx flow control"              },
      { format = "tx",    help = "Enable only Tx flow control"              },
      alt      = { flow_state   = { "auto", "on", "off", "rx", "tx" }       },
      mandatory = { "flow_state" }
    }
  }
})
CLI_addCommand("interface", "no flow-control", {
  func=function(params)
      params.flagNo=true
      return flow_control_func(params)
  end,
  help="Configure flow-control mode"
})
