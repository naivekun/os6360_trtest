--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* interface_led.lua
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
        local apiName = "cpssDxChLedStreamPortClassPolarityInvertEnableSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
                    { "IN", "GT_U8", "devNum", devNum},
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

CLI_type_dict["port_class_indication_values"] = {
  checker = CLI_check_param_enum,
  complete = CLI_complete_param_enum,
  help = "Port class indications",
  enum = {
    ["port-enable"] = {value="CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E",
            help="Enable Port" },
    ["link-ok"]  = {value="CPSS_DXCH_LED_INDICATION_LINK_E",
            help="Port link up" },
    ["activity"]  = {value="CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E",
            help="Receive or Transmit activity" },
    ["rx-error"]  = {value="CPSS_DXCH_LED_INDICATION_RX_ERROR_E",
            help="Receive errors" },
    ["tx-fc"]  = {value="CPSS_DXCH_LED_INDICATION_FC_TX_E",
            help="Flow Control packets are sent on port" },
    ["rx-fc"]  = {value="CPSS_DXCH_LED_INDICATION_FC_RX_E",
            help="Flow Control packets received on port" },
    ["full-duplex"]  = {value="CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E",
            help="Full Duplex"},
    ["gmii-speed"]  = {value="CPSS_DXCH_LED_INDICATION_GMII_SPEED_E",
            help="GMII Speed"},
    ["mii-speed"]  = {value="CPSS_DXCH_LED_INDICATION_MII_SPEED_E",
            help="MII Speed"},
   }
}

-- ************************************************************************
--  portClassIndicationSet
--
--  @description Set the polarity of the selected indications
--
--  @param params - the parameters
--
--  @return  true on success, otherwise false and error message
--
-- ************************************************************************
local function portClassIndicationSet(command_data, devNum, portNum, params)

    local CPSS_DXCH_LED_INDICATION_ENT indication = params["indication"]
    for i,class in pairs(params.classRange) do
        local apiName = "cpssDxChLedStreamPortClassIndicationSet"
        genericCpssApiWithErrorHandler(command_data,
            apiName, {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    { "IN", "GT_U32", "classNum", class},
                    { "IN", "CPSS_DXCH_LED_INDICATION_ENT", "indication", indication}
                })
    end
end

-- ************************************************************************
--
--  portClassIndicationSetFunc
--        @description      Set the indication of the selected class
--
--        @param params     - params["classRange"]: LED stream class range
--
--        @return           true on success, otherwise false and error message
--
local function portClassIndicationSetFunc(params)
    return generic_port_range_func(portClassIndicationSet,params)
end

--------------------------------------------------------------------------------
-- command registration: port class indication 
--------------------------------------------------------------------------------
CLI_addCommand("interface", "led", {
    func   = portClassIndicationSetFunc,
    help   = "Sets the LED indication to the selected class",
    params = {
        {type="named",
        {format="class %classRange", name = "classRange", help = "Class range to set the indication data"},
        {format="indication %port_class_indication_values", name="indication" , help="Indication type"},
         mandatory = {"classRange", "indication"}
        }
    }
})

--------------------------------------------------------------------------------
-- command registration: no port class indication
--------------------------------------------------------------------------------
CLI_addCommand("interface", "no led", {
    func=function(params)
        params.flagNo=true
        params.indication="CPSS_DXCH_LED_INDICATION_LINK_E"
        return portClassIndicationSetFunc(params)
    end,
    help   = "Sets the default LED indication to the selected class",
    params = {
        {type="named",
        {format="class %classRange", name = "classRange", help = "Class range to set the indication data"},
         mandatory = {"classRange"}
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

    local apiName = "cpssDxChLedStreamPortPositionSet"
    genericCpssApiWithErrorHandler(command_data,
        apiName, {
                { "IN", "GT_U8", "devNum", devNum},
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

-- *****************************************************************************
--  extraSgmiiIndicationsEn
--
--  @description By default some important indications of FlexLink
--               (HyperG.stack) ports in SGMII state – activity, GMII speed,
--               MII speed – are not sent to the  LED stream. Enabling this
--               option affects on behavior of the command
--               led class custom-indication. It replaces following indications:
--               p_rej -> activity, jabber -> GMII speed, fragment -> MII speed.
--
--  @param params   - not used.
--
--  @return  true on success, otherwise false and error message
--
local function extraSgmiiIndicationsEn(params)
    local command_data = Command_Data()
    local enable = params.flagNo and false or true
    command_data:initInterfaceDevPortRange()
    for _, devNum, portNum in command_data:getPortIterator() do
        local rc = myGenWrapper(
            "cpssDxChLedStreamPortFlexTriSpeedIndicationEnableSet",{
                {"IN", "GT_U8",                "devNum",  devNum},
                {"IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                { "IN","GT_BOOL",              "enable",  enable}})
        command_data:handleCpssErrorDevPort(
            rc, "enabling extra SGMII indications ", devNum, portNum)
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
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

--------------------------------------------------------------------------------
-- command registration: led enable-extra-sgmii-indications
--------------------------------------------------------------------------------
CLI_addCommand({"interface"}, "led enable-extra-sgmii-indications",
{
    func = extraSgmiiIndicationsEn,
    help = "Enable adding of extra GMII indications (GMII/MII speed, activity)"..
        "for FlexLink ports working in SGMII mode",
})

--------------------------------------------------------------------------------
-- command registration: no led enable-extra-sgmii-indications
--------------------------------------------------------------------------------
CLI_addCommand({"interface"}, "no led enable-extra-sgmii-indications",
{
    func = function(params)
        params.flagNo = true
        return extraSgmiiIndicationsEn
    end,
    help = "Disable adding of extra GMII indications (GMII/MII speed, activity)"..
        "for FlexLink ports working in SGMII mode",

})

CLI_addHelp("interface", "led", "Interface configurations related to LED")

