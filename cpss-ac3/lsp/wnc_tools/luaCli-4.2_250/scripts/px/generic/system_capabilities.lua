--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* system_capabilities.lua
--*
--* DESCRIPTION:
--*       common system, device and port functions
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlCpssIsAsicSimulation")

local isSimulation = wrlCpssIsAsicSimulation()

-- get indication if simulation used
function isSimulationUsed()
    return isSimulation
end

-- 
-- ************************************************************************
---
--  does_port_exist
--        @description  checking of port existance
--
--        @param devID          - device ID
--        @param port           - port number
--
--        @return       true if exists, otherwise false
--
function does_port_exist(devID, port)
    if port == "CPU" then return true end
    if type(port) ~= "number" then return false end

    local result, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devID)
    if(result ~= 0) then maxPort = 255 end

    maxPort = maxPort + 1
    if port < maxPort then
      result, values = myGenWrapper("cpssPxPortPhysicalPortMapIsValidGet",
                                    {{ "IN", "GT_U8" , "dev", devID}, -- devNum
                                     { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", port},
                                     { "OUT","GT_BOOL" , "isValid"}} )
      if result ~= 0 then
        print("Error while calling cpssPxPortPhysicalPortMapIsValidGet. Return code:"..to_string(result))
        return false
      end
      return values["isValid"]
    end

    local c, ret
    local result, devInfo = cpssPerDeviceParamGet("cpssPxCfgDevInfoGet",
                                                  devID, "devInfo",
                                                  "CPSS_PX_CFG_DEV_INFO_STC")
    if  0 == result then
        -- if port is 0..31
        if port < 16 then
            c = bit_and(devInfo["devInfo"]["genDevInfo"]["existingPorts"]["ports"][0],
                        bit_shl(1, port))
        else
            return false
        end

        if 0 == c    then
            ret = false
        else
            ret = true
        end
        return ret
    else
        return false
    end
end

-- ************************************************************************
---
--  system_capability_get_table_size
--        @description  get's sytem capabilities for given device
--
--        @param devId              - device number
--        @param name               - parameter name (see entry_map table)
--
--        @return       GT_OK and requested value, if success otherwise error
--                      code
--
function system_capability_get_table_size(devId, name)
    local entry_map = {
        BMA_PORT_MAPPING = "CPSS_PX_TABLE_BMA_PORT_MAPPING_E",
        BMA_MULTICAST_COUNTERS = "CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E",
        TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG = "CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E",
        TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG = "CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E",
        TXQ_DQ_PFC_RESPONSE_PORT_MAPPING = "CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E",
        TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS = "CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E",
        TXQ_QUEUE_TAIL_DROP_LIMITS_DP0 = "CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E",
        TXQ_QUEUE_TAIL_DROP_LIMITS_DP1 = "CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP1_E",
        TXQ_QUEUE_TAIL_DROP_LIMITS_DP2 = "CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP2_E",
        TXQ_QUEUE_TAIL_DROP_BUF_LIMITS = "CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_BUF_LIMITS_E",
        TXQ_QUEUE_TAIL_DROP_DESC_LIMITS = "CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_DESC_LIMITS_E",
        TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF = "CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E",
        TXQ_QCN_CN_SAMPLE_INTERVALS = "CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E",
        PFC_LLFC_COUNTERS = "CPSS_PX_TABLE_PFC_LLFC_COUNTERS_E",
        CNC_0_COUNTERS = "CPSS_PX_TABLE_CNC_0_COUNTERS_E",
        CNC_1_COUNTERS = "CPSS_PX_TABLE_CNC_1_COUNTERS_E",
        PCP_DST_PORT_MAP_TABLE = "CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E",
        PCP_SRC_PORT_MAP_TABLE = "CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E",
        PHA_HA_TABLE = "CPSS_PX_TABLE_PHA_HA_TABLE_E",
        PHA_SRC_PORT_DATA = "CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E",
        PHA_TARGET_PORT_DATA = "CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E"
    }

    if entry_map[name] == nil then
        return nil
    end

    local result, values = myGenWrapper(
                    "cpssPxCfgTableNumEntriesGet", {
                        { "IN", "GT_SW_DEV_NUM" , "dev", devId}, -- devNum
                        { "IN", "CPSS_PX_TABLE_ENT",  "table", entry_map[name]},
                        { "OUT","GT_U32", "numEntries"}
                    })
                    
    --print(string.format("%s %s", "system_capability_managment debug-1 "..name, tostring(values.numEntries)))
    if result == 0 then
        return values.numEntries
    end

    return nil
end