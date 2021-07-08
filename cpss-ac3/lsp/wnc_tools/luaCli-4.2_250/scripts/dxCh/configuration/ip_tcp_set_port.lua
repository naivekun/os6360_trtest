--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* ip_tcp_set_port.lua
--*
--* DESCRIPTION:
--*       Set cmdshell and luaCLI tcp port
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants 


do

-- ************************************************************************
---
--  service_set_tcp_port
--        @description  Set tcp port for service, restart it
--
--        @param service  -service name
--        @param port     -service port number
--
--        @return       true on success, otherwise false and error message
-- 
local function service_set_tcp_port(service, port)
    local rc
    rc = myGenWrapper("cmdStreamSocketServiceIsRunning", {
        {"IN","string","service",service}})
    if rc ~= 0 then
        -- GT_TRUE
        print("service '"..service.."' running, stop it")
        myGenWrapper("cmdStreamSocketServiceStop", {
            {"IN","string","service",service}})
        myGenWrapper("osTimerWkAfter",{{"IN","GT_U32","mils", 2000}})
    end
    rc = myGenWrapper("cmdStreamSocketServiceSetPort", {
        {"IN", "string", "service", service},
        {"IN", "GT_U16", "port",    port}})
    if rc == 0 then
        -- GT_OK, start service
        print("restarting service '"..service.."'...")
        rc = myGenWrapper("cmdStreamSocketServiceStart", {
            {"IN","string","service",service}})
        if rc ~= 0 then
            print("failed to start service '"..service.."' rc="..to_string(rc))
            return false
        end
    else
        -- rc ~= GT_OK, no service configured yet
        globalSet("_p_"..service, port);
    end
    return true
end

-- ************************************************************************
---
--  service_stop
--        @description  Stop tcp service
--
--        @param service  -service name
--
--        @return       true on success, otherwise false and error message
-- 
local function service_stop(service)
    local rc
    rc = myGenWrapper("cmdStreamSocketServiceIsRunning", {
        {"IN","string","service",service}})
    if rc ~= 0 then
        -- GT_TRUE
        print("service '"..service.."' running, stop it")
        myGenWrapper("cmdStreamSocketServiceStop", {
            {"IN","string","service",service}})
        myGenWrapper("osTimerWkAfter",{{"IN","GT_U32","mils", 2000}})
    else
        print("service '"..service.."' is not running now")
    end
    return true
end

CLI_type_dict["tcp_port"] = {
    help = "TCP port number (1..65535)",
    checker = CLI_check_param_number,
    min = 1,
    max = 65535
}

--------------------------------------------------------
-- command registration: ip tcp cli-port
--------------------------------------------------------
CLI_addHelp("config", "ip tcp", "Configure TCP services")
CLI_addCommand("config", "ip tcp cli-port", {
  func   = function(params)
      return service_set_tcp_port("luaCLI", params.tcp_port)
  end,
  help   = "configure tcp port for cli",
  params = {{ type = "values",
          "%tcp_port"
  }}
})

--------------------------------------------------------
-- command registration: ip tcp cli-port
--------------------------------------------------------
CLI_addCommand("config", "ip tcp cmd-port", {
  func   = function(params)
      return service_set_tcp_port("cmdShell", params.tcp_port)
  end,
  help   = "configure tcp port for cmd",
  params = {{ type = "values",
          "%tcp_port"
  }}
})

CLI_addHelp("config", "no ip tcp", "Stop TCP services")
CLI_addCommand("config", "no ip tcp cli-port", {
  func   = function(params)
      return service_stop("luaCLI")
  end,
  help   = "Stop tcp service for cli"
})
CLI_addCommand("config", "no ip tcp cmd-port", {
  func   = function(params)
      return service_stop("cmdShell")
  end,
  help   = "Stop tcp service for cmd"
})
end
