--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_interfaces_serdes_tuning.lua
--*
--* DESCRIPTION:
--*       display the serdes tuning
--*
--*
--********************************************************************************



--constants



-- ************************************************************************
---
--  show_interfaces_serdes_tuning_port_range
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

local function show_interfaces_serdes_tuning_port_range(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    local GT_OK = 0
    local header_string1, header_string2, header_string3, header_string4, footer_string1, footer_string2, footer_string3, footer_string4
    
    local ret, localSerdesRxPtr
    local ifMode
    local localSerdesTuningPtr
    local laneNum, serdesFrequency
    
    -- Command specific variables initialization.   
    header_string1 = 
      "\n" ..
      "Serdes Tuning Config\n" ..
      "--------------------\n" ..
      "Interface LaneNum   DFE F1    FFE Res    FFE Cap    sampler    SqThresh \n" ..
      "--------- ------- ---------- ---------- ---------- ---------- ----------\n"
    footer_string1 = "\n"  
    
    header_string2 = 
      "\n" ..
      "Interface LaneNum txEmphAmp    txAmp     txAmpAdj     ffeS    txEmphEn \n" ..
      "--------- ------- ---------- ---------- ---------- ---------- -------- \n"
    footer_string2 = "\n" 
    
    header_string3 = 
      "\n" ..
      "Interface LaneNum   txEmph1     align90    txEmphEn   txAmpShft \n" ..
      "--------- ------- ----------- ----------- ---------- -----------\n"
    footer_string3 = "\n" 
    
    header_string4 = 
      "\n" ..
      "Interface LaneNum  dfeVals1   dfeVals2   dfeVals3   dfeVals4   dfeVals5   dfeVals6 \n" ..
      "--------- ------- ---------- ---------- ---------- ---------- ---------- ----------\n"
    footer_string4 = "\n" 
    
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
			command_data:updateStatus()            
  
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
			local startSerdesPtr=localSerdesRxPtr["startSerdesPtr"]
			local numOfSerdesLanesPtr=localSerdesRxPtr["numOfSerdesLanesPtr"]
			if(ret~=GT_OK) then
					command_data:addError("Error at Serdes Rx command")
			end
            
			for i=0,numOfSerdesLanesPtr-1,1 do
				local laneNum = i
				ret, localSerdesTuningPtr = myGenWrapper("cpssDxChPortSerdesTuningGet",{
					{"IN","GT_U8","devNum",devNum},
					{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
					{"IN","GT_U32","laneNum",laneNum},
					{"IN","CPSS_DXCH_PORT_SERDES_SPEED_ENT","serdesFrequency",serdesFrequency},
					{"OUT","CPSS_DXCH_PORT_SERDES_TUNE_STC","tuneValuesPtr"}})
				
				-- Traffic statistic getting.
				command_data:clearLocalStatus()

				command_data:updateStatus()    
					
				if(ret~=GT_OK) then
					command_data:addError("Error at Serdes Tuning command")
				end
				local OutValues=localSerdesTuningPtr["tuneValuesPtr"] 
				local dfe=OutValues.dfe						--dfe
				local ffeR=OutValues.ffeR						--ffeR
				local ffeC=OutValues.ffeC						--ffeC
				local sampler=OutValues.sampler				--sampler
				local sqlch=OutValues.sqlch					--sqlch
				
				local txEmphAmp=OutValues.txEmphAmp			--txEmphAmp
				local txAmp=OutValues.txAmp					--txAmp
				local txAmpAdj=OutValues.txAmpAdj				--txAmpAdj
				local ffeS=OutValues.ffeS						--ffeS
				local txEmphEn=OutValues.txEmphEn				--txEmphEn
				
				local txEmph1=OutValues.txEmph1				--txEmph1
				local align90=OutValues.align90				--align90
				local txEmphEn1=OutValues.txEmphEn1			--txEmphEn1
				local txAmpShft=OutValues.txAmpShft			--txAmpShft
				local dfeValsArray=OutValues.dfeValsArray		--dfeValsArray
				
				local dfeVals1=dfeValsArray[0]
				local dfeVals2=dfeValsArray[1]
				local dfeVals3=dfeValsArray[2]
				local dfeVals4=dfeValsArray[3]
				local dfeVals5=dfeValsArray[4]
				local dfeVals6=dfeValsArray[5]


				laneNum=to_string(i)
				-- First resulting string formatting and adding.	
				dfe=to_string(dfe)
				ffeR=to_string(ffeR)
				ffeC=to_string(ffeC)
				sampler=to_string(sampler)
				sqlch=to_string(sqlch)
				command_data["result"] = 
					string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s", 
								  devnum_portnum_string, 
								  alignLeftToCenterStr(laneNum, 7),
								  alignLeftToCenterStr(dfe, 10), 
								  alignLeftToCenterStr(ffeR, 10),
								  alignLeftToCenterStr(ffeC, 10),    
								  alignLeftToCenterStr(sampler, 10),
								  alignLeftToCenterStr(sqlch, 10))
				command_data:addResultToResultArray()
			   
				-- Second resulting string formatting and adding.
				txEmphAmp=to_string(txEmphAmp)
				txAmp=to_string(txAmp)
				txAmpAdj=to_string(txAmpAdj)
				ffeS=to_string(ffeS)
				txEmphEn=to_string(txEmphEn)
				command_data["result"] = 
					string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-9s", 
								  devnum_portnum_string, 
								  alignLeftToCenterStr(laneNum, 7),    
								  alignLeftToCenterStr(txEmphAmp, 10),
								  alignLeftToCenterStr(txAmp, 10),    
								  alignLeftToCenterStr(txAmpAdj, 10),
								  alignLeftToCenterStr(ffeS, 10),    
								  alignLeftToCenterStr(txEmphEn, 8))
				command_data:addResultToResultArray()
			
				-- Third resulting string formatting and adding.
				txEmph1=to_string(txEmph1)
				align90=to_string(align90)
				txEmphEn1=to_string(txEmphEn1)
				txAmpShft=to_string(txAmpShft)
				command_data["result"] = 
					string.format("%-10s%-8s%-12s%-12s%-11s%-12s", 
								  devnum_portnum_string, 
								  alignLeftToCenterStr(laneNum, 7),    
								  alignLeftToCenterStr(txEmph1, 11),
								  alignLeftToCenterStr(align90, 11),    
								  alignLeftToCenterStr(txEmphEn1, 10),
								  alignLeftToCenterStr(txAmpShft, 11))
				command_data:addResultToResultArray()
				
				-- Fourth resulting string formatting and adding.
				dfeVals1=to_string(dfeVals1)
				dfeVals2=to_string(dfeVals2)
				dfeVals3=to_string(dfeVals3)
				dfeVals4=to_string(dfeVals4)
				dfeVals5=to_string(dfeVals5)
				dfeVals6=to_string(dfeVals6)
				command_data["result"] = 
					string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s%-11s", 
								  devnum_portnum_string, 
								  alignLeftToCenterStr(laneNum, 7),    
								  alignLeftToCenterStr(dfeVals1, 10),
								  alignLeftToCenterStr(dfeVals2, 10),    
								  alignLeftToCenterStr(dfeVals3, 10),
								  alignLeftToCenterStr(dfeVals4, 10),
								  alignLeftToCenterStr(dfeVals5, 10),
								  alignLeftToCenterStr(dfeVals6, 10))
				command_data:addResultToResultArray()

				command_data:updatePorts()
				
			end
		end
		-- Resulting table string formatting.
		
		-- First resulting table string formatting.
		command_data:setResultArrayToResultStr(1, 4)
		command_data:setResultStrOnPortCount(header_string1, command_data["result"], 
											footer_string1,
											"There is no port information to show.\n")
                                         
		-- Second resulting table string formatting.
		command_data:addToResultStrOnPortCount(
			header_string2, command_data:getInterlacedResultArray(2, 4), 
			footer_string2)   					
		
		-- Third resulting table string formatting.
		command_data:addToResultStrOnPortCount(
			header_string3, command_data:getInterlacedResultArray(3, 4), 
			footer_string3)  
		
		-- Fourth resulting table string formatting.
		command_data:addToResultStrOnPortCount(
			header_string4, command_data:getInterlacedResultArray(4, 4), 
			footer_string4)  
			
		command_data:analyzeCommandExecution()
		command_data:printCommandExecutionResults()
	end
	return command_data:getCommandExecutionResults()				
end

  


-- ************************************************************************
---
--  show_interfaces_serdes_tuning_single_port
--        @description  shows interfaces_serdes_tuning with single port 
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
local function show_interfaces_serdes_tuning_single_port(params, command_data)
    -- Common variables declaration
    local devNum, portNum
    -- Command  specific variables declaration
    local devnum_portnum_string
    
    local ret, localSerdesRxPtr
    local ifMode
    local startSerdesPtr
    local numOfSerdesLanesPtr
    local localSerdesTuningPtr
    
    
    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()
    if portNum == "CPU" then portNum = 63 end
    -- DevNum/PortNum string forming.
    devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) .. 
												 '/' .. 
												 tostring(portNum), 9)
    -- Common variables initialization.
    devNum, portNum = command_data:getFirstPort()
	
    local GT_OK = 0
    -- Command specific variables initialization.    
    local serdesFrequency=params.serdes_speed
    command_data:clearLocalStatus()
    local laneNum=params.laneNum
            
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
    -- Traffic statistic strings formatting and adding. 	
		
    
    -- Display Serdes Tuning
	if(laneNum~=nil or numOfSerdesLanesPtr==1) then
		ret, localSerdesTuningPtr = myGenWrapper("cpssDxChPortSerdesTuningGet",{
				{"IN","GT_U8","devNum",devNum},
				{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
				{"IN","GT_U32","laneNum",laneNum},
				{"IN","CPSS_DXCH_PORT_SERDES_SPEED_ENT","serdesFrequency",serdesFrequency},
				{"OUT","CPSS_DXCH_PORT_SERDES_TUNE_STC","tuneValuesPtr"}})
			
		
		if(ret~=GT_OK) then
			print("Error at Serdes Tuning command")
		end
		
		local OutValues=localSerdesTuningPtr["tuneValuesPtr"] 
		local dfe=OutValues.dfe					--dfe
		local ffeR=OutValues.ffeR				--ffeR
		local ffeC=OutValues.ffeC				--ffeC
		local sampler=OutValues.sampler			        --sampler
		local sqlch=OutValues.sqlch				--sqlch
		
		local txEmphAmp=OutValues.txEmphAmp	        	--txEmphAmp
		local txAmp=OutValues.txAmp				--txAmp
		local txAmpAdj=OutValues.txAmpAdj			--txAmpAdj
		local ffeS=OutValues.ffeS				--ffeS
		local txEmphEn=OutValues.txEmphEn			--txEmphEn
		
		local txEmph1=OutValues.txEmph1				--txEmph1
		local align90=OutValues.align90				--align90
		local txEmphEn1=OutValues.txEmphEn1			--txEmphEn1
		local txAmpShft=OutValues.txAmpShft			--txAmpShft
		local dfeValsArray=OutValues.dfeValsArray		--dfeValsArray
		
		print("")
		print("Serdes Tuning")
		print("-------------")
		print("DFE F1:\t\t\t",dfe)
		print("FFE Res:\t\t",ffeR)
		print("FFE Cap:\t\t",ffeR)
		print("Sampler:\t\t",sampler)
		print("SQ Threshold:\t\t",sqlch)
		print("Emphasis Level:\t\t",txEmphAmp)
		print("Amplitude:\t\t",txAmp)
		print("Amplitude Adjust:\t",txAmpAdj)
		print("FFE Swing Control:\t",ffeS)
		print("Pre/De-Emphasis Enable:\t",txEmphEn)
		print("Emph Gen1 bit rates:\t",txEmph1)
		print("Align 90:\t\t",align90)
		print("Emphasis Gen1 Enable:\t",txEmphEn1)
		print("Amplitude Shift:\t",txAmpShft)
		
		print("DFE Coefficient 1:\t",dfeValsArray[0])
		print("DFE Coefficient 2:\t",dfeValsArray[1])
		print("DFE Coefficient 3:\t",dfeValsArray[2])
		print("DFE Coefficient 4:\t",dfeValsArray[3])
		print("DFE Coefficient 5:\t",dfeValsArray[4])
		print("DFE Coefficient 6:\t",dfeValsArray[5])

		-- Resulting string formatting.

		command_data:setResultArrayToResultStr()    
			
		command_data:analyzeCommandExecution()
			
		command_data:printCommandExecutionResults()
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
		local header_string1 = 
				"\n" ..
				"Serdes Tuning Config\n" ..
				"--------------------\n" ..
				"Interface LaneNum   DFE F1    FFE Res    FFE Cap    sampler    SqThresh \n" ..
				"--------- ------- ---------- ---------- ---------- ---------- ----------\n"
		local footer_string1 = "\n"  
			
		local header_string2 = 
				"\n" ..
				"Interface LaneNum txEmphAmp    txAmp     txAmpAdj     ffeS    txEmphEn \n" ..
				"--------- ------- ---------- ---------- ---------- ---------- -------- \n"
		local footer_string2 = "\n" 
			
		local header_string3 = 
				"\n" ..
				"Interface LaneNum   txEmph1     align90    txEmphEn   txAmpShft \n" ..
				"--------- ------- ----------- ----------- ---------- -----------\n"
		local footer_string3 = "\n" 
			
		local header_string4 = 
				"\n" ..
				"Interface LaneNum  dfeVals1   dfeVals2   dfeVals3   dfeVals4   dfeVals5   dfeVals6 \n" ..
				"--------- ------- ---------- ---------- ---------- ---------- ---------- ----------\n"
		local footer_string4 = "\n" 
		
		for i=0,numOfSerdesLanesPtr-1,1 do
			local laneNum = i
			ret, localSerdesTuningPtr = myGenWrapper("cpssDxChPortSerdesTuningGet",{
				{"IN","GT_U8","devNum",devNum},
				{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
				{"IN","GT_U32","laneNum",laneNum},
				{"IN","CPSS_DXCH_PORT_SERDES_SPEED_ENT","serdesFrequency",serdesFrequency},
				{"OUT","CPSS_DXCH_PORT_SERDES_TUNE_STC","tuneValuesPtr"}})
			
	                -- Traffic statistic getting.
	                command_data:clearLocalStatus()

	                command_data:updateStatus()    
				
			if(ret~=GT_OK) then
				command_data:addError("Error at Serdes Tuning command")
			end
			local OutValues=localSerdesTuningPtr["tuneValuesPtr"] 
			local dfe=OutValues.dfe						--dfe
			local ffeR=OutValues.ffeR						--ffeR
			local ffeC=OutValues.ffeC						--ffeC
			local sampler=OutValues.sampler				--sampler
			local sqlch=OutValues.sqlch					--sqlch
			
			local txEmphAmp=OutValues.txEmphAmp			--txEmphAmp
			local txAmp=OutValues.txAmp					--txAmp
			local txAmpAdj=OutValues.txAmpAdj				--txAmpAdj
			local ffeS=OutValues.ffeS						--ffeS
			local txEmphEn=OutValues.txEmphEn				--txEmphEn
			
			local txEmph1=OutValues.txEmph1				--txEmph1
			local align90=OutValues.align90				--align90
			local txEmphEn1=OutValues.txEmphEn1			--txEmphEn1
			local txAmpShft=OutValues.txAmpShft			--txAmpShft
			local dfeValsArray=OutValues.dfeValsArray		--dfeValsArray
			
			local dfeVals1=dfeValsArray[0]
			local dfeVals2=dfeValsArray[1]
			local dfeVals3=dfeValsArray[2]
			local dfeVals4=dfeValsArray[3]
			local dfeVals5=dfeValsArray[4]
			local dfeVals6=dfeValsArray[5]


			laneNum=to_string(i)
			-- First resulting string formatting and adding.	
			dfe=to_string(dfe)
			ffeR=to_string(ffeR)
			ffeC=to_string(ffeC)
			sampler=to_string(sampler)
			sqlch=to_string(sqlch)
			command_data["result"] = 
				string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s", 
							  devnum_portnum_string, 
							  alignLeftToCenterStr(laneNum, 7),
							  alignLeftToCenterStr(dfe, 10), 
							  alignLeftToCenterStr(ffeR, 10),
							  alignLeftToCenterStr(ffeC, 10),    
							  alignLeftToCenterStr(sampler, 10),
							  alignLeftToCenterStr(sqlch, 10))
			command_data:addResultToResultArray()
		   
			-- Second resulting string formatting and adding.
			txEmphAmp=to_string(txEmphAmp)
			txAmp=to_string(txAmp)
			txAmpAdj=to_string(txAmpAdj)
			ffeS=to_string(ffeS)
			txEmphEn=to_string(txEmphEn)
			command_data["result"] = 
				string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-9s", 
							  devnum_portnum_string, 
							  alignLeftToCenterStr(laneNum, 7),    
							  alignLeftToCenterStr(txEmphAmp, 10),
							  alignLeftToCenterStr(txAmp, 10),    
							  alignLeftToCenterStr(txAmpAdj, 10),
							  alignLeftToCenterStr(ffeS, 10),    
							  alignLeftToCenterStr(txEmphEn, 8))
			command_data:addResultToResultArray()
		
			-- Third resulting string formatting and adding.
			txEmph1=to_string(txEmph1)
			align90=to_string(align90)
			txEmphEn1=to_string(txEmphEn1)
			txAmpShft=to_string(txAmpShft)
			command_data["result"] = 
				string.format("%-10s%-8s%-12s%-12s%-11s%-12s", 
							  devnum_portnum_string, 
							  alignLeftToCenterStr(laneNum, 7),    
							  alignLeftToCenterStr(txEmph1, 11),
							  alignLeftToCenterStr(align90, 11),    
							  alignLeftToCenterStr(txEmphEn1, 10),
							  alignLeftToCenterStr(txAmpShft, 11))
			command_data:addResultToResultArray()
			
			-- Fourth resulting string formatting and adding.
			dfeVals1=to_string(dfeVals1)
			dfeVals2=to_string(dfeVals2)
			dfeVals3=to_string(dfeVals3)
			dfeVals4=to_string(dfeVals4)
			dfeVals5=to_string(dfeVals5)
			dfeVals6=to_string(dfeVals6)
			command_data["result"] = 
				string.format("%-10s%-8s%-11s%-11s%-11s%-11s%-11s%-11s", 
							  devnum_portnum_string, 
							  alignLeftToCenterStr(laneNum, 7),    
							  alignLeftToCenterStr(dfeVals1, 10),
							  alignLeftToCenterStr(dfeVals2, 10),    
							  alignLeftToCenterStr(dfeVals3, 10),
							  alignLeftToCenterStr(dfeVals4, 10),
							  alignLeftToCenterStr(dfeVals5, 10),
							  alignLeftToCenterStr(dfeVals6, 10))
			command_data:addResultToResultArray()

			command_data:updatePorts()
			
		end
		-- Resulting table string formatting.
		
		-- First resulting table string formatting.
		command_data:setResultArrayToResultStr(1, 4)
		command_data:setResultStrOnPortCount(header_string1, command_data["result"], 
											footer_string1,
											"There is no port information to show.\n")
                                         
		-- Second resulting table string formatting.
		command_data:addToResultStrOnPortCount(
			header_string2, command_data:getInterlacedResultArray(2, 4), 
			footer_string2)   					
		
		-- Third resulting table string formatting.
		command_data:addToResultStrOnPortCount(
			header_string3, command_data:getInterlacedResultArray(3, 4), 
			footer_string3)  
		
		-- Fourth resulting table string formatting.
		command_data:addToResultStrOnPortCount(
			header_string4, command_data:getInterlacedResultArray(4, 4), 
			footer_string4)  
			
		command_data:analyzeCommandExecution()
		command_data:printCommandExecutionResults()
		
	end
	return command_data:getCommandExecutionResults()
end




-- ************************************************************************
---
--  show_interfaces_serdes_tuning
--        @description  show interfaces serdes tuning
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
local function show_interfaces_serdes_tuning(params)
    local  result, values
    -- Common variables declaration
    local command_data = Command_Data() 
    
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
        result, values = show_interfaces_serdes_tuning_single_port(params, command_data)
		
    else
        result, values = show_interfaces_serdes_tuning_port_range(params, command_data)
    end
    
    return result, values
end

--------------------------------------------------------------------------------
-- command registration: show interfaces serdes tuning
--------------------------------------------------------------------------------

CLI_addCommand("exec", "show interfaces serdes tuning", {
    func = show_interfaces_serdes_tuning,
    help = "Interface(s) serdes tuning",
    params = {
        { type= "named",   {format="serdes_speed %serdes_speed", name="serdes_speed",  help="Defines SERDES speed"  },
              {format="lane_num %lane_number_type", name="laneNum",     help="Serdes lane number"  },
              "#all_interfaces",              
              mandatory={"serdes_speed", "all_interfaces"},
              requirements={
                ["all"] =       {"serdes_speed"},
                ["port-channel"] =  {"serdes_speed"},
                ["ethernet"] =     {"serdes_speed"},
                ["port-channel"] =   {"laneNum"},
                ["laneNum"] =     {"serdes_speed"}
              }
        }
    }
})


