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
--  serdes_rx_func
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
  local serdesRxCfgPtr={}
  local ret
  
  serdesRxCfgPtr.sqlch=params.sqlch      --sqlch
  serdesRxCfgPtr.ffeRes=params.ffeRes      --ffeRes
  serdesRxCfgPtr.ffeCap=params.ffeCap      --ffeCap
  serdesRxCfgPtr.align90=params.align90    --align90
  local laneNum=params.laneNum            --laneNum

  if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in 
                command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()     
            command_data:clearLocalStatus()
      if(nil~=devNum) then 
        ret = myGenWrapper("cpssDxChPortSerdesManualRxConfigSet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
          {"IN","GT_U32","laneNum",laneNum},
          {"IN","CPSS_DXCH_PORT_SERDES_RX_CONFIG_STC","serdesRxCfgPtr",serdesRxCfgPtr}})
        if(ret~=GT_OK) then
          print("Error at serdes rx command")
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
-- command registration: serdes_rx
--------------------------------------------
CLI_addCommand("interface", "serdes manual configuration rx", {
  func   = serdes_rx_func,
  help   = "Configure specific parameters of serdes RX in HW",
  params = {
  {   type="named", { format="lane_num %lane_number_type",name="laneNum", help="Number of SERDES lane of port"      },
            { format="sq_threshold %GT_U32",     name="sqlch",   help="Threshold that trips the Squelch detector"},
            { format="ffe_res %GT_U32",       name="ffeRes",   help="Mainly controls the low frequency gain"  },
                    { format="ffe_cap %GT_U32",       name="ffeCap",   help="Mainly controls the high frequency gain"  },
            { format="align_90 %GT_U32",       name="align90", help="Align 90 Calibration Phase Offset"    },
          mandatory = {"laneNum","sqlch","ffeRes","ffeCap","align90"},
          requirements={
              ["sq_threshold"] =   {"lane_num"},
              ["ffe_res"] =     {"sq_threshold"},
              ["ffe_cap"] =     {"ffe_res"},
              ["align_90"] =     {"ffe_cap"}
            }
         }}}
)

