--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* PX_interface_led.lua
--*
--* DESCRIPTION:
--*       Set interface (per port) configuration related to LED.
--*       LED interface
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes
--require
--constants


-- ************************************************************************
--  portClassPolarityInvertEnable
--
--  @description Set the polarity of the selected indications
--
--  @param params - the parameters
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function portClassPolarityInvertEnable(command_data, devNum, portNum, params)

    local enable = true

    if (params.flagNo==true) then
        enable = false
    end

    for i,class in pairs(params.classRange) do
        local apiName = "cpssPxLedStreamPortClassPolarityInvertEnableSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
                    { "IN", "GT_SW_DEV_NUM", "devNum", devNum},
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    { "IN", "GT_U32", "classNum", class},
                    { "IN", "GT_BOOL", "invertEnable", enable}
                })
    end
end

-- ************************************************************************
--
--  portClassPolarityInvertEnableFunc
--        @description  Set the polarity of the selected indications
--
--        @param params             - params["classRange"]: LED stream class range
--
--        @return       true on success, otherwise false and error message
--
local function portClassPolarityInvertEnableFunc(params)
    return generic_port_range_func(portClassPolarityInvertEnable,params)
end

--------------------------------------------------------------------------------
-- command registration: invert-classes
--------------------------------------------------------------------------------
CLI_addCommand("interface", "led invert-classes", {
    func   = portClassPolarityInvertEnableFunc,
    help   = "Invert the polarity of the selected indications",
    params = {
        { type="values",
            { format = "%classRange", name = "classRange", help = "Invert a LED class data" },
        }
    }
})

--------------------------------------------
-- command registration: no invert-classes
--------------------------------------------
CLI_addCommand("interface", "no led invert-classes", {
    func=function(params)
        params.flagNo=true
        return portClassPolarityInvertEnableFunc(params)
    end,
    help="Don't invert the polarity of the selected indications",
    params = {
        { type="values",
           { format = "%classRange", name = "classRange", help = "Invert a LED class data" },
        }
    }
})



-- ************************************************************************
--  ledStreamPortPositionSet
--
--  @description Sets LED port slot number in the LED's chain
--
--  @param params - the parameters
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function ledStreamPortPositionSet(command_data, devNum, portNum, params)
    local position

    if (params.flagNo==true) then
        position = 0x3f
    else
        position = params["slotNumber"]
    end

    local apiName = "cpssPxLedStreamPortPositionSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
                { "IN", "GT_SW_DEV_NUM", "devNum", devNum},
                { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                { "IN", "GT_U32", "position", position},
            })
end

-- ************************************************************************
--  ledStreamPortPositionSetFunc
--
--  @description Sets LED port slot number in the LED's chain
--
--  @param params - the parameters
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function ledStreamPortPositionSetFunc(params)
    return generic_port_range_func(ledStreamPortPositionSet,params)
end

--------------------------------------------------------------------------------
-- command registration: led port-slot
--------------------------------------------------------------------------------
CLI_addCommand("interface", "led port-slot", {
  func   = ledStreamPortPositionSetFunc,
  help   = "Sets LED port slot number in the LED's chain",
  params = {
        { type="values",
           { format = "%slotNumber", name = "slotNumber", help = "Set LED port slot number in LED stream" },
        }
      }
})

--------------------------------------------
-- command registration: no led port-slot
--------------------------------------------
CLI_addCommand("interface", "no led port-slot", {
    func=function(params)
        params.flagNo=true
        return ledStreamPortPositionSetFunc(params)
    end,
  help="Sets default LED port slot number in the LED's chain",
})
