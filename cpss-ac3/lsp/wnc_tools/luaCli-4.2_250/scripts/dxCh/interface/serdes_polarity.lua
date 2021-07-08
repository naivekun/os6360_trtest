--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_polarity.lua
--*
--* DESCRIPTION:
--*     Configure serdes polarity  
--*
--* 
--*       
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  serdes_polarity_func
--        @description  configure serdes polarity 
--
--
--
--        @return       error message if fails
--
local function serdes_polarity_func(params)
  local command_data = Command_Data()
  local devNum, portNum
  local laneBmp=0
  local GT_OK=0
  local ifMode
  local startSerdesPtr
  local numOfSerdesLanesPtr
        local ret, localSerdesPolarityPtr
  local laneNum
  local invertTx
  local invertRx

  invertTx=params.invertTx
  invertRx=params.invertRx


    if params.laneNum ~= "all" then

      laneNum=params.laneNum
        laneBmp = bit_or(laneBmp, bit_shl(1, laneNum))

      if true == command_data["status"] then
            local iterator
            for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
                command_data:clearPortStatus()     
                command_data:clearLocalStatus()
          if(nil~=devNum) then

            ret = myGenWrapper("cpssDxChPortSerdesPolaritySet",{
              {"IN","GT_U8","devNum",devNum},
              {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
              {"IN","GT_U32","laneBmp",laneBmp},
              {"IN","GT_BOOL","invertTx",invertTx},
              {"IN","GT_BOOL","invertRx",invertRx}})

            if(ret~=GT_OK) then
              print("Error at serdes tx command")
            end
          end
        end
      end
  else
      if true == command_data["status"] then
            local iterator
            for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
                command_data:clearPortStatus()     
                command_data:clearLocalStatus()
          if(nil~=devNum) then


                    ret, localSerdesPolarityPtr = myGenWrapper("cpssDxChPortInterfaceModeGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})

                    ifMode=localSerdesPolarityPtr["ifModePtr"] 

                    if(ret~=GT_OK) then
                            command_data:addError("Error at Serdes Polarity command")
                    end

                    ret, localSerdesPolarityPtr = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                        {"OUT","GT_U32","startSerdesPtr"},
                        {"OUT","GT_U32","numOfSerdesLanesPtr"}})

                    startSerdesPtr=localSerdesPolarityPtr["startSerdesPtr"]
                    numOfSerdesLanesPtr=localSerdesPolarityPtr["numOfSerdesLanesPtr"]

                    if(ret~=GT_OK) then
                            command_data:addError("Error at Serdes Polarity command")
                    end

                    for i=0,numOfSerdesLanesPtr-1,1 do

                        laneNum=i
                        laneBmp = bit_or(laneBmp, bit_shl(1, laneNum))

                ret = myGenWrapper("cpssDxChPortSerdesPolaritySet",{
                  {"IN","GT_U8","devNum",devNum},
                  {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                  {"IN","GT_U32","laneBmp",laneBmp},
                  {"IN","GT_BOOL","invertTx",invertTx},
                  {"IN","GT_BOOL","invertRx",invertRx}})

                if(ret~=GT_OK) then
                  print("Error at serdes tx command")
                end
                    end
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
-- command registration: serdes_polarity
--------------------------------------------

CLI_addCommand("interface", "serdes polarity", {
  func   = serdes_polarity_func,
  help   = "Configure Rx/Tx polarity of serdes",
  params = {
     {   type="named", { format="lane_num  %LaneNumberOrAll",name="laneNum",  help="Number of SERDES lane of port or all"},
                       { format="invert_tx %bool",           name="invertTx", help= "Is the Tx polarity Reversed"},
                       { format="invert_rx %bool",           name="invertRx", help= "Is the Rx polarity Reversed"},
             requirements={
                    ["invert_tx"] = {"lane_num"},
                    ["invert_rx"] = {"invert_tx"}
                },
             mandatory = {"laneNum","invertTx","invertRx"}
                       }
      }
   }
)

CLI_addHelp("interface", "serdes", "Configure serdes parameters")


