-- ************************************************************************

function setSerdesDefaultPort(param)
local mode
    serdes_default_test_port = param["serdes_default_test_port"]

    --print("setSerdesDefaultPort mode = "..to_string(serdes_default_test_port))
    if serdes_default_test_port == nil then
    print("Error: undefined serdes_default_test_port")
        return false
    end

    Test_Serdes_Port = tonumber(serdes_default_test_port)
    serdes_env.PortNum  = Test_Serdes_Port
    print("serdes-default-test-port = "..to_string(serdes_env.PortNum))

    return true
end

-- ************************************************************************

CLI_addCommand("debug", "set serdes-default-test-port", {
    func = setSerdesDefaultPort,
    help = "Set serdes default port for tests",
	params={
		{   type="values",
            { format="%GT_U8",name="serdes_default_test_port",
                help="serdes default test port",
            }
		}
    }
})
