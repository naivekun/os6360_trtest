--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* diag_temp_sensors_select.lua
--*
--* DESCRIPTION:
--*       Select Temperature Sensors.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

-- Types defined:
--
-- Descriptor types,
--
-- Note: in case the ENUM CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT in the file: mainPpDrv\h\cpss\dxCh\dxChxGen\diag\cpssDxChDiag.h
-- changes this file must be updated as well
--
CLI_type_dict["sensorType"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "temperature sensor types",
    enum = {
        ["TEMPERATURE_SENSOR_0"] = { value=0, help="Temperature Sensor 0" },
        ["TEMPERATURE_SENSOR_1"] = { value=1, help="Temperature Sensor 1" },
        ["TEMPERATURE_SENSOR_2"] = { value=2, help="Temperature Sensor 2" },
        ["TEMPERATURE_SENSOR_3"] = { value=3, help="Temperature Sensor 3" },
        ["TEMPERATURE_SENSOR_4"] = { value=4, help="Temperature Sensor 4" },
        ["TEMPERATURE_SENSOR_AVERAGE"] = { value=5, help="Average temperature of all sensors" }
    }
} 

-- ************************************************************************
---
--  temperatureSensorsSelect
--        @description   Select Temperature Sensors.
--
--        @param params         - The parameters
--
local function temperatureSensorsSelect(params)
    local result,values
    local status, err

    result, values = myGenWrapper(
        "cpssDxChDiagDeviceTemperatureSensorsSelectSet", {
            { "IN", "GT_U8"  , "devNum", params["devID"]},	-- devNum
            { "IN", "GT_U32" , "sensorType",  params["sensorType"]},
            }
    )

    if (result ~= 0) then
        status = false
        err = returnCodes[result]
    end		
    
    return status, err
end
CLI_addHelp("config", "diag", "Configure diag")
CLI_addCommand("config", "diag temperature sensors select", {
    func = temperatureSensorsSelect,
    help = "Select Temperature Sensors",
    params={
        { type="values", "%devID" },{ type="named",	{ format="sensorType %sensorType",name="sensorType", help="(sensorType) - the type of the Temperature Sensors\n" } ,mandatory={"devID","sensorType"},requirements={sensorType={"devID"}}}
    }
})
 
