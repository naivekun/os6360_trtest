--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* initEnvironment.lua
--*
--* DESCRIPTION:
--*       create and fill device depended globals
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

cli_C_functions_registered("prvLuaTgfPortLoopbackModeEnableSet")
cli_C_functions_registered("wrlCpssIsAsicSimulation")
cli_C_functions_registered("wrlDxChPortLoopbackModeEnableGet")
cli_C_functions_registered("wrlIsCpuMii")

-- ************************************************************************
-- global device dependent parameters. !! Defined for device #0 only !!
multiDevEnv = nil
devEnv = nil
-- table dedicated to passing data from test.lua files to configuration files
global_test_data_env = {};
-- board config
boardEnv = nil

-- ports used in examples if preset before fillDeviceEnvironment call
-- values managed in run.lus - MUST be global
preset_test_ports = {};
preset_test_cpu_port = {};
preset_test_device = nil;

-- currently Falcon ports limited to 128 physical ports
local tile_0 =  0
local tile_1 = 56
local tile_2 = 80
local tile_3 = 100

local commonCpuPort = 63
-- default configurations
-- additional ports 1 and 2 for tests that use more than 4 ports
local defaultNumOfPorts = 6;
local devPortPatterns =
{
        ["CPSS_PP_FAMILY_DXCH_BOBCAT3_E"] = {
             {0, 18, 36, 58, 1, 2}
            ,{0, 18, 36, 65, 1, 2}
            -- values that are 256 more than 'regular' to test : '512 ports' mode
            -- so values are: 256, 274, 292, 314, 257, 258
            ,{0+256, 18+256, 36+256, 58+256, 1+256, 2+256}
        },

        ["CPSS_PP_FAMILY_DXCH_BOBCAT2_E"] = {
            {0, 18, 58, 82, 1, 2},
            {0, 18, 36, 58, 1, 2},
            {0, 18, 36, 24, 1, 2},
            {0,  8, 12, 16, 1, 2}

            ,{-- AC3X-Z0 24 ports
                AC3X = true,
                59,    --[[remote port 1G on 88e1690 #6 local port 8]]
                17,    --[[remote port 1G on 88e1690 #1 local port 6]]
                4,     --[[local 40G ]]
                49,    --[[remote port 1G on 88e1690 #5 local port 6]]
                12,    --[[remote port 1G on 88e1690 #1 local port 1]]
                11     --[[local 10G]]
              }
            },

        ["CPSS_PP_FAMILY_DXCH_XCAT3_E"] = {
            {0,  8, 18, 24, 20, 2},
            -- for B2B xCat. Port 24,26,27 are used as cascade trunks to connect B2B devices
            {0,  8, 18, 25, 20, 2}},

        ["CPSS_PP_FAMILY_DXCH_XCAT2_E"] = {
            {0,  8, 18, 23, 20, 3},
            {0,  8, 18, 28, 20, 3}},

        ["CPSS_PP_FAMILY_DXCH_LION_E"] = {
            {0,  18, 36, 58, 8, 17}},

        ["CPSS_PP_FAMILY_DXCH_LION2_E"] = {
            {120, 56, 86, 102, 1, 2},
            {120,  56, 84, 104, 0, 16}, -- 40G ports
            {0, 18, 36, 58, 1, 2}       -- hooper device
        },
        ["CPSS_PP_FAMILY_DXCH_ALDRIN_E"] = {
            {0, 8, 12, 30, 1, 2}
        },
        ["CPSS_PP_FAMILY_DXCH_AC3X_E"] = {
            {0, 8, 12, 30, 1, 2}
            ,{-- AC3X 48 ports
                AC3X = true,
                35,    --[[remote port 1G on 88e1690 #3 local port 8]]
                17,    --[[remote port 1G on 88e1690 #1 local port 6]]
                4,     --[[local 40G ]]
                25,    --[[remote port 1G on 88e1690 #2 local port 6]]
                12,    --[[remote port 1G on 88e1690 #1 local port 1]]
                11     --[[local 10G]]
              }
        },
        ["CPSS_PP_FAMILY_DXCH_ALDRIN2_E"] = {
             {16, -- 100G , Global MAC port 16 , DP[1] local port  4
              58, -- 10G  , Global MAC port 58 , DP[3] local port 10
               0, -- 100G , Global MAC port  0 , DP[0] local port  0
              76, -- 10G  , Global MAC port 71 , DP[3] local port 23
              24, -- 10G  , Global MAC port 24 , DP[2] local port  0
              77} -- 10G  , Global MAC port 72 , DP[0] local port 24
        },
        -- currently Falcon ports limited to 128 physical ports
        -- CPSS_TBD_BOOKMARK_FALCON
        ["CPSS_PP_FAMILY_DXCH_FALCON_E"] = {
            --[[ 4 tiles ports :
                116 is mac 200 (tile 3) , 105 is mac 178 (tile 2) ,  68 is mac 104 (tile 1) , 
                 54 is mac  54 (tile 0) , 122 is mac 212 (tile 3) , 110 is mac 188 (tile 2)
            ]]
             {116, 105, 68, 54, 122, 110} 
            ,{0, 18+tile_1, 36+tile_0, 58+tile_1, 1+tile_1, 2+tile_0} -- 2 tiles ports
            ,{0, 18+tile_0, 36+tile_0, 58+tile_0, 1+tile_0, 2+tile_0} -- single tile ports
        }
}

-- ports with different capabilities
local xcat3x_ports = {
    ports_aldrin = {
        {min = 0 , max = 11}
    },
    ports_88e1690= {
        {min = 12, max = 59}
    }
}
-- check if AC3X port is one of aldrin
local function is_xCat3x_aldrin_port(portNum)
    if xcat3x_ports.ports_aldrin[1].min >= portNum and
       xcat3x_ports.ports_aldrin[1].max <= portNum
    then
        return true
    end

    return false
end

-- check if AC3X port is one of 88e1690
local function is_xCat3x_88e1690_port(portNum)
    if xcat3x_ports.ports_88e1690[1].min >= portNum and
       xcat3x_ports.ports_88e1690[1].max <= portNum
    then
        return true
    end

    return false
end

local xCat3x_ports_with_aldrin_only = {
    -- 6 ports of Aldrin
    ports = {9,8,4,0,10,11}
}
local aCat3x_restore_ports = nil

-- xcat3x : set all tested ports to be those of aldrin
function xCat3x_replace_working_ports_with_aldrin_only(portsArr)

    --print("xCat3x_replace_working_ports_with_aldrin_only:" , to_string(portsArr))

    local didChange = nil
    if(aCat3x_restore_ports == nil) then
        aCat3x_restore_ports = deepcopy(devEnv.port)
    end

    if(portsArr == nil) then
        didChange = xCat3x_ports_with_aldrin_only.ports
        devEnv.port = deepcopy(xCat3x_ports_with_aldrin_only.ports)
    else
        -- we got explicit list of ports that need to be replaced
        for jj=1,#portsArr do
            for ii=1,#devEnv.port do
                if(devEnv.port[ii] == portsArr[jj]) then
                    -- replace the port
                    if devEnv.port[ii] ~= xCat3x_ports_with_aldrin_only.ports[ii] then
                        if not didChange then
                            didChange = {}
                        end

                        didChange[1 + #didChange] = xCat3x_ports_with_aldrin_only.ports[ii]
                    end
                    devEnv.port[ii] = xCat3x_ports_with_aldrin_only.ports[ii]
                    break
                end
            end
        end
    end

    if(didChange) then
        --print ("changed ports to",to_string(didChange))
        luaTgfConfigTestedPorts(devEnv.devNum,didChange,true)
    end

end

-- xcat3x : restore tested ports
function xCat3x_restore_working_ports(devNum)
    if (aCat3x_restore_ports == nil) then
        -- nothing to restore
        return
    end

    if (devNum == nil) or (devNum == devEnv.dev) then
        devEnv.port = aCat3x_restore_ports
        aCat3x_restore_ports = nil
    end
end

local is_xCat3x_in_system_flag = false
function is_xCat3x_in_system()
    return is_xCat3x_in_system_flag
end

local devCpuPortPatterns =
{
    ["CPSS_PP_FAMILY_DXCH_BOBCAT3_E"] = commonCpuPort,
    ["CPSS_PP_FAMILY_DXCH_BOBCAT2_E"] = commonCpuPort,
    ["CPSS_PP_FAMILY_DXCH_XCAT3_E"]   = commonCpuPort,
    ["CPSS_PP_FAMILY_DXCH_XCAT2_E"]   = commonCpuPort,
    ["CPSS_PP_FAMILY_DXCH_LION_E"]    = commonCpuPort,
    ["CPSS_PP_FAMILY_DXCH_LION2_E"]   = commonCpuPort,
    ["CPSS_PP_FAMILY_DXCH_ALDRIN_E"]  = commonCpuPort,
    ["CPSS_PP_FAMILY_DXCH_AC3X_E"]    = commonCpuPort,
    ["CPSS_PP_FAMILY_DXCH_ALDRIN2_E"] = commonCpuPort,
    ["CPSS_PP_FAMILY_DXCH_FALCON_E"]  = commonCpuPort
}

-- check port is able to support loopback (needed by the tests)
local function check_port_loop_back_able(dev, port)
    local rc, enablePtr =  wrlDxChPortLoopbackModeEnableGet(dev, port)

    return (rc == 0);
end
-- check port is not cascade port (test should set port as cascade if needed to tests it like that)
function check_port_is_cascade(devNum, portNum)
    local apiName = "cpssDxChCscdPortTypeGet"
    local portDirection = "CPSS_PORT_DIRECTION_BOTH_E"
    local ret,val = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",          "devNum",     devNum },
            { "IN",     "GT_PHYSICAL_PORT_NUM",    "portNum",    portNum},
            { "IN",     "CPSS_PORT_DIRECTION_ENT",    "portDirection",    portDirection},
            { "OUT",     "CPSS_CSCD_PORT_TYPE_ENT",    "portType"}
        })

    if ret ~= 0 or val == nil or
       val.portType ~= "CPSS_CSCD_PORT_NETWORK_E"
    then
        -- the port can not serve as valid port in the tested ports
        return true
    end

    -- the port is not cascade port
    return false;
end

-- check port is not trunk member
-- return : if      trunk member : true,trunkId
--          else    false
function check_port_is_trunk_member(devNum, portNum)
    local apiName = "cpssDxChTrunkDbIsMemberOfTrunk"
    local hwDevNum , dummyPortNum =
        Command_Data:getHWDevicePort(devNum,portNum)
    local trunkMember = {device--[[converted to hwDevice]] = hwDevNum , port = portNum}
    local GT_NOT_FOUND = (0x0B)   --Item not found
    local GT_NOT_INITIALIZED = (0x12) --trunk unit was not initialized
    local ret , value = myGenWrapper(
            apiName, {
            { "IN",     "GT_U8",          "devNum",     devNum },
            { "IN",     "CPSS_TRUNK_MEMBER_STC",    "memberPtr",    trunkMember},
            { "OUT",     "GT_TRUNK_ID",    "trunkId"}
        })

    if ret ~= GT_NOT_FOUND and ret~=GT_NOT_INITIALIZED then
        -- error or
        -- the port is member in trunk
         return true , value.trunkId
    end

    -- the port is not trunk member
    return false;
end

-- check that all ports in the list are 'regular' and are able to perform tests
local function check_port_list_regular_ports(dev, port_list)
    for dummy, port in pairs(port_list) do
        if type(port) == "number" then
            -- check port is able to support loopback (needed by the tests)
            if not check_port_loop_back_able(dev, port) then
                print ("device " .. dev .. " port " .. port .. " failed list because " ..
                    " not able to loopback" )
                return nil;
            end
            -- check port is not cascade port (test should set port as cascade if needed to tests it like that)

            if check_port_is_cascade(dev, port) then
                print ("device " .. dev .. " port " .. port .. " failed list because " ..
                    " cascade port" )
                return nil;
            end

            -- check that the port is not trunk member
            if check_port_is_trunk_member(dev, port) then
                print ("device " .. dev .. " port " .. port .. " failed list because " ..
                    "trunk member" )
                return nil;
            end
        end
    end
    return true;
end


local function check_port_list(port_list, all_ports)
    local port_bmp = {};
    for dummy, port in pairs(all_ports) do
        port_bmp[port] = {};
    end
    for dummy, port in pairs(port_list) do
        if type(port) == "number" then
            if not port_bmp[port] then
                return nil;
            end
        end
    end
    return true;
end

-- global DB - initialized as empty table on registration
cmdLuaCLI_registerCfunction("wrlCpssDxChTcamTtiHitInfoGet")
-- used extenal data and functions:
local function ttiDbAccordingToAppDemo(devNum)

    if not is_sip_5(devNum) then
        -- not implemented
        return
    end

    local result, hit0_base , hit0_size , hit1_base , hit1_size ,
          hit2_base , hit2_size , hit3_base , hit3_size =
            wrlCpssDxChTcamTtiHitInfoGet(devNum)
    if hit0_base == nil then
        return
    end


    -- state the ranges used by TTI lookup 0
    local rulesDb, msg = tcamClientsChunksGet(devNum, "tti", 0)
    if rulesDb == nil then
        printLog("ERROR: " .. msg)
    else
        sharedResourceNamedTti0RulesMemoReset(rulesDb)
    end

    -- state the ranges used by TTI lookup 1
    local rulesDb, msg = tcamClientsChunksGet(devNum, "tti", 1)
    if rulesDb == nil then
        printLog("ERROR: " .. msg)
    else
        sharedResourceNamedTti1RulesMemoReset(rulesDb)
    end

    -- state the ranges used by PCL lookup
    local rulesDb, msg = tcamClientsChunksGet(devNum, "pcl")
    if rulesDb == nil then
        printLog("ERROR: " .. msg)
    else
        sharedResourceNamedPclRulesMemoReset(rulesDb)
    end

    --print("tti0RulesFreeMemoDB",to_string(tti0RulesFreeMemoDB))
    --print("tti1RulesFreeMemoDB",to_string(tti1RulesFreeMemoDB))

    -- state the ranges used by TTI lookup 0
    setTtiIndexRangeForPool(hit0_base, hit0_base + hit0_size - 1 , 0)
    -- state the ranges used by TTI lookup 1
    setTtiIndexRangeForPool(hit1_base, hit1_base + hit1_size - 1 , 1)
end


-- ************************************************************************

local Serdes_Port_Num     = 0
local Dev_Port_Lane_Table = nil
Serdes_Port_Lst     = {};




-- *****************************************************************
function fill_Serdes_Port_Lst()

    local devNum, portNum, laneNum, ports
    local ret=0
    local numOfSerdesLanesPtr=0
    local Serdes_Port=0
    local LaneNum = 0

    devNum  = devEnv.dev
    --print("devNum = "..to_string(devNum))
    local sysInfo = luaCLI_getDevInfo();
    ports = sysInfo[devNum];

    Dev_Port_Lane_Table = {}
    Dev_Port_Lane_Table[devNum] = nil
    Dev_Port_Lane_Table[devNum] = tableInsert(Dev_Port_Lane_Table[devNum],nil)

    Serdes_Port_Lst = {};
    for _, portNum in ipairs(ports) do
             --if portNum ~= 0 then
                 ret,numOfSerdesLanesPtr=GetNuberSerdesOfPort(devNum,portNum)
                 if( ret == 0 and numOfSerdesLanesPtr > 0) then

                         for i=0,numOfSerdesLanesPtr-1,1 do
                             laneNum=i

                              Dev_Port_Lane_Table[devNum][portNum] = tableInsert(Dev_Port_Lane_Table[devNum][portNum], nil)
                              Dev_Port_Lane_Table[devNum][portNum].laneNum = laneNum

                              Serdes_Port_Lst[#Serdes_Port_Lst+1] = portNum
                              Serdes_Port_Num = Serdes_Port_Num + 1;
                         end
                 end
             --end --if portNum ~= 0 then
    end

    --print("fill_Serdes_Port_Lst: Serdes_Port_Lst  = "..to_string(Serdes_Port_Lst))

    local valid = {}
    for i = 1,#devEnv.port do
        --printLog("port = "..to_string(devEnv.port[i]))
        valid[devEnv.port[i]] = true
    end


    if #Serdes_Port_Lst == 0 then
        Test_Serdes_Port = 0
    else
        for i = 1,#Serdes_Port_Lst do

            Serdes_Port = Serdes_Port_Lst[#Serdes_Port_Lst-i]

            if valid[Serdes_Port] then
                Test_Serdes_Port = Serdes_Port
                break
            end
        end


        if Test_Serdes_Port == nil then
            Test_Serdes_Port = Serdes_Port_Lst[#Serdes_Port_Lst-5]
        end
    end

    --print("fill_Serdes_Port_Lst: Test_Serdes_Port  = "..to_string(Test_Serdes_Port))

    --if DeviceSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E" then
    --    Transmit_Mode_List= {"regular","prbs","zeros","ones","cyclic","7","9","15","23","31","1t","2t","5t","10t","dfet_raining"}
    --else --if DeviceSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E"
        Transmit_Mode_List= {"7","15","23","31"}
    --end --if DeviceSubFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E"
    Default_Transmit_Mode = "7"

    Default_LaneNum  = 0
    LaneNum = 0

    Test_Parameter_Def={
    ["serdes_prbs_set_show_port_all_mode_all"]     ={NN= 1,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=3, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { "all" }, mode2= {}},
    ["serdes_prbs_set_show_port_1_mode_1"]         ={NN= 2,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { "7" }, mode2= {}},
    ["serdes_prbs_set_show_port_1_mode_all"]       ={NN= 3,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=2, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { "all" }, mode2= {}},
    ["serdes_prbs_set_show_port_all_mode_1"]       ={NN= 4,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=2, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { "7"   }, mode2= {}},
    ["serdes_prbs_set_no_enable_port_all_lane_all"]={NN= 5,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=3, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {"all" }, mode2= {}},
    ["serdes_prbs_set_no_enable_port_1_lane_all"]  ={NN= 6,enable_WM="Y", enable_BM="Y", feature="SERDES_PRBS"        ,CoverageLvl=2, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {"no" }, mode2= {}},
    ["serdes_auto_tune_show_port_all"]             ={NN= 7,enable_WM="Y", enable_BM="Y", feature="SERDES_AUTO_TUNE"   ,CoverageLvl=3, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_auto_tune_show_port_1"]               ={NN= 8,enable_WM="Y", enable_BM="Y", feature="SERDES_AUTO_TUNE"   ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_no_reset_port_all"]                   ={NN= 9,enable_WM="Y", enable_BM="Y", feature="SERDES_RESET"       ,CoverageLvl=2, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_no_reset_port_1"]                     ={NN=10,enable_WM="Y", enable_BM="Y", feature="SERDES_RESET"       ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_reset_port_all"]                      ={NN=11,enable_WM="Y", enable_BM="Y", feature="SERDES_RESET"       ,CoverageLvl=2, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_reset_port_1"]                        ={NN=12,enable_WM="Y", enable_BM="Y", feature="SERDES_RESET"       ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_voltage_port_all"]                    ={NN=13,enable_WM="Y", enable_BM="Y", feature="SERDES_VOLTAGE"     ,CoverageLvl=1, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_voltage_port_1"]                      ={NN=14,enable_WM="Y", enable_BM="Y", feature="SERDES_VOLTAGE"     ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_temperature_port_all"]                ={NN=15,enable_WM="Y", enable_BM="Y", feature="SERDES_TEMPERATURE" ,CoverageLvl=1, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_temperature_port_1"]                  ={NN=16,enable_WM="Y", enable_BM="Y", feature="SERDES_TEMPERATURE" ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= {}, mode2= {}},
    ["serdes_polarity_trigger_true_port_all"]      ={NN=17,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { true  }, mode2= {}},
    ["serdes_polarity_trigger_true_port_1"]        ={NN=18,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { true  }, mode2= {}},
    ["serdes_polarity_trigger_false_port_all"]     ={NN=20,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { false }, mode2= {}},
    ["serdes_polarity_trigger_false_port_1"]       ={NN=21,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { false }, mode2= {}},
    ["serdes_polarity_Tx_false_Rx_true_port_all"]  ={NN=22,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { false }, mode2= { true }},
    ["serdes_polarity_Tx_false_Rx_true_port_1"]    ={NN=23,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { false }, mode2= { true }},
    ["serdes_polarity_Tx_true_Rx_false_port_all"]  ={NN=24,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { true },  mode2= {false }},
    ["serdes_polarity_Tx_true_Rx_false_port_1"]    ={NN=25,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { true },  mode2= {false }},
    ["serdes_polarity_Tx_true_Rx_true_port_all"]   ={NN=26,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { true },  mode2= {true }},
    ["serdes_polarity_Tx_true_Rx_true_port_1"]     ={NN=27,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { true },  mode2= {true }},
    ["serdes_polarity_Tx_false_Rx_false_port_all"] ={NN=28,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=2, DevNum  ="Default", PortNum ="all",     LaneNum ="Default", mode1= { false },  mode2= {false }},
    ["serdes_polarity_Tx_false_Rx_false_port_1"]   ={NN=29,enable_WM="Y", enable_BM="Y", feature="SERDES_POLARITY"    ,CoverageLvl=1, DevNum  ="Default", PortNum ="Default", LaneNum ="Default", mode1= { false },  mode2= {false }},
    }

    --Serdes_Port_Properties = {}
    --Serdes_Port_Properties = Serdes_Port_Properties_Update(Serdes_Port_Properties)
    --print("Serdes_Port_Properties  = "..to_string(Serdes_Port_Properties))


    --ret,Port_Intef_config = show_interfaces_configuration(params)
    --print("Port_Intef_config  = "..to_string(Port_Intef_config))
    --
    --ret,Port_Interf_Status = show_interfaces_status(params)
    --print("Port_Interf_Status  = "..to_string(Port_Interf_Status))



    serdes_env =
    {
            devNum        = tostring(devNum),
            PortNum       = tostring(Test_Serdes_Port),
            LaneNum       = tostring(0),
            Invert_Rx     = tostring(true),
            Invert_Tx     = tostring(false),
            Transmit_Mode = Default_Transmit_Mode,
    };

    --print("serdes_env  = "..to_string(serdes_env))
    --print("serdes_env.PortNum  = "..to_string(serdes_env.PortNum))

    --CoverageLvl    = 1
    --CoverageLvlTxt = "min"
    --
    --print("fill_Serdes_Port_Lst CoverageLvl  = "..to_string(CoverageLvl))
    --print("fill_Serdes_Port_Lst CoverageLvlTxt  = "..CoverageLvlTxt)

    printLog("========= fill_Serdes_Port_Lst: - Ended ==========")


end

-- was called from CLI command of 'cpssInitSystem' but need to be called
-- also after 'fast boot' that doing 'second' cpssInitSystem
function ending_fillDeviceEnvironment()
    -- init board specific global parameters
    if fillBoardEnv ~= nil then fillBoardEnv() end

    -- run registered callbacks
    if updateCLItypes ~= nil then updateCLItypes() end

    -- configure all devices regardless to PBR using
    pbr_set_default_cfg_tab_access_mode();

end

-- ************************************************************************
---
--  fillDeviceEnvironment
--        @description   - fill devEnv table with device-depended parameters
--
--        @param         - devEnv
--
function fillDeviceEnvironment()
    local dev, ports, family, dev_index
    local port_list, p_lst, cpu_port;

    local sysInfo = luaCLI_getDevInfo();
    if type(sysInfo) ~= "table" then
        -- case of invoking before cpssInitSystem
        return ending_fillDeviceEnvironment();
    end
    dev_index = 1;
    multiDevEnv = {};
    for i = 0,127 do
        if sysInfo[i] then
            dev = i;
            ports = sysInfo[dev];
            family = wrlCpssDeviceFamilyGet(dev);
            -- print("dev " .. to_string(dev) .." family ".. to_string(family));
            multiDevEnv[dev_index] = {dev = dev};
            -- CPU port
            cpu_port = 63; -- default
            if preset_test_cpu_port[dev] then
                cpu_port = preset_test_cpu_port[dev];
            elseif preset_test_cpu_port[family] then
                cpu_port = preset_test_cpu_port[family];
            elseif devCpuPortPatterns[family] then
                cpu_port = devCpuPortPatterns[family];
            end
            multiDevEnv[dev_index].portCPU = cpu_port;
            -- ports
            port_list = nil
            if preset_test_ports[dev] then
                port_list = preset_test_ports[dev];
            elseif preset_test_ports[family] then
                port_list = preset_test_ports[family];
            end
            if port_list then
                multiDevEnv[dev_index].port = port_list;
            else
                -- list of possible port lists
                -- find first that contains only ports present on device
                port_list = devPortPatterns[family];
                if port_list then
                    for dummy, p_lst in pairs(port_list) do
                        --print("p_lst " .. to_string(p_lst));
                        --print("ports " .. to_string(ports));
                        if check_port_list(p_lst, ports)
                            and check_port_list_regular_ports(dev, p_lst)
                        then
                            multiDevEnv[dev_index].port = p_lst;
                            if multiDevEnv[dev_index].port.AC3X then
                                -- remove the indication
                                -- and set global flag
                                multiDevEnv[dev_index].port.AC3X = nil
                                is_xCat3x_in_system_flag = true
                            end
                            --print("p_lst " .. to_string(p_lst));
                            break;
                        end
                    end
                end
                -- default for not found family
                if not multiDevEnv[dev_index].port then
                    p_lst = {};
                    local ports_num = 0;
                    for _, port in ipairs(ports) do
                        if check_port_loop_back_able(dev, port) then
                            p_lst[#p_lst+1] = port
                            ports_num = ports_num + 1;
                        end
                        if ports_num >= defaultNumOfPorts then
                            break;
                        end
                    end
                    multiDevEnv[dev_index].port = p_lst;
                end
            end
            if (is_sip_5_15(devNum)
                and (not is_sip_5_16(devNum))) -- bobk only
            then
                multiDevEnv[dev_index].ledIf = 4
            else
                multiDevEnv[dev_index].ledIf = 0
            end

            dev_index = dev_index + 1;
        end
    end

    -- use preset device (if exists) for one device tests
    dev_index = 1;
    if preset_test_device then
        for i,cfg in pairs(multiDevEnv) do
            if cfg.dev == preset_test_device then
                dev_index = i;
                break;
            end
        end
    end
    devEnv = multiDevEnv[dev_index];

    system_capability_managment({devID = devEnv.dev})
    print("devices, ports to be used by LUA tests , and configuration files ==> " .. to_string(multiDevEnv))

    -- init DB range for TTI
    ttiDbAccordingToAppDemo(devEnv.dev)

    -- init DB ranges for other tables

    -- ROUTER_ECMP_QOS is L2 Ecmp, L3 Ecmp called ROUTER_ECMP
    sharedResourceNamedL2EcmpMemoReset(
        {{base = 0, size = NumberOfEntriesIn_ROUTER_ECMP_QOS_table}});
    sharedResourceNamedVidxMemoReset(
        {{base = 0, size = NumberOfEntriesIn_VIDX_table}});
    sharedResourceNamedL2andL3MllMemoReset(
        {{base = 0, size = NumberOfEntriesIn_MLL_PAIR_table}});
    sharedResourceNamedMac2meMemoReset(
        {{base = 0, size = NumberOfEntriesIn_MAC_TO_ME_table}});
    sharedResourceNamedTsMemoReset(
        {{base = 0, size = NumberOfEntriesIn_TUNNEL_START_table}});
    sharedResourceNamedEVidMemoReset(
        {{base = 0, size = NumberOfEntriesIn__table}});
    -- EVidx from max VIDX, not from 0. Amount according to LTT table size
    -- EVidx should start from 4K because the vidx 0xFFF (flood vidx) is 'reserved'
    -- regardless to NumberOfEntriesIn_VIDX_table (that may be lower than 4K)
    local FLOOD_VIDX_CNS = 0xFFF
    sharedResourceNamedEVidxMemoReset(
        {{base = (FLOOD_VIDX_CNS + 1), size = NumberOfEntriesIn_L2_MLL_LTT_table}});

    fill_Serdes_Port_Lst()
    --print("Serdes_Port_Lst ==> " .. to_string(Serdes_Port_Lst))
    --print("multiDevEnv ==> " .. to_string(multiDevEnv))
    print("#Serdes_Port_Lst ==> " .. to_string(#Serdes_Port_Lst))

    return ending_fillDeviceEnvironment();
end

-- called from initSysem function that handles cpssInitSystem command ("exec" context)
-- but should also be automatically called at reloading LUA CLI
fillDeviceEnvironment()


-- ************************************************************************
---
--  fillBoardEnv
--        @description   - fill boardEnv table with board-depended parameters
--
--        @param         - boardEnv global struct
--
function fillBoardEnv()
  local dev, ports, family, dev_index
  local port_list, p_lst, cpu_port;
  local sysInfo = luaCLI_getDevInfo();
  if type(sysInfo) ~= "table" then
    -- case of invoking before cpssInitSystem
    return;
  end

  boardEnv = {}
  boardEnv.FamiliesList = {}
  boardEnv.devList = {}

  for i = 0, 127 do
    if sysInfo[i] then
      family = wrlCpssDeviceFamilyGet(i);
      table.insert(boardEnv.devList, {dev = i, type = family})
      if not boardEnv.FamiliesList[family] then
        boardEnv.FamiliesList[family] = {isExist = True}
      end

      if isGmUsed() or isEmulatorUsed() then
        -- state that the EMULATOR/GM needs the FDB shadow.
        per_device__debug_fdb_use_shadow(nil,i,nil)
        -- NOTE: see command in 'debug-mode' , to allow use this FDB shadow also when not GM .
        -- see "fdb-use-shadow"
        -- fdb_use_shadow : global variable that hold indication that the 'fdb shadow is used'

      end

    end
  end

  -- print("DBG: boardEnv: " .. to_string(boardEnv));
end

-- fill table with board specific info for multidevices configuration
-- called from initSysem function that handles cpssInitSystem command ("exec" context)
-- but should also be automatically called at reloading LUA CLI
fillBoardEnv()


-- global function to get tested port
-- INPUT  : portIndex  - the index of the port 1..4 or "cpu"
-- OUTPUT : pornNumber - the port number in the 'index'
function getTestPort(portIndex)
    if devEnv == nil then
        return nil -- will cause exception
    end

    if portIndex == "cpu" then
        return devEnv.portCPU
    end

    return devEnv.port[portIndex]
end

-- global function to get tested port that used by specific family
-- INPUT  : portIndex  - the index of the port 1..4 or "cpu"
-- OUTPUT : pornNumber - the port number in the 'index'
function getTestPortExplicitFamily(explicitFamily , portIndex)
    local myDevEnv, family;

    if type(explicitFamily) ~= "string" then
        return nil -- will cause exception
    end
    family = "CPSS_PP_FAMILY_DXCH_" .. string.upper(explicitFamily) .. "_E";
    myDevEnv = nil;
    if portIndex == "cpu" or portIndex == "CPU" then
        if preset_test_cpu_port[family] then
            return preset_test_cpu_port[family];
        end
        if devCpuPortPatterns[family] then
            return devCpuPortPatterns[family];
        end
        return 63; -- default
    end
    --print("family " .. to_string(family));
    --print("preset_test_ports " .. to_string(preset_test_ports));
    if preset_test_ports[family] then
        return preset_test_ports[family][portIndex];
    end
    if devPortPatterns[family] then
        return devPortPatterns[family][1][portIndex];
    end
    return nil -- will cause exception
end

-- global function to be used by configuration files that need remote port from remote family
-- INPUT  : remoteIdentifier - the 'identifier' (usually family , like "XCAT3") ,
--                              but can be: "vss-lc"
--          portIndex  - the index of the port 1..4 or "cpu"
-- OUTPUT : pornNumber - the port number in the 'index'
function remotePort(remoteIdentifier , portIndex)
    if (remoteIdentifier == "vss-lc") then
        -- return port from 'xcat3' type of ports
        return getTestPortExplicitFamily("XCAT3" , portIndex)
    end

    -- by default call according to family
    return getTestPortExplicitFamily(remoteIdentifier , portIndex)
end
-- called directly from the configuration files as $(vssLcPort[1]) ...
function vssLcPort(portIndex)
    return remotePort("vss-lc",portIndex)
end

--[[
    function return info of format:
    typedef struct
    {
        GT_PHYSICAL_PORT_NUM                physicalPortNumber;
        CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;    -- string !!!
        GT_U32                              portGroup;
        GT_U32                              interfaceNum;
        GT_U32                              txqPortNumber;
        GT_BOOL                             tmEnable;
        GT_U32                              tmPortInd;
    }CPSS_DXCH_PORT_MAP_STC;

]]--
function getPortMapInfo(devNum,portNum)
    local portMapShadowPtrPtr
    local rc, valid, mappingType, trafficManagerEn, portGroup, macNum, rxDmaNum,
            txDmaNum, txFifoPortNum, txqNum, ilknChannel, tmPortIdx =
                wrLogWrapper("wrlCpssDxChPortPhysicalPortMapShadowDBGet",
                "(dev, portNum, portMapShadowPtrPtr)",
                devNum, portNum, portMapShadowPtrPtr)

    if rc ~= 0 or not valid then
        return nil
    end

    local function mappingTypeStrGet(mappingType)
        local my_str = {    [0] = "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E",
                            [1] = "CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E",
                            [2] = "CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E",
                            [3] = "CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E",
                            [4] = "CPSS_DXCH_PORT_MAPPING_TYPE_MAX_E" }

        return my_str[mappingType] or "unknown"
    end


    local outParam = {
        physicalPortNumber = portNum,
        mappingType = mappingTypeStrGet(mappingType),
        portGroup = portGroup,
        interfaceNum = macNum,
        txqPortNumber = txqNum,
        tmEnable = trafficManagerEn,
        tmPortInd = tmPortIdx
    }

    --printLog("outParam",to_string(outParam))


    return outParam
end

-- ************************************************************************
---
--  SUPPORTED_DEV_DACLARE
--        @description     check device family and exit from test if not in param list
--
--        @param devNum  - device number
--        @param ...     - list of applicable device(s), for example
--               "CPSS_PP_FAMILY_DXCH_BOBCAT2_E", "CPSS_PP_FAMILY_DXCH_XCAT_E",
--
--
function SUPPORTED_DEV_DECLARE(devNum, ... )
--[[
  res, devFamily = wrlCpssDeviceFamilyGet(devNum)
  res, devRevision = wrlCpssDeviceRevisionGet(devNum)
  res, devType = wrlCpssDeviceTypeGet(devNum)
]]--

  local list
  local idx
  local res, devFamily, devRevision, devType

  devFamily = wrlCpssDeviceFamilyGet(devNum)

  if devFamily == nil then
    -- device not exists
    isDeviceNotSupported = true
    error()
  end


  list = {...}

  for idx = 1, #list do
    if type(list[idx]) == "string" then
      if list[idx] == devFamily then
        return true
      end
    elseif type(list[idx]) == "table" then
      local idxRecursive
      for idxRecursive = 1, #list[idx] do
        if SUPPORTED_DEV_DECLARE(devNum, list[idx][idxRecursive]) then return end
      end
    end
  end

  isDeviceNotSupported = true
  error()
end

-- ************************************************************************
---
--  NOT_SUPPORTED_DEV_DACLARE
--        @description     check device family and exit from test if in param list
--
--        @param devNum  - device number
--        @param ...     - list of applicable device(s), for example
--               "CPSS_PP_FAMILY_DXCH_BOBCAT2_E", "CPSS_PP_FAMILY_DXCH_XCAT_E",
--
--
function NOT_SUPPORTED_DEV_DECLARE(devNum, ... )

  local list
  local idx
  local res, devFamily, devRevision, devType

  devFamily = wrlCpssDeviceFamilyGet(devNum)

  if devFamily == nil then
    -- device not exists
    isDeviceNotSupported = true
    error()
  end

  list = {...}

  for idx = 1, #list do
    if type(list[idx]) == "string" then
      if list[idx] == devFamily then
        isDeviceNotSupported = true
        error()
      end
    elseif type(list[idx]) == "table" then
      local idxRecursive
      for idxRecursive = 1, #list[idx] do
        NOT_SUPPORTED_DEV_DECLARE(devNum, list[idx][idxRecursive]);
      end
    end
  end
end

-- ************************************************************************
---
--  SUPPORTED_SIP_DECLARE
--        @description     check device SIP version and exit from test if not in required_sip_version
--
--        @param devNum  - device number
--        @param required_sip_version - the required sip version from the device
--                          can be exact sip version: "SIP_5","SIP_5_10","SIP_5_15"
--                          can be "ANY_SIP"
--                          can NOT be : "SIP_LEGACY" , nil
--        @param hold_restrictions - (optional parameter) indication to hold restrictions  (default is 'no restrictions')
--        @param restricted_sip_version - (optional parameter)
--                                  a sip version that is restricted !
--                                  (must not be supported by the device)
--                          can be nil
--                          can be exact sip version: "SIP_5","SIP_5_10","SIP_5_15","SIP_LEGACY"
--                          can NOT be : "ANY_SIP"
--
--         examples:
--          1. supported by all devices :               SUPPORTED_SIP_DECLARE(devNum , "ANY_SIP"    )
--          2. require sip5_10 support :                SUPPORTED_SIP_DECLARE(devNum , "SIP_5_10"   )
--          3. require sip5 but not sip5_15 support :   SUPPORTED_SIP_DECLARE(devNum , "SIP_5"      , true, "SIP_5_15"  )
--          4. not supported by sip5 :                  SUPPORTED_SIP_DECLARE(devNum , "ANY_SIP"    , true, "SIP_5"     )
--
--
function SUPPORTED_SIP_DECLARE(devNum, required_sip_version , hold_restrictions , restricted_sip_version)
    local isSupported
    local isRestricted

    if required_sip_version == "ANY_SIP" then
        isSupported = true
    else
        isSupported = isSipVersion(devNum,required_sip_version)
    end

    if restricted_sip_version ~= nil and hold_restrictions == true then
        isRestricted = isSipVersion(devNum,restricted_sip_version)
    else
        isRestricted = false
    end

    if isSupported == false or isRestricted == true then
        if(isSupported == false) then
            printLog("NOTE: for current test : device must be " .. to_string(required_sip_version) .. " so test skipped")
        else -- isRestricted == true
            printLog("NOTE: for current test : device must not be " .. to_string(restricted_sip_version) .. " so test skipped")
        end

        -- the device not supported or it is restricted
        isDeviceNotSupported = true
        error()

        return false
    else
        return true
    end
end

-- for xcat3x set needed ports from Aldrin
local function xCat3x_set_ports_needed_from_aldrin(extraParamName--[["ports"]],extraParamValue--[[list of ports]])
    if (extraParamName ~= "ports" or extraParamValue == nil) then
        -- we need to use only ports that supports cascading
        -- the Aldrin ports support it
        print("all ports must be Aldrin")
        xCat3x_replace_working_ports_with_aldrin_only()
    else
        -- we need to replace only explicit ports
        print("ports needed from Aldrin:",to_string(extraParamValue))
        xCat3x_replace_working_ports_with_aldrin_only(extraParamValue)
    end
end

-- ************************************************************************
--
--  is_supported_feature
--        @description     check device for supporting one of the features in the list.
--                          return true  - if feature     supported
--                          return false - if feature not supported
--
--        @param devNum  - device number
--        @param singleFeature    - the required feature.
--          ""                              --> no specific feature
--          "HARDWARE"                      --> 'HW' only supported
--          "DYNAMIC_SLAN"                  --> 'dynamic slan' needed slans that
--                                             can be configured as peer ports,
--                                             or for other reason.
--          "BPE_802_1_BR_CONTROL_BRIDGE" --> BPE_802_1_BR control bridge
--          "BPE_802_1_BR_PORT_EXTENDER"  --> BPE_802_1_BR port extender
--          "VSS-LC"                      --> VSS line card
--          "VSS-CC"                      --> VSS control card
--          "NAT44_CMD_SUPPORT"           --> Support nat44 command
--          "NAT66_CMD_SUPPORT"           --> Support nat66 command
--          "MPLS_CMD_SUPPORT"            --> Support mpls command
--          "WIRESPEED_EMULATION"         --> do we support the wirespeed emulation
--          "VPLS"                        --> Support vpls command
--          "NOT_BC2_GM"                  --> is this a not GM simulation
--          "RXDMA_PIP"                   --> do we support the RXDMA PIP
--          "FULL_MAC_CONFIG_AND_COUNTERS"--> do we support the full mac config and counters
--          "MEF10.3"                     --> SIP_5_15 GM and HW (not WM yet)
--          "BRIDGE_RATE_LIMIT"           --> for storm control command

--          "SERDES_PRBS"
--          "SERDES_AUTO_TUNE"
--          "SERDES_RESET"
--          "SERDES_VOLTAGE"
--          "SERDES_TEMPERATURE"
--          "SERDES_POLARITY"
--          "PORT_ISOLATION"              --> support of port isolation (xcat and above)
--          "JUMBO_FRAME"                 --> support of JUMBO frames (not GM)
--                                            only for CPU with SDMA (not SGMII)) , because SGMII LSP doing "kernel panic"
--          "gtShutdownAndCoreRestart"    --> PSS SW supported function of gtShutdownAndCoreRestart() that was restoring the
--                                            ROS (MTS application) to stage that was called 'after phase2'
--                                            The purpose of this test is to check that the CPSS allow to synch with the
--                                            DMA indexes and buffers and descriptors of device that application 'removes' from
--                                            the CPSS but without 'HW reset' it, and than application do 'initialization' all
--                                            over again
--          "IP_MC_ROUTING"               --> support of Multicast Routing
--          "DSA_CASCADE"                 --> tested ports need to support DSA (cascade ports)
--          "eVlan>=4K"                   --> tested ports need to support PVID>=4K
--          "EEE"                         --> support of EEE feature
--                                              tested ports need to support 'EEE'
--          "SIP5_LED"                    --> sip5 devices : LED stream support
--          "MICRO_INIT_TOOL"             --> micro-init tool
-- ---------------------
--
--        @param extraParamName - the name of the 'extra param'
--          for list of supported 'names' : see function is_supported_feature
--        @param extraParamValue - the value of the 'extra param'
--             examples:
--              1. feature 'SIP_5' device but need to support EVLAN 4097 :
--                      SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum , "SIP_5" , "EVLAN" , 4097 )
--                  NOTE: Bobcat3 in 512 ports mode not supports EVLAN > 4095 ... so test not relevant.
--
--
function is_supported_feature(devNum, singleFeature , extraParamName , extraParamValue)
    local devFamily , subFamily = wrlCpssDeviceFamilyGet(devNum)
    local devRevision = wrlCpssDeviceRevisionGet(devNum)
    local isSupported = false
    local dev_is_sip_5 , dev_is_sip_5_10, dev_is_sip_5_15 , dev_is_sip_5_20 , dev_is_sip_5_15_only
    local dev_is_sip_6
    local got_fail_reason = false
    local featureInSip5NeedMoreThan4kVlans = false
    if devFamily == nil or singleFeature == nil then
        return false
    end

    local isCpuMii = wrlIsCpuMii(devNum)

    dev_is_sip_5         = is_sip_5(devNum)
    dev_is_sip_5_10      = is_sip_5_10(devNum)
    dev_is_sip_5_15      = is_sip_5_15(devNum)
    dev_is_sip_5_20      = is_sip_5_20(devNum)
    dev_is_sip_5_15_only = (dev_is_sip_5_15 and not dev_is_sip_5_20)
    dev_is_sip_6         = is_sip_6(devNum)

    if singleFeature == "" then
        isSupported = true
    end

    if singleFeature == "BPE_802_1_BR_PORT_EXTENDER" then
       isSupported = (devFamily == "CPSS_PP_FAMILY_DXCH_XCAT3_E" or dev_is_sip_5)
        featureInSip5NeedMoreThan4kVlans = true
    elseif singleFeature == "BPE_802_1_BR_CONTROL_BRIDGE" then
        isSupported =  dev_is_sip_5
        featureInSip5NeedMoreThan4kVlans = true
    elseif singleFeature == "VSS-LC" then
        isSupported =  (devFamily == "CPSS_PP_FAMILY_DXCH_XCAT3_E")

    elseif singleFeature == "VSS-CC"     then
        isSupported = dev_is_sip_5

    elseif singleFeature == "HARDWARE" then
        isSupported =  not wrlCpssIsAsicSimulation()

    elseif singleFeature == "BRIDGE_RATE_LIMIT" then
        isSupported =  not wrlCpssIsAsicSimulation()

        -- BobK with TM do not support the test
        if isTmEnabled(devNum) and dev_is_sip_5_15_only then
            isSupported = false
        end

        if (devFamily == "CPSS_PP_FAMILY_DXCH_LION_E" or
            devFamily == "CPSS_PP_FAMILY_DXCH_LION2_E") then
            -- Lion and Lion2 do not support the feature
            isSupported = false
        end

    elseif singleFeature == "BRIDGE_EGR_COUNTER" then
        isSupported = (dev_is_sip_5 and (not dev_is_sip_5_15_only))

    elseif singleFeature == "DYNAMIC_SLAN" then
       isSupported = wrlCpssIsAsicSimulation()

    elseif singleFeature == "NAT44_CMD_SUPPORT" then
        isSupported = dev_is_sip_5_10

    elseif singleFeature == "NAT66_CMD_SUPPORT" then
        isSupported = dev_is_sip_5_15

    elseif singleFeature == "MPLS_CMD_SUPPORT" then
        isSupported = dev_is_sip_5

    elseif singleFeature == "NOT_BC2_GM" then
        isSupported = not (wrlCpssIsGmUsed() and
                          dev_is_sip_5)

    elseif singleFeature == "VPLS" then
        isSupported = dev_is_sip_5 or isVplsModeEnabled(devNum)
    elseif singleFeature == "RXDMA_PIP" then
        -- GM not supported the feature
        isSupported = dev_is_sip_5_10 and (not isGmUsed())
        if (is_xCat3x_in_system()) then
            xCat3x_set_ports_needed_from_aldrin(nil)
        end
    elseif singleFeature == "FULL_MAC_CONFIG_AND_COUNTERS" then
        isSupported = not isGmUsed()
    elseif singleFeature == "WIRESPEED_EMULATION" then
        if isGmUsed() then
            -- the feature generate loopback on traffic with mirroring to ingress port.
            -- in GM the loopback calls the ingress pipe directly ... without context switch...
            -- this cause stack over flow !!!
        elseif (devFamily == "CPSS_PP_FAMILY_CHEETAH_E" or
                devFamily == "CPSS_PP_FAMILY_CHEETAH2_E" or
                devFamily == "CPSS_PP_FAMILY_CHEETAH3_E") then
            -- the feature requires mirroring implementation that
            -- currently not supports the ch1,2,3
        else
           isSupported = true
        end

        if is_xCat3x_in_system() and isSimulationUsed() then
           printLog(singleFeature .. ": causing too much buffers lost in WM")
           isSupported = false
        end

    elseif singleFeature == "CPU_CODE_RATE_LIMIT" then
        -- simulation and CH+ device does not support the feature
        isSupported =  not (wrlCpssIsAsicSimulation() or (devFamily == "CPSS_PP_FAMILY_CHEETAH_E"))
    elseif singleFeature == "CPSS_LOG" then
        -- CPSS_LOG lua tests does not support the feature
        isSupported = (devNum == 0) and (not isGmUsed())
    elseif singleFeature == "MEF10.3" then
        -- HW and GM support the feature, WM not supports
        -- isSupported = dev_is_sip_5_15 and (isGmUsed() or (not wrlCpssIsAsicSimulation()));
        isSupported = dev_is_sip_5_15;


    elseif    (singleFeature == "SERDES_AUTO_TUNE"  )
           or (singleFeature == "SERDES_RESET"   ) then

            if #Serdes_Port_Lst ~= 0 then
                if (dev_is_sip_5 == true ) then
                    if (isSimulationUsed() == true ) then
                        if (dev_is_sip_5_15_only == true ) then
                            isSupported = false
                        else --if (dev_is_sip_5_15_only == true )
                            isSupported = true
                        end --if (dev_is_sip_5_15_only == true )
                    else --if (isSimulationUsed == true )
                        isSupported = true
                    end --if (isSimulationUsed == true )
                else --if (dev_is_sip_5 == true )
                    isSupported = false
                end --if (dev_is_sip_5 == true )
            end -- #Serdes_Port_Lst ~= 0 then

    elseif    (singleFeature == "SERDES_POLARITY"       ) then
            if #Serdes_Port_Lst ~= 0 then
                if (dev_is_sip_5 == true ) then
                    if (isSimulationUsed() == true ) then
                        if (dev_is_sip_5_15) then
                            isSupported = false
                        else --if (dev_is_sip_5_15)
                            isSupported = true
                        end --if (dev_is_sip_5_15 == true )

                    else --if (isSimulationUsed == true )
                        isSupported = true
                    end --if (isSimulationUsed == true )
                else --if (dev_is_sip_5 == true )
                    isSupported = false
                end --if (dev_is_sip_5 == true )
            end -- #Serdes_Port_Lst ~= 0 then


    elseif    (singleFeature == "SERDES_PRBS"       ) then
            if #Serdes_Port_Lst ~= 0 then
                if (dev_is_sip_5 == true ) then
                    if (isSimulationUsed() == true ) then
                        isSupported = false
                    else --if (isSimulationUsed == true )
                        isSupported = true
                    end --if (isSimulationUsed == true )
                else --if (dev_is_sip_5 == true )
                    isSupported = false
                end --if (dev_is_sip_5 == true )
            end -- #Serdes_Port_Lst ~= 0 then


    elseif    (singleFeature == "SERDES_VOLTAGE"     )
           or (singleFeature == "SERDES_TEMPERATURE" ) then

            --print("SERDES_VOLTAGE #Serdes_Port_Lst = " .. to_string(#Serdes_Port_Lst))
            if #Serdes_Port_Lst ~= 0 then
                if (dev_is_sip_5 == true ) then
                    if (isSimulationUsed() == true ) then
                        isSupported = false
                    else --if (isSimulationUsed == true )
                        if (dev_is_sip_5_15_only == true ) then
                            isSupported = true
                        else --if (dev_is_sip_5_15_only == true )
                            isSupported = false
                        end --if (dev_is_sip_5_15_only == true )
                    end --if (isSimulationUsed == true )
                else --if (dev_is_sip_5 == true )
                    isSupported = false
                end --if (dev_is_sip_5 == true )
            --else -- #Serdes_Port_Lst ~= 0 then
            --    print("SERDES_VOLTAGE #Serdes_Port_Lst = 0 !!!")
            end  -- #Serdes_Port_Lst ~= 0 then
            --print("SERDES_VOLTAGE isSupported = " .. to_string(isSupported))


    elseif (singleFeature == "PORT_ISOLATION_LOOKUP_BITS") then
            if (devEnv.port[1] >= 256 or
                devEnv.port[2] >= 256 or
                devEnv.port[3] >= 256 or
                devEnv.port[4] >= 256 )
            then
                -- the feature is not supported for ports >= 256
                print ("feature " .. singleFeature .. " is is not supported for ports >= 256")
                isSupported = false
            else
                isSupported = dev_is_sip_5
            end
    elseif (singleFeature == "PORT_ISOLATION") then
        if (devFamily == "CPSS_PP_FAMILY_CHEETAH_E" or
            devFamily == "CPSS_PP_FAMILY_CHEETAH2_E" or
            devFamily == "CPSS_PP_FAMILY_CHEETAH3_E")
        then
            -- xcat and above support it
            isSupported = false
        else
            if (devEnv.port[1] >= 256 or
                devEnv.port[2] >= 256 or
                devEnv.port[3] >= 256 or
                devEnv.port[4] >= 256 )
            then
                -- the feature is not supported for ports >= 256
                print ("feature " .. singleFeature .. " is is not supported for ports >= 256")
                isSupported = false
            else
                isSupported = true
            end
        end
    elseif (singleFeature == "DSA_CASCADE") then
        isSupported = true
        if (is_xCat3x_in_system()) then
            xCat3x_set_ports_needed_from_aldrin(extraParamName,extraParamValue)
        end
    elseif (singleFeature == "eVlan>=4K") then
        isSupported = dev_is_sip_5 and (not is_xCat3x_in_system())
        if (is_xCat3x_in_system()) then
            xCat3x_set_ports_needed_from_aldrin(extraParamName,extraParamValue)
        end
    elseif (singleFeature == "EEE") then
        isSupported = dev_is_sip_5 and not wrlCpssIsGmUsed()
        if (is_xCat3x_in_system()) then
            xCat3x_set_ports_needed_from_aldrin(extraParamName,extraParamValue)
        end
    elseif (singleFeature == "SIP5_LED") then
        isSupported = dev_is_sip_5 and not wrlCpssIsGmUsed()
        if (is_xCat3x_in_system()) then
            xCat3x_set_ports_needed_from_aldrin(extraParamName,extraParamValue)
        end
    elseif (singleFeature == "SIP5_PORT_AUTONEG") then
        isSupported = dev_is_sip_5
        if (is_xCat3x_in_system()) then
            xCat3x_set_ports_needed_from_aldrin(extraParamName,extraParamValue)
        end
    elseif (singleFeature == "RMON_HISTOGRAM") then
        isSupported = true
        if (is_xCat3x_in_system()) then
            -- the Rmon histogram using 'disable' of both Rx and Tx counting that are not supported by the 88e1690 ports
            xCat3x_set_ports_needed_from_aldrin(extraParamName,extraParamValue)
        end
    elseif (singleFeature == "JUMBO_FRAME") then
        -- only for CPU with SDMA (not SGMII)) , because SGMII LSP doing "kernel panic"
        -- the GM crash on JUMBO (somewhere in the test...)
        isSupported = not isCpuMii and not wrlCpssIsGmUsed()
    elseif (singleFeature ==  "gtShutdownAndCoreRestart") then
        -- currently all devices expected to support it.
        isSupported = true
    elseif (singleFeature == "HASH_IN_DSA") then
        isSupported = dev_is_sip_5_20
    elseif (singleFeature == "IP_MC_ROUTING") then
        isSupported =  not (
            (devFamily == "CPSS_PP_FAMILY_CHEETAH_E") or
            (devFamily == "CPSS_PP_FAMILY_DXCH_XCAT2_E") or
            ((devFamily == "CPSS_PP_FAMILY_DXCH_XCAT_E") and ((devRevision == 4) or (devRevision == 6))))
    elseif (singleFeature == "MICRO_INIT_TOOL") then
        if (((devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT2_E") and (subFamily == "CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E")) or
            (devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT3_E") or
            (devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN_E") or
            (devFamily == "CPSS_PP_FAMILY_DXCH_ALDRIN2_E") or
            (devFamily == "CPSS_PP_FAMILY_DXCH_AC3X_E") or
            (devFamily == "CPSS_PP_FAMILY_DXCH_XCAT3_E")) then
            isSupported = true
        end
    elseif singleFeature == "SERDES" then
        -- the tests of SERDES on emulator hold no meaning
        isSupported = not isEmulatorUsed()

    elseif singleFeature == "FWD_TO_LB" then
        -- forwarding to loopback/service port
        isSupported = dev_is_sip_5
        if is_xCat3x_in_system() then
            xCat3x_set_ports_needed_from_aldrin(extraParamName,extraParamValue)
        end

    else
        -- unknown feature ?!
    end

    if singleFeature == "" then
        singleFeature = extraParamName .. "=[" .. extraParamValue .. "]"
    end

    if dev_is_sip_5 and featureInSip5NeedMoreThan4kVlans and extraParamName == nil then
        -- this assignment will check if we have no support of more than 4K vlans
        -- needed for Bobcat3 in 512 ports mode
        extraParamName  = "EVLAN"
        extraParamValue = 4096
    end

    if (isSupported and
        dev_is_sip_5_20 and
        (devEnv.port[1] >= 256 or devEnv.port[2] >= 256 or
         devEnv.port[3] >= 256 or devEnv.port[4] >= 256))
    then
        local supportedLimitedValueArr = {["EVLAN"] = 4096 , ["TRUNK_ID"] = 2048 , ["SOURCE-ID"] = 2048 , ["STG"] = 2048}

        if extraParamName and extraParamValue then
            for name, value in pairs(supportedLimitedValueArr) do
                if name == extraParamName and extraParamValue >= value then
                    isSupported = false
                    print ("param " .. extraParamName .. " with value[" .. extraParamValue .. "] is more than supported [" .. (value-1) .."]")

                    got_fail_reason = true
                    break
                end
            end
        end
    end


    if got_fail_reason then
        -- add no more info
    else
        local strSupported = isSupported and "supported" or "not supported"
        local strWmBm = isSimulationUsed() and "WM" or "BM"

        printLog("Device " .. to_string(devNum) .. ": feature " .. singleFeature
                  .. " is " .. strSupported .. " in " .. strWmBm)
    end

    return isSupported
end


--[[ ************************************************************************
--
--  SUPPORTED_FEATURE_DECLARE
--        @description     check device for supporting one of the features in the list.
--                         and exit from test if not support any of the features
--
--        @param devNum  - device number
--        @param ...    - (list of)the required features.
--          for example : see function is_supported_feature
--
--         examples:
--          1. feature 'BPE_802_1_BR' for 'control bridge' and for 'port extender' :
--                  SUPPORTED_FEATURE_DECLARE(devNum , "BPE_802_1_BR_CONTROL_BRIDGE" , "BPE_802_1_BR_PORT_EXTENDER"    )
--
]]--
function SUPPORTED_FEATURE_DECLARE(devNum, ...)
    local devFamily , subFamily = wrlCpssDeviceFamilyGet(devNum)
    local isSupported = false
    local dev_is_sip_5

    local featuresList = {...}

    local numInList = #featuresList
    local index

    for index = 1, numInList do
        -- we need that one of the features to be supported by the device
        if is_supported_feature(devNum, featuresList[index]) then
            return true
        end
    end

    -- if we got here
    -- the device not support any of the features

    if devFamily == nil then
        -- device not exists
        printLog("NOTE: device not exists so test skipped")
        isDeviceNotSupported = true
        error()
        return false
    end

    if(numInList > 1) then
        printLog("NOTE: for current test : device must support one of next features:" .. to_string(featuresList) .. " so test skipped")
    else
        printLog("NOTE: for current test : device must support next feature:" .. to_string(featuresList[1]) .. " so test skipped")
    end

    -- the device not supported
    isDeviceNotSupported = true
    error()

    return false

end

--[[ ************************************************************************
--
--  SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM
--        @description     check device for supporting a features with extra param info
--                         and exit from test if not support any of the features
--
--        @param devNum  - device number
--        @param singleFeature - the required feature.
--          for example : see function is_supported_feature
--        @param extraParamName - the name of the 'extra param'
--          for list of supported 'names' : see function is_supported_feature
--        @param extraParamValue - the value of the 'extra param'
--
--         examples:
--          1. feature 'SIP_5' device but need to support EVLAN 4097 :
--                  SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum , "" , "EVLAN" , 4097 )
--              NOTE: Bobcat3 in 512 ports mode not supports EVLAN > 4095 ... so test not relevant.
--          2. feature supported for specific type of ports :
--              in this case "DSA_CASCADE" , which list of ports {ingressPort} (single port)
--              must support the feature ... but can be replaced with other port that may support it.
--              example : SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum, "DSA_CASCADE" , "ports" , {ingressPort})
--
]]--
function SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum, singleFeature , extraParamName , extraParamValue)
    if (true == is_supported_feature(devNum, singleFeature , extraParamName , extraParamValue)) then
        return true
    end

    isDeviceNotSupported = true
    error()
    return false

end

luaGlobalStore("rxDataFormat", "RX_DATA_STC")
