--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ap-engine.lua
--*
--* DESCRIPTION:
--*     Set AppDemo DataBase value. This value will be considered during system initialization process.  
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
--        @description  set appDemo database value
--
--
--
--        @return       error message if fails
--
local function ap_func_engine(params)
  local command_data = Command_Data()
  local GT_OK = 0
  local ret
  
  if(params.enable==true) then  
    ret = myGenWrapper("appDemoDbEntryAdd",{
        {"IN","string","namePtr","apEnable"},
        {"IN","GT_U32","value",1}
    })
  else
    ret = myGenWrapper("appDemoDbEntryAdd",{
        {"IN","string","namePtr","apEnable"},
        {"IN","GT_U32","value",0}
    })
  end
  
  if(ret~=GT_OK) then
    command_data:addError("Error at ap-engine command")
  end
  
  -- Command data postprocessing    
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()  
  

      
end


--------------------------------------------
-- command registration: ap-engine
--------------------------------------------
CLI_addCommand("config", "ap-engine", {
  func=ap_func_engine,
  help="This command should be called before cpssInitSystem",
  params={ 
      {   type="named", { format="enable %bool", name="enable"},
        mandatory={"enable"}
      }
    }
  }
)

