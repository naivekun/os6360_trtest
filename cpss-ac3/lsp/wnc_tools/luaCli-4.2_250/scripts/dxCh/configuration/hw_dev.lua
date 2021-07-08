--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* flow_control.lua
--*
--* DESCRIPTION:
--*       configuration of the flow control of a given interface running at 
--*       full-duplex 
--*
--* FILE REVISION NUMBER:
--*       $Revision: 9 $
--*


--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  hw_devid
--        @description        Sets value of appDemoHwDevNumOffset used during device HW ID calculation
--                            based on device SW ID..
--
--        @param params         - params["hwdevid"]: device id 
--
--
--        @return       true on success, otherwise false and error message
--
local function hw_devid(params)
  local status, err
  local ret, values
  local command_data = Command_Data()

  local id = params["hwdevid"]
  ret = myGenWrapper("appDemoHwDevNumOffsetSet",
                        {{"IN","GT_U8","hwDevNumOffset", tonumber(id)}}) 
  if (ret ~= 0) then
      command_data:setFailStatus()
      command_data:addError("Error at hw Device number: %d",
                                  tonumber(id))
  end    
  ret = myGenWrapper("cpssDxChCfgHwDevNumSet",
                        {{"IN","GT_U8","devNum", devNum},
                         {"IN","GT_HW_DEV_NUM","hwDevNum",tonumber(id)}})
  if (ret ~= 0) then
      command_data:setFailStatus()
      command_data:addError("If you used this command before doing initialization ignore this error. Otherwise: Error occurred at hw Device number: %d",
                                  tonumber(id))
  end               
  command_data:analyzeCommandExecution()
  command_data:printCommandExecutionResults()
  return command_data:getCommandExecutionResults()
end

-- ************************************************************************
---
--  show_hw_devid
--        @description        Show hwDevice ID offset
--
--        @param params       - none 
--
--        @return       true on success, otherwise false and error message
--
local function show_hw_devid(params)
  -- Common variables declaration
  local result, values
  local command_data = Command_Data()
  -- Command  specific variables declaration
  local hw_device_id_number, hw_device_id_number_string 
  local hw_device_id_number_string_prefix, hw_device_id_number_string_suffix
  local header_string, footer_string  

  -- Command specific variables initialization. 
  header_string                            = ""
  hw_device_id_number_string_prefix        = "HW Device number\t\t\t\t\t\t"
  hw_device_id_number_string_suffix        = ""
  footer_string                            = "\n"


  -- Common variables initialization.
  command_data:clearResultArray()    

  -- Getting of hwDevice_id offset.  
  command_data:clearStatus()

  if true  == command_data["status"] then 
    result, values = myGenWrapper("cpssDxChCfgHwDevNumGet", 
                                    {{"IN","GT_U8","devNum", devNum},
                                     {"OUT","GT_HW_DEV_NUM","hwDevNumPtr"}})

    if 0 == result then 
      hw_device_id_number = values["hwDevNumPtr"]
    elseif 0 ~= result then
      command_data:setFailStatus()
      command_data:addError("Error at hw Device number of device %d: %s ",
                                  devNum, values["hwDevNumPtr"])
    end
        
    if 0 == result then
      hw_device_id_number_string = strIfNotNil(hw_device_id_number)
    else 
      hw_device_id_number_string = "n/a"     
    end
  end
    
  command_data:updateStatus() 

  -- hwDevice ID offset string formatting and adding.
  command_data:setResultStr(hw_device_id_number_string_prefix, 
                              hw_device_id_number_string,
                              hw_device_id_number_string_suffix)
  command_data:addResultToResultArray()

  -- Resulting table string formatting.
  command_data:setResultArrayToResultStr()
  command_data:setResultStr(header_string, command_data["result"], footer_string)
  command_data:analyzeCommandExecution()
  command_data:printCommandExecutionResults()
    
  return command_data:getCommandExecutionResults()
end



--------------------------------------------
-- command registration: hwDevice offset 
--------------------------------------------
CLI_addCommand("config", "hwDevice", {
    func=hw_devid,
    help="Set the HW device Id",
    params={
        {   type="named", 
        { format="id %hwdevid", name="hwdevid", help="The device number"},
          mandatory = {"hwdevid"}
        }
    }
})

--------------------------------------------
-- command registration: show hwDevice 
--------------------------------------------
CLI_addHelp("exec", "show hwDevice", "Show hwDevice offset")
CLI_addCommand("exec", "show hwDevice", {
    func=show_hw_devid,
    help="Show hwDevice offset",
    params={}
})
