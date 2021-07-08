--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_reset.lua
--*
--* DESCRIPTION:
--*     Configure serdes reset  
--*
--* 
--*       
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  serdes_reset_func
--        @description  configure serdes reset 
--
--
--
--        @return       error message if fails
--

--------------------------------------------
local function serdes_reset_common_func(params)

  local command_data = Command_Data()
  local devNum, portNum
  local GT_OK=0
    local ret
  local state=0
  local val=0
  local ifMode=0
  local startSerdesPtr=0
  local numOfSerdesLanesPtr=0

    if params.flagNo == true then
        state=false
    else
        state=true
    end

    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in command_data:getInterfacePortIterator() do
            command_data:clearPortStatus()     
            command_data:clearLocalStatus()
            if(nil~=devNum) then

                command_data:clearLocalStatus()

                ret, val = myGenWrapper("cpssDxChPortInterfaceModeGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})


                if(ret==GT_OK) then
                    ifMode=val["ifModePtr"] 
                    ret, val = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                        {"OUT","GT_U32","startSerdesPtr"},
                        {"OUT","GT_U32","numOfSerdesLanesPtr"}})



                    if(ret==GT_OK) then

                        startSerdesPtr=val["startSerdesPtr"]
                        numOfSerdesLanesPtr=val["numOfSerdesLanesPtr"]

                        if(numOfSerdesLanesPtr~=0) then
                            ret = myGenWrapper("cpssDxChPortSerdesResetStateSet",{
                                {"IN","GT_U8","devNum",devNum},
                                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                                {"IN","GT_BOOL","state",state}})

                            if(ret~=GT_OK) then
                                print(tostring(devNum).."/"..tostring(portNum).." Error cpssDxChPortSerdesResetStateSet ret "..to_string(ret).." = "..returnCodes[ret])
                            --else
                            --    print(tostring(devNum).."/"..tostring(portNum).." cpssDxChPortSerdesResetStateSet Reset = "..to_string(state))
                            end                    
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

----------------------------------------------
local function serdes_no_reset_func(params)
    params.flagNo=true
    return serdes_reset_common_func(params)
end

----------------------------------------------
CLI_addCommand("interface", "serdes reset", {
func   = serdes_reset_common_func,
      help   = "Serdes Reset",
      params = {}
   }
)

--------------------------------------------
-- command registration: serdes_no_reset
--------------------------------------------

CLI_addCommand("interface", "no serdes reset", {
      func   = serdes_no_reset_func,
      help   = "Take Serdes out of reset",
      params = {}
   }
)
