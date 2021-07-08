--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_ap_rx.lua
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
--  serdes_ap_rx_func
--        @description  configure specific parameters of serdes rx in hw
--
--
--
--        @return       error message if fails
--
local function serdes_ap_rx_func(params)
  local command_data = Command_Data()
  local devNum, portNum
  local GT_OK=0
  local numOfSerdesLanesPtr=0
  local devnum_portnum_string
  local ret
  local serdesLane = 0
  local serdesRxCfgPtr={}
  
  serdesRxCfgPtr.squelch=params.sqlch                      --squelch
  serdesRxCfgPtr.lowFrequency=params.ffeRes                --ffeRes
  serdesRxCfgPtr.highFrequency=params.ffeCap               --ffeCap
  serdesRxCfgPtr.dcGain=params.dcGain                      --dcGain
  serdesRxCfgPtr.bandWidth=params.bandWidth                --bandWidth
  serdesRxCfgPtr.loopBandwidth=params.loopBandwidth        --loopBandwidth
  serdesRxCfgPtr.etlMinDelay=params.etlMinDelay            --etlMinDelay
  serdesRxCfgPtr.etlMaxDelay=params.etlMaxDelay            --etlMaxDelay
  serdesRxCfgPtr.etlEnable=params.etlEnable                --etlEnable
  serdesRxCfgPtr.fieldOverrideBmp=params.fieldBmp          --fieldBmp
  serdesRxCfgPtr.serdesSpeed=params.speed                  --speed
 
  if true == command_data["status"] then
    local iterator
    for iterator, devNum, portNum in command_data:getInterfacePortIterator() do

            command_data:clearPortStatus()
            command_data:clearLocalStatus()

      if(nil~=devNum) then
                devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                                                         '/' ..
                                                         tostring(portNum), 9)
          ret = myGenWrapper("cpssDxChPortApSerdesRxParametersManualSet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
            {"IN","GT_U8","serdesLane",serdesLane},
            {"IN","CPSS_PORT_AP_SERDES_RX_CONFIG_STC","serdesRxCfgPtr",serdesRxCfgPtr}})
          if(ret~=GT_OK) then
            print(devnum_portnum_string.." Error cpssDxChPortApSerdesRxParametersManualSet ret: "..to_string(ret).." "..returnCodes[ret])
          end
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
CLI_addCommand("interface", "serdes manual ap configuration rx", {
  func   = serdes_ap_rx_func,
  help   = "Configure specific parameters of serdes RX in HW for ap port",
  params = {
    {   type="named", { format="cfg_speed %GT_U8",                     name="speed",         help="the serdes speed(8=10G or 19=25G)"},
                      { format="sq_threshold %type_sqlch",             name="sqlch",         help="Threshold that trips the Squelch detector"},
                      { format="ffe_res %type_ffeRes",                 name="ffeRes",        help="Mainly controls the low frequency gain"    },
                      { format="ffe_cap %type_ffeCap",                 name="ffeCap",        help="Mainly controls the high frequency gain"  },
                      { format="dc_gain %type_dcGain",                 name="dcGain",        help="DC-Gain value"                           },
                      { format="band_width %type_bandWidth",           name="bandWidth",     help="CTLE Band-width "                         },
                      { format="loop_bandwidth %type_loopBandwidth",   name="loopBandwidth", help="DCTLE Loop Band-width"                     },
                      { format="etl_min_delay %GT_U8",                 name="etlMinDelay",   help="ETL min delay"                     },
                      { format="etl_max_delay %GT_U8",                 name="etlMaxDelay",   help="ETL max delay"                     },
                      { format="etl_enable %bool",                     name="etlEnable",     help="ETL enableh"                     },
                      { format="field_bmp %GT_U16",                     name="fieldBmp",     help="Field Bitmap: Bit0-SQLCH,Bit1-LF,Bit2-HF,Bit3-DCGAIN,Bit4-BANDW,Bit5-LOOPB,Bit6-ETLMIN,bit7-ETLMAX,bit8-ETLEN"                            },
					  mandatory = {"speed","sqlch","ffeRes","ffeCap","dcGain","bandWidth","loopBandwidth","etlMinDelay","etlMaxDelay","etlEnable", "fieldBmp"},
                    requirements={
                            ["sq_threshold"] =      {"cfg_speed"},
                            ["ffe_res"] =      {"sq_threshold"},
                            ["ffe_cap"] =      {"ffe_res"},
                            ["dc_gain"]  =      {"ffe_cap"},
                            ["band_width"]  =    {"dc_gain"},
                            ["loop_bandwidth"] = {"band_width"},
                            ["etl_min_delay"] = {"loop_bandwidth"},
                            ["etl_max_delay"] = {"etl_min_delay"},
                            ["etl_enable"] = {"etl_max_delay"},
                            ["field_bmp"] = {"etl_enable"}
                        }
               }}}
)
