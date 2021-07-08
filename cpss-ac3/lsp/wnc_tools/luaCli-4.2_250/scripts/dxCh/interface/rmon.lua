--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* rmon.lua
--*
--* DESCRIPTION:
--*       determine what kind of traffic the histogram MAC counters will count
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

--types defined:eventType

CLI_type_dict["countModeType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "disable | rx | tx | rx-tx",
    enum = {
        ["disable"] = { value=0, help="histogram counters disabled" },
        ["rx"]      = { value=1, help="histogram counters count only RX traffic" },
        ["tx"]      = { value=2, help="histogram counters count only TX traffic" },
        ["rx-tx"]   = { value=3, help="histogram counters count both RX and TX traffic – HW default option" }
    }
}

--rmon
local function rmon_func(params)
    local result, values
    local devNum, portNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local modeTx, modeRx

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()

    -- Parse parameters
    modeTx = false
    modeRx = false
    if 0 ~= bit_and(params["mode"], 0x01) then
      modeRx = true
    end
    if 0 ~= bit_and(params["mode"], 0x02) then
      modeTx = true
    end
    
    -- Main port handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
    
            -- Port enabling.
            command_data:clearLocalStatus()

            if true == command_data["local_status"]  then
                result, values = 
                    cpssPerPortParamSet("cpssDxChPortMacCountersRxHistogramEnable",
                                        devNum, portNum,
                                        modeRx, "enable", "GT_BOOL")
                
                if     0x10 == result then
                    command_data:setFailPortStatus()   
                    command_data:addWarning("It is not allowed to go to " ..
                                            "updating of the RMON Etherstat Rx histogram " ..
                                            "for port %d of device %d.", 
                                            portNum, devNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at going to " ..
                                          "updating of the RMON Etherstat Rx histogram " ..
                                          "port %d device %d: %s.", portNum, devNum,
                                          returnCodes[result])
                end
            end

            if true == command_data["local_status"]  then
                result, values = 
                    cpssPerPortParamSet("cpssDxChPortMacCountersTxHistogramEnable",
                                        devNum, portNum,
                                        modeTx, "enable", "GT_BOOL")

                if     0x10 == result then
                    command_data:setFailPortStatus()   
                    command_data:addWarning("It is not allowed to go to " ..
                                            "updating of the RMON Etherstat Rx histogram " ..
                                            "for port %d of device %d.", 
                                            portNum, devNum)
                elseif 0 ~= result then
                    command_data:setFailPortAndLocalStatus()
                    command_data:addError("Error at going to " ..
                                          "updating of the RMON Etherstat Rx histogram " ..
                                          "port %d device %d: %s.", portNum, devNum,
                                          returnCodes[result])
                end
            end
            

            command_data:updateStatus()

            command_data:updatePorts()
        end
        command_data:addWarningIfNoSuccessPorts(
            "Can not going to down all processed ports.")
    end
    
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
    
end


--------------------------------------------------------------------------------
-- command registration: rmon
--------------------------------------------------------------------------------
CLI_addHelp("interface",    "rmon",  "Determine what kind of traffic the histogram mac counters will count")
CLI_addCommand("interface", "rmon", {
    func   = rmon_func,
    help   = "Determine what kind of traffic the histogram mac counters will count",
    params = {
      { type="named",
        { format="histogram %countModeType", name="mode", help="histogram [disable|rx|tx|rx-tx]\n" } ,
        mandatory={"mode"}
      }
    }
})
