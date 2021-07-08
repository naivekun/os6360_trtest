--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* auto_tune.lua
--*
--* DESCRIPTION:
--*       start/stop/show status training mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
-- ************************************************************************

local function Ret_Table_Update(Ret_Table, devNum, portNum,OutValues)

    --local tuple    = {}
    if (Ret_Table ~= nil ) and (OutValues ~= nil ) then

        Ret_Table[devNum]          = tableInsert(Ret_Table[devNum],nil)
        Ret_Table[devNum][portNum] = tableInsert(Ret_Table[devNum][portNum], nil)

        Ret_Table[devNum][portNum].invertTx = invertTx
        Ret_Table[devNum][portNum].invertRx = invertRx

            Ret_Table[devNum][portNum].DC     = OutValues.DC
            Ret_Table[devNum][portNum].LF     = OutValues.LF
            Ret_Table[devNum][portNum].HF     = OutValues.HF
            Ret_Table[devNum][portNum].BW     = OutValues.BW
            Ret_Table[devNum][portNum].LB     = OutValues.LB
            Ret_Table[devNum][portNum].EO     = OutValues.EO

            Ret_Table[devNum][portNum].DFE_0   = OutValues.DFE[0]
            Ret_Table[devNum][portNum].DFE_1   = OutValues.DFE[1]
            Ret_Table[devNum][portNum].DFE_2   = OutValues.DFE[2]
            Ret_Table[devNum][portNum].DFE_3   = OutValues.DFE[3]
            Ret_Table[devNum][portNum].DFE_4   = OutValues.DFE[4]
            Ret_Table[devNum][portNum].DFE_5   = OutValues.DFE[5]
            Ret_Table[devNum][portNum].DFE_6   = OutValues.DFE[6]
            Ret_Table[devNum][portNum].DFE_7   = OutValues.DFE[7]
            Ret_Table[devNum][portNum].DFE_8   = OutValues.DFE[8]
            Ret_Table[devNum][portNum].DFE_9   = OutValues.DFE[9]
            Ret_Table[devNum][portNum].DFE_10  = OutValues.DFE[10]
            Ret_Table[devNum][portNum].DFE_11  = OutValues.DFE[11]
            Ret_Table[devNum][portNum].DFE_12  = OutValues.DFE[12]

            Ret_Table[devNum][portNum].txAmp     = OutValues.txAmp
            Ret_Table[devNum][portNum].txEmphAmp = OutValues.txEmphAmp
            Ret_Table[devNum][portNum].txEmph1   = OutValues.txEmph1


    end
    return Ret_Table

end


-- ************************************************************************
local function autoTuneHandler_pipe(params)
    local ret=0
    local printSpaceAtEnd=false
    local param=params["auto_tune"]
    local tableMsg={CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E="TRX",
                 CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E="RX"
                }
    local all_ports, dev_ports, device
    local numOfSerdesLanesPtr=0

    local  min_LF, max_LF, NA_NUMBER

    if true == params.flagNo  then
        param="CPSS_PORT_SERDES_AUTO_TUNE_MODE_TRAINING_STOP_E"
    end
    NA_NUMBER=30

    all_ports = getGlobal("ifRange")--get table of ports

    if params.min_LF == nil then
        min_LF=0
    else
        min_LF=params.min_LF
    end
    --print("min_LF= "..to_string(min_LF))

    if params.max_LF == nil then
        max_LF=0
    else
        max_LF=params.max_LF
    end
    --print("max_LF= "..to_string(max_LF))

    for device, dev_ports in pairs(all_ports) do
        for k, port in pairs(dev_ports) do

            ret,numOfSerdesLanesPtr=PxGetNuberSerdesOfPort(device,port)
            if( ret == 0 and numOfSerdesLanesPtr > 0) then

                if "CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E" == param then  --if TRX then CFG is needed

                    ret = myGenWrapper("cpssPxPortSerdesAutoTune",{   --set configure:
                    {"IN","GT_SW_DEV_NUM","devNum",device},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                    {"IN","CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT","portTuningMode", "CPSS_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E"}
                    })

                end

                if 0==ret then  --if TRX- then CFG should succeed, otherwise ret already==0

                    ret = myGenWrapper("cpssPxPortSerdesEnhancedAutoTune",{   --set configure:
                        {"IN","GT_SW_DEV_NUM","devNum",device},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                        {"IN","GT_U8","min_LF", min_LF},
                        {"IN","GT_U8","max_LF", max_LF}
                    })

                    if 0~=ret then
                        if ret==NA_NUMBER then
                            if true == params.flagNo  then
                                print("Error at stopping auto tune: "..returnCodes[ret])
                            else
                                print("Error at start "..tableMsg[param].." training: "..returnCodes[ret])
                            end
                            break
                        end
                        if true == params.flagNo  then
                            print(device.."/"..port..": Error at stopping auto tune: "..returnCodes[ret])
                        else
                            print(device.."/"..port..": Error at start "..tableMsg[param].." training: "..returnCodes[ret])
                        end
                        printSpaceAtEnd=true
                    end

                else
                    if ret==NA_NUMBER then
                        print("Error at TX training configure state: "..returnCodes[ret])
                        break
                    end
                    print(device.."/"..port..": Error at TX training configure state: "..returnCodes[ret])
                    printSpaceAtEnd=true
                end


            end -- PxGetNuberSerdesOfPort
        end
    end
    if true==printSpaceAtEnd then
        print()
    end

end

-- ************************************************************************
---
--  autoTuneShow
--        @description  show results of SERDES auto tuning for the interface
--
--        @param params         none
--
--        @return       true on success, otherwise false and error message
--
-- ************************************************************************
function autoTuneShow(params)
    local statusTable={
                      CPSS_PORT_SERDES_AUTO_TUNE_PASS_E="Pass",
                      CPSS_PORT_SERDES_AUTO_TUNE_FAIL_E="Fail",
                      CPSS_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E="Not complited"
                    }
    local all_ports, dev_ports
    local port,device, devPort
    local ret,val
    local statusTRX, statusRX
    local printHeader=false
    local info=""
    local numOfSerdesLanesPtr=0
    local errorMsg=""
    all_ports = getGlobal("ifRange")--get table of ports
    local OutValues
  local GT_OK=0                   --  (0x00) /* Operation succeeded */
  local GT_NOT_IMPLEMENTED=17     --  (0x11) /* This request is not implemented       */
  local IfDevPortSerdesExist=false


    --print("params  = "..to_string(params))
    local Ret_Table = {}

    --print("Serdes Auto Tune Results")
    print("")

    for device, dev_ports in pairs(all_ports) do
        for k, port in pairs(dev_ports) do

             ret,numOfSerdesLanesPtr=PxGetNuberSerdesOfPort(device,port)
           if( ret == GT_OK and numOfSerdesLanesPtr > 0) then
                 IfDevPortSerdesExist=true

                 devPort=tostring(device).."/"..tostring(port)
                 ret,val = myGenWrapper("cpssPxPortSerdesAutoTuneStatusGet",{  --get configure:
                         {"IN","GT_SW_DEV_NUM","devNum",device},
                         {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                         {"OUT","CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT","rxTuneStatusPtr"},
                         {"OUT","CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT","txTuneStatusPtr"}
                         })

                 if ((ret == GT_OK) or (ret == GT_NOT_IMPLEMENTED)) then

                     ret, val = myGenWrapper("cpssPxPortSerdesAutoTuneResultsGet",{
                     {"IN","GT_SW_DEV_NUM","devNum",device},
                         {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                         {"IN","GT_U32","laneNum",laneNum},
                         {"OUT","CPSS_PORT_SERDES_TUNE_STC","serdesTunePtr"}})

                     if ret == GT_OK then

                         OutValues=val["serdesTunePtr"]
                         --print("val         = "..to_string(val ))
                         --print("OutValues   = "..to_string(OutValues ))
                            print("--------------------------------")
                            print("devPort= "..devPort      )
                            print("--------------------------------")
                            print("DC      = "..to_string(OutValues.DC  ))
                            print("LF      = "..to_string(OutValues.LF  ))
                            print("HF      = "..to_string(OutValues.HF  ))
                            print("BW      = "..to_string(OutValues.BW  ))
                            print("LB      = "..to_string(OutValues.LB  ))
                            print("EO      = "..to_string(OutValues.EO  ))

                            print("DFE[0]  = "..to_string(OutValues.DFE[0] ))
                            print("DFE[1]  = "..to_string(OutValues.DFE[1] ))
                            print("DFE[2]  = "..to_string(OutValues.DFE[2] ))
                            print("DFE[3]  = "..to_string(OutValues.DFE[3] ))
                            print("DFE[4]  = "..to_string(OutValues.DFE[4] ))
                            print("DFE[5]  = "..to_string(OutValues.DFE[5] ))
                            print("DFE[6]  = "..to_string(OutValues.DFE[6] ))
                            print("DFE[7]  = "..to_string(OutValues.DFE[7] ))
                            print("DFE[8]  = "..to_string(OutValues.DFE[8] ))
                            print("DFE[9]  = "..to_string(OutValues.DFE[9] ))
                            print("DFE[10] = "..to_string(OutValues.DFE[10]))
                            print("DFE[11] = "..to_string(OutValues.DFE[11]))
                            print("DFE[12] = "..to_string(OutValues.DFE[12]))

                            print("txAmp   = "..to_string(OutValues.txAmp    ))
                            print("txEmph0 = "..to_string(OutValues.txEmphAmp))
                            print("txEmph1 = "..to_string(OutValues.txEmph1  ))
                         print("")
                         Ret_Table = Ret_Table_Update(Ret_Table, device, port,OutValues)
                     else
                         print(devPort.." Error cpssPxPortSerdesAutoTuneResultsGet ret: "..to_string(ret).." "..returnCodes[ret])
                     end -- cpssPxPortSerdesAutoTuneResultsGet
                 else
                     print(devPort.." Error cpssPxPortSerdesAutoTuneStatusGet ret: "..to_string(ret).." "..returnCodes[ret])
                 end -- cpssPxPortSerdesAutoTuneResultsGet
             end -- if ((ret == GT_OK) or (ret == GT_NOT_IMPLEMENTED))
        end --for k, port in pairs(dev_ports)
    end -- for device, dev_ports

  if IfDevPortSerdesExist == false then
        print("There is no port serdes information to show.\n")
    end
    --print("Ret_Table  = "..to_string(Ret_Table))
    return true, Ret_Table

end -- function autoTuneShow(params)





CLI_type_dict["type_min_LF"]={
  checker = CLI_check_param_number,
  min=0,
  max=15,
  complete = CLI_complete_param_number,
  help = "min LF - Minimum LF value that can be set on Serdes"
}

CLI_type_dict["type_max_LF"]={
  checker = CLI_check_param_number,
  min=0,
  max=15,
  complete = CLI_complete_param_number,
  help = "max LF - Maximum LF value that can be set on Serdes"
}

--------------------------------------------------------------------------------
-- command registration: auto-tune start
--------------------------------------------------------------------------------
CLI_addHelp("interface", "auto-tune", "Handle auto tune")

--function autotune_config()

        CLI_addCommand("interface", "auto-tune start", {
          func   = autoTuneHandler_pipe,
          help   = "Start auto tune",
          params = {{ type="values",
                       {format="%auto_tune",               name="auto_tune"}
                    },
                    { type="named",
                       {format="min_LF %type_min_LF", name="min_LF", help="min LF - Minimum LF value that can be set on Serdes (0...15)"},
                       {format="max_LF %type_max_LF", name="max_LF", help="max LF - Maximum LF value that can be set on Serdes (0...15)"},
                       mandatory={"auto_tune"}
                    }
           }
         }
        )

--end  autotune_config()

--
--------------------------------------------------------------------------------
-- command registration: no auto-tune start
--------------------------------------------------------------------------------
CLI_addHelp("interface", "no auto-tune", "Stop auto tune")
CLI_addCommand("interface", "no auto-tune start", {
    func=function(params)
        params.flagNo=true
        return autoTuneHandler_pipe(params)
    end,
     help   = "Stop auto tune",
    params ={}


  }
)


--------------------------------------------------------------------------------
-- command registration: auto-tune show
--------------------------------------------------------------------------------
CLI_addCommand("interface", "auto-tune show",
  {
  func   = autoTuneShow,
  help   = "Show auto tune",
  params = {}
  }
)
