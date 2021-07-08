--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ap-port.lua
--*
--* DESCRIPTION:
--*     Enable/Disable AP process on port  
--*
--* 
--*       
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  ap_func
--        @description  enable/disable ap process on port
--
--
--
--        @return       error message if fails
--
local function ap_func(params)
  local command_data = Command_Data()
  local devNum, portNum, ret, enable
  local GT_OK=0
  local apParamsPtr={}
  local fecA
  local fecR

  apParamsPtr.fcPause=params.fcPause         --fcPause
  apParamsPtr.fcAsmDir=params.fcAsmDir       --fcAsmDir
--  apParamsPtr.fecSupported=params.fecSupported    --fecSupported
--  apParamsPtr.fecRequired=params.fecRequired     --fecRequired
  apParamsPtr.noneceDisable=true           --noneceDisable
  apParamsPtr.laneNum=params.laneNum         --laneNum

  if(params.fecSupported == CPSS_DXCH_PORT_FEC_MODE_ENABLED_E) then
    apParamsPtr.fecSupported = true
  else
    apParamsPtr.fecSupported = false
  end
  fecA = params.fecSupported

  if(params.fecRequired == CPSS_DXCH_PORT_FEC_MODE_ENABLED_E) then
    apParamsPtr.fecRequired = true
  else
    apParamsPtr.fecRequired = false
  end
  fecR = params.fecRequired

  apParamsPtr.modesAdvertiseArr={}
  apParamsPtr.fecAbilityArr={}
  apParamsPtr.fecRequestedArr={}

  apParamsPtr.modesAdvertiseArr[0]={}          --mode1 speed1
  apParamsPtr.modesAdvertiseArr[0].ifMode=params.mode1
  apParamsPtr.modesAdvertiseArr[0].speed=params.speed1
  apParamsPtr.fecAbilityArr[0]=fecA
  apParamsPtr.fecRequestedArr[0]=fecR

  apParamsPtr.modesAdvertiseArr[1]={}          --mode2 speed2
  apParamsPtr.modesAdvertiseArr[1].ifMode=params.mode2
  apParamsPtr.modesAdvertiseArr[1].speed=params.speed2
  apParamsPtr.fecAbilityArr[1]=fecA
  apParamsPtr.fecRequestedArr[1]=fecR

  apParamsPtr.modesAdvertiseArr[2]={}          --mode3 speed3
  apParamsPtr.modesAdvertiseArr[2].ifMode=params.mode3
  apParamsPtr.modesAdvertiseArr[2].speed=params.speed3
  apParamsPtr.fecAbilityArr[2]=fecA
  apParamsPtr.fecRequestedArr[2]=fecR

  apParamsPtr.modesAdvertiseArr[3]={}          --mode4 speed4
  apParamsPtr.modesAdvertiseArr[3].ifMode=params.mode4
  apParamsPtr.modesAdvertiseArr[3].speed=params.speed4
  apParamsPtr.fecAbilityArr[3]=fecA
  apParamsPtr.fecRequestedArr[3]=fecR

  apParamsPtr.modesAdvertiseArr[4]={}          --mode5 speed5
  apParamsPtr.modesAdvertiseArr[4].ifMode=params.mode5
  apParamsPtr.modesAdvertiseArr[4].speed=params.speed5
  apParamsPtr.fecAbilityArr[4]=fecA
  apParamsPtr.fecRequestedArr[4]=fecR

  apParamsPtr.modesAdvertiseArr[5]={}          --mode6 speed6
  apParamsPtr.modesAdvertiseArr[5].ifMode=params.mode6
  apParamsPtr.modesAdvertiseArr[5].speed=params.speed6
  apParamsPtr.fecAbilityArr[5]=fecA
  apParamsPtr.fecRequestedArr[5]=fecR

  apParamsPtr.modesAdvertiseArr[6]={}          --mode7 speed7
  apParamsPtr.modesAdvertiseArr[6].ifMode=params.mode7
  apParamsPtr.modesAdvertiseArr[6].speed=params.speed7
  apParamsPtr.fecAbilityArr[6]=fecA
  apParamsPtr.fecRequestedArr[6]=fecR

  apParamsPtr.modesAdvertiseArr[7]={}          --mode8 speed8
  apParamsPtr.modesAdvertiseArr[7].ifMode=params.mode8
  apParamsPtr.modesAdvertiseArr[7].speed=params.speed8
  apParamsPtr.fecAbilityArr[7]=fecA
  apParamsPtr.fecRequestedArr[7]=fecR

  apParamsPtr.modesAdvertiseArr[8]={}          --mode9 speed9
  apParamsPtr.modesAdvertiseArr[8].ifMode=params.mode9
  apParamsPtr.modesAdvertiseArr[8].speed=params.speed9
  apParamsPtr.fecAbilityArr[8]=fecA
  apParamsPtr.fecRequestedArr[8]=fecR

  apParamsPtr.modesAdvertiseArr[9]={}          --mode10 speed10
  apParamsPtr.modesAdvertiseArr[9].ifMode=params.mode10
  apParamsPtr.modesAdvertiseArr[9].speed=params.speed10
  apParamsPtr.fecAbilityArr[9]=fecA
  apParamsPtr.fecRequestedArr[9]=fecR

  local checkModeNA="CPSS_PORT_INTERFACE_MODE_NA_E"
  local checkSpeedNA="CPSS_PORT_SPEED_10000_E"
  if(apParamsPtr.modesAdvertiseArr[1].ifMode==nil) then
    apParamsPtr.modesAdvertiseArr[1].ifMode=checkModeNA
    apParamsPtr.modesAdvertiseArr[1].speed=checkSpeedNA
  end
  if (apParamsPtr.modesAdvertiseArr[2].ifMode==nil) then
    apParamsPtr.modesAdvertiseArr[2].ifMode=checkModeNA
    apParamsPtr.modesAdvertiseArr[2].speed=checkSpeedNA
  end
  if (apParamsPtr.modesAdvertiseArr[3].ifMode==nil) then
    apParamsPtr.modesAdvertiseArr[3].ifMode=checkModeNA
    apParamsPtr.modesAdvertiseArr[3].speed=checkSpeedNA
  end
  if (apParamsPtr.modesAdvertiseArr[4].ifMode==nil) then
    apParamsPtr.modesAdvertiseArr[4].ifMode=checkModeNA
    apParamsPtr.modesAdvertiseArr[4].speed=checkSpeedNA
  end
  if (apParamsPtr.modesAdvertiseArr[5].ifMode==nil) then
    apParamsPtr.modesAdvertiseArr[5].ifMode=checkModeNA
    apParamsPtr.modesAdvertiseArr[5].speed=checkSpeedNA
  end
  if (apParamsPtr.modesAdvertiseArr[6].ifMode==nil) then
    apParamsPtr.modesAdvertiseArr[6].ifMode=checkModeNA
    apParamsPtr.modesAdvertiseArr[6].speed=checkSpeedNA
  end
  if (apParamsPtr.modesAdvertiseArr[7].ifMode==nil) then
    apParamsPtr.modesAdvertiseArr[7].ifMode=checkModeNA
    apParamsPtr.modesAdvertiseArr[7].speed=checkSpeedNA
  end
  if (apParamsPtr.modesAdvertiseArr[8].ifMode==nil) then
    apParamsPtr.modesAdvertiseArr[8].ifMode=checkModeNA
    apParamsPtr.modesAdvertiseArr[8].speed=checkSpeedNA
  end
  if (apParamsPtr.modesAdvertiseArr[9].ifMode==nil) then
    apParamsPtr.modesAdvertiseArr[9].ifMode=checkModeNA
    apParamsPtr.modesAdvertiseArr[9].speed=checkSpeedNA
  end

  enable = command_data:getTrueIfFlagNoIsNil(params)

  if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in
                                    command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()
      if(nil~=devNum) then
        ret = myGenWrapper("cpssDxChPortApPortConfigSet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
          {"IN","GT_BOOL","apEnable",enable},
          {"IN","CPSS_DXCH_PORT_AP_PARAMS_STC","apParamsPtr",apParamsPtr}})
        if(ret~=GT_OK) then
          print("Error at ap command")
        end
      end
    end
  end



  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: ap-port
--------------------------------------------
CLI_addCommand("interface", "ap-port", {
  func   = ap_func,
  help   = "Enable AP process on port",
  params = {
  {   type="named", { format="fc_pause %bool", name="fcPause"},
            { format="fc_asm_dir %ap_flow_control", name="fcAsmDir"},
            { format="fec_supported %ap_port_fec_mode", name="fecSupported"},
            { format="fec_required %ap_port_fec_mode", name="fecRequired"},
            { format="lane_num %GT_U32", name="laneNum"},

            { format="mode1 %ap_port_interface_mode", name="mode1"},
            { format="speed1 %ap_port_speed", name="speed1"},

            mandatory = {"fcPause","fcAsmDir","fecSupported","fecRequired","laneNum","mode1","speed1"},

            { format="mode2 %ap_port_interface_mode", name="mode2"},
            { format="speed2 %ap_port_speed", name="speed2"},

            { format="mode3 %ap_port_interface_mode", name="mode3"},
            { format="speed3 %ap_port_speed", name="speed3"},

            { format="mode4 %ap_port_interface_mode", name="mode4"},
            { format="speed4 %ap_port_speed", name="speed4"},

            { format="mode5 %ap_port_interface_mode", name="mode5"},
            { format="speed5 %ap_port_speed", name="speed5"},

            { format="mode6 %ap_port_interface_mode", name="mode6"},
            { format="speed6 %ap_port_speed", name="speed6"},

            { format="mode7 %ap_port_interface_mode", name="mode7"},
            { format="speed7 %ap_port_speed", name="speed7"},

            { format="mode8 %ap_port_interface_mode", name="mode8"},
            { format="speed8 %ap_port_speed", name="speed8"},

            { format="mode9 %ap_port_interface_mode", name="mode9"},
            { format="speed9 %ap_port_speed", name="speed9"},

            { format="mode10 %ap_port_interface_mode", name="mode10"},
            { format="speed10 %ap_port_speed", name="speed10"},

              requirements={
              ["speed1"] = {"mode1"},
              ["mode2"] = {"speed1"},
              ["speed2"] = {"mode2"},
              ["mode3"] = {"speed2"},
              ["speed3"] = {"mode3"},
              ["mode4"] = {"speed3"},
              ["speed4"] = {"mode4"},
              ["mode5"] = {"speed4"},
              ["speed5"] = {"mode5"},
              ["mode6"] = {"speed5"},
              ["speed6"] = {"mode6"},
              ["mode7"] = {"speed6"},
              ["speed7"] = {"mode7"},
              ["mode8"] = {"speed7"},
              ["speed8"] = {"mode8"},
              ["mode9"] = {"speed8"},
              ["speed9"] = {"mode9"},
              ["mode10"] = {"speed9"},
              ["speed10"] = {"mode10"}
            }
         }}}
)

CLI_addCommand("interface", "no ap-port", {
  func = function(params)
    params.flagNo = true
    return ap_func(params)
    end,
  help = "Disable AP process on port"
})
