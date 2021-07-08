--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* storm_control.lua
--*
--* DESCRIPTION:
--*       set Ingress Port Storm Rate Limit Enforcement
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants

-- 
-- ************************************************************************
---
--  stormControl
--        @description  stormControl - Set Ingress Port Storm Rate Limit Enforcement
--
--        @param params         - params["flagNo"]: no command property
--
--        @return       true on success, otherwise false and error message
--
local function stormControlEnable(command_data, devNum, portNum, params)
    
    local command_data = params.command_data
    local did_error, result, values
    local portGfgPtr, brgRateLimitPtr

    local enable = true  

    if (params.flagNo == true) then
        enable = false
    end
    ----- WRAPPER for cpssDxChBrgGenPortRateLimitGet 
    did_error, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitGet", {
                     {"IN", "GT_U8", "devNum", devNum},
                     {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                     {"OUT", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC", "portGfgPtr"}})
    
    portGfgPtr = values["portGfgPtr"] 
    if enable == true then      
        if params.broadcast then
                portGfgPtr.enableBc = true
        end
        if params.known then
                portGfgPtr.enableUcKnown = true
        end
        if params.unknown then
                portGfgPtr.enableUcUnk = true
        end
        if params.unregistered then
                portGfgPtr.enableMc = true
        end
        if params.registered then
                portGfgPtr.enableMcReg = true
        end
        if params.tcp_syn then  ----- WRAPPER for cpssDxChBrgGenPortRateLimitTcpSynSet 

                genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitTcpSynSet", {  
                         {"IN", "GT_U8", "devNum", devNum},
                         {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                         {"IN", "GT_BOOL", "enable", GT_TRUE}})
        end 

    ----- WRAPPER for cpssDxChBrgGenRateLimitGlobalCfgGet
        did_error, result, values = genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenRateLimitGlobalCfgGet", {  
                         {"IN", "GT_U8", "devNum", devNum},
                         {"OUT", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC", "brgRateLimitPtr"}})         
        
        brgRateLimitPtr = values["brgRateLimitPtr"]                 
        local windowSize = 1024;
        brgRateLimitPtr.rMode       = "CPSS_RATE_LIMIT_BYTE_BASED_E"
        brgRateLimitPtr.win10Mbps   = windowSize
        brgRateLimitPtr.win100Mbps  = windowSize
        brgRateLimitPtr.win1000Mbps = windowSize
        brgRateLimitPtr.win10Gbps   = windowSize

        genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenRateLimitGlobalCfgSet", 
        {  
                     {"IN", "GT_U8", "devNum", devNum},
                     {"IN", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC", "brgRateLimitPtr", brgRateLimitPtr}})
  --      if params.level then
          
   --     end
        if params.kbps then
            local rateGranularityInBytes, maxRateValue
            if is_sip_5_10(devNum) then
                rateGranularityInBytes = 1
                maxRateValue = 4194303
            else
                rateGranularityInBytes = 64
                maxRateValue = 65535
            end

            local tempLimit = (params.kbps * (windowSize/1000000) * 1000) / (rateGranularityInBytes * 8) -- we can reduce equsion to [params.kbps / (64 * 8)]
            if tempLimit <= maxRateValue then 
                portGfgPtr.rateLimit = tempLimit
            else
                command_data:handleCpssErrorDevPort(
                result, string.format("Rate limit out of range [%s] for kbps = %s ", tempLimit, params.kbps))
            end    
        end

    else  --- flag no  
        portGfgPtr.enableBc = false
        portGfgPtr.enableUcKnown = false
        portGfgPtr.enableUcUnk = false
        portGfgPtr.enableMc = false
        portGfgPtr.enableMcReg = false
        
        genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitTcpSynSet", {  
                         {"IN", "GT_U8", "devNum", devNum},
                         {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                         {"IN", "GT_BOOL", "enable", GT_FALSE}})
    end  --- end for enable check     

    genericCpssApiWithErrorHandler(command_data, "cpssDxChBrgGenPortRateLimitSet", 
        {  
                     {"IN", "GT_U8", "devNum", devNum},
                     {"IN", "GT_PHYSICAL_PORT_NUM", "port", portNum},
                     {"IN", "CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC", "portGfgPtr", portGfgPtr}})

   


   --command_data:initInterfaceDevPortRange()
   --command_data:iterateOverPorts(func, params)
   command_data:analyzeCommandExecution()
   command_data:printCommandExecutionResults()
   return command_data:getCommandExecutionResults()

   end -- func
--
--  stormControlInvertEnableFunc
--        @description  Set Ingress Port Storm Rate Limit Enforcement
--
--  @param params   - parameters
--     - params["kbps"]         - setup maximum of kilobits per second of traffic on a port
--     - params["broadcast"]    - To enable rate limiting on a physical port for Broadcast packets
--     - params["known"]        - To enable rate limiting on a physical port for Known Unicast packets
--     - params["unknown"]      - To enable rate limiting on a physical port for Unknown Unicast packets
--     - params["unregistered"] - To enable rate limiting on a physical port for Unregistered Multicast packets
--     - params["registered"]   - To enable rate limiting on a physical port for Registered Multicast packets
--     - params["tcp-syn"]      - To enable rate limiting on a physical port for TCP SYN packets
--     
--
--
--        @return       true on success, otherwise false and error message
--
local function stormControlEnableFunc(params)
    params.command_data = Command_Data()
    return generic_port_range_func(stormControlEnable, params)
end

   

--------------------------------------------------------------------------------
-- command registration: storm-control
--------------------------------------------------------------------------------
CLI_addHelp("interface", "storm-control", "Set Ingress Port Storm Rate Limit Enforcement")
CLI_addCommand("interface", "storm-control", {
    func   = stormControlEnableFunc,
    help   = "Enabled Ingress Port Storm Rate Limit Enforcement for current port",
    params = {      
        { type="named",
           -- { format = "level %percent", help = "Suppression level in percentage" },
            { format = "kbps %GT_U32", help = "Maximum of kilobits per second of traffic on a port" },
            { format = "broadcast", help = "To enable rate limiting on a physical port for Broadcast packets"},
            { format = "known", help = "To enable rate limiting on a physical port for Known Unicast packets"},
            { format = "unknown", help = "To enable rate limiting on a physical port for Unknown Unicast packets"},
            { format = "unregistered", help = "To enable rate limiting on a physical port for Unregistered Multicast packets"},
            { format = "registered", help = "To enable rate limiting on a physical port for Registered Multicast packets"},
            { format = "tcp-syn", help = "To enable rate limiting on a physical port for TCP SYN packets"},

            --alt = { limit = {"level", "kbps"}},
            
            mandatory = {"kbps"} 
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: no storm-control
--------------------------------------------------------------------------------
CLI_addHelp("interface", "no storm-control", "Disable Ingress Port Storm Rate Limit Enforcement")
CLI_addCommand("interface", "no storm-control", {
    func   = function(params)
    params.flagNo = true
    return stormControlEnableFunc(params) --need add storm enable
    end,
    help   = "Disable Ingress Port Storm Rate Limit Enforcement for current Port",
})
