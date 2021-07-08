--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_serdes_rx.lua
--*
--* DESCRIPTION:
--*       display the serdes rx configuration
--*
--*
--********************************************************************************



--constants



-- ************************************************************************
---
--  show_interfaces_serdes_rx_port_range
--        @description  shows statistics on traffic seen by the physical 
--                      interface with many ports
--
--        @param params         - params["all_device"]: all or given 
--                                devices iterating property, could be 
--                                irrelevant
--                                params["mac-address"]: mac-address, could 
--                                be irrelevant;
--                                params["ethernet"]: interface range, 
--                                could be irrelevant;
--                                params["port-channel"]: trunk id,could be 
--                                irrelevant;
--                                params["vlan"]: vlan Id, could be 
--                                irrelevant;
--                                params["skipped"]: skipping property, 
--                                could be irrelevant 
--        @param command_data   - command execution data object                     
--
--        @return        true on success, otherwise false and error message
--

local function show_interfaces_serdes_rx_port_range(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local GT_OK = 0
    local header_string1, footer_string1
    
    local ret, localSerdesRxPtr
    local ifMode
  
  
    -- Command specific variables initialization.
    local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(0)   
    if((devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
       (devFamily == "CPSS_PP_FAMILY_DXCH_AC3X_E") or
       ((devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") and (devSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E")) )
    then
        header_string1 = 
            "\n" ..
            "Serdes Rx Config\n" ..
            "----------------\n" ..
            "Interface LaneNum  SqThresh   FFE Res    FFE Cap    bandWidth    dcGain    loopBandwidth \n" ..
            "--------- ------- ---------- ---------- ---------- ----------- ---------- ---------------\n"
    else
        header_string1 = 
            "\n" ..
            "Serdes Rx Config\n" ..
            "----------------\n" ..
            "Interface LaneNum  SqThresh   FFE Res    FFE Cap    Align 90 \n" ..
            "--------- ------- ---------- ---------- ---------- ----------\n"
            footer_string1 = "\n" 
    end 
    
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()     

            if portNum == "CPU" then portNum = 63 end
            -- DevNum/PortNum string forming.
            devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) .. 
                    '/' .. 
                    tostring(portNum), 9)
            -- Traffic statistic getting.
            command_data:clearLocalStatus()
            -- Get ifMode
            ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
            ifMode=localSerdesRxPtr["ifModePtr"] 
			
            if(ret~=GT_OK) then
                command_data:addError("Error at Serdes Rx command")
            end
            -- Get laneNum
            ret, localSerdesRxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                    {"OUT","GT_U32","startSerdesPtr"},
                    {"OUT","GT_U32","numOfSerdesLanesPtr"}})
            local startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
            local  numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
            if(ret~=GT_OK) then
                command_data:addError("Error at Serdes Rx command")
            end
			
            for i=0,numOfSerdesLanesPtr-1,1 do
                local laneNum = i
                ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortSerdesManualRxConfigGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","GT_U32","laneNum",laneNum},
                        {"OUT","CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr"}})
				
                -- Traffic statistic getting.
                command_data:clearLocalStatus()

                command_data:updateStatus()    
					
                if(ret~=0) then
                    --command_data:addError("Error at Serdes Rx command")
                    print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigGet ret: "..to_string(ret).." "..returnCodes[ret])
                else
                    local OutValues=localSerdesRxPtr["serdesRxCfgPtr"] 
                    if((devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
                      (devFamily == "CPSS_PP_FAMILY_DXCH_AC3X_E") or
                      ((devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") and (devSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E")) )
                then
                    local sqlch = OutValues.sqlch					        --sqlch
                    local ffeRes = OutValues.ffeRes				        	--ffeRes
                    local ffeCap = OutValues.ffeCap					        --ffeCap
                    local bandWidth = OutValues.bandWidth                   --bandWidth
                    local dcGain = OutValues.dcGain                         --dcGain
                    local loopBandwidth = OutValues.loopBandwidth           --loopBandwidth

                    sqlch=to_string(sqlch)
                    ffeRes=to_string(ffeRes)
                    ffeCap=to_string(ffeCap)
                    bandWidth=to_string(bandWidth)
                    dcGain=to_string(dcGain)
                    loopBandwidth=to_string(loopBandwidth)
                    laneNum=to_string(i)

                    command_data["result"] = 
                            string.format("%-10s%-8s%-11s%-11s%-11s%-12s%-11s%-15s", 
                            devnum_portnum_string, 
                            alignLeftToCenterStr(laneNum, 7),
                            alignLeftToCenterStr(sqlch, 10), 
                            alignLeftToCenterStr(ffeRes, 10),
                            alignLeftToCenterStr(ffeCap, 10),    
                            alignLeftToCenterStr(bandWidth, 10),
                            alignLeftToCenterStr(dcGain, 10),
                            alignLeftToCenterStr(loopBandwidth, 14))
                else
                    local sqlch=OutValues.sqlch					--sqlch
                    local ffeRes=OutValues.ffeRes					--ffeRes
                    local ffeCap=OutValues.ffeCap					--ffeCap
                    local align90=OutValues.align90				--align90
						
                    sqlch=to_string(sqlch)
                    ffeRes=to_string(ffeRes)
                    ffeCap=to_string(ffeCap)
                    align90=to_string(align90)
                    laneNum=to_string(i)

                    command_data["result"] = 
                            string.format("%-10s%-8s%-11s%-11s%-11s%-11s", 
                            devnum_portnum_string, 
                            alignLeftToCenterStr(laneNum, 7),
                            alignLeftToCenterStr(sqlch, 10), 
                            alignLeftToCenterStr(ffeRes, 10),
                            alignLeftToCenterStr(ffeCap, 10),    
                            alignLeftToCenterStr(align90, 10))
                end
                    command_data:addResultToResultArray()
                    command_data:updatePorts()
                end
            end
        end
    -- Resulting table string formatting.
    command_data:setResultArrayToResultStr(1, 1)
    command_data:setResultStrOnPortCount(header_string1, command_data["result"], 
            footer_string1,
            "There is no port information to show.\n")
    command_data:analyzeCommandExecution()							
    command_data:printCommandExecutionResults()
	
    end
    return command_data:getCommandExecutionResults()
end   
  


-- ************************************************************************
---
--  show_interfaces_serdes_rx_single_port
--        @description  shows interfaces_serdes_rx with single port 
--
--        @param params         - params["all"]: all devices port or all 
--                                ports of given device cheking switch, 
--                                could be irrelevant;
--                                params["devID"]: checked device number, 
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil;
--                                params["port-channel"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil
--        @param command_data   - command execution data object
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_serdes_rx_single_port(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    
    
    local ret, localSerdesRxPtr
    
    
    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()
    if portNum == "CPU" then portNum = 63 end
        -- DevNum/PortNum string forming.
        devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) .. 
                '/' .. 
                tostring(portNum), 9)
    local GT_OK = 0
    -- Command specific variables initialization.    

    command_data:clearLocalStatus()

	
    ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
    local ifMode=localSerdesRxPtr["ifModePtr"] 
    if(ret~=GT_OK) then
        command_data:addError("Error at Serdes Rx command")
    end
	
    ret, localSerdesRxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
            {"OUT","GT_U32","startSerdesPtr"},
            {"OUT","GT_U32","numOfSerdesLanesPtr"}})
    local startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
    local numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
    -- Traffic statistic getting.    
            

    -- Traffic statistic strings formatting and adding. 
    local laneNum=params.laneNum
    -- Display Serdes Rx 
    if(laneNum~=nil or numOfSerdesLanesPtr==1) then
        ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortSerdesManualRxConfigGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"OUT","CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr"}})

        if(ret~=GT_OK) then
            --command_data:addError("Error at Serdes Rx command")
            print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigGet ret: "..to_string(ret).." "..returnCodes[ret])
        else

            local OutValues=localSerdesRxPtr["serdesRxCfgPtr"] 
            local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(0)
            if((devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
              (devFamily == "CPSS_PP_FAMILY_DXCH_AC3X_E") or
              ((devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") and (devSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E")) )
            then
                local sqlch = OutValues.sqlch					        --sqlch
                local ffeRes = OutValues.ffeRes				        	--ffeRes
                local ffeCap = OutValues.ffeCap					        --ffeCap
                local bandWidth = OutValues.bandWidth                   --bandWidth
                local dcGain = OutValues.dcGain                         --dcGain
                local loopBandwidth = OutValues.loopBandwidth           --loopBandwidth

                print("")
                print("Serdes RX Config")
                print("----------------")
                print("SQ Threshold:     ",sqlch)
                print("FFE Res:          ",ffeRes)
                print("FFE Cap:          ",ffeCap)
                print("bandWidth:        ",bandWidth)
                print("dcGain:           ",dcGain)
                print("loopBandwidth:    ",loopBandwidth)
            else

                local sqlch=OutValues.sqlch					--sqlch
                local ffeRes=OutValues.ffeRes					--ffeRes
                local ffeCap=OutValues.ffeCap					--ffeCap
                local align90=OutValues.align90				--align90
			
            print("")
            print("Serdes RX Config")
            print("----------------")
            print("SQ Threshold:\t",sqlch)
            print("FFE Res:\t",ffeRes)
            print("FFE Cap:\t",ffeCap)
            print("Align 90:\t",align90)
			
            -- Resulting string formatting.
            command_data:setResultArrayToResultStr()      
            
            command_data:analyzeCommandExecution()
			   
                command_data:printCommandExecutionResults()
            end
        end


    else
        ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})
        ifMode=localSerdesRxPtr["ifModePtr"] 
        if(ret~=GT_OK) then
             command_data:addError("Error at Serdes Rx command")
        end
		
        ret, localSerdesRxPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                {"OUT","GT_U32","startSerdesPtr"},
                {"OUT","GT_U32","numOfSerdesLanesPtr"}})
        startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
        numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
        if(ret~=GT_OK) then
            command_data:addError("Error at Serdes Rx command")
        end
        local header_string1
        local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(0)

        if((devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
            (devFamily == "CPSS_PP_FAMILY_DXCH_AC3X_E") or
              ((devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") and (devSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E")) )
        then
        header_string1 = 
                "\n" ..
                "Serdes Rx Config\n" ..
                "----------------\n" ..
                "Interface LaneNum  SqThresh   FFE Res    FFE Cap    bandWidth    dcGain    loopBandwidth \n" ..
                "--------- ------- ---------- ---------- ---------- ----------- ---------- ---------------\n"
        else
        header_string1 = 
                "\n" ..
                "Serdes Rx Config\n" ..
                "----------------\n" ..
                "Interface LaneNum  SqThresh   FFE Res    FFE Cap    Align 90 \n" ..
                "--------- ------- ---------- ---------- ---------- ----------\n"
        end
        local  footer_string1 = "\n"  
		
        for i=0,numOfSerdesLanesPtr-1,1 do
            local laneNum = i
            ret, localSerdesRxPtr = myGenWrapper("cpssDxChPortSerdesManualRxConfigGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","GT_U32","laneNum",laneNum},
                    {"OUT","CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr"}})
			
            -- Traffic statistic getting.
            command_data:clearLocalStatus()

            command_data:updateStatus()    
				
            if(ret~=GT_OK) then
                --command_data:addError("Error at Serdes Rx command")
                print(devnum_portnum_string.." Error cpssDxChPortSerdesManualRxConfigGet ret: "..to_string(ret).." "..returnCodes[ret])
            else
                local OutValues=localSerdesRxPtr["serdesRxCfgPtr"] 
                if((devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
                    (devFamily == "CPSS_PP_FAMILY_DXCH_AC3X_E") or
                      ((devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") and (devSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E")) )
                then
                    local sqlch = OutValues.sqlch					        --sqlch
                    local ffeRes = OutValues.ffeRes				        	--ffeRes
                    local ffeCap = OutValues.ffeCap					        --ffeCap
                    local bandWidth = OutValues.bandWidth                   --bandWidth
                    local dcGain = OutValues.dcGain                         --dcGain
                    local loopBandwidth = OutValues.loopBandwidth           --loopBandwidth

                    sqlch=to_string(sqlch)
                    ffeRes=to_string(ffeRes)
                    ffeCap=to_string(ffeCap)
                    bandWidth=to_string(bandWidth)
                    dcGain=to_string(dcGain)
                    loopBandwidth=to_string(loopBandwidth)
                    laneNum=to_string(i)

                    command_data["result"] = 
                            string.format("%-10s%-8s%-11s%-11s%-11s%-12s%-11s%-15s", 
                            devnum_portnum_string, 
                            alignLeftToCenterStr(laneNum, 7),
                            alignLeftToCenterStr(sqlch, 10), 
                            alignLeftToCenterStr(ffeRes, 10),
                            alignLeftToCenterStr(ffeCap, 10),    
                            alignLeftToCenterStr(bandWidth, 10),
                            alignLeftToCenterStr(dcGain, 10),
                            alignLeftToCenterStr(loopBandwidth, 14))
                else
                    local sqlch=OutValues.sqlch					--sqlch
                    local ffeRes=OutValues.ffeRes					--ffeRes
                    local ffeCap=OutValues.ffeCap					--ffeCap
                    local align90=OutValues.align90			   	        --align90
					
                    sqlch=to_string(sqlch)
                    ffeRes=to_string(ffeRes)
                    ffeCap=to_string(ffeCap)
                    align90=to_string(align90)
                    laneNum=to_string(i)
					
                    command_data["result"] = 
                            string.format("%-10s%-8s%-11s%-11s%-11s%-11s", 
                            devnum_portnum_string, 
                            alignLeftToCenterStr(laneNum, 7),
                            alignLeftToCenterStr(sqlch, 10), 
                            alignLeftToCenterStr(ffeRes, 10),
                            alignLeftToCenterStr(ffeCap, 10),    
                            alignLeftToCenterStr(align90, 10))
                end
                command_data:addResultToResultArray()

						
                command_data:updatePorts()
            end

			
        end
        -- Resulting table string formatting.
        command_data:setResultArrayToResultStr(1, 1)
        command_data:setResultStrOnPortCount(header_string1, command_data["result"], 
												footer_string1,
												"There is no port information to show.\n")
        command_data:analyzeCommandExecution()							
        command_data:printCommandExecutionResults()
    end
    return command_data:getCommandExecutionResults()
end




-- ************************************************************************
---
--  show_interfaces_serdes_rx
--        @description  show interfaces serdes rx
--
--        @param params         - params["all"]: all devices port or all 
--                                ports of given device cheking switch, 
--                                could be irrelevant;
--                                params["devID"]: checked device number, 
--                                relevant if params["all"] is not nil;
--                                params["ethernet"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil;
--                                params["port-channel"]: checked interface 
--                                name, relevant if params["all"] is not 
--                                nil
--
--        @return       true on success, otherwise false and error message
--
local function show_interfaces_serdes_rx(params) 
    -- Common variables declaration
    local command_data = Command_Data() 
    
    local result, values
    
    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)    
    if params.all ~= nil then
        local dev, ports
        for dev,ports in pairs(command_data.dev_port_range) do
            table.insert(ports,"CPU")
        end
    end
    
    -- System specific data initialization.
    command_data:enablePausedPrinting()       
    
    if true == command_data:isOnePortInPortRange() then
        result, values = show_interfaces_serdes_rx_single_port(params, command_data)
    else
        result, values = show_interfaces_serdes_rx_port_range(params, command_data)
    end
    
    return result, values
end

--------------------------------------------------------------------------------
-- command registration: show interfaces serdes rx
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show interfaces serdes rx", {
    func = show_interfaces_serdes_rx,
    help = "Interface(s) serdes rx",
    params = {
        { type= "named", {format="lane_num %lane_number_type", name="laneNum", help="Serdes lane number"}, 
            "#all_interfaces",     
            mandatory={"all_interfaces"}
        }
    }
})
    
