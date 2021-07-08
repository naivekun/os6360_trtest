--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_bridge_sanity_check.lua
--*
--* DESCRIPTION:  show bridge sanity-check 
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

-- variables definitions
local 	 enumCheckTypeTbl

-- ************************************************************************
---
--  getCheckTypeName
--        @description  get the packet sanity check type name from the table
--
local function getCheckTypeName(check)
    return enumCheckTypeTbl[check]
end


-- ************************************************************************
---
--  createCheckTypeTbl
--        @description  if the packet sanity checks table is null, than create it
--        @param dev             - device number
--
local function createCheckTypeTbl(dev)
    if (enumCheckTypeTbl == nil) then
        enumCheckTypeTbl = {}
        table.insert(enumCheckTypeTbl,"tcp-syn-data")
        table.insert(enumCheckTypeTbl,"tcp-over-mac-mc-bc")
        table.insert(enumCheckTypeTbl,"tcp-flag-zero")
        table.insert(enumCheckTypeTbl,"tcp-flags-fin-urg-psh")
        table.insert(enumCheckTypeTbl,"tcp-flags-syn-fin")
        table.insert(enumCheckTypeTbl,"tcp-flags-syn-rst")
        table.insert(enumCheckTypeTbl,"tcp-udp-port-zero")
        --table.insert(enumCheckTypeTbl,"tcp-all")
        table.insert(enumCheckTypeTbl,"frag-ipv4-icmp")
        table.insert(enumCheckTypeTbl,"arp-mac-sa-mismatch")
        table.insert(enumCheckTypeTbl,"sip-is-dip")
        table.insert(enumCheckTypeTbl,"tcp-without-full-header")
        table.insert(enumCheckTypeTbl,"tcp-fin-without-ack")
        table.insert(enumCheckTypeTbl,"tcp-udp-sport-is-dport")
    end
end


-- ************************************************************************
--  bridge_sanity_check_show_func
--        @description  show bridge sanity-check
--
--        @param params             -params["device"]: specific device number 
--
--
--        @return       true on success, otherwise false and error message
-- 
local function bridge_sanity_check_show_func(params)
    -- print(to_string(params))
    local devNum
    local command_data = Command_Data()
    local checkType = params["sanity_check"]
    local enable, sanity_check_string
    local header_string = 
        "Dev Protocol                check\n" ..
        "--- ----------------------- ---------"
    local footer_string = "\n"  
    local devId = params["devID"]
    createCheckTypeTbl(devId)

    local CPSS_NST_CHECK_TCP_ALL_E = 7 -- index in loop starts from 0
    local CPSS_NST_CHECK_SIP_IS_DIP_E = 9

    params["all"] = true

    startAPILog("Start of <bridge_sanity_check_show_func> (" .. to_string(params) .. ")")

    params.all = true
    if params.devID == "all" then
        params.devID = nil
    end

    -- Common variables initialization
    command_data:initAllAvailableDevicesRange(params)
    command_data:clearLocalStatus();
    command_data:clearResultArray()

    --print(to_string(command_data))
    --print(to_string(params))

    startLoopLog("for devNum")

    -- Main device handling cycle
    if true == command_data["status"] then
        local lastdev = nil
        local iterator
        for iterator, devNum in command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()     

            -- Packet sanity checks getting.
            command_data:clearLocalStatus()
            if devNum ~= lastdev then
                if true == command_data["local_status"] then
                    startLoopLog("for sanity_check")
            		for i=0, #enumCheckTypeTbl-1 do
            			tableIndex = i + 1     -- index for getCheckTypeName(), start from 1
            			if i < CPSS_NST_CHECK_TCP_ALL_E then
             		        checkIndex = i     -- tcp-all is 7 and it will not be displayed
             		    else
             		        checkIndex = i + 1
             		    end

                        if ( i < CPSS_NST_CHECK_SIP_IS_DIP_E ) or -- 'sip-is-dip' and above just supported in SIP5
                           ( i >= CPSS_NST_CHECK_SIP_IS_DIP_E and is_sip_5(devNum)) then 
                            enable = false
                            ret,val = myGenWrapper("cpssDxChNstProtSanityCheckGet",{                    
                                {"IN","GT_U8","devNum",devNum},
                                {"IN","CPSS_NST_CHECK_ENT","checkType",checkIndex},
                                {"OUT","GT_BOOL","enablePtr"}
                            })

                            if        0 == ret then 
        						enable = val["enablePtr"]
                            elseif 0x10 == ret then
                                command_data:setFailDeviceStatus() 
                                command_data:addWarning("Packet sanity checks " ..
                                                        "getting is not allowed on " ..
                                                        "device %d.", devNum)                     
        					elseif (ret ~= 0) then
        	                    command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error at packet sanity checks " ..
                                                  "getting on device %d.", 
                                                  devNum, returnCodes[ret])
        					end
                            
                            if 0 == ret then       
                                sanity_check_string = getCheckTypeName(tableIndex)
                            else
                                sanity_check_string = "n/a"
                            end

                            command_data:updateStatus()             

                            -- Resulting string formatting and adding.
                            command_data["result"] = 
                                string.format("%-3s %-23s %-9s", tostring(devNum), 
                                              sanity_check_string,
                                              boolEnabledUpperStrGet(enable))  
                            command_data:addResultToResultArray()
                            
                            command_data:updateDevices()
                        end
     				end
                    stopLoopLog()
    			end
			end
			lastdev = devNum
			
            command_data:updateStatus();
            command_data:updateDevices();
		end
		stopLoopLog()
    end

    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr()
    command_data:setResultStrOnDevicesCount(header_string, command_data["result"], 
                                            footer_string,
                                            "There is no bridge sanity checks to show.\n")

    leaveAPILog("end of <bridge_sanity_check_show_func>  ")
        
    command_data:analyzeCommandExecution()
       
    command_data:printCommandExecutionResults()
    
	return command_data:getCommandExecutionResults()            
end


--------------------------------------------------------------------------------
-- command registration: show bridge sanity-check
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show bridge", "Bridging infomation")
CLI_addCommand("exec", "show bridge sanity-check", {
  func   = bridge_sanity_check_show_func,
  help   = "Show the status of packet sanity checks",
  params = {
         {   
         type= "named", 
             { format="device %devID_all", name="device", help="Device ID"},
             mandatory = {"device"}
         } 
  }
})

