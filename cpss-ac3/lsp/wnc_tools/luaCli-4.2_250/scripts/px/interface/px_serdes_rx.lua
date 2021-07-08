--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_rx.lua
--*
--* DESCRIPTION:
--*     Configure specific parameters of serdes RX in HW
--*
--*
--*
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  serdes_rx_func_bobk
--        @description  configure specific parameters of serdes rx in hw
--
--
--
--        @return       error message if fails
--
local function serdes_rx_func(params)
  local command_data = Command_Data()
  local devNum, portNum
  local GT_OK=0
  local numOfSerdesLanesPtr=0
  local devnum_portnum_string
  local ret
  local serdesRxCfgPtr={}

  serdesRxCfgPtr.sqlch=params.sqlch              --sqlch
  serdesRxCfgPtr.ffeRes=params.ffeRes              --ffeRes
  serdesRxCfgPtr.ffeCap=params.ffeCap              --ffeCap
  serdesRxCfgPtr.dcGain=params.dcGain              --dcGain
  serdesRxCfgPtr.bandWidth=params.bandWidth        --bandWidth
  serdesRxCfgPtr.loopBandwidth=params.loopBandwidth --loopBandwidth

  local laneNum=params.laneNum                     --laneNum

  if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do

            command_data:clearPortStatus()
            command_data:clearLocalStatus()

      if(nil~=devNum) then
                devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                                                         '/' ..
                                                         tostring(portNum), 9)
        if laneNum == "all" then
          numOfSerdesLanesPtr=16
          if(numOfSerdesLanesPtr > 0) then
            for i=0,numOfSerdesLanesPtr-1,1 do
              laneNum=i
              ret = myGenWrapper("cpssPxPortSerdesManualRxConfigSet",{
                {"IN","GT_SW_DEV_NUM","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","GT_U32","laneNum",laneNum},
                {"IN","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr",serdesRxCfgPtr}})
              if(ret~=GT_OK) then
                print(devnum_portnum_string.." Error cpssPxPortSerdesManualRxConfigSet ret: "..to_string(ret).." "..returnCodes[ret])
              end
            end  -- for i=0,numOfSerdesLanesPtr-1,1
          else -- if( ret == 0 and numOfSerdesLanesPtr > 0)
                        print(devnum_portnum_string.." There is no port serdes\n")
          end -- if( ret == 0 and numOfSerdesLanesPtr > 0)

        else
          ret = myGenWrapper("cpssPxPortSerdesManualRxConfigSet",{
            {"IN","GT_SW_DEV_NUM","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"IN","GT_U32","laneNum",laneNum},
            {"IN","CPSS_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr",serdesRxCfgPtr}})
          if(ret~=GT_OK) then
            print(devnum_portnum_string.." Error cpssPxPortSerdesManualRxConfigSet ret: "..to_string(ret).." "..returnCodes[ret])
          end
        end  -- if laneNum == "all"
      end -- if(nil~=devNum)
    end -- for iterator, devNum, portNum
  end -- if true == command_data["status"] then

  -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()

end
--------------------------------------------
-- command registration: serdes_rx
--------------------------------------------

CLI_addCommand("interface", "serdes manual configuration rx", {
  func   = serdes_rx_func,
  help   = "Configure specific parameters of serdes RX in HW",
  params = {
    {   type="named", { format="lane_num %lane_number_type",             name="laneNum",       help="Number of SERDES lane of port or all"    },
                      { format="sq_threshold %type_sqlch",               name="sqlch",         help="Threshold that trips the Squelch detector"},
                      { format="ffe_res %type_ffeRes",                 name="ffeRes",        help="Mainly controls the low frequency gain"    },
                      { format="ffe_cap %type_ffeCap",                 name="ffeCap",        help="Mainly controls the high frequency gain"  },
                      { format="dc_gain %type_dcGain",                 name="dcGain",        help="DC-Gain value"                           },
                      { format="band_width %type_bandWidth",         name="bandWidth",     help="CTLE Band-width "                         },
                      { format="loop_bandwidth %type_loopBandwidth",   name="loopBandwidth", help="DCTLE Loop Band-width"                     },
        mandatory = {"laneNum","sqlch","ffeRes","ffeCap","dcGain","bandWidth","loopBandwidth"},
                    requirements={
                            ["sq_threshold"] =    {"lane_num"},
                            ["ffe_res"] =      {"sq_threshold"},
                            ["ffe_cap"] =      {"ffe_res"},
                            ["dc_gain"]  =      {"ffe_cap"},
                            ["band_width"]  =    {"dc_gain"},
                            ["loop_bandwidth"] = {"band_width"}
                        }
               }}}
)


