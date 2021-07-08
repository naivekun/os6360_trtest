--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* prbs.lua
--*
--* DESCRIPTION:
--*       enable, disable and show prbs
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

--********************************************************************************

local function Ret_Table_Update(Ret_Table,PrbsStatus,devNum,portNum,laneNum,devPort,val,lockedPtr,errorCntrPtr,patternCntrPtr)

	--print("Ret_Table_Update Ret_Table  = "..to_string(Ret_Table))
	--print("Ret_Table_Update devNum  = "..to_string(devNum))
	--print("Ret_Table_Update portNum  = "..to_string(portNum))
	--print("Ret_Table_Update laneNum  = "..to_string(laneNum))
	--print("Ret_Table_Update PrbsStatus  = "..to_string(PrbsStatus))

    if       (Ret_Table  ~= nil ) 
        and  (devNum     ~= nil ) 
        and  (portNum    ~= nil )  
        and  (laneNum    ~= nil )  
        and  (PrbsStatus ~= nil )  then

        Ret_Table[devNum]                   = tableInsert(Ret_Table[devNum],nil)
        Ret_Table[devNum][portNum]          = tableInsert(Ret_Table[devNum][portNum], nil)
        Ret_Table[devNum][portNum][laneNum] = tableInsert(Ret_Table[devNum][portNum][laneNum], nil)

        Ret_Table[devNum][portNum][laneNum].PrbsStatus = PrbsStatus

        if (PrbsStatus == "Locked" ) then

            --local lockedPtr   =val["lockedPtr"]
            local errorCntrPtr=val["errorCntrPtr"]
            --local patternCntrPtrStr = string.format("0x%X", wrlCpssGtU64StrGet(val["patternCntrPtr"]))

            Ret_Table[devNum][portNum][laneNum].errorCntrPtr   = errorCntrPtr                              
            Ret_Table[devNum][portNum][laneNum].patternCntrPtr = tonumber(wrlCpssGtU64StrGet(val["patternCntrPtr"]))

        else --if (PrbsStatus == "Locked" ) then

            Ret_Table[devNum][portNum][laneNum].errorCntrPtr   = nil
            Ret_Table[devNum][portNum][laneNum].patternCntrPtr = nil

        end  --if (PrbsStatus == "Locked" ) then
        --print("Ret_Table_Update out Ret_Table  = "..to_string(Ret_Table))
        --return Ret_Table
        	
    --else
    --    return nil
    end
    --print("Ret_Table_Update out Ret_Table  = "..to_string(Ret_Table))
    return Ret_Table

end

-- ************************************************************************
---
--  isPrbsEnable
--        @description  chcecks if PRBS is enabled on given devNum, portNum, lane number.
--
--        @param params         devNum:    device number
--                 portNum:     portNum number
--                laneNum:  lane number
--        @return       result of check and value (if result=0)
--  
-- ************************************************************************
local function isPrbsEnable(command_data,devNum, portNum, laneNum)

    local ret=0
    local val
    ret,val = myGenWrapper("cpssDxChDiagPrbsSerdesTestEnableGet",{  --get enable:
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","GT_U32","laneNum",laneNum},
                    {"OUT","GT_BOOL","enablePtr"}
                    })  
    --if 0~=ret then
    --    --print(devNum.."/"..portNum..": Error cpssDxChDiagPrbsSerdesTestEnableGet: "..returnCodes[ret])
    --    command_data:setFailPortAndLocalStatus()
    --    command_data:addError("Error cpssDxChDiagPrbsSerdesTestEnableGet: device %d " ..
    --        "port %d: lane %d: %s", devNum, portNum, laneNum,
    --        returnCodes[ret])
    --end

    return ret,val["enablePtr"]
    
end
-- ************************************************************************

local function PrbsSetDisable(command_data,devNum, portNum,laneNum)

    local ret
    ret = myGenWrapper("cpssDxChDiagPrbsSerdesTestEnableSet",{  --set enable:
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
        {"IN","GT_U32","laneNum",laneNum},
        {"IN","GT_BOOL","enable",0}
        })
    
    if 0~=ret then
        --print(device.."/"..port..": Error cpssDxChDiagPrbsSerdesTestEnableSet: "..returnCodes[ret])

        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error cpssDxChDiagPrbsSerdesTestEnableSet: device %d " ..
                              "port %d: lane %d: %s", devNum, portNum, laneNum,
                              returnCodes[ret])
    end
      
    return ret

end

-- ************************************************************************

local function PrbsSetEnable(command_data,devNum,portNum,laneNum,mode)

    local ret=0
    local val

    ret,val=isPrbsEnable(command_data,devNum, portNum, laneNum)
    if ret==0 then
        if false==val then
            ret = myGenWrapper("cpssDxChDiagPrbsSerdesTransmitModeSet",{  --set mode:
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"IN","CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT","mode",mode}
                })

            if 0~=ret then
                --print(devNum.."/"..portNum..": Error cpssDxChDiagPrbsSerdesTransmitModeSet: "..returnCodes[ret])
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error cpssDxChDiagPrbsSerdesTransmitModeSet: device %d " ..
                                      "port %d: lane %d: %s", devNum, portNum, laneNum,
                                      returnCodes[ret])
            else
                ret = myGenWrapper("cpssDxChDiagPrbsSerdesTestEnableSet",{  --set enable:
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","GT_U32","laneNum",laneNum},
                    {"IN","GT_BOOL","enable",1}
                    })

                --if 0~=ret then
                --    --print(devNum.."/"..portNum..": Error cpssDxChDiagPrbsSerdesTestEnableSet: "..returnCodes[ret])
                --    command_data:setFailPortAndLocalStatus()
                --    command_data:addError("Error cpssDxChDiagPrbsSerdesTestEnableSet: device %d " ..
                --                          "port %d: lane %d: %s", devNum, portNum, laneNum,
                --                          returnCodes[ret])
                --end
            end
        else
            print(devNum.."/"..portNum..": PRBS is already enabled")
        end
    else
    --    print(devNum.."/"..portNum..": Error at PRBS status getting: "..returnCodes[ret])
        command_data:setFailPortAndLocalStatus()
        command_data:addError("Error at PRBS status getting isPrbsEnable: device %d " ..
                              "port %d: lane %d: %s", devNum, portNum, laneNum,
                              returnCodes[ret])
    end

    return ret

end

-- ************************************************************************

local function PrbsPrintLocked(command_data,Ret_Table,devNum, portNum,laneNum,devPort,val,lockedPtr,
				errorCntrPtr,patternCntrPtr,val1,CounterClearOnReadEnablePtr)

    local lockedPtr=val["lockedPtr"]
    local errorCntrPtr=val["errorCntrPtr"]
    local patternCntrPtrStr
    local CounterClearOnReadEnablePtr=val1["CounterClearOnReadEnablePtr"]
    local lockPtrStr

    if tonumber(wrlCpssGtU64StrGet(val["patternCntrPtr"]))== 0 then
        patternCntrPtrStr = string.format("N/A")
    else
        patternCntrPtrStr = string.format("0x%X", wrlCpssGtU64StrGet(val["patternCntrPtr"]))
    end
    if true==lockedPtr then
        if is_sip_5_15(devNum) then
            lockPtrStr =  string.format("Signal detected")
        else
            lockPtrStr =  string.format("Locked         ")
        end
        command_data:updateStatus()
 	if CounterClearOnReadEnablePtr then
		command_data["result"] = string.format("%-6s",devPort).."         " .. lockPtrStr .. "          "
		..string.format("%-10d",errorCntrPtr).."        " .. patternCntrPtrStr .. "                           Yes"
	else
		command_data["result"] = string.format("%-6s",devPort).."         " .. lockPtrStr .. "          "
		..string.format("%-10d",errorCntrPtr).."        " .. patternCntrPtrStr .. "                           No"
	end
       command_data:addResultToResultArray()
        --command_data:updatePorts()
        --print("PrbsPrintLocked 1 Ret_Table  = "..to_string(Ret_Table))
        --print("PrbsPrintLocked 1 Locked")
        Ret_Table = Ret_Table_Update(Ret_Table,"Locked",devNum,portNum,laneNum,devPort,val,lockedPtr,errorCntrPtr,patternCntrPtr)
        --print("PrbsPrintLocked 1 out Ret_Table  = "..to_string(Ret_Table))
    else
        command_data:updateStatus()
        command_data["result"] = string.format("%-6s",devPort).."         Not locked            "
	if CounterClearOnReadEnablePtr then
		command_data["result"] = string.format("%-6s",devPort).."         Not locked            "
		.."                                                   Yes"
	else
		command_data["result"] = string.format("%-6s",devPort).."         Not locked            "
		.."                                                   No"
	end
        command_data:addResultToResultArray()
        --command_data:updatePorts()
        --print("PrbsPrintLocked 2 Ret_Table  = "..to_string(Ret_Table))
        --print("PrbsPrintLocked 1 Not locked")
        Ret_Table = Ret_Table_Update(Ret_Table,"Not locked",devNum,portNum,laneNum,devPort,val,lockedPtr,errorCntrPtr,patternCntrPtr)
        --print("PrbsPrintLocked 2 out Ret_Table  = "..to_string(Ret_Table))
         	
    end
    return Ret_Table

end

-- ************************************************************************

local function PrbsPrintLane(Ret_Table,command_data,devNum,portNum,laneNum)

    local devPort
    local ret,val,val,lockedPtr,errorCntrPtr,patternCntrPtr,val1,CounterClearOnReadEnablePtr

    ret,val=isPrbsEnable(command_data,devNum, portNum, laneNum)
    devPort=tostring(devNum).."/"..tostring(portNum).."/"..tostring(laneNum)

    if 0==ret then
	ret,val1 = myGenWrapper("cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet",
	{
		{"IN","GT_U8","devNum",devNum},
		{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
		{"IN","GT_U32","laneNum",laneNum},
		{"OUT","GT_BOOL","CounterClearOnReadEnablePtr"}
	})
    end
	
    --print("PrbsPrintLane 0 Ret_Table  = "..to_string(Ret_Table))
    if 0==ret then
        if true==val then
            ret,val = myGenWrapper("cpssDxChDiagPrbsSerdesStatusGet",{	--get counter:
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"OUT","GT_BOOL","lockedPtr"},
                {"OUT","GT_U32","errorCntrPtr"},
                {"OUT","GT_U64","patternCntrPtr"}

                })	
            if 0==ret then
                --print("PrbsPrintLane 1 Ret_Table  = "..to_string(Ret_Table))
                Ret_Table = PrbsPrintLocked(command_data,Ret_Table,devNum,portNum,laneNum,devPort,val,lockedPtr,
											errorCntrPtr,patternCntrPtr,val1,CounterClearOnReadEnablePtr)
            else
                command_data:setFailPortAndLocalStatus()
                command_data:addError("Error cpssDxChDiagPrbsSerdesStatusGet: device %d " ..
                                      "port %d: lane %d: %s", devNum, portNum, laneNum,
                                      returnCodes[ret])

            end
        else
            command_data:updateStatus()
            command_data["result"] = string.format("%-6s",devPort).."         Disabled            "
            command_data:addResultToResultArray()
            --command_data:updatePorts()
            --print("PrbsPrintLane 2 Ret_Table  = "..to_string(Ret_Table))
            --print("PrbsPrintLane 2 Not locked")
            Ret_Table = Ret_Table_Update(Ret_Table,"Disabled",devNum,portNum,laneNum,devPort,val,lockedPtr,errorCntrPtr,patternCntrPtr)
        end

    --else
    ----    errorMsg=errorMsg..devNum.."/"..portNum..": Error at PRBS status getting: " ..returnCodes[ret].."\n"
    --    command_data:setFailPortAndLocalStatus()
    --    command_data:addError("Error at PRBS status getting isPrbsEnable: device %d " ..
    --                          "port %d: lane %d: %s", devNum, portNum, laneNum,
    --                          returnCodes[ret])
    end
    return Ret_Table
end

--------------------------------------------------------------------------------
local function SerdeLaneHandler(Ret_Table,command_data,devNum,portNum,laneNum,state,mode)

    local ret=0

    if state == "show" then
        --print("PrbsPrintLane 1 Ret_Table  = "..to_string(Ret_Table))
        Ret_Table = PrbsPrintLane(Ret_Table,command_data,devNum,portNum,laneNum)
        --print("PrbsPrintLane out Ret_Table  = "..to_string(Ret_Table))
        return ret,Ret_Table
    elseif state == "prbs_enable" then
        ret=PrbsSetEnable(command_data,devNum,portNum,laneNum,mode)
        return ret,nil
    else -- prbs_no_enable
        ret=PrbsSetDisable(command_data,devNum,portNum,laneNum)
        return ret,nil
    end  
    return ret,Ret_Table

end
-- ************************************************************************

--------------------------------------------------------------------------------
local function CounterClearOnReadEnableSet(params, devNum,portNum,laneNum)
	local enable = -1
	local ret = 0
	
	if "no-enable-clear-on-read-counter" == params.enableClearOnReadCounter then
		enable = 0
	else
		enable = 1
	end
	if enable ~= -1 then
		ret = myGenWrapper("cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet",
		{
			{"IN","GT_U8","devNum",devNum},
			{"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
			{"IN","GT_U32","laneNum",laneNum},
			{"IN","GT_BOOL","enable",enable}
		})
		if ret ~= 0 then
			if 1 == enable then
				print("device "..devNum.."   port "..portNum.."  lane "..laneNum.. ": Cant't set Prbs Counter Clear On Read enable")
			else
				print("device "..devNum.."   port "..portNum.."  lane "..laneNum.. ": Cant't set Prbs Counter Clear On Read disable")
			end
		end
	end
	return ret
end
-- ************************************************************************

function prbsCommonDevPortLoop(params)
    local portNum,devNum, devPort
    local all_ports, dev_ports            
    local laneNum
    local ret=0
    local val
    local errorMsg=""
    local result, values
    local state=""
    local numOfSerdesLanesPtr=0
    local mode=0
    local header_string=""
    local footer_string=""

    -- Common variables declaration
    local result, values
    local command_data = Command_Data() 
    local Ret_Table = {}


	--print("prbsCommonDevPortLoop params  = "..to_string(params))
    Ret_Table = {}

    -- Common variables initialization
    command_data:initInterfaceDevPortRange()
    command_data:clearResultArray()

    state=params.state
    mode=0

    if state == "prbs_enable" then
        mode=params["mode"]
    elseif state == "show" then
        -- Command specific variables initialization    
        header_string = 
           "\n" ..
           "Dev/Port/Lane Pattern detector    PRBS Error counter    Pattern counter    Counter clear on read enable\n" ..
           "------------- ------------------  --------------------  -----------------  ------------------------------\n"
        footer_string = "\n"   
    end   

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(header_string, 
                                                     footer_string) 

    if true == command_data["status"] then
        all_ports = getGlobal("ifRange")--get table of ports
        laneNum=params["laneNum"]

        for iterator, devNum, portNum in command_data:getPortIterator() do
            --command_data:clearPortStatus()   

            if laneNum == "all" then

                ret,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)
                if( ret == 0 ) then

                    for i=0,numOfSerdesLanesPtr-1,1 do
                        laneNum=i
                        --print("prbsCommonDevPortLoop 1 Ret_Table  = "..to_string(Ret_Table))
                        ret,Ret_Table=SerdeLaneHandler(Ret_Table,command_data,devNum,portNum,laneNum,state,mode)
			if 0 == ret then
				ret=CounterClearOnReadEnableSet(params, devNum,portNum,laneNum)
			end
                        --print("prbsCommonDevPortLoop out Ret_Table  = "..to_string(Ret_Table))
                    end                            
                end
            else
                --print("prbsCommonDevPortLoop 2 Ret_Table  = "..to_string(Ret_Table))
                ret,Ret_Table=SerdeLaneHandler(Ret_Table,command_data,devNum,portNum,laneNum,state,mode)
		if 0 == ret then
			ret=CounterClearOnReadEnableSet(params, devNum,portNum,laneNum)
		end
            end  -- if laneNum == "all"
        end -- for iterator, devNum, portNum

    end -- if true == command_data["status"] then

    if state == "show" then

        command_data:setResultArrayToResultStr()
        command_data:setResultStr(header_string, command_data["result"], footer_string)
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()

    else
        command_data:setResultArrayToResultStr()
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
    end 

	--print("prbsCommonDevPortLoop Ret_Table  = "..to_string(Ret_Table))
	--print("prbsCommonDevPortLoop ret        = "..to_string(ret))
    
    if ret == 0 then
        return true,Ret_Table
    else
        return false,Ret_Table
    end
end

--------------------------------------------------------------------------------
local function prbsShowHandler(params)

    params.state="show"
    return prbsCommonDevPortLoop(params)

end

--------------------------------------------------------------------------------
local function prbsEnableHandler(params)

    if params.flagNo == true then
        params.state="prbs_no_enable"
    else
        params.state="prbs_enable"
    end
    return prbsCommonDevPortLoop(params)

end

--------------------------------------------------------------------------------
-- command registration: prbs enable
--------------------------------------------------------------------------------
CLI_addHelp("interface", "prbs", "Prbs")
CLI_addCommand("interface", "prbs enable", {
  func   = prbsEnableHandler,
  help   = "Configure PRBS",
  params = {{type= "named", {format="lane %LaneNumberOrAll", name="laneNum", help="Serdes lane number or all"},
                            {format="mode %prbs_mode", name="mode", help="PRBS mode"},
							{format="no-enable-clear-on-read-counter", name = "enableClearOnReadCounter", help="Disable clear on read counter"}
                            , mandatory={"mode", "laneNum"}}
  }
})

--------------------------------------------------------------------------------
-- command registration: no prbs
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no prbs enable", {
    func=function(params)
        params.flagNo=true
        return prbsEnableHandler(params)
    end,
    help   = "Disable prbs",
    params ={{type= "named", {format="lane %LaneNumberOrAll", name="laneNum", help="Serdes lane number or all"}
                            , mandatory={"laneNum"}}

  }}
)

--------------------------------------------------------------------------------
-- command registration: prbs show
--------------------------------------------------------------------------------

CLI_addCommand("interface", "prbs show", {
  func   = prbsShowHandler,
  help   = "Show PRBS counter",
  params = {{type= "named", {format="lane %LaneNumberOrAll", name="laneNum", help="Serdes lane number or all"}
                            , mandatory={"laneNum"}}
  }
})
--------------------------------------------------------------------------------





