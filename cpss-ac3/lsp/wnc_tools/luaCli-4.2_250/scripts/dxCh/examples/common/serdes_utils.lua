--********************************************************************************
--*              (C), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_utils.lua
--*
--* DESCRIPTION:
--*       Any functions, variables etc that are required in several tests.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


--*****************************************************************************
local function check_expected_boolean_value(Device, Port, txt, val)


  local isSuccess   = false

  if val ~= nil then
      if type(val) == "boolean" then
          isSuccess = true
      end
  end

  if isSuccess ~= true then

    isSuccess = false
    printMsgLvlLog("errors", string.format("Failed: Device=%s Port=%s %s=%s is not boolean",
          to_string(Device),
          to_string(Port),
          txt,
          to_string(val)))

  else
     printMsgLvlLog("information", string.format("Passed: Device=%s Port=%s %s=%s is boolean",
            to_string(Device),
            to_string(Port),
            txt,
            to_string(val)))

  end -- if val_Rx ~= expected_Rx then

  return isSuccess;
      end

--********************************************************************************
local function check_expected_numerics_value(Device, Port, txt, val)


    local isSuccess   = false

    if val ~= nil then
        if type(val) == "number" then
            isSuccess = true
        end
    end

    if isSuccess ~= true then

      isSuccess = false
      printMsgLvlLog("errors", string.format("Failed: Device=%s Port=%s %s=%s is not number",
            to_string(Device),
            to_string(Port),
            txt,
            to_string(val)))

    else
       printMsgLvlLog(6, string.format("Passed: Device=%s Port=%s %s=%s is number",
              to_string(Device),
              to_string(Port),
              txt,
              to_string(val)))

    end -- if val_Rx ~= expected_Rx then

    return isSuccess;
end

--********************************************************************************

local function check_expected_text_1(Device, Port, Lane, invert_txt, val, expected)


    local isSuccess = true

    if val ~= expected then

      isSuccess = false

      printMsgLvlLog("errors", string.format("Failed: Device=%s Port=%s Lane=%s %s is %s expected %s",
            to_string(Device),
            to_string(Port),
            to_string(Lane),
            invert_txt,
            to_string(val),
            to_string(expected)))

    else

      printMsgLvlLog("information", string.format("Passed: Device=%s Port=%s Lane=%s %s is %s",
            to_string(Device),
            to_string(Port),
            to_string(Lane),
            invert_txt,
            to_string(expected)))

    end -- if val_Rx ~= expected_Rx then

    return isSuccess;
end

--********************************************************************************

local function check_expected_serdes_polarity_all(devNum, LaneNum, params, expected_Tx, expected_Rx)

    local testStillOk = true
	local ret = 0

    local isSuccess = true
    local Device,  Port, Invert, Value, Lane, val, expected
    executeStringCliCommands("end");
    executeStringCliCommands("configure");
    executeStringCliCommands("interface range ethernet all");
    --executeStringCliCommands("serdes reset")
    --executeStringCliCommands("no serdes reset")

    executeStringCliCommands("serdes polarity lane_num all invert_tx "..to_string(expected_Tx).." invert_rx "..to_string(expected_Rx));
    executeStringCliCommands("end");

    --executeStringCliCommands("show interfaces serdes polarity lane_num all all");
    --executeStringCliCommands("end");

    ret, Ret_Table = show_interfaces_serdes_polarity(params)
    --print("Ret_Table  = "..to_string(Ret_Table))


    if ret ~= true then
        local testStillOk = false
    else
        if Ret_Table ~= nil then

            Device = devNum
            Lane   = LaneNum

            for _, Port in ipairs(Serdes_Port_Lst) do

                val_Tx   = Ret_Table[Device][Port][Lane].invertTx
                val_Rx   = Ret_Table[Device][Port][Lane].invertRx

                val=val_Tx
                invert_txt="invertTx"
                expected=expected_Tx
                isSuccess = check_expected_text_1(Device, Port, Lane, invert_txt, val, expected)
                if (isSuccess == false) then
                    testStillOk = false
                end

                val=val_Rx
                invert_txt="invertRx"
                expected=expected_Rx
                isSuccess = check_expected_text_1(Device, Port, Lane, invert_txt, val, expected)
                if (isSuccess == false) then
                    testStillOk = false
                end
            end

        end --if Ret_Table ~= nil then
    end -- if ret ~= true then

    return testStillOk;
end

--********************************************************************************

local function check_expected_serdes_polarity_trigger(devNum, LaneNum, polarity_trigger)

    local testStillOk = true
	local ret = 0

    local isSuccess = true
    local prm = nil
    local trigger
    local expected_Tx
    local expected_Rx
    local Device,  Port, Invert, Value, Lane, val, expected

    Device = devNum
    Lane   = LaneNum

    trigger=polarity_trigger

    for _, Port in ipairs(Serdes_Port_Lst) do

        isSuccess = true

        if trigger == true then
            expected_Tx=true
            expected_Rx=false
        else
            expected_Tx=false
            expected_Rx=true
        end

        --print("Ret_Table[Device]["..Port.."][Lane].invertTx   = "..to_string(Ret_Table[Device][Port][Lane].invertTx))
        --print("Ret_Table[Device]["..Port.."][Lane].invertRx   = "..to_string(Ret_Table[Device][Port][Lane].invertRx))

        executeStringCliCommands("end");
        executeStringCliCommands("configure");
        executeStringCliCommands("interface ethernet "..to_string(devNum).."/"..to_string(Port));
        --executeStringCliCommands("serdes reset")
        --executeStringCliCommands("no serdes reset")
        executeStringCliCommands("serdes polarity lane_num "..to_string(LaneNum).." invert_tx "..to_string(expected_Tx).." invert_rx "..to_string(expected_Rx));
        --print("serdes polarity lane_num "..to_string(LaneNum).." invert_tx "..to_string(expected_Tx).." invert_rx "..to_string(expected_Rx))
        executeStringCliCommands("end");

        prm = {}
        prm["all_interfaces"] = "ethernet"
        prm["laneNum"]          = 0
        prm["ethernet"] = nil
        prm["ethernet"] = {[devNum]={ Port }}

        ret, Ret_Table = show_interfaces_serdes_polarity(prm)

        --print("Ret_Table  = "..to_string(Ret_Table))
        --print("ret        = "..to_string(ret))

        if ret ~= true then
            local testStillOk = false
        else
            if Ret_Table ~= nil then
                val_Tx   = Ret_Table[Device][Port][Lane].invertTx
                val_Rx   = Ret_Table[Device][Port][Lane].invertRx

                val=val_Tx
                invert_txt="invertTx"
                expected=expected_Tx
                isSuccess = check_expected_text_1(Device, Port, Lane, invert_txt, val, expected)
                if (isSuccess == false) then testStillOk = false;  end


                val=val_Rx
                invert_txt="invertRx"
                expected=expected_Rx
                isSuccess = check_expected_text_1(Device, Port, Lane, invert_txt, val, expected)
                if (isSuccess == false) then testStillOk = false;  end
            end --if Ret_Table ~= nil then
        end

        if trigger == true then
            trigger=false
        else
            trigger=true
        end

        if (isSuccess == false) then
            testStillOk = false
        end

    end

    return testStillOk;

end

--********************************************************************************
local function check_expected_numerics_lane_value(Device, Port, laneNum, txt, val)


    local isSuccess   = false

    if val ~= nil then
        if type(val) == "number" then
            isSuccess = true
        end
    end

    if isSuccess ~= true then

      isSuccess = false
      printMsgLvlLog("errors", string.format("Failed: Device=%s Port=%s Lane=%s %s=%s is not number",
        to_string(Device),
        to_string(Port),
        to_string(laneNum),
        txt,
        to_string(val)))

    else
       printMsgLvlLog("information", string.format("Passed: Device=%s Port=%s Lane=%s %s=%s is number",
            to_string(Device),
            to_string(Port),
            to_string(laneNum),
            txt,
            to_string(val)))

    end -- if val_Rx ~= expected_Rx then

    return isSuccess;
end

-- ************************************************************************
local function isPrbsEnable1(devNum, portNum, laneNum)

    local ret=0
    local val
    ret,val = myGenWrapper("cpssDxChDiagPrbsSerdesTestEnableGet",{	--get enable:
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN","GT_U32","laneNum",lane},
                    {"OUT","GT_BOOL","enablePtr"}
                    })

    return ret,val["enablePtr"]

end
--********************************************************************************

local function Check_prbs_mode_1(Ret_Table,devNum,Port,LaneNum,mode)

    local testStillOk = true
    local ret = 0
    local Parm
    local val
    local ParmList_Count = 0

    local isSuccess = true
    local Invert, Value, Lane, val, expected

    --ret,val=isPrbsEnable1(devNum, portNum, laneNum)

    --printLog("mode = "..to_string(mode))
    --printLog("Ret_Table = "..to_string(Ret_Table))
    --printLog("Ret_Table[devNum] = "..to_string(Ret_Table[devNum]))
    --printLog("Ret_Table[devNum][Port] = "..to_string(Ret_Table[devNum][Port]))


    --printMsgLvlLog("debug", string.format("isPrbsEnable1: devNum=%s Port=%s LaneNum=%s ret=%s mode=%s",
    --to_string(devNum),
    --     to_string(Port),
    --     to_string(LaneNum),
    --     to_string(ret),
    --     to_string(mode)))

    --if ret==0 then
        --if val ~= nil then
            if Ret_Table[devNum] ~= nil then
                if Ret_Table[devNum][Port] ~= nil then
                    if Ret_Table[devNum][Port][LaneNum] ~= nil then
                        if Ret_Table[devNum][Port][LaneNum].PrbsStatus ~= nil then

                            --printMsgLvlLog("debug", string.format("Ok: devNum=%s Port=%s LaneNum=%s prbs Status is %s",
                            --    to_string(devNum),
                            --    to_string(Port),
                            --    to_string(LaneNum),
                            --    Ret_Table[devNum][Port][LaneNum].PrbsStatus))

                            if (mode == "Disabled") then
                                isSuccess = check_expected_text_1(devNum, Port, LaneNum, "PRBS", Ret_Table[devNum][Port][LaneNum].PrbsStatus,mode)
                                --printLog("Disabled isSuccess = "..to_string(isSuccess))
                                if (isSuccess == false) then
                                    testStillOk = false
                                end
                            else --if (mode == "Disabled") then

                                if (mode == "Locked") then

                                    --isSuccess = check_expected_text_1(devNum, Port, LaneNum, "PRBS", Ret_Table[devNum][Port][LaneNum].PrbsStatus,"Not locked")
                                    --printLog("Not locked isSuccess = "..to_string(isSuccess))
                                    --if (isSuccess == true) then
                                    if (Ret_Table[devNum][Port][LaneNum].PrbsStatus == "Not locked") then
                                        testStillOk = true
                                    else --"Not locked"
                                        isSuccess = check_expected_text_1(devNum, Port, LaneNum, "PRBS", Ret_Table[devNum][Port][LaneNum].PrbsStatus,"Locked")
                                        --printLog("Locked isSuccess = "..to_string(isSuccess))
                                        if (isSuccess == false) then
                                            testStillOk = false
                                        else
                                            isSuccess = check_expected_numerics_lane_value(devNum, Port, LaneNum, "errorCntrPtr", Ret_Table[devNum][Port][LaneNum].errorCntrPtr )
                                            if (isSuccess == false) then
                                                testStillOk = false
                                            end
                                            isSuccess = check_expected_numerics_lane_value(devNum, Port, LaneNum, "patternCntrPtr", Ret_Table[devNum][Port][LaneNum].patternCntrPtr )
                                            if (isSuccess == false) then
                                                testStillOk = false
                                            end
                                        end
                                    end --"Not locked"
                                end --if (mode == "Locked") then
                            end --if (mode == "Disabled") then

                        else
                            printMsgLvlLog("errors", string.format("Check_prbs_mode_1 Failed 602: devNum=%s Port=%s LaneNum=%s prbs Status is nil",
                                to_string(devNum),
                                to_string(Port),
                                to_string(LaneNum)))
                            testStillOk = false
                        end --if Ret_Table[devNum][Port][LaneNum].PrbsStatus ~= nil then

                    else
                        printMsgLvlLog("errors", string.format("Check_prbs_mode_1 Failed 610: devNum=%s Port=%s LaneNum=%s prbs show result is nil",
                                to_string(devNum),
                                to_string(Port),
                                to_string(LaneNum)))
                        testStillOk = false
                    end   --if Ret_Table[devNum][Port][LaneNum] ~= nil then
                else

                    printMsgLvlLog("errors", string.format("Check_prbs_mode_1 Failed 618: devNum=%s Port=%s prbs show result is %s",
                        to_string(devNum),
                        to_string(Port),
                        to_string(Ret_Table[devNum][Port])))
                    testStillOk = false
                end --if Ret_Table[devNum][Port] ~= nil then

            else
                printMsgLvlLog("errors", string.format("Check_prbs_mode_1 Failed 625: devNum=%s prbs show result is nil",
                    to_string(devNum)))
                testStillOk = false
            end --if Ret_Table[devNum] ~= nil then

        --else --if val ~= nil then
        --    --print(devNum.."/"..portNum.."/"..laneNum..": PRBS is already enabled")
        --    printMsgLvlLog("errors", string.format("Check_prbs_mode_1 Failed 632: devNum=%s Port=%s LaneNum=%s prbs is enabled",
        --            to_string(devNum),
        --            to_string(Port),
        --            to_string(LaneNum)))
        --    testStillOk = false
        --
        --end --if val ~= nil then
    --else
    --
    --    if (mode == "Disabled") then
    --        isSuccess = check_expected_text_1(devNum, Port, LaneNum, "PRBS", Ret_Table[devNum][Port][LaneNum].PrbsStatus,mode)
    --        if (isSuccess == false) then
    --            printMsgLvlLog("errors", string.format("Check_prbs_mode_1 Failed 661: devNum=%s Port=%s LaneNum=%s prbs show %s",
    --                    to_string(devNum),
    --                    to_string(Port),
    --                    to_string(LaneNum),
    --                    returnCodes[ret]))
    --            testStillOk = false
    --        end
    --    else --if (mode == "Disabled") then
    --        printMsgLvlLog("errors", string.format("Check_prbs_mode_1 Failed 672: devNum=%s Port=%s LaneNum=%s prbs show %s",
    --                to_string(devNum),
    --                to_string(Port),
    --                to_string(LaneNum),
    --                returnCodes[ret]))
    --        testStillOk = false
    --    end --if (mode == "Disabled") then
    --
    --
    --end -- if ret==0 then
    --print("#auto_Tune_List  = "..to_string(#auto_Tune_List))
    --print("Check_prbs_mode_1 testStillOk  = "..to_string(testStillOk))
    return testStillOk;
end


--********************************************************************************

local function Check_auto_tune_show(devNum,PortNum,LaneNum)

    local testStillOk = true
    local ret = 0
    local Parm
    local ParmList_Count = 0

    local isSuccess = true
    local Device,  Port, Invert, Value, Lane, val, expected
    local params = {}
    local Ret_Table = {}

    local auto_Tune_Bobcat2_Bobk_List      = {"DC","LF","HF","BW","LB","EO","DFE_0","DFE_1","DFE_2","DFE_3","DFE_4","DFE_5","DFE_6","DFE_7","DFE_8","DFE_9","DFE_10","DFE_11","DFE_12"}
    local auto_Tune_Bobcat2_Bobk_Bool_List = {}

    local auto_Tune_Other_List        = {"align90","dfe","ffeC","ffeR","ffeS","sampler","sqlch","txAmp","txAmpAdj","txEmph1","txEmphAmp","dfeValsArray_0","dfeValsArray_1","dfeValsArray_2","dfeValsArray_3","dfeValsArray_4","dfeValsArray_5"}
    local auto_Tune_Other_Bool_List   = {"txAmpShft","txEmphEn","txEmphEn1"}

    local devFamily, devSubFamily = wrlCpssDeviceFamilyGet(devNum)
    if (devSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E"
        or devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E"
        or devFamily == "CPSS_PP_FAMILY_DXCH_AC3X_E")
    then
        ParmList_Count      = #auto_Tune_Bobcat2_Bobk_List
        auto_Tune_List      =  auto_Tune_Bobcat2_Bobk_List
        auto_Tune_Bool_List =  auto_Tune_Bobcat2_Bobk_Bool_List
    else
        ParmList_Count      = #auto_Tune_Other_List
        auto_Tune_List      =  auto_Tune_Other_List
        auto_Tune_Bool_List =  auto_Tune_Other_Bool_List
    end
    print("#auto_Tune_List  = "..to_string(#auto_Tune_List))


    executeStringCliCommands("end");
    executeStringCliCommands("configure");

    if PortNum == "all" then
        executeStringCliCommands("interface range ethernet all");
    else
        Port = PortNum
        executeStringCliCommands("interface ethernet "..devNum.."/"..PortNum);
    end

    --executeStringCliCommands("serdes reset")
    --executeStringCliCommands("no serdes reset")
    --executeStringCliCommands("auto-tune show");
    ret, Ret_Table = autoTuneShow(params)

    if ret ~= true then
        testStillOk = false

        printMsgLvlLog("errors", string.format("Failed: Device=%s autoTune Show return is false",
            to_string(Device)))
        testStillOk = false

    else
        if Ret_Table ~= nil then

            Device = devNum
            Lane   = LaneNum

            if PortNum == "all" then

                for Indx, Port in ipairs(Serdes_Port_Lst) do
                    if Ret_Table[devNum][Port] ~= nil then
                        for i = 1,#auto_Tune_List do
                            Parm = auto_Tune_List[i];
                            if Ret_Table[devNum][Port][Parm] ~= nil then
                                isSuccess = check_expected_numerics_value(Device, Port, Parm, Ret_Table[devNum][Port][Parm])
                                testStillOk  = warning_handle("Check_auto_tune_show "..devNum.."/"..Port.."/"..LaneNum, isSuccess, testStillOk )
                            else
                                testStillOk = false
                            end
                        end -- for i = 1,#auto_Tune_List do

                        for i = 1,#auto_Tune_Bool_List do
                            Parm = auto_Tune_Bool_List[i];
                            if Ret_Table[devNum][Port][Parm] ~= nil then
                                isSuccess = check_expected_boolean_value(Device, Port, Parm, Ret_Table[devNum][Port][Parm])
                                testStillOk  = warning_handle("Check_auto_tune_show "..devNum.."/"..Port.."/"..LaneNum, isSuccess, testStillOk )
                            else
                                testStillOk = false
                            end
                        end -- for i = 1,#auto_Tune_Bool_List do
                    else
                        printMsgLvlLog("errors", string.format("Failed: Device=%s Port=%s autoTune Show result is nil",
                            to_string(Device),
                            to_string(Port)))
                        testStillOk  = warning_handle("Check_auto_tune_show "..devNum.."/"..Port.."/"..LaneNum, false, testStillOk )
                        testStillOk = false
                    end
               end --for Indx, Port in ipairs(Serdes_Port_Lst) do
            else --if PortNum == "all" then

                Port=PortNum
                if Ret_Table[devNum][Port] ~= nil then
                    for i = 1,#auto_Tune_List do
                        Parm = auto_Tune_List[i];
                        if Ret_Table[devNum][Port][Parm] ~= nil then
                            isSuccess = check_expected_numerics_value(Device, Port, Parm, Ret_Table[devNum][Port][Parm])
                            testStillOk  = warning_handle("Check_auto_tune_show "..devNum.."/"..Port.."/"..LaneNum, isSuccess, testStillOk )
                        else
                            testStillOk = false
                        end
                    end -- for i = 1,#auto_Tune_List do

                    for i = 1,#auto_Tune_Bool_List do
                        Parm = auto_Tune_Bool_List[i];
                        if Ret_Table[devNum][Port][Parm] ~= nil then
                            isSuccess = check_expected_boolean_value(Device, Port, Parm, Ret_Table[devNum][Port][Parm])
                            testStillOk  = warning_handle("Check_auto_tune_show "..devNum.."/"..PortNum.."/"..LaneNum, isSuccess, testStillOk )
                        else
                            testStillOk = false
                        end
                    end -- for i = 1,#auto_Tune_Bool_List do
                else
                    printMsgLvlLog("errors", string.format("Failed: Device=%s Port=%s autoTune Show result is nil",
                        to_string(Device),
                        to_string(Port)))
                    testStillOk  = warning_handle("Check_auto_tune_show "..devNum.."/"..PortNum.."/"..LaneNum, false, testStillOk )
                    testStillOk = false
                end
            end --if PortNum == "all" then
        else --if Ret_Table ~= nil then
            printMsgLvlLog("errors", string.format("Failed: Device=%s autoTune Show result is nil",
                to_string(Device)))
            testStillOk = false

        end --if Ret_Table ~= nil then
    end -- if ret ~= true then

    executeStringCliCommands("end");
    --print("#auto_Tune_List  = "..to_string(#auto_Tune_List))
    return testStillOk;
end

--********************************************************************************
local function Check_serdes_reset_port_all(params,devNum,PortNum,txt)

    local testStillOk = true
    local Port

    if PortNum == "all" then
        for _, Port in ipairs(Serdes_Port_Lst) do

            serdes_env.devNum   = devNum
            serdes_env.PortNum  = Port
            --print("serdes_env.PortNum  = "..to_string(serdes_env.PortNum))

            if txt == "no" then
                  printMsgLvlLog("debug", string.format("serdes no reset: devNum=%s Port=%s ",to_string(devNum), to_string(Port)))
            else
                executeLocalConfig("dxCh/examples/configurations/serdes_reset.txt")
            end
        end --for _, Port in ipairs(Serdes_Port_Lst) do
    else
        Port = PortNum

        serdes_env.PortNum  = Port
        --print("serdes_env.PortNum  = "..to_string(serdes_env.PortNum))

        if txt == "no" then
              printMsgLvlLog("debug", string.format("serdes no reset: devNum=%s Port=%s ",to_string(devNum), to_string(Port)))
        else
            executeLocalConfig("dxCh/examples/configurations/serdes_reset.txt")
        end
    end
    return testStillOk;

end

--********************************************************************************

local function Check_serdes_show_temper_voltage(params,devNum,PortNum,txt)

    local testStillOk = true
    local ret = 0

    local isSuccess   = true
    local Ret_Table
    local Port, Invert, Value, val, expected

    executeStringCliCommands("end");

    if txt == "temperature" then
        ret, Ret_Table = show_interfaces_serdes_temperature(params)
    elseif txt == "voltage" then
        ret, Ret_Table = show_interfaces_serdes_voltage(params)
    end

    --print("ret        = "..to_string(ret))

    if ret ~= true then
        local testStillOk = false
    else
        if Ret_Table ~= nil then
            if Ret_Table[devNum] ~= nil then
                if PortNum == "all" then
                    for _, Port in ipairs(Serdes_Port_Lst) do

                        if Ret_Table[devNum][Port] ~= nil then
                            if txt == "temperature" then
                                val   = Ret_Table[devNum][Port].temperature
                            elseif txt == "voltage" then
                                val   = Ret_Table[devNum][Port].voltage
                            end

                            isSuccess = check_expected_numerics_value(devNum, Port, txt, val)
                            if (isSuccess == false) then
                                testStillOk = false
                            end
                        else
                            testStillOk = false
                        end
                    end --for _, Port in ipairs(Serdes_Port_Lst) do
                else
                    Port =  PortNum

                    if Ret_Table[devNum][Port] ~= nil then
                        if txt == "temperature" then
                            val   = Ret_Table[devNum][Port].temperature
                        elseif txt == "voltage" then
                            val   = Ret_Table[devNum][Port].voltage
                        end

                        isSuccess = check_expected_numerics_value(devNum, Port, txt, val)
                        if (isSuccess == false) then
                            testStillOk = false
                        end
                    else
                        testStillOk = false
                    end
                end --if PortNum == "all" then

            else
                testStillOk = false
            end
        else
            testStillOk = false
        end --if Ret_Table ~= nil then
    end

    return testStillOk;

end

--********************************************************************************

local function check_serdes_polarity_port_1(devNum,PortNum, LaneNum, invert_Tx, invert_Rx )

    local testStillOk = true
    local ret = 0

    local isSuccess = true
    local prm = nil
    --local trigger
    --local expected_Tx
    --local expected_Rx
    local Device,  Port, Invert, Value, Lane, val, expected

    Device = devNum
    Lane   = LaneNum

    Port = PortNum

    serdes_env.devNum     = devNum
    serdes_env.PortNum    = Port
    serdes_env.LaneNum    = LaneNum
    serdes_env.Invert_Tx  = to_string(invert_Tx)
    serdes_env.Invert_Rx  = to_string(invert_Rx)

    --print("check_serdes_polarity_port_1 Port        = "..to_string(Port))
    --print("check_serdes_polarity_port_1 Invert_Tx        = "..to_string(Invert_Tx))
    --print("check_serdes_polarity_port_1 invert_Rxc           = "..to_string(invert_Rx))
    executeLocalConfig("dxCh/examples/configurations/serdes_polarity.txt")

    prm = {}
    prm["all_interfaces"] = "ethernet"
    prm["laneNum"]          = 0
    prm["ethernet"] = nil
    prm["ethernet"] = {[devNum]={ Port }}

    ret, Ret_Table = show_interfaces_serdes_polarity(prm)

    --print("Ret_Table  = "..to_string(Ret_Table))
    --print("ret        = "..to_string(ret))

    if ret ~= true then
        local testStillOk = false
    else
        if Ret_Table ~= nil then
            val_Tx   = Ret_Table[Device][Port][Lane].invertTx
            val_Rx   = Ret_Table[Device][Port][Lane].invertRx

            val=val_Tx
            invert_txt="invertTx"
            expected=invert_Tx
            isSuccess = check_expected_text_1(Device, Port, Lane, invert_txt, val, expected)
            if (isSuccess == false) then testStillOk = false;  end

            val=val_Rx
            invert_txt="invertRx"
            expected= invert_Rx
            isSuccess = check_expected_text_1(Device, Port, Lane, invert_txt, val, expected)
            if (isSuccess == false) then testStillOk = false;  end
        end --if Ret_Table ~= nil then
    end

    if (isSuccess == false) then
        testStillOk = false
    end

    return testStillOk;

end

--********************************************************************************

local function check_serdes_polarity_for_ports(devNum,PortNum, LaneNum,exp_Tx, exp_Rx,trigger)

    local testStillOk = true
    local Port, expected_Tx, expected_Rx
    --local ret = 0

    --local trigger
    local isSuccess = true
    --local Device,  Port, Invert, Value, Lane, val, expected
    --local params = {}

    expected_Tx=exp_Tx
    expected_Rx=exp_Rx

    if PortNum == "all" then
        for _, Port in ipairs(Serdes_Port_Lst) do

            isSuccess = check_serdes_polarity_port_1(devNum,Port, LaneNum, expected_Tx, expected_Rx )
            if (isSuccess == false) then testStillOk = false;  end

            if trigger ~= nil then
                if expected_Rx == true then
                    --trigger=false
                    expected_Tx=true
                    expected_Rx=false
                else
                    --trigger=true
                    expected_Tx=false
                    expected_Rx=true
                end
            end --if trigger ~= nil then
        end --for _, Port in ipairs(Serdes_Port_Lst) do
    else --if PortNum == "all" then

        Port = PortNum
        isSuccess = check_serdes_polarity_port_1(devNum,PortNum, LaneNum, expected_Tx, expected_Rx )
        if (isSuccess == false) then testStillOk = false;  end

    end --if PortNum == "all" then

    return testStillOk;
end

--********************************************************************************

local function check_serdes_polarity_port_trigger(devNum,PortNum, LaneNum, polarity_trigger)

    local testStillOk = true
    local ret = 0

    local isSuccess = true
    local prm = nil
    local trigger
    local expected_Tx
    local expected_Rx
    local Device,  Port, Invert, Value, Lane, val, expected

    Device = devNum
    Lane   = LaneNum

    trigger=polarity_trigger

    if trigger == true then
        expected_Tx=true
        expected_Rx=false
    else
        expected_Tx=false
        expected_Rx=true
    end


    isSuccess =  check_serdes_polarity_for_ports(devNum,PortNum, LaneNum,expected_Tx, expected_Rx,trigger)
    if (isSuccess == false) then testStillOk = false;  end

    return testStillOk;

end


--********************************************************************************

local function Check_prbs_for_port_1(devNum,Port,LaneNum,mode)

    local testStillOk = true
    local ret = 0
    local Parm
    local ParmList_Count = 0

    local isSuccess = true
    local Device, Invert, Value, Lane, val, expected
    local params = {}
    local Ret_Table = {}

    --printMsgLvlLog("debug", string.format("PRBS expect status="..mode))

    params={ laneNum="all", state="show" }
    --print("Check_prbs_for_port_1 params  = "..to_string(params))

    ret, Ret_Table = prbsCommonDevPortLoop(params)
    --print("Check_prbs_for_port_1 Ret_Table  = "..to_string(Ret_Table))


    if ret ~= true then
        testStillOk = false

        printMsgLvlLog("errors", string.format("Failed: Device=%s prbs show return is false",
            to_string(Device)))
        testStillOk = false

    else --if ret ~= true then
        if Ret_Table == nil then
            printMsgLvlLog("errors", string.format("Failed: Device=%s prbs show result is nil",
                to_string(Device)))
            testStillOk = false
        else --if Ret_Table == nil then
            --print("Check_prbs_for_port_1 Ret_Table  = "..to_string(Ret_Table))
            Device = devNum
            Lane   = 0

            isSuccess = Check_prbs_mode_1(Ret_Table,devNum,Port,Lane,mode)
            if (isSuccess == false) then testStillOk = false;  end

        end --if Ret_Table == nil then

    end --if ret ~= true then

    return testStillOk;
end


--********************************************************************************

local function Check_prbs_set_show_interface_mode(devNum,PortNum,LaneNum,mode,No_Flag)

    local testStillOk = true
    local ret = 0
    local ParmList_Count = 0

    local isSuccess = true
    local Invert, Value, Lane, val, expected
    local Port


    if mode ~= nil and PortNum ~= nil then
        serdes_env.devNum     = devNum

        if No_Flag ~= "no" then
            if mode == "all" then
                if Transmit_Mode_List ~= nil then
                    for i = 1,#Transmit_Mode_List do

                        serdes_env.Transmit_Mode = Transmit_Mode_List[i]
                        if PortNum == "all" then

                            for _, Port in ipairs(Serdes_Port_Lst) do
                                serdes_env.PortNum       = Port
                                executeLocalConfig("dxCh/examples/configurations/serdes_prbs.txt")
                                isSuccess = Check_prbs_for_port_1(devNum,Port,LaneNum,"Locked")
                                testStillOk  = warning_handle("Check_serdes_prbs_set_show "..devNum.."/"..Port.."/"..LaneNum.." mode="..serdes_env.Transmit_Mode, isSuccess, testStillOk )
                                executeLocalConfig("dxCh/examples/configurations/serdes_prbs_deconfig.txt")
                            end --for _, Port in ipairs(Serdes_Port_Lst) do

                        else --if PortNum == "all" then

                            Port = PortNum
                            serdes_env.PortNum       = Port
                            executeLocalConfig("dxCh/examples/configurations/serdes_prbs.txt")
                            isSuccess = Check_prbs_for_port_1(devNum,PortNum,LaneNum,"Locked")
                            testStillOk  = warning_handle("Check_serdes_prbs_set_show "..devNum.."/"..PortNum.."/"..LaneNum.." mode="..serdes_env.Transmit_Mode, isSuccess, testStillOk )
                            executeLocalConfig("dxCh/examples/configurations/serdes_prbs_deconfig.txt")

                            --isSuccess = check_serdes_polarity_port_1(devNum,PortNum, LaneNum, expected_Tx, expected_Rx )
                            if (isSuccess == false) then testStillOk = false;  end
                        end --if PortNum == "all" then


                    end --for i = 1,#Transmit_Mode_List do
                end  -- if Transmit_Mode_List ~= nil then

            else -- if mode == "all" then

                serdes_env.Transmit_Mode = mode
                if PortNum == "all" then

                    for _, Port in ipairs(Serdes_Port_Lst) do
                        serdes_env.PortNum       = Port
                        executeLocalConfig("dxCh/examples/configurations/serdes_prbs.txt")
                        isSuccess = Check_prbs_for_port_1(devNum,Port,LaneNum,"Locked")
                        testStillOk  = warning_handle("Check_serdes_prbs_set_show "..devNum.."/"..Port.."/"..LaneNum.." mode="..serdes_env.Transmit_Mode, isSuccess, testStillOk )
                        executeLocalConfig("dxCh/examples/configurations/serdes_prbs_deconfig.txt")
                    end --for _, Port in ipairs(Serdes_Port_Lst) do

                else --if PortNum == "all" then

                    Port = PortNum
                    serdes_env.PortNum       = Port
                    executeLocalConfig("dxCh/examples/configurations/serdes_prbs.txt")
                    isSuccess = Check_prbs_for_port_1(devNum,PortNum,LaneNum,"Locked")
                    testStillOk  = warning_handle("Check_serdes_prbs_set_show "..devNum.."/"..PortNum.."/"..LaneNum.." mode="..serdes_env.Transmit_Mode, isSuccess, testStillOk )
                    executeLocalConfig("dxCh/examples/configurations/serdes_prbs_deconfig.txt")

                    --isSuccess = check_serdes_polarity_port_1(devNum,PortNum, LaneNum, expected_Tx, expected_Rx )
                    if (isSuccess == false) then testStillOk = false;  end
                end --if PortNum == "all" then


            end  -- if mode == "all" then
        else -- if No_Flag ~= "no" then            --serdes_polarity_deconfig.txt

            serdes_env.Transmit_Mode = mode
            if PortNum == "all" then

                for _, Port in ipairs(Serdes_Port_Lst) do
                    serdes_env.PortNum       = Port
                    executeLocalConfig("dxCh/examples/configurations/serdes_prbs_deconfig.txt")
                    isSuccess = Check_prbs_for_port_1(devNum,Port,LaneNum,"Disabled")
                    testStillOk  = warning_handle("Check_serdes_prbs_set_show "..devNum.."/"..Port.."/"..LaneNum.." mode="..mode, isSuccess, testStillOk )
                end --for _, Port in ipairs(Serdes_Port_Lst) do

            else --if PortNum == "all" then

                Port = PortNum
                serdes_env.PortNum       = Port
                executeLocalConfig("dxCh/examples/configurations/serdes_prbs_deconfig.txt")
                isSuccess = Check_prbs_for_port_1(devNum,PortNum,LaneNum,"Disabled")
                testStillOk  = warning_handle("Check_serdes_prbs_set_show "..devNum.."/"..PortNum.."/"..LaneNum.." mode="..mode, isSuccess, testStillOk )

                --isSuccess = check_serdes_polarity_port_1(devNum,PortNum, LaneNum, expected_Tx, expected_Rx )
                if (isSuccess == false) then testStillOk = false;  end
            end --if PortNum == "all" then
        end -- if No_Flag ~= "no" then

    else --if mode ~= nil and PortNum ~= nil then
        printMsgLvlLog("errors", string.format("Check_prbs_set_show_interface_mode Failed: devNum=%s PortNum=%s LaneNum=%s %s prbs mode or port is nil",
                to_string(devNum),
                to_string(PortNum),
                to_string(LaneNum),
                Parm))
    end --if mode ~= nil and PortNum ~= nil then

    return testStillOk;
end

--********************************************************************************

local function Run_serdes_Test_Case_Dispatcher(Test_Case,parm_NN,Test_Case_Number,testStillOk, devNum, PortNum, LaneNum, mode1,mode2)

    --local isSuccess   = true
    local params = {}
    local txt   =  ""

    --  ---------------------------------------------------------------
    if    Test_Case == "serdes_reset_port_1"
       or Test_Case == "serdes_reset_port_all"
    then
        params = {}
        txt   =  ""
        --printMsgLvlLog("debug", string.format("Run_serdes_Test_Case devNum=%s",to_string(devNum)))
        printMsgLvlLog("debug", string.format("Call Check_serdes_reset_port_all"))
        testStillOk    = Check_serdes_reset_port_all(params,devNum,PortNum,txt)

    ------  ---------------------------------------------------------------
    elseif Test_Case == "serdes_no_reset_port_1"
        or Test_Case == "serdes_no_reset_port_all"
    then
        params = {}
        txt   =  "no"
        printMsgLvlLog("debug", string.format("Call Check_serdes_reset_port_all"))
        testStillOk    = Check_serdes_reset_port_all(params,devNum,PortNum,txt)

    ------  ---------------------------------------------------------------
    elseif     Test_Case == "serdes_prbs_set_show_port_1_mode_1"
            or Test_Case == "serdes_prbs_set_show_port_all_mode_all"
            or Test_Case == "serdes_prbs_set_show_port_1_mode_all"
            or Test_Case == "serdes_prbs_set_show_port_all_mode_1"
    then
        printMsgLvlLog("debug", string.format("Call Check_prbs_set_show_interface_mode"))
        testStillOk =  Check_prbs_set_show_interface_mode(devNum,PortNum,LaneNum,mode1,"enable")

    ------  ---------------------------------------------------------------
    elseif     Test_Case == "serdes_prbs_set_no_enable_port_all_lane_all"
            or Test_Case == "serdes_prbs_set_no_enable_port_1_lane_all"
    then
        printMsgLvlLog("debug", string.format("Call Check_prbs_set_show_interface_mode"))
        testStillOk =  Check_prbs_set_show_interface_mode(devNum,PortNum,LaneNum,mode1,"no")

    ------  ---------------------------------------------------------------
    elseif     Test_Case == "serdes_auto_tune_show_port_1"
            or Test_Case == "serdes_auto_tune_show_port_all"
    then
        printMsgLvlLog("debug", string.format("Call Check_auto_tune_show"))
        testStillOk    = Check_auto_tune_show(devNum,PortNum, LaneNum)

    ------  ---------------------------------------------------------------
    elseif     Test_Case == "serdes_voltage_port_all"
    then
        params["ethernet"]= {[devNum]=Serdes_Port_Lst}
        params["laneNum"]          = 0
        txt = "voltage"
        --printMsgLvlLog("debug", string.format("Run_serdes_Test_Case params=%s",to_string(params)))

        printMsgLvlLog("debug", string.format("Call Check_serdes_show_temper_voltage"))
        testStillOk    =  Check_serdes_show_temper_voltage(params,devNum,PortNum,txt)

    ------  ---------------------------------------------------------------
    elseif     Test_Case == "serdes_temperature_port_all"
    then
        params["ethernet"]= {[devNum]=Serdes_Port_Lst}
        params["laneNum"]          = 0
        txt = "temperature"
        --printMsgLvlLog("debug", string.format("Run_serdes_Test_Case params=%s",to_string(params)))

        printMsgLvlLog("debug", string.format("Call Check_serdes_show_temper_voltage"))
        testStillOk    =  Check_serdes_show_temper_voltage(params,devNum,PortNum,txt)

    ------  ---------------------------------------------------------------
    elseif     Test_Case == "serdes_voltage_port_1"
    then
        params["ethernet"]= {[devNum]={ PortNum, }}
        params["laneNum"]          = 0
        txt = "voltage"

        printMsgLvlLog("debug", string.format("Call Check_serdes_show_temper_voltage"))
        testStillOk    =  Check_serdes_show_temper_voltage(params,devNum,PortNum,txt)

    ------  ---------------------------------------------------------------
    elseif     Test_Case == "serdes_temperature_port_1"
    then
        params["ethernet"]= {[devNum]={ PortNum, }}
        params["laneNum"]          = 0
        txt = "temperature"

        --printMsgLvlLog("debug", string.format("Run_serdes_Test_Case params=%s",to_string(params)))
        printMsgLvlLog("debug", string.format("Call Check_serdes_show_temper_voltage"))
        testStillOk    =  Check_serdes_show_temper_voltage(params,devNum,PortNum,txt)

        ------  ---------------------------------------------------------------
        elseif     Test_Case == "serdes_polarity_trigger_true_port_all"
                or Test_Case == "serdes_polarity_trigger_true_port_1"
                or Test_Case == "serdes_polarity_trigger_false_port_all"
                or Test_Case == "serdes_polarity_trigger_false_port_1"
        then

            params["ethernet"]= {[devNum]=Serdes_Port_Lst}
            polarity_trigger=mode1

            --printMsgLvlLog("debug", string.format("mode1 params=%s",to_string(mode1)))
            printMsgLvlLog("debug", string.format("Call check_serdes_polarity_port_trigger"))
            testStillOk    = check_serdes_polarity_port_trigger(devNum,PortNum, LaneNum, polarity_trigger)


        ------  ---------------------------------------------------------------
        elseif     Test_Case == "serdes_polarity_Tx_false_Rx_true_port_all"
                or Test_Case == "serdes_polarity_Tx_false_Rx_true_port_1"

                or Test_Case == "serdes_polarity_Tx_true_Rx_false_port_all"
                or Test_Case == "serdes_polarity_Tx_true_Rx_false_port_1"

                or Test_Case == "serdes_polarity_Tx_true_Rx_true_port_all"
                or Test_Case == "serdes_polarity_Tx_true_Rx_true_port_1"

                or Test_Case == "serdes_polarity_Tx_false_Rx_false_port_all"
                or Test_Case == "serdes_polarity_Tx_false_Rx_false_port_1"

        then

            expected_Tx=mode1
            expected_Rx=mode2

            printMsgLvlLog("debug", string.format("Call check_serdes_polarity_for_ports"))
            testStillOk    = check_serdes_polarity_for_ports(devNum,PortNum, LaneNum, expected_Tx, expected_Rx, nil)

    ---  ---------------------------------------------------------------
    end
    return testStillOk;
end
----********************************************************************************

function Set_Test_Case_Parameters(
parm_MsgLevel,
    parm_DevNum,
    parm_PortNum,
    parm_LaneNum,
    parm_mode1,
    parm_mode2)

    local Test_Case_DevNum = 0
    local Test_Case_PortNum
    local Test_Case_LaneNum = 0

    local mode1 = nil
    local mode2 = nil

    if parm_DevNum == "Default" then
        Test_Case_DevNum =devEnv.dev
    end --if parm_DevNum == "Default" then
    --printMsgLvlLog("debug", string.format("Set_PRBS_Parameter devNum=%s",to_string(devNum)))

    if parm_PortNum == "Default" then
        Test_Case_PortNum =Test_Serdes_Port
    else --if parm_DevNum == "Default" then
        Test_Case_PortNum =parm_PortNum
    end --if parm_DevNum == "Default" then

    if parm_LaneNum == "Default" then
        Test_Case_LaneNum = Default_LaneNum
    else --if parm_LaneNum == "Default" then
        Test_Case_LaneNum = parm_LaneNum
    end --if parm_LaneNum == "Default" then

    --printMsgLvlLog("debug", string.format("Set_PRBS_Parameter parm_LaneNum=%s",to_string(parm_LaneNum)))
    --printMsgLvlLog("debug", string.format("Set_PRBS_Parameter LaneNum=%s",to_string(LaneNum)))

    if parm_mode1[1] ~= nil then
        mode1 = parm_mode1[1]
    else--if parm_LaneNum == "Default" then
        mode1 = "7"
    end --if parm_LaneNum == "Default" then

    if parm_mode2[1] ~= nil then
        mode2 = parm_mode2[1]
    else--if parm_LaneNum == "Default" then
        mode2 = true
    end --if parm_LaneNum == "Default" then


    return Test_Case_DevNum,
           Test_Case_PortNum,
           Test_Case_LaneNum,
           mode1,
           mode2

end

--********************************************************************************

function get_Test_Parameter_Def(key)

    if key ~= nil then

        --printLog("Test_Parameter_Def[key].NN          = "..to_string(Test_Parameter_Def[key].NN ))
        --printLog("Test_Parameter_Def[key].enable_WM   = "..to_string(Test_Parameter_Def[key].enable_WM ))
        --printLog("Test_Parameter_Def[key].enable_BM   = "..to_string(Test_Parameter_Def[key].enable_BM ))
        --printLog("Test_Parameter_Def[key].DevNum      = "..to_string(Test_Parameter_Def[key].DevNum ))
        --printLog("Test_Parameter_Def[key].PortNum     = "..to_string(Test_Parameter_Def[key].PortNum))
        --printLog("Test_Parameter_Def[key].LaneNum     = "..to_string(Test_Parameter_Def[key].LaneNum))
        --printLog("Test_Parameter_Def[key].mode1       = "..to_string(Test_Parameter_Def[key].mode1  ))
        --printLog("Test_Parameter_Def[key].mode2       = "..to_string(Test_Parameter_Def[key].mode2  ))
        --printLog("Test_Parameter_Def[key].mode1.invertTx    = "..to_string(Test_Parameter_Def[key].mode1.invertTx  ))
        --printLog("Test_Parameter_Def[key].mode2.invertRx    = "..to_string(Test_Parameter_Def[key].mode2.invertRx  ))

        return true,
            Test_Parameter_Def[key].NN,
            Test_Parameter_Def[key].enable_WM,
            Test_Parameter_Def[key].enable_BM,
            Test_Parameter_Def[key].CoverageLvl,
            Test_Parameter_Def[key].DevNum,
            Test_Parameter_Def[key].PortNum,
            Test_Parameter_Def[key].LaneNum,
            Test_Parameter_Def[key].mode1,
            Test_Parameter_Def[key].mode2,
            Test_Parameter_Def[key].feature
    else
        return false
    end
end

-- ************************************************************************

function Serdes_Run_Test_case(Test_Case)

    local isSuccess   = true
    local testStillOk = true

    --Test_Case = "serdes_voltage_port_1"


    local params = {}
    --local valid = {}
    local Test_Case_VerboseLevel

    devNum  = devEnv.dev
    devFamily = wrlCpssDeviceFamilyGet(devEnv.dev)

    local     dev_is_sip_5          = is_sip_5(devNum)
    local     dev_is_sip_5_10       = is_sip_5_10(devNum)
    local     dev_is_sip_5_15       = is_sip_5_15(devNum)
    local     dev_is_sip_5_20       = is_sip_5_20(devNum)
    local     dev_is_sip_5_15_only = (is_sip_5_15(devNum) and (not is_sip_5_20(devNum)))

    --printLog("devFamily             = "..to_string(devFamily))
    --printLog("DeviceSubFamily       = "..to_string(DeviceSubFamily))
    --printLog("dev_is_sip_5          = "..to_string(dev_is_sip_5        ))
    --printLog("dev_is_sip_5_10       = "..to_string(dev_is_sip_5_10     ))
    --printLog("dev_is_sip_5_15       = "..to_string(dev_is_sip_5_15     ))
    --printLog("dev_is_sip_5_20       = "..to_string(dev_is_sip_5_20     ))
    --printLog("dev_is_sip_5_15_only  = "..to_string(dev_is_sip_5_15_only))
    --
    --printLog("SERDES_POLARITY       = "..to_string(is_supported_feature(devEnv.dev, "SERDES_POLARITY")))
    --printLog("Coverage Level        = "..to_string(CoverageLvl))
    --printLog("Verbose Level         = "..to_string(VerboseLevel))

    printMsgLvlLog("debug", string.format("devFamily             = "..to_string(devFamily)))
    printMsgLvlLog("debug", string.format("DeviceSubFamily       = "..to_string(DeviceSubFamily)))
    --printMsgLvlLog("debug", string.format("dev_is_sip_5          = "..to_string(dev_is_sip_5)))
    --printMsgLvlLog("debug", string.format("dev_is_sip_5_10       = "..to_string(dev_is_sip_5_10)))
    --printMsgLvlLog("debug", string.format("dev_is_sip_5_15       = "..to_string(dev_is_sip_5_15)))
    --printMsgLvlLog("debug", string.format("dev_is_sip_5_20       = "..to_string(dev_is_sip_5_20)))
    --printMsgLvlLog("debug", string.format("dev_is_sip_5_15_only  = "..to_string(dev_is_sip_5_15_only)))
    --printMsgLvlLog("debug", string.format("SERDES_POLARITY       = "..to_string(SERDES_POLARITY)))
    printMsgLvlLog("debug", string.format("Coverage Level        = "..to_string(CoverageLvl)))
    printMsgLvlLog("debug", string.format("Verbose Level         = "..to_string(VerboseLevel) .." "..VerboseLvlTxt  ))

    params["all_interfaces"] = "ethernet"
    params["laneNum"]          = 0
    params["ethernet"]= {[devNum]={ Test_Serdes_Port, }}

    ret, parm_NN, parm_enable_WM, parm_enable_BM, parm_CoverageLvl, parm_DevNum, parm_PortNum, parm_LaneNum,parm_mode1, parm_mode2,parm_feature = get_Test_Parameter_Def(Test_Case)

    --print("parm_NN  = "..to_string(parm_NN))
    printMsgLvlLog("debug",string.format(" Test_Case=%s ret=%s parm_enable_WM=%s",to_string(Test_Case),to_string(ret),to_string(parm_enable_WM)))

    if     (       (ret == true)
       and ((      (isSimulationUsed() == true ) and (parm_enable_WM == "Y"))
              or ( (isSimulationUsed() == false) and (parm_enable_BM == "Y"))))
    then

        if parm_CoverageLvl <= CoverageLvl then

            if      (true == is_supported_feature(devEnv.dev, parm_feature)) then

                --Test_Case_Number = 12
                Test_Case_Number = parm_NN
                DevNum, PortNum, LaneNum, mode1,mode2=Set_Test_Case_Parameters(parm_MsgLevel, parm_DevNum, parm_PortNum, parm_LaneNum, parm_mode1, parm_mode2)

                params["ethernet"]= {[devNum]={ PortNum, }}

                --ret,Port_Interf_Status = show_interfaces_status(params)
                --print("Port_Interf_Status  = "..to_string(Port_Interf_Status))
                --
                --ret,Port_Intef_config = show_interfaces_configuration(params)
                --print("Port_Intef_config  = "..to_string(Port_Intef_config))


                local isSuccess   = true
                printMsgLvlLog("debug", string.format("---------- "..to_string(Test_Case_Number).." Test_Case #"..to_string(parm_NN).." Started"))
                isSuccess  =  Run_serdes_Test_Case_Dispatcher(Test_Case,parm_NN,Test_Case_Number,testStillOk, DevNum, PortNum, LaneNum, mode1,mode2)
                testStillOk  = error_handle("********** "..to_string(Test_Case_Number).." Test_Case #"..to_string(parm_NN).." "..Test_Case, isSuccess, testStillOk )

                --Serdes_Set_Speed_Port_Properties(devNum,
                --    PortNum,
                --    Port_Interf_Status[devNum][PortNum].port_speed,
                --    Port_Interf_Status[devNum][PortNum].port_interface_mode)
                --
                --Serdes_Set_flow_control_Port(devNum,
                --     PortNum,
                --     Port_Intef_config[devNum][PortNum].flow_control_autonegatiation,
                --     Port_Intef_config[devNum][PortNum].flow_control)

                --ret,Port_Intef_config = show_interfaces_configuration(params)
                --print("Port_Intef_config  = "..to_string(Port_Intef_config))

            end--if (true == is_supported_feature(devEnv.dev, parm_feature))

        end --if VerboseLevel >= parm_CoverageLvl

    end --ret == true and parm_enable == "Y" then

    if true ~= testStillOk then
          setFailState()
    end


    cmdLuaCLI_execute("end"); cmdLuaCLI_execute("examples");

    printLog("------------------------------------")
    printLog("Coverage test level: " .. to_string(CoverageLvl))
    printLog("Verbose: ".. VerboseLvlTxt .."  - log level:  " .. to_string(VerboseLevel))
    printLog("------------------------------------")

    testStillOk = error_handle("Test ".. Test_Case .." ended: ", isSuccess, testStillOk );

    return testStillOk;
end

