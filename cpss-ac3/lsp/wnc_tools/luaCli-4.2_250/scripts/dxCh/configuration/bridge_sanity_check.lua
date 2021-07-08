--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* bridge_sanity_check.lua
--*
--* DESCRIPTION:
--*       set packet sanity checks
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  bridge_sanity_check
--        @description  Set packet sanity checks
--
--        @param params         - The parameters
--
--        @return       true if there was no error otherwise false
--
local function bridge_sanity_check(params)
    local command_data = Command_Data()
    local iterator
    local result, values

    local enable

    local CPSS_NST_CHECK_TCP_ALL_E = 7 + 1
    local CPSS_NST_CHECK_SIP_IS_DIP_E = 10 + 1

    startAPILog("Start of <bridge_sanity_check> (" .. to_string(params) .. ")")

    if (params.flagNo == nil) then
      enable = true
    else
      enable = false
    end

    params.all = true
    if params.devID == "all" then
        params.devID = nil
    end

    if  params.sanity_check == "all" then -- ALL
        params.sanity_check = {}
        for i = 0,13 do
            table.insert(params.sanity_check, i)
        end
    else
        params.sanity_check = {params.sanity_check}
    end

    command_data:initAllAvailableDevicesRange()
    command_data:clearLocalStatus()

    startLoopLog("for devNum")

    local lastdev = nil
    for iterator, devNum in command_data:getDevicesIterator() do
         command_data:clearDeviceStatus()
         
         command_data:clearLocalStatus()
         
         if devNum ~= lastdev then
             startLoopLog("for sanity_check")
             for i,sanity_check in pairs(params.sanity_check) do
                 if i ~= CPSS_NST_CHECK_TCP_ALL_E then -- skip tcp-all to avoid duplication
                     if ( i < CPSS_NST_CHECK_SIP_IS_DIP_E ) or -- 'sip-is-dip' and above supported in SIP5 only
                        ( i >= CPSS_NST_CHECK_SIP_IS_DIP_E and is_sip_5(devNum)) then 
                        
                         result, values = myGenWrapper("cpssDxChNstProtSanityCheckSet",{
                             {"IN","GT_U8","devNum",devNum},
                             {"IN","CPSS_NST_CHECK_ENT","checkType",sanity_check},
                             {"IN","GT_BOOL","enable",enable}
                           })
                         command_data:handleCpssErrorDevice(result, "cpssDxChNstProtSanityCheckSet()")

                         if     0x10 == result then
                             command_data:setFailDeviceStatus() 
                             command_data:addWarning("Sanity Check %d is not allowed on " ..
                                                     "device %d.", 
                                                     sanity_check, 
                                                     devNum)                     
                         elseif 0x1E == result then
                             command_data:setFailDeviceStatus() 
                             command_data:addWarning("Sanity Check %d is not supported on " ..
                                                     "device %d.", 
                                                     sanity_check, 
                                                     devNum)                                             
                         elseif    0 ~= result then
                             command_data:setFailDeviceAndLocalStatus()
                             command_data:addError("Error at setting sanity check %s on device %d: %s.", 
                                                   sanity_check,
                                                   devNum, returnCodes[result])
                         end
                     end -- end of "if ( i < CPSS_NST_CHECK_SIP_IS_DIP_E ) or "
                 end -- end of "if i < CPSS_NST_CHECK_SIP_IS_DIP_E"
             end -- end of "for i,sanity_check" loop
             stopLoopLog()
        end -- end of "if devNum ~= lastdev"
        lastdev = devNum

        command_data:updateStatus()
        
        command_data:updateDevices()
    end
    stopLoopLog()

    leaveAPILog("end of <bridge_sanity_check>  ")

    command_data:analyzeCommandExecution()
    
    command_data:printCommandExecutionResults()
    
    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- type registration: sanity_check
--------------------------------------------
CLI_type_dict["sanity_check"] = {
    checker     = CLI_check_param_enum,
    complete    = CLI_complete_param_enum,
    help        = "",
    enum = {
        ["tcp-syn-data"] = { value=0, -- "CPSS_NST_CHECK_TCP_SYN_DATA_E"
            help="TCP SYN packets with data check" },
        ["tcp-over-mac-mc-bc"] = { value=1, -- "CPSS_NST_CHECK_TCP_OVER_MAC_MC_BC_E"
            help="TCP over MC/BC packets check" },
        ["tcp-flag-zero"] = { value=2, -- "CPSS_NST_CHECK_TCP_FLAG_ZERO_E"
            help="TCP packets with all flags zero check" },
        ["tcp-flags-fin-urg-psh"] = { value=3, -- "CPSS_NST_CHECK_TCP_FLAGS_FIN_URG_PSH_E"
            help="TCP packets with the TCP FIN, URG, and PSH flags check" },
        ["tcp-flags-syn-fin"] = { value=4, -- "CPSS_NST_CHECK_TCP_FLAGS_SYN_FIN_E"
            help="TCP packet with the TCP SYN and FIN flags" },
        ["tcp-flags-syn-rst"] = { value=5, -- "CPSS_NST_CHECK_TCP_FLAGS_SYN_RST_E"
            help="TCP packet with the TCP SYN and RST flags" },
        ["tcp-udp-port-zero"] = { value=6, -- "CPSS_NST_CHECK_TCP_UDP_PORT_ZERO_E"
            help="TCP/UDP packet with a zero source or destination port" },
        ["tcp-all"] = { value=7, -- "CPSS_NST_CHECK_TCP_ALL_E"
            help="All kind of TCP checks. frag-ipv4-icmp, arp-mac-sa-mismatch and sip-is-dip are not included" },
        ["frag-ipv4-icmp"] = { value=8, -- "CPSS_NST_CHECK_FRAG_IPV4_ICMP_E"
            help="IPv4 fragmented packet check" },
        ["arp-mac-sa-mismatch"] = { value=9, -- "CPSS_NST_CHECK_ARP_MAC_SA_MISMATCH_E"
            help="ARP MAC SA Mismatch check" },
        ["sip-is-dip"] = { value=10, -- "CPSS_NST_CHECK_SIP_IS_DIP_E"
            help="IPv4/IPv6 packets with SIP address equal to DIP address" },
        ["tcp-without-full-header"] = { value=11, -- "CPSS_NST_CHECK_TCP_WITHOUT_FULL_HEADER_E"
            help="TCP first fragment (or non-fragmented) packets without full TCP header check" },
        ["tcp-fin-without-ack"] = { value=12, -- "CPSS_NST_CHECK_TCP_FIN_WITHOUT_ACK_E"
            help="TCP packets with the TCP FIN flag set and the TCP ACK not set" },
        ["tcp-udp-sport-is-dport"] = { value=13, -- "CPSS_NST_CHECK_TCP_UDP_SPORT_IS_DPORT_E"
            help="TCP/UDP packets with source TCP/UDP port equal to destination TCP/UDP port" },
        ["all"] = { value="all", -- "CPSS_NST_CHECK_TCP_ALL_E"
            help="All kinds of packet sanity checks" },
    }
}

--------------------------------------------
-- command registration: bridge sanity-check
--------------------------------------------
CLI_addCommand("config", "bridge sanity-check", {
    func=bridge_sanity_check,
    help="Set packet sanity checks of the device",
    params={
        {   type="values", "%sanity_check" },
        {   type="named",
            { format="device %devID_all", name="devID", help = "Device ID"},
            mandatory={"devID"}
        }
    }
})

--------------------------------------------
-- command registration: no bridge sanity-check
--------------------------------------------
CLI_addCommand("config", "no bridge sanity-check", {
    func   = function(params) 
               params.flagNo = true
               bridge_sanity_check(params)
           end,
    help   = "Disable packet sanity checks of the device",
    params={
        {   type="values", "%sanity_check" },
        {   type="named",
            { format="device %devID_all", name="devID", help = "Device ID"},
            mandatory={"devID"}
        }
    }
})

