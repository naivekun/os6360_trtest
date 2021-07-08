--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_tx.lua
--*
--* DESCRIPTION:
--*     Configure specific parameters of serdes TX in HW  
--*
--* 
--*       
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  serdes_tx_func
--        @description  configure specific parameters of serdes tx in hw
--
--
--
--        @return       error message if fails
--
local function serdes_tx_func(params)
  local command_data = Command_Data()
  local devNum, portNum
  local GT_OK=0
  local serdesTxCfgPtr={}
  local ret
  serdesTxCfgPtr.txAmp=params.txAmp      --txAmp
  serdesTxCfgPtr.txAmpAdjEn=params.txAmpAdjEn  --txAmpAdjEn
  serdesTxCfgPtr.emph0=params.emph0      --emph0
  serdesTxCfgPtr.emph1=params.emph1      --emph1
  serdesTxCfgPtr.txAmpShft=params.txAmpShft  --txAmpShft
  local laneNum=params.laneNum            --laneNum
  

  if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in 
                command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()     
            command_data:clearLocalStatus()
      if(nil~=devNum) then
        ret = myGenWrapper("cpssDxChPortSerdesManualTxConfigSet",{
          {"IN","GT_U8","devNum",devNum},
          {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
          {"IN","GT_U32","laneNum",laneNum},
          {"IN","CPSS_DXCH_PORT_SERDES_TX_CONFIG_STC","serdesTxCfgPtr",serdesTxCfgPtr}})
        if(ret~=GT_OK) then
          print("Error at serdes tx command")
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
-- command registration: serdes_tx
--------------------------------------------
CLI_addCommand("interface", "serdes manual configuration tx", {
  func   = serdes_tx_func,
  help   = "Configure specific parameters of serdes TX in HW",
  params = {
  {   type="named", { format="lane_num %lane_number_type",name="laneNum",   help="Number of SERDES lane of port"    },
            { format="amp %GT_U32",         name="txAmp",     help="Tx Driver output amplitude"      },
            { format="amp_adj_enable %bool",     name="txAmpAdjEn",   help="Transmitter Amplitude Adjust"      },
                    { format="emph_0 %GT_U32",       name="emph0",     help="Emphasis amplitude for Gen0 bit rates"},
            { format="emph_1 %GT_U32",       name="emph1",     help="Emphasis amplitude for Gen1 bit rates"},
              { format="amp_shift %bool",       name="txAmpShft",   help="Transmitter Amplitude Shift"      },    
          mandatory = {"laneNum","txAmp","txAmpAdjEn","emph0","emph1","txAmpShft"},
          requirements={
              ["amp"] =        {"lane_num"},
              ["amp_adj_enable"] = {"amp"},
              ["emph_0"] =      {"amp_adj_enable"},
              ["emph_1"] =      {"emph_0"},
              ["amp_shift"] =    {"emph_1"}
            }
         }}}
)

