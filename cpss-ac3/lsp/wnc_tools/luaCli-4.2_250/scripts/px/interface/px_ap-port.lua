--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_ap-port.lua
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
--  ap_func_pipe
--        @description  enable/disable ap process on port
--
--
--
--        @return       error message if fails
--
local function ap_func_pipe(params)
  local command_data = Command_Data()
  local devNum, portNum, ret, enable
  local GT_OK=0
  local apParams
  local fecA
  local fecR

  fecA = params.fecSupported
  fecR = params.fecRequired

  apParams = {
        fcPause = params.fcPause,
        fcAsmDir = params.fcAsmDir,
        fecSupported = (params.fecSupported == CPSS_PORT_FEC_MODE_ENABLED_E),
        fecRequired = (params.fecRequired == CPSS_PORT_FEC_MODE_ENABLED_E),
        noneceDisable = true,
        laneNum = params.laneNum,
        modesAdvertiseArr={
            [0] = {
                ifMode = params.mode1,
                speed=params.speed1 },
            [1] = {
                ifMode = params.mode2,
                speed=params.speed2 },
            [2] = {
                ifMode = params.mode3,
                speed=params.speed3 },
            [3] = {
                ifMode = params.mode4,
                speed=params.speed4 },
            [4] = {
                ifMode = params.mode5,
                speed=params.speed5 },
            [5] = {
                ifMode = params.mode6,
                speed=params.speed6 },
            [6] = {
                ifMode = params.mode7,
                speed=params.speed7 },
            [7] = {
                ifMode = params.mode8,
                speed=params.speed8 },
            [8] = {
                ifMode = params.mode9,
                speed=params.speed9 },
            [9] = {
                ifMode = params.moda10,
                speed=params.speed10 }
        },
        fecAbilityArr = {
            [0] = fecA,
            [1] = fecA,
            [2] = fecA,
            [3] = fecA,
            [4] = fecA,
            [5] = fecA,
            [6] = fecA,
            [7] = fecA,
            [8] = fecA,
            [9] = fecA
        },
        fecRequestedArr = {
            [0] = fecR,
            [1] = fecR,
            [2] = fecR,
            [3] = fecR,
            [4] = fecR,
            [5] = fecR,
            [6] = fecR,
            [7] = fecR,
            [8] = fecR,
            [9] = fecR
        }
  }


  local checkModeNA="CPSS_PORT_INTERFACE_MODE_NA_E"
  local checkSpeedNA="CPSS_PORT_SPEED_10000_E"
  local i
  for i=1, 9 do
      if apParams.modesAdvertiseArr[i].ifMode==nil then
          apParams.modesAdvertiseArr[i] = {
              ifMode=checkModeNA,
              speed=checkSpeedNA }
      end
  end

  enable = command_data:getTrueIfFlagNoIsNil(params)

  if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in
                                    command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()
      if(nil~=devNum) then
        ret = myGenWrapper("cpssPxPortApPortConfigSet",{
          {"IN","GT_U32","devNum",devNum},
          {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
          {"IN","GT_BOOL","apEnable",enable},
          {"IN","CPSS_PX_PORT_AP_PARAMS_STC","apParamsPtr",apParams}})
        if(ret~=GT_OK) then
          print("Error at ap command rc="..to_string(ret))
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
  func   = ap_func_pipe,
  help   = "Enable AP process on port",
  params = {
  {   type="named", { format="fc_pause %bool", name="fcPause"},
            { format="fc_asm_dir %ap_flow_control", name="fcAsmDir"},
            { format="fec_supported %ap_port_fec_mode_gen", name="fecSupported"},
            { format="fec_required %ap_port_fec_mode_gen", name="fecRequired"},
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
    return ap_func_pipe(params)
    end,
  help = "Disable AP process on port"
})
