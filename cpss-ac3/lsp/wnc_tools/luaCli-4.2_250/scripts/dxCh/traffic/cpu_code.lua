--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpu_code.lua
--*
--* DESCRIPTION:
--*       CPU code to physical port and/or queue mapping.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


local bc3Mg2DmaArr = { [0]=74, [1]=0x75, [2]=0x76,[3]=0x77 }

local function func_cpu_code_bind(params)
    local command_data = Command_Data() 
    local i
    local cpuCode, values
    local entry
    local mg
    

    
    local devFamily = wrLogWrapper("wrlCpssDeviceFamilyGet", "(devNum)", devNum)

    params.devID = tonumber(params.devID)
    if (params.port ~=nil) then
        params.port = tonumber(params.port)
    end


    if (params.cpu_code_range ~= nil) then
        for i,cpu_code in pairs(params.cpu_code_range) do                
            -- CPU Code CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + given HW value
            cpuCode = dsa_cpu_code_to_api_convert(cpu_code)
            if (params.queue ~= nil) then
                --read mofify write
                result, values = myGenWrapper("cpssDxChNetIfCpuCodeTableGet", {
                                        { "IN",     "GT_U8",      "devNum",     devNum },
                                        { "IN",     "CPSS_NET_RX_CPU_CODE_ENT",    "cpuCode",    cpuCode },
                                        { "OUT",     "CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC",    "entryInfoPtr"}
                                      })


                if (result ~= 0) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at CPU Code table get" ..
                                  "on device %d CPU Code %d: %s",
                                  params.devID, cpuCode, returnCodes[result])
                end	
                values["entryInfoPtr"].tc = params.queue

                result = myGenWrapper("cpssDxChNetIfCpuCodeTableSet",{
                                    {"IN","GT_U8","devNum",params.devID},
                                    {"IN",TYPE["ENUM"],"cpuCode",cpuCode},
                                    {"IN","CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC","entryInfoPtr",values["entryInfoPtr"]}
                                })

                if (result ~= 0) then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at CPU Code table set" ..
                                              "on device %d CPU Code %d: %s",
                                              params.devID, cpuCode, returnCodes[result])
                end	
                if ("CPSS_PP_FAMILY_DXCH_BOBCAT3_E" == devFamily or
                    "CPSS_PP_FAMILY_DXCH_ALDRIN2_E" == devFamily) 
                then
                     mg = math.floor(params.queue/8)
                    
                     --cpu_port = convert_global_dma_to_physical_port(mg2DmaArr[mg])
                     result,cpu_port = myGenWrapper("cpssDxChPortPhysicalPortMapReverseMappingGet",{
                                    {"IN","GT_U8","devNum",params.devID},
                                    {"IN",TYPE["ENUM"],"origPortType",1},--CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E
                                    {"IN","GT_U32","origPortNum",bc3Mg2DmaArr[mg]},
                                    {"OUT","GT_PHYSICAL_PORT_NUM","physicalPortNumPtr"}
                                })
                     if (result ~= 0) then
                         command_data:setFailDeviceAndLocalStatus()
                         command_data:addError("Error at CPU Code to physical port set" ..
                                              "on device %d CPU Code %d: %s",
                                              params.devID, cpuCode, returnCodes[result])
                     end	
                     
                     result = myGenWrapper("cpssDxChNetIfCpuCodeToPhysicalPortSet",{
                                    {"IN","GT_U8","devNum",params.devID},
                                    {"IN",TYPE["ENUM"],"cpuCode",cpuCode},
                                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",cpu_port["physicalPortNumPtr"]}
                                })
                     if (result ~= 0) then
                          command_data:setFailDeviceAndLocalStatus()
                           command_data:addError("Error at CPU Code to physical port set" ..
                                              "on device %d CPU Code %d: %s",
                                              params.devID, cpuCode, returnCodes[result])
                     end	
                else
                    if (params.port ~= nil) then
                        result = myGenWrapper("cpssDxChNetIfCpuCodeToPhysicalPortSet",{
                                    {"IN","GT_U8","devNum",params.devID},
                                    {"IN",TYPE["ENUM"],"cpuCode",cpuCode},
                                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",params.port}
                                })
                      if (result ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error at CPU Code to physical port set" ..
                                              "on device %d CPU Code %d: %s",
                                              params.devID, cpuCode, returnCodes[result])
                      end
                    end
                    	
                end

             else --no queue given
                 if (params.port ~= nil) then
                     result = myGenWrapper("cpssDxChNetIfCpuCodeToPhysicalPortSet",{
                                    {"IN","GT_U8","devNum",params.devID},
                                    {"IN",TYPE["ENUM"],"cpuCode",cpuCode},
                                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",params.port}
                                })
                     if (result ~= 0) then
                                command_data:setFailDeviceAndLocalStatus()
                                command_data:addError("Error at CPU Code to physical port set" ..
                                              "on device %d CPU Code %d: %s",
                                              params.devID, cpuCode, returnCodes[result])
                     end
                 else
                     command_data:setFailDeviceAndLocalStatus()
                     command_data:addError("Error at CPU Code to physical port set" ..
                                           "No <port> or <queue> given")
                 end
             end
         end
     end
        
    command_data:updateStatus()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults() 
end 

-- ************************************************************************
---
--  show_bridge_address_table
--        @description  Displays the FDB entries and filters them
--
--        @param params         - The hash table with the parameters
--
--        @return       true, if successful otherwise the error message
--
function show_cpu_code_binds(params)
	local cpuCode
	local cpuCode
	local result, values, values1, OutValues
	local matches
    local command_data = Command_Data() 
	local fdbEnd = false
    

	print("")
	print("cpu code          port         queue")
	print("------------      -----------  -----")


	for i,cpu_code in pairs(params.cpu_code_range) do                
        -- CPU Code CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + given HW value
        cpuCode = dsa_cpu_code_to_api_convert(cpu_code)
        
        result, values = myGenWrapper("cpssDxChNetIfCpuCodeTableGet", {
                                        { "IN",     "GT_U8",      "devNum",     devNum },
                                        { "IN",     "CPSS_NET_RX_CPU_CODE_ENT",    "cpuCode",    cpuCode },
                                        { "OUT",     "CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC",    "entryInfoPtr"}
                                      })


        if (result ~= 0) then
            command_data:setFailDeviceAndLocalStatus()
            command_data:addError("Error at CPU Code table get" ..
                                  "on device %d CPU Code %d: %s",
                                  params.devID, cpuCode, returnCodes[result])
        end	
        result, values1 = myGenWrapper("cpssDxChNetIfCpuCodeToPhysicalPortGet", {
                                        { "IN",     "GT_U8",      "devNum",     devNum },
                                        { "IN",     "CPSS_NET_RX_CPU_CODE_ENT",    "cpuCode",    cpuCode },
                                        { "OUT",     "GT_PHYSICAL_PORT_NUM",    "portNumPtr"}
                                      })


        if (result ~= 0) then
            command_data:setFailDeviceAndLocalStatus()
            command_data:addError("Error at CPU Code to physical port get" ..
                                  "on device %d CPU Code %d: %s",
                                  params.devID, cpuCode, returnCodes[result])
        end	
        OutValues = values["entryInfoPtr"]
        print(string.format("%d                  %d            %d", cpu_code , values1["portNumPtr"], OutValues.tc))
                      
    end
    	
    print("")

	command_data:updateStatus()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults() 
end


--[[
Command "cpu code bind "
Command to bind between a cpu code (reason) to a destination port and/or a queue

a packet that feets the conditiones to be sent to cpu with the given code (reason), will be sent to the given port
the parameter "port" is optional, if not supplied, only the cpu-code to queue table will be configured
the parameter "queue" is optional, if not supplied, only the cpu-code to physical-port table will be configured

Console(traffic)# cpu code bind device <devNum> cpu-code <cpu-code-range> [port<Port>] [queue <queue-number>]
]]
CLI_addHelp("traffic", "cpu code bind", "bind between a cpu code to a destination physical port and queue number.") 
CLI_addCommand("traffic", "cpu code bind", {
    func = func_cpu_code_bind,
    help="bind between a cpu code to a destination physical port and queue number.",
    params={ { type="named",
        { format="device %devID", name="devID", help = "Device ID"},
        { format="cpu-code %cpu_code_range", name="cpu_code_range", help = "Range of CPU Code numbers from functional specification"},
        { format="port %port",name = "port" ,help="The port number" },
        { format="queue %queue_id_no_all", name="queue", help="The queue number" },
        mandatory={"devID","cpu_code_range"}
      }}}

)

-- *exec*
-- show cpu code binds [cpu_code/all]
CLI_addHelp("exec", "show cpu code binds", "port and queue bound to the cpu code")
CLI_addCommand("exec", "show cpu code binds", {
    func = show_cpu_code_binds,
    help = "Display port and queue bound to the cpu code",
    params={
        { type="values", "%devID" },
        { type="named",
            { format="device %devID", name="devID", help = "Device ID"},
            { format="cpu-code %cpu_code_range", name="cpu_code_range", help = "Range of CPU Code numbers from functional specification"},
            mandatory={"devID","cpu_code_range"}
        }
    }
})






